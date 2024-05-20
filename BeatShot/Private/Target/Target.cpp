// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Target/Target.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "AbilitySystem/Globals/BSAttributeSetBase.h"
#include "BeatShot/BSGameplayTags.h"
#include "Character/BSHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "SaveGames/SaveGamePlayerSettings.h"


FTargetDamageEvent::FTargetDamageEvent(const FDamageEventData& InData, const float InTimeAlive, ATarget* InTarget)
{
	Target = InTarget;
	// For testing and main menu purposes
	if (InData.EffectSpec->GetDynamicAssetTags().HasTagExact(BSGameplayTags::Target_TreatAsExternalDamage))
	{
		bDamagedSelf = false;
		DamageType = InTarget->GetTargetDamageType();
		TimeAlive = InTimeAlive;
	}
	else
	{
		bDamagedSelf = InData.DamageType == ETargetDamageType::Self;
		DamageType = InData.DamageType;
		TimeAlive = InData.DamageType == ETargetDamageType::Self ? -1.f : InTimeAlive; // Override to -1 if damaged self
	}

	DamageCauser = InData.EffectCauser;
	bOutOfHealth = InData.NewValue <= 0.f;
	Guid = InTarget->GetGuid();
	CurrentHealth = InData.NewValue;
	DamageDelta = abs(InData.OldValue - InData.NewValue);
	Transform = InTarget->GetActorTransform();

	// Variables not changed on construction
	bWillDeactivate = false;
	bWillDestroy = false;
	VulnerableToDamageTypes = TArray<ETargetDamageType>();
	TotalPossibleTrackingDamage = 0.f;
	Streak = -1;
	CurrentDeactivationHealthThreshold = 0.f;
}

void FTargetDamageEvent::SetTargetData(const float InCurrentDeactivationHealthThreshold,
	const TArray<ETargetDamageType>& InTypes)
{
	CurrentDeactivationHealthThreshold = InCurrentDeactivationHealthThreshold;
	VulnerableToDamageTypes = InTypes;
}

void FTargetDamageEvent::SetTargetManagerData(const bool bDeactivate, const bool bDestroy, const int32 InStreak,
	const float InTotalPossibleTrackingDamage)
{
	bWillDeactivate = bDeactivate;
	bWillDestroy = bDestroy;
	TotalPossibleTrackingDamage = InTotalPossibleTrackingDamage;
	Streak = InStreak;
}

ATarget::ATarget()
{
	PrimaryActorTick.bCanEverTick = true;

	if (!RootComponent)
	{
		CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("Capsule Component");
		CapsuleComponent->SetCapsuleRadius(Constants::SphereTargetRadius);
		CapsuleComponent->SetCapsuleHalfHeight(Constants::SphereTargetRadius);
		RootComponent = CapsuleComponent;
	}

	if (!SphereMesh)
	{
		SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>("Sphere Mesh");
		SphereMesh->SetupAttachment(CapsuleComponent);
	}

	if (!ProjectileMovementComponent)
	{
		ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(
			TEXT("Projectile Movement Component"));
		ProjectileMovementComponent->SetUpdatedComponent(CapsuleComponent);
		ProjectileMovementComponent->InitialSpeed = 0.0f;
		ProjectileMovementComponent->MaxSpeed = 0.0f;
		ProjectileMovementComponent->bRotationFollowsVelocity = false;
		ProjectileMovementComponent->bShouldBounce = true;
		ProjectileMovementComponent->Bounciness = 1.f;
		ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
		ProjectileMovementComponent->Velocity = FVector(0.f);
	}

	if (!HealthComponent)
	{
		HealthComponent = CreateDefaultSubobject<UBSHealthComponent>("Health Component");
	}

	// Create ability system component, and set it to be explicitly replicated
	if (!AbilitySystemComponent)
	{
		AbilitySystemComponent = CreateDefaultSubobject<UBSAbilitySystemComponent>("Ability System Component");
		// Minimal Mode means that no GameplayEffects will replicate. They will only live on the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	}

	// Create the attribute set, this replicates by default
	// Adding it as a sub object of the owning actor of an AbilitySystemComponent
	// automatically registers the AttributeSet with the AbilitySystemComponent
	if (!AttributeSetBase)
	{
		AttributeSetBase = CreateDefaultSubobject<UBSAttributeSetBase>("Attribute Set Base");
		AbilitySystemComponent->SetIsReplicated(true);
	}

	InitialLifeSpan = 0.f;
	CurrentDeactivationHealthThreshold = 0.f;
	TargetLocation_Spawn = FVector::ZeroVector;
	TargetLocation_Activation = FVector::ZeroVector;
	TargetScale_Spawn = FVector::ZeroVector;
	TargetScale_Activation = FVector::ZeroVector;
	TargetScale_Deactivation = FVector::ZeroVector;
	ColorWhenDamageTaken = FLinearColor();
	StartToPeakTimelinePlayRate = 1.f;
	PeakToEndTimelinePlayRate = 1.f;
	bApplyLifetimeTargetScaling = false;
	bHasBeenActivated = false;
}

void ATarget::BeginPlay()
{
	Super::BeginPlay();

#if !UE_BUILD_SHIPPING
	if (GIsAutomationTesting)
	{
		return;
	}
#endif
	/* Use Color Changing Material, this is required in order to change color using C++ */
	TargetColorChangeMaterial = UMaterialInstanceDynamic::Create(SphereMesh->GetMaterial(0), this);
	SphereMesh->SetMaterial(0, TargetColorChangeMaterial);

	/* Start to Peak Target Color */
	OnStartToPeak.BindDynamic(this, &ATarget::InterpStartToPeak);
	StartToPeakTimeline.AddInterpFloat(StartToPeakCurve, OnStartToPeak);

	/* Play InterpPeakToEnd when InterpStartToPeak is finished */
	OnStartToPeakFinished.BindDynamic(this, &ATarget::PlayPeakToEndTimeline);
	StartToPeakTimeline.SetTimelineFinishedFunc(OnStartToPeakFinished);

	/* Peak Color to Fade Color */
	OnPeakToFade.BindDynamic(this, &ATarget::InterpPeakToEnd);
	PeakToEndTimeline.AddInterpFloat(PeakToEndCurve, OnPeakToFade);

	/* Set the playback rates based on TargetMaxLifeSpan */
	StartToPeakTimelinePlayRate = 1 / Config.SpawnBeatDelay;
	PeakToEndTimelinePlayRate = 1 / (Config.TargetMaxLifeSpan - Config.SpawnBeatDelay);
	StartToPeakTimeline.SetPlayRate(StartToPeakTimelinePlayRate);
	PeakToEndTimeline.SetPlayRate(PeakToEndTimelinePlayRate);
	ShrinkQuickAndGrowSlowTimeline.SetPlayRate(StartToPeakTimelinePlayRate);

	SetTargetColor(Config.OnSpawnColor);

	if (Config.bUseSeparateOutlineColor)
	{
		SetUseSeparateOutlineColor(true);
	}

	if (Config.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ShrinkQuickGrowSlow))
	{
		/* Fade the target from ColorWhenDamageTaken to BeatGridInactiveColor */
		OnShrinkQuickAndGrowSlow.BindDynamic(this, &ATarget::InterpShrinkQuickAndGrowSlow);
		ShrinkQuickAndGrowSlowTimeline.AddInterpFloat(ShrinkQuickAndGrowSlowCurve, OnShrinkQuickAndGrowSlow);
	}
}

void ATarget::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->InitAbilityActorInfo(this, this);
		if (const UBSAttributeSetBase* Set = GetAbilitySystemComponent()->GetSet<UBSAttributeSetBase>())
		{
			if (Config.MaxHealth <= 0.f)
			{
				ASC->SetNumericAttributeBase(Set->GetMaxHealthAttribute(), 10000);
			}
			else
			{
				ASC->SetNumericAttributeBase(Set->GetMaxHealthAttribute(), Config.MaxHealth);
			}
			ASC->SetNumericAttributeBase(Set->GetHitDamageAttribute(), 0.f);
			ASC->SetNumericAttributeBase(Set->GetTrackingDamageAttribute(), 0.f);
			ASC->SetNumericAttributeBase(Set->GetSelfDamageAttribute(), Config.ExpirationHealthPenalty);
		}

		CurrentDeactivationHealthThreshold = Config.MaxHealth - Config.DeactivationHealthLostThreshold;

		HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
		HealthComponent->OnDamageTakenDelegate.AddUObject(this, &ATarget::OnIncomingDamageTaken);
		ASC->OnImmunityBlockGameplayEffectDelegate.AddUObject(this, &ATarget::OnImmunityBlockGameplayEffect);

		switch (Config.TargetDamageType)
		{
		case ETargetDamageType::None:
		case ETargetDamageType::Hit:
			ActiveGE_TrackingImmunity = ASC->ApplyGameplayEffectToSelf(GE_TrackingImmunity.GetDefaultObject(), 1.f,
				GetAbilitySystemComponent()->MakeEffectContext());
			break;
		case ETargetDamageType::Tracking:
			ActiveGE_HitImmunity = ASC->ApplyGameplayEffectToSelf(GE_HitImmunity.GetDefaultObject(), 1.f,
				GetAbilitySystemComponent()->MakeEffectContext());
			break;
		case ETargetDamageType::Combined:
		case ETargetDamageType::Self:
			break;
		}
	}

	TargetScale_Spawn = GetActorScale();
	TargetLocation_Spawn = GetActorLocation();

	if (Config.TargetActivationResponses.Contains(ETargetActivationResponse::ApplyLifetimeTargetScaling))
	{
		bApplyLifetimeTargetScaling = true;
	}
	else
	{
		bApplyLifetimeTargetScaling = false;
	}

	if (ProjectileMovementComponent)
	{
		if (Config.MovingTargetDirectionMode == EMovingTargetDirectionMode::None)
		{
			ProjectileMovementComponent->Deactivate();
		}
		else
		{
			ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &ATarget::OnProjectileBounce);
			if (Config.MovingTargetDirectionMode == EMovingTargetDirectionMode::HorizontalOnly || Config.
				MovingTargetDirectionMode == EMovingTargetDirectionMode::VerticalOnly || Config.
				MovingTargetDirectionMode == EMovingTargetDirectionMode::AlternateHorizontalVertical)
			{
				ProjectileMovementComponent->bConstrainToPlane = true;
				ProjectileMovementComponent->SetPlaneConstraintNormal(FVector(1.f, 0.f, 0.f));
			}
		}
	}
}

void ATarget::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	StartToPeakTimeline.TickTimeline(DeltaSeconds);
	PeakToEndTimeline.TickTimeline(DeltaSeconds);
	ShrinkQuickAndGrowSlowTimeline.TickTimeline(DeltaSeconds);
}

void ATarget::Init(const FBS_TargetConfig& InTargetConfig)
{
	Config = InTargetConfig;
	Guid = FGuid::NewGuid();
}

void ATarget::OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	const FVector Normal = ImpactResult.Normal;
	FVector RoundedNormal = FVector::ZeroVector;
	switch (Config.MovingTargetDirectionMode)
	{
	case EMovingTargetDirectionMode::None:
		break;
	case EMovingTargetDirectionMode::HorizontalOnly:
		if (Normal.Y > 0.f)
		{
			RoundedNormal.Y = 1.f;
		}
		else
		{
			RoundedNormal.Y = -1.f;
		}
		break;
	case EMovingTargetDirectionMode::VerticalOnly:
		if (FMath::Abs(Normal.Z) > 0.f)
		{
			RoundedNormal.Z = 1.f;
		}
		else
		{
			RoundedNormal.Z = -1.f;
		}
		break;
	case EMovingTargetDirectionMode::AlternateHorizontalVertical:
		if (FMath::Abs(Normal.Y) > FMath::Abs(Normal.Z))
		{
			if (Normal.Y > 0.f)
			{
				RoundedNormal.Y = 1.f;
				RoundedNormal.Z = 0.f;
			}
			else
			{
				RoundedNormal.Y = -1.f;
				RoundedNormal.Z = 0.f;
			}
		}
		else
		{
			if (Normal.Z > 0.f)
			{
				RoundedNormal.Y = 0.f;
				RoundedNormal.Z = 1.f;
			}
			else
			{
				RoundedNormal.Y = 0.f;
				RoundedNormal.Z = -1.f;
			}
		}
		break;
	case EMovingTargetDirectionMode::Any: default:
		RoundedNormal = Normal;
		break;
	}

	const FVector NewVelocity = RoundedNormal * ImpactVelocity.Length();
	ProjectileMovementComponent->Velocity = NewVelocity;
}

void ATarget::UpdatePlayerSettings(const FPlayerSettings_Game& InPlayerSettings)
{
	SetUseSeparateOutlineColor(InPlayerSettings.bUseSeparateOutlineColor);
}

/* ----------------------------- */
/* -- AbilitySystem functions -- */
/* ----------------------------- */

UAbilitySystemComponent* ATarget::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ATarget::ApplyImmunityEffect()
{
	UAbilitySystemComponent* Comp = GetAbilitySystemComponent();
	if (!Comp || IsImmuneToDamage())
	{
		return;
	}

	const FActiveGameplayEffectHandle Handle = Comp->ApplyGameplayEffectToSelf(GE_TargetImmunity.GetDefaultObject(),
		1.f, Comp->MakeEffectContext());
	if (Handle.WasSuccessfullyApplied())
	{
		ActiveGE_TargetImmunity = Handle;
	}
}

void ATarget::RemoveImmunityEffect()
{
	UAbilitySystemComponent* Comp = GetAbilitySystemComponent();
	if (!Comp || !IsImmuneToDamage())
	{
		return;
	}

	Comp->RemoveActiveGameplayEffect(ActiveGE_TargetImmunity);
	ActiveGE_TargetImmunity.Invalidate();
}

void ATarget::OnImmunityBlockGameplayEffect(const FGameplayEffectSpec& Spec, const FActiveGameplayEffect* Effect)
{
	UE_LOG(LogTemp, Display, TEXT("Blocked tag"));
}

void ATarget::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	GetAbilitySystemComponent()->GetOwnedGameplayTags(TagContainer);
}

/* ----------------------------- */
/* -- Damage/Health functions -- */
/* ----------------------------- */

void ATarget::OnIncomingDamageTaken(const FDamageEventData& InData)
{
	if (InData.NewValue > InData.OldValue)
	{
		return;
	}

	FTimerManager& TimerManager = GetWorldTimerManager();
	const float ElapsedTime = TimerManager.GetTimerElapsed(ExpirationTimer);

	// Don't stop timer if tracking damage type
	if (InData.DamageType != ETargetDamageType::Tracking)
	{
		TimerManager.ClearTimer(ExpirationTimer);
	}

	// Save current color for use during deactivation responses
	if (TargetColorChangeMaterial)
	{
		TargetColorChangeMaterial->GetVectorParameterValue(MaterialParameterColorName, ColorWhenDamageTaken);
	}

	FTargetDamageEvent Event(InData, ElapsedTime, this);
	Event.SetTargetData(CurrentDeactivationHealthThreshold, {ETargetDamageType::Self, GetTargetDamageType()});
	OnTargetDamageEvent.Broadcast(Event);
}

void ATarget::OnLifeSpanExpired()
{
	DamageSelf();
}

void ATarget::DamageSelf(const bool bTreatAsExternalDamage)
{
	if (UAbilitySystemComponent* Comp = GetAbilitySystemComponent())
	{
		FGameplayEffectContextHandle EffectContextHandle = Comp->MakeEffectContext();
		EffectContextHandle.Get()->AddInstigator(this, this);
		const FGameplayEffectSpecHandle Handle = Comp->MakeOutgoingSpec(GE_ExpirationHealthPenalty, 1.f,
			EffectContextHandle);
		FGameplayEffectSpec* Spec = Handle.Data.Get();
		if (bTreatAsExternalDamage)
		{
			Spec->AddDynamicAssetTag(BSGameplayTags::Target_TreatAsExternalDamage);
		}
		Comp->ApplyGameplayEffectSpecToSelf(*Spec);
	}
}

void ATarget::ResetHealth()
{
	if (UAbilitySystemComponent* Comp = GetAbilitySystemComponent())
	{
		FGameplayEffectContextHandle EffectContextHandle = Comp->MakeEffectContext();
		EffectContextHandle.Get()->AddInstigator(this, this);
		const FGameplayEffectSpecHandle Handle = Comp->MakeOutgoingSpec(GE_ResetHealth, 1.f, EffectContextHandle);
		const FGameplayEffectSpec* Spec = Handle.Data.Get();
		Comp->ApplyGameplayEffectSpecToSelf(*Spec);
	}
}

/* ------------------------------------------- */
/* -- Activation, Deactivation, Destruction -- */
/* ------------------------------------------- */

bool ATarget::ActivateTarget(const float Lifespan)
{
	if (IsActivated() && !Config.bAllowActivationWhileActivated)
	{
		return false;
	}

	// Don't update activation target scale for re-activations
	if (!IsActivated())
	{
		TargetScale_Activation = GetActorScale();
	}

	// Only set an expiration timer if Lifespan > 0
	if (Lifespan > 0.f)
	{
		FTimerManager& TimerManager = GetWorldTimerManager();
		TimerManager.ClearTimer(ExpirationTimer);
		TimerManager.SetTimer(ExpirationTimer, this, &ATarget::OnLifeSpanExpired, Lifespan, false);
		PlayStartToPeakTimeline();
	}
	else if (!IsActivated())
	{
		SetTargetColor(GetNotTakingTrackingDamageColor());
	}

	// This is the only place where this value changes
	bHasBeenActivated = true;

	// This value is only changed here and DeactivateTarget
	bIsCurrentlyActivated = true;

	return true;
}

void ATarget::DeactivateTarget()
{
	StopAllTimelines();
	TargetScale_Deactivation = GetActorScale();
	CurrentDeactivationHealthThreshold -= Config.DeactivationHealthLostThreshold;
	bIsCurrentlyActivated = false;
}

void ATarget::CheckForHealthReset(const bool bOutOfHealth)
{
	const bool bUnlimitedHealth = Config.MaxHealth <= 0.f;
	if (bUnlimitedHealth && bOutOfHealth)
	{
		ResetHealth();
	}
}

/* ------------------------ */
/* -- Timeline Functions -- */
/* ------------------------ */

void ATarget::PlayStartToPeakTimeline()
{
	StopAllTimelines();
	StartToPeakTimeline.PlayFromStart();
}

void ATarget::PlayPeakToEndTimeline()
{
	StopAllTimelines();
	PeakToEndTimeline.PlayFromStart();
}

void ATarget::PlayShrinkQuickAndGrowSlowTimeline()
{
	StopAllTimelines();
	ShrinkQuickAndGrowSlowTimeline.PlayFromStart();
}

void ATarget::StopAllTimelines()
{
	if (StartToPeakTimeline.IsPlaying())
	{
		StartToPeakTimeline.Stop();
	}
	if (PeakToEndTimeline.IsPlaying())
	{
		PeakToEndTimeline.Stop();
	}
	if (ShrinkQuickAndGrowSlowTimeline.IsPlaying())
	{
		ShrinkQuickAndGrowSlowTimeline.Stop();
	}
}

/* ------------------------ */
/* -- Timeline Callbacks -- */
/* ------------------------ */

void ATarget::InterpStartToPeak(const float Alpha)
{
	SetTargetColor(UKismetMathLibrary::LinearColorLerp(Config.StartColor, Config.PeakColor, Alpha));
	if (bApplyLifetimeTargetScaling)
	{
		SetTargetScale(FVector(UKismetMathLibrary::Lerp(GetTargetScale_Deactivation().X,
			GetTargetScale_Deactivation().X * Config.LifetimeTargetScaleMultiplier,
			StartToPeakTimeline.GetPlaybackPosition() * Config.SpawnBeatDelay / Config.TargetMaxLifeSpan)));
	}
}

void ATarget::InterpPeakToEnd(const float Alpha)
{
	SetTargetColor(UKismetMathLibrary::LinearColorLerp(Config.PeakColor, Config.EndColor, Alpha));
	if (bApplyLifetimeTargetScaling)
	{
		SetTargetScale(FVector(UKismetMathLibrary::Lerp(GetTargetScale_Deactivation().X,
			GetTargetScale_Deactivation().X * Config.LifetimeTargetScaleMultiplier,
			(PeakToEndTimeline.GetPlaybackPosition() * (Config.TargetMaxLifeSpan - Config.SpawnBeatDelay) + Config.
				SpawnBeatDelay) / Config.TargetMaxLifeSpan)));
	}
}

void ATarget::InterpShrinkQuickAndGrowSlow(const float Alpha)
{
	SetTargetScale(FVector(UKismetMathLibrary::Lerp(Constants::MinShrinkTargetScale, GetTargetScale_Activation().X,
		Alpha)));
	const FLinearColor Color = UKismetMathLibrary::LinearColorLerp(ColorWhenDamageTaken, Config.InactiveTargetColor,
		ShrinkQuickAndGrowSlowTimeline.GetPlaybackPosition());
	SetTargetColor(Color);
}

/* ---------------------- */
/* -- Setter functions -- */
/* ---------------------- */

void ATarget::SetTargetColor(const FLinearColor& Color)
{
#if !UE_BUILD_SHIPPING
	if (GIsAutomationTesting)
	{
		return;
	}
#endif
	TargetColorChangeMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
}

void ATarget::SetTargetOutlineColor(const FLinearColor& Color)
{
#if !UE_BUILD_SHIPPING
	if (GIsAutomationTesting)
	{
		return;
	}
#endif
	TargetColorChangeMaterial->SetVectorParameterValue(TEXT("OutlineColor"), Color);
}

void ATarget::SetUseSeparateOutlineColor(const bool bUseSeparateOutlineColor)
{
#if !UE_BUILD_SHIPPING
	if (GIsAutomationTesting)
	{
		return;
	}
#endif
	if (bUseSeparateOutlineColor)
	{
		SetTargetOutlineColor(Config.OutlineColor);
		TargetColorChangeMaterial->SetScalarParameterValue("bUseSeparateOutlineColor", 1.f);
		return;
	}
	TargetColorChangeMaterial->SetScalarParameterValue("bUseSeparateOutlineColor", 0.f);
}

void ATarget::SetTargetColorToInactiveColor()
{
	SetTargetColor(Config.InactiveTargetColor);
}

void ATarget::SetTargetDirection(const FVector& NewDirection) const
{
	if (ProjectileMovementComponent->IsActive())
	{
		ProjectileMovementComponent->Velocity = NewDirection * ProjectileMovementComponent->InitialSpeed;
	}
}

void ATarget::SetTargetSpeed(const float NewMovingTargetSpeed) const
{
	if (ProjectileMovementComponent->IsActive())
	{
		FVector DirectionUnitVector;
		if (ProjectileMovementComponent->Velocity.IsNearlyZero() || FMath::IsNearlyZero(
			ProjectileMovementComponent->InitialSpeed))
		{
			DirectionUnitVector = FVector::ZeroVector;
		}
		else
		{
			DirectionUnitVector = ProjectileMovementComponent->Velocity / ProjectileMovementComponent->InitialSpeed;
		}
		ProjectileMovementComponent->InitialSpeed = NewMovingTargetSpeed;
		ProjectileMovementComponent->Velocity = DirectionUnitVector * ProjectileMovementComponent->InitialSpeed;
	}
}

void ATarget::SetTargetScale(const FVector& NewScale) const
{
	// Cap target scale at MaxValue_TargetScale
	CapsuleComponent->SetRelativeScale3D(NewScale.X < Constants::MaxValue_TargetScale
		? NewScale
		: FVector(Constants::MaxValue_TargetScale));
}

void ATarget::SetTargetDamageType(const ETargetDamageType& InType)
{
	TargetDamageType = InType;
}

void ATarget::PlayExplosionEffect(const FVector& ExplosionLocation, const float SphereRadius,
	const FLinearColor& InColorWhenDestroyed) const
{
#if !UE_BUILD_SHIPPING
	if (GIsAutomationTesting)
	{
		return;
	}
#endif
	if (TargetExplosion && Config.TargetDamageType == ETargetDamageType::Hit)
	{
		if (UNiagaraComponent* ExplosionComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
			TargetExplosion, ExplosionLocation))
		{
			ExplosionComp->SetFloatParameter(TargetExplosionSphereRadiusParameterName, SphereRadius);
			ExplosionComp->SetColorParameter(TargetExplosionColorParameterName, InColorWhenDestroyed);
		}
	}
}

/* ---------------------- */
/* -- Getter functions -- */
/* ---------------------- */

FLinearColor ATarget::GetPeakTargetColor() const
{
	return Config.PeakColor;
}

FLinearColor ATarget::GetEndTargetColor() const
{
	return Config.EndColor;
}

FLinearColor ATarget::GetInActiveTargetColor() const
{
	return Config.InactiveTargetColor;
}

FLinearColor ATarget::GetTakingTrackingDamageColor() const
{
	return Config.TakingTrackingDamageColor;
}

FLinearColor ATarget::GetNotTakingTrackingDamageColor() const
{
	return Config.NotTakingTrackingDamageColor;
}

bool ATarget::HasBeenActivatedBefore() const
{
	return bHasBeenActivated;
}

bool ATarget::IsActivated() const
{
	return bIsCurrentlyActivated;
}

bool ATarget::IsImmuneToDamage() const
{
	return ActiveGE_TargetImmunity.IsValid();
}

bool ATarget::IsImmuneToHitDamage() const
{
	return ActiveGE_TargetImmunity.IsValid() || ActiveGE_HitImmunity.IsValid();
}

bool ATarget::IsImmuneToTrackingDamage() const
{
	return ActiveGE_TargetImmunity.IsValid() || ActiveGE_TrackingImmunity.IsValid();
}

ETargetDamageType ATarget::GetTargetDamageType() const
{
	if (!IsImmuneToHitDamage() && TargetDamageType == ETargetDamageType::Hit)
	{
		return TargetDamageType;
	}
	if (!IsImmuneToTrackingDamage() && TargetDamageType == ETargetDamageType::Tracking)
	{
		return TargetDamageType;
	}
	if (!IsImmuneToHitDamage() && !IsImmuneToTrackingDamage() && TargetDamageType == ETargetDamageType::Combined)
	{
		return TargetDamageType;
	}
	UE_LOG(LogTemp, Warning, TEXT("Target DamageType != its immune counterpart."));
	return ETargetDamageType::None;
}

FVector ATarget::GetTargetDirection() const
{
	if (ProjectileMovementComponent->IsActive())
	{
		return ProjectileMovementComponent->Velocity / ProjectileMovementComponent->InitialSpeed;
	}
	return FVector(0.f);
}

FVector ATarget::GetTargetScale_Activation() const
{
	if (TargetScale_Activation != FVector::ZeroVector)
	{
		return TargetScale_Activation;
	}
	return GetTargetScale_Spawn();
}

FVector ATarget::GetTargetScale_Deactivation() const
{
	if (TargetScale_Deactivation != FVector::ZeroVector)
	{
		return TargetScale_Deactivation;
	}
	return GetTargetScale_Activation();
}

FVector ATarget::GetTargetScale_Spawn() const
{
	return TargetScale_Spawn;
}

FVector ATarget::GetTargetLocation_Activation() const
{
	if (TargetLocation_Activation != FVector::ZeroVector)
	{
		return TargetLocation_Activation;
	}
	return TargetLocation_Spawn;
}

FVector ATarget::GetTargetLocation_Spawn() const
{
	return TargetLocation_Spawn;
}

float ATarget::GetTargetSpeed() const
{
	if (ProjectileMovementComponent->IsActive())
	{
		return ProjectileMovementComponent->InitialSpeed;
	}
	return 0.f;
}

FVector ATarget::GetTargetVelocity() const
{
	if (ProjectileMovementComponent->IsActive())
	{
		return ProjectileMovementComponent->Velocity;
	}
	return FVector(0.f);
}

float ATarget::GetSpawnBeatDelay() const
{
	return Config.SpawnBeatDelay;
}
