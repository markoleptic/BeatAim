// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "Target/TargetManager.h"
#include "BSGameMode.h"
#include "GlobalConstants.h"
#include "Target/Target.h"
#include "Components/BoxComponent.h"
#include "Engine/CompositeCurveTable.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Target/ReinforcementLearningComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Target/SpawnArea.h"
#include "Target/SpawnAreaManagerComponent.h"

FVector (&RandBoxPoint)(const FVector Center, const FVector Extents) = UKismetMathLibrary::RandomPointInBoundingBox;
DEFINE_LOG_CATEGORY(LogTargetManager);
using namespace Constants;

ATargetManager::ATargetManager()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Spawn Box"));
	RootComponent = SpawnBox;

	StaticExtentsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Static Extents Box"));
	StaticExtentsBox->SetupAttachment(SpawnBox);
	
	SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Spawn Volume"));
	TopBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Top Box"));
	BottomBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bottom Box"));
	LeftBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Box"));
	RightBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Box"));
	ForwardBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Forward Box"));
	BackwardBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Backward Box"));
	SpawnAreaManager = CreateDefaultSubobject<USpawnAreaManagerComponent>(TEXT("Spawn Area Manager Component"));
	RLComponent = CreateDefaultSubobject<UReinforcementLearningComponent>(TEXT("Reinforcement Learning Component"));

	CurrentStreak = 0;
	BSConfig = nullptr;
	ShouldSpawn = false;
	LastTargetDamageType = ETargetDamageType::Tracking;
	CurrentTargetScale = FVector(1.f);
	StaticExtrema = FExtrema();
	StaticExtents = FVector();
	CurrentStreak = 0;
	DynamicLookUpValue_TargetScale = 0;
	DynamicLookUpValue_SpawnAreaScale = 0;
	ManagedTargets = TMap<FGuid, ATarget*>();
	TotalPossibleDamage = 0.f;
	bLastSpawnedTargetDirectionChangeHorizontal = false;
	bLastActivatedTargetDirectionChangeHorizontal = false;
}

void ATargetManager::BeginPlay()
{
	Super::BeginPlay();

	TargetSpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	TargetSpawnInfo.Owner = this;
	TargetSpawnInfo.CustomPreSpawnInitalization = [this] (AActor* SpawnedActor)
	{
		if (ATarget* Target = Cast<ATarget>(SpawnedActor))
		{
			Target->Init(GetBSConfig()->TargetConfig);
		}
	};
}

void ATargetManager::Destroyed()
{
	SetShouldSpawn(false);
	
	if (!ManagedTargets.IsEmpty())
	{
		for (ATarget* Target : GetManagedTargets())
		{
			if (Target)
			{
				Target->Destroy();
			}
		}
		ManagedTargets.Empty();
	}
	
	Super::Destroyed();
}

void ATargetManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (ShouldSpawn && TrackingTargetIsDamageable())
	{
		UpdateTotalPossibleDamage();
	}
}

void ATargetManager::Init(const TSharedPtr<FBSConfig>& InConfig, const FPlayerSettings_Game& InPlayerSettings)
{
	Clear();
	BSConfig = InConfig;
	
	// Initialize target colors
	GetBSConfig()->InitColors(InPlayerSettings.bUseSeparateOutlineColor, InPlayerSettings.InactiveTargetColor,
		InPlayerSettings.TargetOutlineColor, InPlayerSettings.StartTargetColor, InPlayerSettings.PeakTargetColor,
		InPlayerSettings.EndTargetColor, InPlayerSettings.TakingTrackingDamageColor,
		InPlayerSettings.NotTakingTrackingDamageColor);

	// Set SpawnBox location & BoxExtent, StaticExtents, and StaticExtrema
	SpawnBox->SetRelativeLocation(GenerateStaticLocation(GetBSConfig().Get()));
	StaticExtents = GenerateStaticExtents(GetBSConfig().Get());
	SpawnBox->SetBoxExtent(StaticExtents);
	StaticExtentsBox->SetBoxExtent(StaticExtents);
	StaticExtrema = GenerateStaticExtrema(GetBSConfig().Get(), GetSpawnBoxOrigin(), StaticExtents);

	// Initialize the CompositeCurveTables in case they need to be modified
	Init_Tables();

	// Initialize the SpawnAreaManager
	SpawnAreaManager->Init(GetBSConfig(), GetSpawnBoxOrigin(), StaticExtents, StaticExtrema);
	SpawnAreaManager->GetSpawnAreaRequestDelegate().BindUObject(RLComponent, &UReinforcementLearningComponent::ChooseNextActionIndex);

	// Initialize SpawnBox extents and the SpawnVolume extents & location
	const bool bDynamic = GetBSConfig()->TargetConfig.BoundsScalingPolicy == EBoundsScalingPolicy::Dynamic;
	const float Factor = bDynamic ? GetCurveTableValue(true, DynamicLookUpValue_SpawnAreaScale) : 1.f;
	UpdateSpawnBoxExtents(Factor);
	UpdateSpawnVolume(Factor);
	SpawnAreaManager->OnExtremaChanged(GetSpawnBoxExtrema());

	// Init RLC
	if (GetBSConfig()->IsCompatibleWithReinforcementLearning())
	{
		const FCommonScoreInfo CommonScoreInfo = FindOrAddCommonScoreInfo(GetBSConfig()->DefiningConfig);
		const FRLAgentParams Params(GetBSConfig()->AIConfig, CommonScoreInfo, SpawnAreaManager->GetSpawnAreaSize());
		RLComponent->Init(Params);

		#if !UE_BUILD_SHIPPING
		if (RLComponent->bPrintDebug_QTableInit && !GIsAutomationTesting)
		{
			// Print loaded QTable
			FNumberFormattingOptions Options;
			Options.MinimumFractionalDigits = 2;
			Options.MaximumFractionalDigits = 2;
			Options.MaximumIntegralDigits = 1;
			Options.MinimumIntegralDigits = 1;
			UE_LOG(LogTargetManager, Display, TEXT("Loaded QTable:"));
			USaveGamePlayerScore::PrintQTable(BSConfig->DefiningConfig, CommonScoreInfo, Options);
		}
		#endif
	}

	SpawnAreaManager->SetShouldAskRLCForSpawnAreas(RLComponent->GetReinforcementLearningMode() == EReinforcementLearningMode::Exploration || RLComponent->
		GetReinforcementLearningMode() == EReinforcementLearningMode::ActiveAgent);

	RandomNumToActivateStream.Initialize(FMath::Rand());

	// Spawn any targets if needed
	if (GetBSConfig()->TargetConfig.TargetSpawningPolicy == ETargetSpawningPolicy::UpfrontOnly)
	{
		HandleUpfrontSpawning();
	}
}

void ATargetManager::Init_Tables() const
{
	FRealCurve* PreThresholdCurve = CCT_SpawnArea->GetCurves()[0].CurveToEdit;
	int32 CurveIndex = GetBSConfig()->DynamicSpawnAreaScaling.bIsCubicInterpolation ? 2 : 1;
	FRealCurve* ThresholdMetCurve = CCT_SpawnArea->GetCurves()[CurveIndex].CurveToEdit;

	PreThresholdCurve->SetKeyTime(PreThresholdCurve->GetFirstKeyHandle(), 0);
	PreThresholdCurve->SetKeyTime(PreThresholdCurve->GetLastKeyHandle(),
		GetBSConfig()->DynamicSpawnAreaScaling.StartThreshold);
	ThresholdMetCurve->SetKeyTime(ThresholdMetCurve->GetFirstKeyHandle(),
		GetBSConfig()->DynamicSpawnAreaScaling.StartThreshold);
	ThresholdMetCurve->SetKeyTime(ThresholdMetCurve->GetLastKeyHandle(),
		GetBSConfig()->DynamicSpawnAreaScaling.EndThreshold);

	PreThresholdCurve = CCT_TargetScale->GetCurves()[0].CurveToEdit;
	CurveIndex = GetBSConfig()->DynamicTargetScaling.bIsCubicInterpolation ? 2 : 1;
	ThresholdMetCurve = CCT_TargetScale->GetCurves()[CurveIndex].CurveToEdit;

	PreThresholdCurve->SetKeyTime(PreThresholdCurve->GetFirstKeyHandle(), 0);
	PreThresholdCurve->SetKeyTime(PreThresholdCurve->GetLastKeyHandle(),
		GetBSConfig()->DynamicTargetScaling.StartThreshold);
	ThresholdMetCurve->SetKeyTime(ThresholdMetCurve->GetFirstKeyHandle(),
		GetBSConfig()->DynamicTargetScaling.StartThreshold);
	ThresholdMetCurve->SetKeyTime(ThresholdMetCurve->GetLastKeyHandle(),
		GetBSConfig()->DynamicTargetScaling.EndThreshold);
}

void ATargetManager::Clear()
{
	ShouldSpawn = false;
	if (!ManagedTargets.IsEmpty())
	{
		for (ATarget* Target : GetManagedTargets())
		{
			if (Target)
			{
				Target->Destroy();
			}
		}
		ManagedTargets.Empty();
	}
	CurrentStreak = 0;
	BSConfig = nullptr;
	LastTargetDamageType = ETargetDamageType::Tracking;
	CurrentTargetScale = FVector(1.f);
	StaticExtrema = FExtrema();
	StaticExtents = FVector();
	CurrentStreak = 0;
	DynamicLookUpValue_TargetScale = 0;
	DynamicLookUpValue_SpawnAreaScale = 0;
	TotalPossibleDamage = 0.f;
	bLastSpawnedTargetDirectionChangeHorizontal = false;
	bLastActivatedTargetDirectionChangeHorizontal = false;
	
	RLComponent->Clear();
	SpawnAreaManager->Clear();
}

void ATargetManager::SetShouldSpawn(const bool bShouldSpawn)
{
	if (bShouldSpawn)
	{
		if (GetBSConfig()->TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::HideTarget))
		{
			for (const auto TargetPair : ManagedTargets)
			{
				TargetPair.Value->SetActorHiddenInGame(true);
			}
		}
	}
	ShouldSpawn = bShouldSpawn;
}

void ATargetManager::OnPlayerStopTrackingTarget()
{
	for (ATarget* Target : GetManagedTargets())
	{
		if (Target && !Target->IsImmuneToTrackingDamage())
		{
			Target->SetTargetColor(GetBSConfig()->TargetConfig.NotTakingTrackingDamageColor);
		}
	}
}

/* ------------------------------------ */
/* -- Target spawning and activation -- */
/* ------------------------------------ */

void ATargetManager::OnAudioAnalyzerBeat()
{
	if (!ShouldSpawn) return;
	
	const int32 LastSeed = RandomNumToActivateStream.GetCurrentSeed();
	HandleRuntimeSpawning();
	RandomNumToActivateStream.Initialize(LastSeed);
	HandleTargetActivation();
	
	#if !UE_BUILD_SHIPPING
	SpawnAreaManager->RefreshDebugBoxes();
	#endif
}

ATarget* ATargetManager::SpawnTarget(USpawnArea* InSpawnArea)
{
	if (!InSpawnArea) return nullptr;
	
	const FTransform TForm(FRotator(0), InSpawnArea->GetChosenPoint(), InSpawnArea->GetTargetScale());
	ATarget* Target = GetWorld()->SpawnActor<ATarget>(TargetToSpawn, TForm, TargetSpawnInfo);
	
	#if !UE_BUILD_SHIPPING
	if (GIsAutomationTesting) Target->DispatchBeginPlay();
	#endif
	
	Target->SetTargetDamageType(FindNextTargetDamageType());
	Target->OnTargetDamageEvent.AddUObject(this, &ATargetManager::OnTargetDamageEvent);
	AddToManagedTargets(Target, InSpawnArea);

	if (!Target) return nullptr;
	
	// Handle spawn responses
	if (BSConfig->TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::AddImmunity))
	{
		Target->ApplyImmunityEffect();
	}
	if (BSConfig->TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeVelocity))
	{
		const float SpawnVelocity = FMath::FRandRange(GetBSConfig()->TargetConfig.MinSpawnedTargetSpeed,
			GetBSConfig()->TargetConfig.MaxSpawnedTargetSpeed);
		Target->SetTargetSpeed(SpawnVelocity);

		// In case no direction change is provided
		if (!GetBSConfig()->TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeDirection)
			&& GetBSConfig()->TargetConfig.MovingTargetDirectionMode != EMovingTargetDirectionMode::None)
		{
			ChangeTargetDirection(Target, 0);
		}
	}
	if (GetBSConfig()->TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeDirection))
	{
		ChangeTargetDirection(Target, 0);
	}
	return Target;
}

void ATargetManager::AddToManagedTargets(ATarget* SpawnTarget, USpawnArea* SpawnArea)
{
	ManagedTargets.Add(SpawnTarget->GetGuid(), SpawnTarget);
	SpawnAreaManager->FlagSpawnAreaAsManaged(SpawnArea, SpawnTarget->GetGuid());
}

bool ATargetManager::ActivateTarget(ATarget* InTarget) const
{
	if (!InTarget || !SpawnAreaManager->IsSpawnAreaValid(SpawnAreaManager->GetSpawnArea(InTarget->GetGuid())))
	{
		return false;
	}

	const TArray<ETargetActivationResponse>& Responses = BSConfig->TargetConfig.TargetActivationResponses;
	
	// Only perform some Activation Responses if already activated
	const bool bAlreadyActivated = InTarget->IsActivated();

	// Make sure it isn't hidden
	if (!bAlreadyActivated && InTarget->IsHidden())
	{
		InTarget->SetActorHiddenInGame(false);
	}
	if (!bAlreadyActivated && Responses.Contains(ETargetActivationResponse::AddImmunity))
	{
		InTarget->ApplyImmunityEffect();
	}
	if (!bAlreadyActivated && Responses.Contains(ETargetActivationResponse::RemoveImmunity))
	{
		InTarget->RemoveImmunityEffect();
	}
	if (!bAlreadyActivated && Responses.Contains(ETargetActivationResponse::ToggleImmunity))
	{
		InTarget->IsImmuneToDamage() ? InTarget->RemoveImmunityEffect() : InTarget->ApplyImmunityEffect();
	}
	if (Responses.Contains(ETargetActivationResponse::ChangeVelocity))
	{
		InTarget->SetTargetSpeed(FMath::FRandRange(GetBSConfig()->TargetConfig.MinActivatedTargetSpeed,
			GetBSConfig()->TargetConfig.MaxActivatedTargetSpeed));
		if (!Responses.Contains(ETargetActivationResponse::ChangeDirection)
			&& GetBSConfig()->TargetConfig.MovingTargetDirectionMode != EMovingTargetDirectionMode::None)
		{
			ChangeTargetDirection(InTarget, 1);
		}
	}
	if (Responses.Contains(ETargetActivationResponse::ChangeDirection))
	{
		ChangeTargetDirection(InTarget, 1);
	}

	// TODO: Overall weird and confusing setting, probably remove or refactor
	if (!bAlreadyActivated && InTarget->HasBeenActivatedBefore() &&
		Responses.Contains(ETargetActivationResponse::ApplyConsecutiveTargetScale))
	{
		InTarget->SetTargetScale(FindNextSpawnedTargetScale());
	}
	
	const bool bActivated = InTarget->ActivateTarget(GetBSConfig()->TargetConfig.TargetMaxLifeSpan);

	// Don't continue if failed to activate
	if (!bActivated) return false;

	// Cache the previous SpawnArea
	const USpawnArea* Previous = SpawnAreaManager->GetMostRecentSpawnArea();

	// Update SpawnArea and scale
	SpawnAreaManager->FlagSpawnAreaAsActivated(InTarget->GetGuid(), InTarget->GetActorScale());

	OnTargetActivated.Broadcast(InTarget->GetTargetDamageType());
	OnTargetActivated_AimBot.Broadcast(InTarget);
	
	// Don't continue if the target was already activated and succeeded the reactivation
	if (bAlreadyActivated && bActivated)
	{
		#if !UE_BUILD_SHIPPING
		if (!GIsAutomationTesting)
		{
			UE_LOG(LogTargetManager, Display, TEXT("Reactivated Target"));
		}
		#endif
		return true;
	}
	
	if (RLComponent->GetReinforcementLearningMode() != EReinforcementLearningMode::None && Previous)
	{
		if (const USpawnArea* Current = SpawnAreaManager->GetSpawnArea(InTarget->GetGuid()))
		{
			RLComponent->AddToActiveTargetPairs(Previous->GetIndex(), Current->GetIndex());
		}
	}
	return true;
}

void ATargetManager::DeactivateTarget(ATarget* InTarget, const bool bExpired, const bool bOutOfHealth) const
{
	const FBS_TargetConfig& Config = BSConfig->TargetConfig;
	const TArray<ETargetDeactivationResponse>& Responses = BSConfig->TargetConfig.TargetDeactivationResponses;
	
	// Immunity
	if (Responses.Contains(ETargetDeactivationResponse::RemoveImmunity))
	{
		InTarget->RemoveImmunityEffect();
	}
	if (Responses.Contains(ETargetDeactivationResponse::AddImmunity))
	{
		InTarget->ApplyImmunityEffect();
	}
	if (Responses.Contains(ETargetDeactivationResponse::ToggleImmunity))
	{
		InTarget->IsImmuneToDamage() ? InTarget->RemoveImmunityEffect() : InTarget->ApplyImmunityEffect();
	}

	// Scale
	if (Responses.Contains(ETargetDeactivationResponse::ResetScaleToSpawnedScale))
	{
		InTarget->SetTargetScale(InTarget->GetTargetScale_Spawn());
	}
	if (Responses.Contains(ETargetDeactivationResponse::ResetScaleToActivatedScale))
	{
		InTarget->SetTargetScale(InTarget->GetTargetScale_Activation());
	}
	if (Responses.Contains(ETargetDeactivationResponse::ApplyDeactivatedTargetScaleMultiplier))
	{
		InTarget->SetTargetScale(InTarget->GetActorScale() * Config.ConsecutiveChargeScaleMultiplier);
	}

	// Position
	if (Responses.Contains(ETargetDeactivationResponse::ResetPositionToSpawnedPosition))
	{
		InTarget->SetActorLocation(InTarget->GetTargetLocation_Spawn());
	}
	if (Responses.Contains(ETargetDeactivationResponse::ResetPositionToActivatedPosition))
	{
		InTarget->SetActorLocation(InTarget->GetTargetLocation_Activation());
	}

	// Velocity
	if (Responses.Contains(ETargetDeactivationResponse::ChangeVelocity))
	{
		InTarget->SetTargetSpeed(FMath::FRandRange(Config.MinDeactivatedTargetSpeed, Config.MaxDeactivatedTargetSpeed));
		
		if (!Responses.Contains(ETargetDeactivationResponse::ChangeDirection) &&
			Config.MovingTargetDirectionMode != EMovingTargetDirectionMode::None)
		{
			ChangeTargetDirection(InTarget, 2);
		}
	}

	// Direction
	if (Responses.Contains(ETargetDeactivationResponse::ChangeDirection))
	{
		ChangeTargetDirection(InTarget, 2);
	}

	// Effects
	if (Responses.Contains(ETargetDeactivationResponse::PlayExplosionEffect) && !bExpired)
	{
		const FVector Loc = InTarget->SphereMesh->GetComponentLocation();
		const float SphereRadius =  SphereTargetRadius * InTarget->GetActorScale().X;
		InTarget->PlayExplosionEffect(Loc, SphereRadius, InTarget->ColorWhenDamageTaken);
	}
	if (Responses.Contains(ETargetDeactivationResponse::ShrinkQuickGrowSlow) && !bExpired)
	{
		InTarget->PlayShrinkQuickAndGrowSlowTimeline();
	}
	
	// Hide target
	if (Responses.Contains(ETargetDeactivationResponse::HideTarget))
	{
		InTarget->SetActorHiddenInGame(true);
	}

	// Colors
	if (Responses.Contains(ETargetDeactivationResponse::ResetColorToInactiveColor))
	{
		InTarget->SetTargetColor(Config.InactiveTargetColor);
	}

	InTarget->DeactivateTarget();
	InTarget->CheckForHealthReset(bOutOfHealth);
	
	// Handle reactivation
	if (Responses.Contains(ETargetDeactivationResponse::Reactivate))
	{
		ActivateTarget(InTarget);
	}
}

bool ATargetManager::ShouldDeactivateTarget(const bool bExpired, const float CurrentHealth,
	const float DeactivationThreshold) const
{
	const bool bOutOfHealth = CurrentHealth <= 0.f;
	const bool ThresholdPassed = CurrentHealth <= DeactivationThreshold;
	const TArray<ETargetDeactivationCondition>& Conditions = BSConfig->TargetConfig.TargetDeactivationConditions;
	
	if (Conditions.Contains(ETargetDeactivationCondition::Persistent))
	{
		return false;
	}
	if (bExpired && Conditions.Contains(ETargetDeactivationCondition::OnExpiration))
	{
		return true;
	}
	if (bOutOfHealth && Conditions.Contains(ETargetDeactivationCondition::OnHealthReachedZero))
	{
		return true;
	}
	if (ThresholdPassed && Conditions.Contains(ETargetDeactivationCondition::OnSpecificHealthLost))
	{
		return true;
	}
	if (!bExpired && Conditions.Contains(ETargetDeactivationCondition::OnAnyExternalDamageTaken))
	{
		return true;
	}
	return false;
}

bool ATargetManager::ShouldDestroyTarget(const bool bExpired, const bool bOutOfHealth) const
{
	const TArray<ETargetDestructionCondition>& Conditions = BSConfig->TargetConfig.TargetDestructionConditions;

	if (Conditions.Contains(ETargetDestructionCondition::Persistent))
	{
		return false;
	}
	if (BSConfig->TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::Destroy))
	{
		return true;
	}
	if (Conditions.Contains(ETargetDestructionCondition::OnDeactivation))
	{
		return true;
	}
	if (Conditions.Contains(ETargetDestructionCondition::OnExpiration) && bExpired)
	{
		return true;
	}
	if (Conditions.Contains(ETargetDestructionCondition::OnHealthReachedZero) && bOutOfHealth)
	{
		return true;
	}
	if (Conditions.Contains(ETargetDestructionCondition::OnAnyExternalDamageTaken) && !bExpired)
	{
		return true;
	}
	return false;
}

void ATargetManager::HandleUpfrontSpawning()
{
	if (GetBSConfig()->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid)
	{
		for (int i = 0; i < SpawnAreaManager->GetSpawnAreaSize().Y * SpawnAreaManager->GetSpawnAreaSize().Z; i++)
		{
			SpawnAreaManager->GetSpawnArea(i)->SetTargetScale(FindNextSpawnedTargetScale());
			SpawnTarget(SpawnAreaManager->GetSpawnArea(i));
		}
	}
	else
	{
		for (USpawnArea* SpawnArea : FindNextSpawnAreasForSpawn(GetBSConfig()->TargetConfig.NumUpfrontTargetsToSpawn))
		{
			if (SpawnArea)
			{
				SpawnTarget(SpawnArea);
			}
		}
	}
}

int32 ATargetManager::HandleRuntimeSpawning()
{
	const auto& Cfg = GetBSConfig()->TargetConfig;
	
	if (Cfg.TargetSpawningPolicy != ETargetSpawningPolicy::RuntimeOnly) return 0;
	
	int32 NumberToSpawn = GetNumberOfRuntimeTargetsToSpawn();

	// Only spawn targets that can be activated unless allowed
	if (!Cfg.bAllowSpawnWithoutActivation)
	{
		int32 MaxToActivate = FMath::Max(Cfg.MinNumTargetsToActivateAtOnce, Cfg.MaxNumTargetsToActivateAtOnce);
		if (Cfg.MaxNumActivatedTargetsAtOnce >= 1)
		{
			MaxToActivate = FMath::Min(Cfg.MaxNumActivatedTargetsAtOnce, MaxToActivate);
		}
		const int32 MaxAvailable = FMath::Min(NumberToSpawn, MaxToActivate);
		NumberToSpawn = GetNumberOfTargetsToActivate(MaxAvailable, SpawnAreaManager->GetNumActivated());
	}
	
	int32 NumSpawned = 0;
	for (USpawnArea* SpawnArea : FindNextSpawnAreasForSpawn(NumberToSpawn))
	{
		if (SpawnArea && SpawnTarget(SpawnArea))
		{
			NumSpawned++;
		}
		else
		{
			#if !UE_BUILD_SHIPPING
			UE_LOG(LogTargetManager, Warning, TEXT("Failed to spawn target."));
			#endif
		}
	}

	return NumSpawned;
}

int32 ATargetManager::HandleTargetActivation() const
{
	if (ManagedTargets.IsEmpty()) return 0;

	const auto& Cfg = GetBSConfig()->TargetConfig;

	// Persistent Targets are the only type that can always receive continuous activation
	if (Cfg.TargetDeactivationConditions.Contains(ETargetDeactivationCondition::Persistent))
	{
		HandlePermanentlyActiveTargetActivation();
		return 0;
	}
	
	// See if theres any Spawn Areas available to activate
	const int32 NumAvailableToActivate = SpawnAreaManager->GetNumDeactivated();
	const int32 NumCurrentlyActivated = SpawnAreaManager->GetNumActivated();

	// If not allowed to spawn without activation and runtime spawning,
	// HandleRuntimeSpawning spawned the number of targets that can be activated
	const int32 NumToActivate = !Cfg.bAllowSpawnWithoutActivation &&
		Cfg.TargetSpawningPolicy == ETargetSpawningPolicy::RuntimeOnly
		? NumAvailableToActivate 
		: GetNumberOfTargetsToActivate(NumAvailableToActivate, NumCurrentlyActivated);

	#if !UE_BUILD_SHIPPING
	if (!Cfg.bAllowSpawnWithoutActivation && Cfg.TargetSpawningPolicy == ETargetSpawningPolicy::RuntimeOnly)
	{
		// Max Allowed has higher priority than Max Available
		const int32 MaxAllowed = Cfg.MaxNumActivatedTargetsAtOnce >= 1
			? Cfg.MaxNumActivatedTargetsAtOnce
			: DefaultNumTargetsToActivate;
		const int32 RemainingActivations = FMath::Max(0, MaxAllowed - NumCurrentlyActivated);
		const int32 NumToActivateCheck = FMath::Min(RemainingActivations, NumAvailableToActivate);
		if (NumToActivateCheck != NumAvailableToActivate)
		{
			if (NumToActivateCheck < NumAvailableToActivate)
			{
				UE_LOG(LogTemp, Display, TEXT("NumToActivateCheck < NumAvailableToActivate %d %d"),
					NumToActivateCheck, NumAvailableToActivate);
			}
			else
			{
				UE_LOG(LogTemp, Display, TEXT("NumToActivateCheck > NumAvailableToActivate %d %d"),
					NumToActivateCheck, NumAvailableToActivate);
			}
		}
	}
	#endif
	
	// Scuffed temporary solution to make tracking more interesting
	if (NumAvailableToActivate == 0 && NumToActivate == 0 && Cfg.bAllowActivationWhileActivated)
	{
		HandleActivateAlreadyActivated();
		return 0;
	}

	int32 NumActivated = 0;
	for (const USpawnArea* SpawnArea : SpawnAreaManager->GetActivatableSpawnAreas(NumToActivate))
	{
		if (ATarget* Target = ManagedTargets.FindRef(SpawnArea->GetGuid()))
		{
			if (ActivateTarget(Target))
			{
				NumActivated++;
			}
			else
			{
				#if !UE_BUILD_SHIPPING
				UE_LOG(LogTargetManager, Warning, TEXT("Failed to activate target."));
				#endif
			}
		}
		else
		{
			#if !UE_BUILD_SHIPPING
			UE_LOG(LogTargetManager, Warning, TEXT("Failed to find target guid for activation."));
			#endif
		}
	}
	return NumActivated;
}

void ATargetManager::HandlePermanentlyActiveTargetActivation() const
{
	// Handle initial activation
	auto SpawnAreas = SpawnAreaManager->GetActivatedSpawnAreas();
	if (SpawnAreas.IsEmpty())
	{
		SpawnAreas = SpawnAreaManager->GetDeactivatedSpawnAreas();
	}

	for (const USpawnArea* SpawnArea : SpawnAreas)
	{
		if (ATarget* Target = ManagedTargets.FindRef(SpawnArea->GetGuid()))
		{
			ActivateTarget(Target);
		}
	}
}

void ATargetManager::HandleActivateAlreadyActivated() const
{
	// Default to very high number if less than 1
	int32 MinToActivate = FMath::Min(GetBSConfig()->TargetConfig.MinNumTargetsToActivateAtOnce,
		GetBSConfig()->TargetConfig.MaxNumTargetsToActivateAtOnce);
	int32 MaxToActivate = FMath::Max(GetBSConfig()->TargetConfig.MinNumTargetsToActivateAtOnce,
		GetBSConfig()->TargetConfig.MaxNumTargetsToActivateAtOnce);

	TArray<USpawnArea*> SpawnAreas = SpawnAreaManager->GetActivatedSpawnAreas().Array();

	TArray<int32> Indices;
	for (int i = 0; i < SpawnAreas.Num(); i++)
	{
		Indices.Add(i);
	}
	
	const int32 MinClampValue = MinToActivate == 0 ? 0 : 1;
	MinToActivate = FMath::Clamp(MinToActivate, MinClampValue, SpawnAreas.Num());
	MaxToActivate = FMath::Clamp(MaxToActivate, 1, SpawnAreas.Num());

	TArray<int32> ChosenIndices;
	const int32 NumToActivate = FMath::RandRange(MinToActivate, MaxToActivate);
	for (int i = 0; i < NumToActivate; i++)
	{
		if (Indices.Num() - 1 < 0) break;
		int32 RandomIndex = FMath::RandRange(MinToActivate, Indices.Num() - 1);
		ChosenIndices.Add(RandomIndex);
		Indices.Remove(RandomIndex);
	}
	
	for (const int32 Index : ChosenIndices)
	{
		if (SpawnAreas.IsValidIndex(Index))
		{
			ActivateTarget(ManagedTargets.FindRef(SpawnAreas[Index]->GetGuid()));
		}
	}
}

int32 ATargetManager::GetNumberOfRuntimeTargetsToSpawn() const
{
	const auto& Cfg = GetBSConfig()->TargetConfig;
	const int32 NumManaged = ManagedTargets.Num();
	
	// Batch spawning waits until there are no more Activated and Deactivated target(s)
	if (Cfg.bUseBatchSpawning)
	{
		if (NumManaged > 0) return 0;
		if (SpawnAreaManager->GetNumActivated() > 0) return 0;
		if (SpawnAreaManager->GetNumDeactivated() > 0) return 0;
	}

	// Set default value to number of runtime targets to spawn to NumRuntimeTargetsToSpawn
	int32 NumAllowedToSpawn = Cfg.NumRuntimeTargetsToSpawn == -1 ? 1 : Cfg.NumRuntimeTargetsToSpawn;

	// Return NumRuntimeTargetsToSpawn if no Max
	if (Cfg.MaxNumTargetsAtOnce == -1)
	{
		return NumAllowedToSpawn;
	}

	NumAllowedToSpawn = Cfg.MaxNumTargetsAtOnce - NumManaged;

	// Don't let NumAllowedToSpawn exceed NumRuntimeTargetsToSpawn
	if (NumAllowedToSpawn > Cfg.NumRuntimeTargetsToSpawn)
	{
		return Cfg.NumRuntimeTargetsToSpawn;
	}

	return NumAllowedToSpawn;
}

int32 ATargetManager::GetNumberOfTargetsToActivate(const int32 MaxAvailable, const int32 NumActivated) const
{
	const auto& Cfg = GetBSConfig()->TargetConfig;
	
	const int32 MaxAllowed = Cfg.MaxNumActivatedTargetsAtOnce >= 1
		? Cfg.MaxNumActivatedTargetsAtOnce
		: DefaultNumTargetsToActivate;

	// Constraints: Max Available & Max Allowed (both must be satisfied, so pick min)
	const int32 UpperLimit = FMath::Min(FMath::Max(0, MaxAllowed - NumActivated), MaxAvailable);
	
	if (UpperLimit <= 0) return 0;
	
	// Can activate at least 1 at this point
	int32 MinToActivate = FMath::Min(Cfg.MinNumTargetsToActivateAtOnce, Cfg.MaxNumTargetsToActivateAtOnce);
	int32 MaxToActivate = FMath::Max(Cfg.MinNumTargetsToActivateAtOnce, Cfg.MaxNumTargetsToActivateAtOnce);

	// Allow for a minimum of 0, but default to 1 unless explicitly chosen
	int32 MinToActivate_MinClamp = MinToActivate == 0
		? MinToActivate_MinClamp = MinToActivate
		: MinToActivate_MinClamp = DefaultMinToActivate_MinClamp;
	
	MinToActivate = FMath::Clamp(MinToActivate, MinToActivate_MinClamp, UpperLimit);
	MaxToActivate = FMath::Clamp(MaxToActivate, MaxToActivate_MinClamp, UpperLimit);

	return RandomNumToActivateStream.RandRange(MinToActivate, MaxToActivate);
}

FVector ATargetManager::FindNextSpawnedTargetScale() const
{
	const auto& Cfg = GetBSConfig()->TargetConfig;
	
	if (Cfg.ConsecutiveTargetScalePolicy == EConsecutiveTargetScalePolicy::SkillBased)
	{
		const float NewFactor = GetCurveTableValue(false, DynamicLookUpValue_TargetScale);
		return FVector(UKismetMathLibrary::Lerp(Cfg.MaxSpawnedTargetScale, Cfg.MinSpawnedTargetScale, NewFactor));
	}
	return FVector(FMath::FRandRange(Cfg.MinSpawnedTargetScale, Cfg.MaxSpawnedTargetScale));
}

TSet<USpawnArea*> ATargetManager::FindNextSpawnAreasForSpawn(const int32 NumToSpawn) const
{
	if (NumToSpawn == 0) return {};
	
	// Change the BoxExtent of the SpawnBox if dynamic
	if (GetBSConfig()->TargetConfig.BoundsScalingPolicy == EBoundsScalingPolicy::Dynamic)
	{
		const float Factor = GetCurveTableValue(true, DynamicLookUpValue_SpawnAreaScale);
		UpdateSpawnBoxExtents(Factor);
		UpdateSpawnVolume(Factor);
	}
	
	TArray<FVector> Scales;
	Scales.Reserve(NumToSpawn);
	for (int i = 0; i < NumToSpawn; i++)
	{
		Scales.Add(FindNextSpawnedTargetScale());
	}

	#if !UE_BUILD_SHIPPING
	const double StartTime = FPlatformTime::Seconds();
	#endif
	
	TSet<USpawnArea*> Out = SpawnAreaManager->GetSpawnableSpawnAreas(Scales, NumToSpawn);
	
	#if !UE_BUILD_SHIPPING
	const double EndTime = FPlatformTime::Seconds();
	const double ElapsedTime = EndTime - StartTime;
	
	if (Out.IsEmpty() && !GIsAutomationTesting)
	{
		UE_LOG(LogTargetManager, Display, TEXT("ValidSpawnableSpawnAreas is empty."));
	}
	if (SpawnableSpawnAreasExecutionTimeDelegate.IsBound())
	{
		SpawnableSpawnAreasExecutionTimeDelegate.Execute(ElapsedTime);
	}
	#endif
	
	return Out;
}

ETargetDamageType ATargetManager::FindNextTargetDamageType()
{
	if (GetBSConfig()->TargetConfig.TargetDamageType == ETargetDamageType::Combined)
	{
		LastTargetDamageType = (LastTargetDamageType == ETargetDamageType::Hit)
			? ETargetDamageType::Tracking
			: ETargetDamageType::Hit;
		return LastTargetDamageType;
	}
	return GetBSConfig()->TargetConfig.TargetDamageType;
}

/* ---------------------------------- */
/* -- Deactivation and Destruction -- */
/* ---------------------------------- */

void ATargetManager::OnTargetDamageEvent(FTargetDamageEvent& Event)
{
	// Set TargetManagerData
	Event.SetTargetManagerData(
		ShouldDeactivateTarget(Event.bDamagedSelf, Event.CurrentHealth, Event.CurrentDeactivationHealthThreshold),
		ShouldDestroyTarget(Event.bDamagedSelf, Event.bOutOfHealth),
		CurrentStreak,
		TotalPossibleDamage);

	// Can deactivate immediately
	if (Event.bWillDeactivate)
	{
		DeactivateTarget(Event.Target, Event.bDamagedSelf, Event.bOutOfHealth);
	}
	
	// Update CurrentStreak and look up values before setting TargetManager data
	UpdateCurrentStreak(Event);
	UpdateDynamicLookUpValues(Event);
	
	// Broadcast event to game mode
	PostTargetDamageEvent.Broadcast(Event);

	// Pass event to Spawn Area Manager
	SpawnAreaManager->HandleTargetDamageEvent(Event);
	
	// Update RLC if settings permit
	if (RLComponent->GetReinforcementLearningMode() != EReinforcementLearningMode::None)
	{
		if (const USpawnArea* Found = SpawnAreaManager->GetSpawnArea(Event.Guid))
		{
			RLComponent->SetActiveTargetPairReward(Found->GetIndex(), !Event.bDamagedSelf);
		}
	}

	// Handle destruction after everything else is handled
	if (Event.bWillDestroy)
	{
		RemoveFromManagedTargets(Event.Guid);
		Event.Target->Destroy();
	}
	
	// TODO Immediately spawn targets if ...?
}

void ATargetManager::UpdateCurrentStreak(const FTargetDamageEvent& Event)
{
	if (Event.DamageType == ETargetDamageType::Self)
	{
		CurrentStreak = 0;
	}
	else if (Event.DamageType == ETargetDamageType::Hit)
	{
		CurrentStreak++;
	}
}

void ATargetManager::UpdateDynamicLookUpValues(const FTargetDamageEvent& Event)
{
	if (Event.DamageType == ETargetDamageType::Self)
	{
		DynamicLookUpValue_TargetScale = FMath::Clamp(
			DynamicLookUpValue_TargetScale - GetBSConfig()->DynamicTargetScaling.DecrementAmount, 0,
			GetBSConfig()->DynamicTargetScaling.EndThreshold);
		DynamicLookUpValue_SpawnAreaScale = FMath::Clamp(
			DynamicLookUpValue_SpawnAreaScale - GetBSConfig()->DynamicSpawnAreaScaling.DecrementAmount, 0,
			GetBSConfig()->DynamicSpawnAreaScaling.EndThreshold);
	}
	else if (Event.DamageType == ETargetDamageType::Hit)
	{
		DynamicLookUpValue_TargetScale = FMath::Clamp(DynamicLookUpValue_TargetScale + 1, 0,
			GetBSConfig()->DynamicTargetScaling.EndThreshold);
		DynamicLookUpValue_SpawnAreaScale = FMath::Clamp(DynamicLookUpValue_SpawnAreaScale + 1, 0,
			GetBSConfig()->DynamicSpawnAreaScaling.EndThreshold);
	}
}

void ATargetManager::RemoveFromManagedTargets(const FGuid GuidToRemove)
{
	ManagedTargets.Remove(GuidToRemove);
}

/* -------------------------------------------------- */
/* -- SpawnBox and SpawnVolume - Static Generators -- */
/* -------------------------------------------------- */

FVector ATargetManager::GenerateStaticLocation(const FBSConfig* InCfg)
{
	const FBS_TargetConfig& Cfg = InCfg->TargetConfig;
	FVector SpawnBoxCenter = DefaultTargetManagerLocation;
	const bool bDynamic = Cfg.BoundsScalingPolicy == EBoundsScalingPolicy::Dynamic;

	switch (InCfg->TargetConfig.TargetDistributionPolicy)
	{
	case ETargetDistributionPolicy::HeadshotHeightOnly:
	case ETargetDistributionPolicy::None:
	case ETargetDistributionPolicy::EdgeOnly:
	case ETargetDistributionPolicy::FullRange:
		{
			const float HalfHeight = bDynamic
				? FMath::Max(Cfg.BoxBounds.Z, InCfg->DynamicSpawnAreaScaling.StartBounds.Z) / 2.f
				: Cfg.BoxBounds.Z / 2.f;
			SpawnBoxCenter.Z = HalfHeight + Cfg.FloorDistance;
		}
		break;
	case ETargetDistributionPolicy::Grid:
		{
			const float MaxDiameter = GetMaxTargetDiameter(Cfg);
			const float VSpacing = InCfg->GridConfig.GridSpacing.Y * (InCfg->GridConfig.NumVerticalGridTargets - 1);
			const float VTargetWidth = (InCfg->GridConfig.NumVerticalGridTargets - 1) * MaxDiameter;
			const float HalfHeight = (VSpacing + VTargetWidth) * 0.5f;
			SpawnBoxCenter.Z = HalfHeight + Cfg.FloorDistance;
		}
		break;
	}
	return SpawnBoxCenter;
}

FVector ATargetManager::GenerateStaticExtents(const FBSConfig* InCfg)
{
	FVector Out = InCfg->TargetConfig.BoxBounds * 0.5f;

	switch (InCfg->TargetConfig.TargetDistributionPolicy)
	{
	case ETargetDistributionPolicy::HeadshotHeightOnly:
	case ETargetDistributionPolicy::None:
	case ETargetDistributionPolicy::EdgeOnly:
	case ETargetDistributionPolicy::FullRange:
		break;
	case ETargetDistributionPolicy::Grid:
		{
			const FBS_GridConfig& GridCfg = InCfg->GridConfig;
			const float MaxTargetDiameter = GetMaxTargetDiameter(InCfg->TargetConfig);

			const float HSpacing = GridCfg.GridSpacing.X * (GridCfg.NumHorizontalGridTargets - 1);
			const float VSpacing = GridCfg.GridSpacing.Y * (GridCfg.NumVerticalGridTargets - 1);

			const float HTargetWidth = (GridCfg.NumHorizontalGridTargets - 1) * MaxTargetDiameter;
			const float VTargetWidth = (GridCfg.NumVerticalGridTargets - 1) * MaxTargetDiameter;

			const float HalfWidth = (HSpacing + HTargetWidth) * 0.5f;
			const float HalfHeight = (VSpacing + VTargetWidth) * 0.5f;

			Out = FVector(Out.X, HalfWidth, HalfHeight);
		}
		break;
	}

	if (InCfg->TargetConfig.BoundsScalingPolicy == EBoundsScalingPolicy::Dynamic)
	{
		// If user made StartExtents > BoxBounds
		const FVector Start = InCfg->DynamicSpawnAreaScaling.GetStartExtents();

		const float MaxX = FMath::Max(Start.X, Out.X);
		const float MaxY = FMath::Max(Start.Y, Out.Y);
		const float MaxZ = FMath::Max(Start.Z, Out.Z);

		Out = FVector(MaxX, MaxY, MaxZ);
	}

	return Out;
}

FExtrema ATargetManager::GenerateStaticExtrema(const FBSConfig* InCfg, const FVector& InOrigin,
	const FVector& InStaticExtents)
{
	const float MaxTargetDiameter = GetMaxTargetDiameter(InCfg->TargetConfig);

	const float MinX = InOrigin.X - 2 * InStaticExtents.X - MaxTargetDiameter;
	const float MaxX = InOrigin.X + MaxTargetDiameter;
	float MinY, MaxY, MinZ, MaxZ;
	
	if (InCfg->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid)
	{
		const FBS_GridConfig& GridCfg = InCfg->GridConfig;
		const float HSpacing = GridCfg.GridSpacing.X * (GridCfg.NumHorizontalGridTargets - 1);
		const float VSpacing = GridCfg.GridSpacing.Y * (GridCfg.NumVerticalGridTargets - 1);

		const float HTargetWidth = (GridCfg.NumHorizontalGridTargets - 1) * MaxTargetDiameter;
		const float VTargetWidth = (GridCfg.NumVerticalGridTargets - 1) * MaxTargetDiameter;

		const float HalfWidth = (HSpacing + HTargetWidth) * 0.5f;
		const float HalfHeight = (VSpacing + VTargetWidth) * 0.5f;

		MinY = InOrigin.Y - HalfWidth;
		MaxY = InOrigin.Y + HalfWidth;
		MinZ = InOrigin.Z - HalfHeight;
		MaxZ = InOrigin.Z + HalfHeight;
	}
	else
	{
		MinY = InOrigin.Y - InStaticExtents.Y;
		MaxY = InOrigin.Y + InStaticExtents.Y;
		MinZ = InOrigin.Z - InStaticExtents.Z;
		MaxZ = InOrigin.Z + InStaticExtents.Z;
	}
	
	return FExtrema(FVector(MinX, MinY, MinZ), FVector(MaxX, MaxY, MaxZ));
}

FVector ATargetManager::GenerateSpawnVolumeLocation(const FVector& InOrigin, const FVector& InDynamicStartExtents,
	const FVector& InStaticExtents, const float Factor)
{
	// Y and Z will be the same as SpawnBox
	FVector Out = InOrigin;
	
	const float LerpX = UKismetMathLibrary::Lerp(InDynamicStartExtents.X, InStaticExtents.X, Factor);
	Out.X = Out.X - LerpX;

	return Out;
}

FVector ATargetManager::GenerateSpawnVolumeExtents(const FBSConfig* InCfg, const FVector& InSpawnBoxExtents,
	const FVector& InStaticExtents, const float Factor)
{
	FVector Out = InSpawnBoxExtents;

	const FVector Start = InCfg->DynamicSpawnAreaScaling.GetStartExtents();
	const float LerpX = UKismetMathLibrary::Lerp(Start.X, InStaticExtents.X, Factor);
	const float MaxRadius = GetMaxTargetRadius(InCfg->TargetConfig);
	
	// Account for target scale, use radius since this is half-size
	Out.X = LerpX + MaxRadius + DirBoxPadding;
	Out.Y += MaxRadius + 2.f;
	Out.Z += MaxRadius + 2.f;

	return Out;
}

FExtrema ATargetManager::GenerateMaxSpawnVolumeExtrema(const FBSConfig* InCfg, const FVector& InOrigin,
	const FVector& InStaticExtents)
{
	FVector AbsMaxOrigin = InOrigin;
	FVector AbsMaxExtents = InStaticExtents;
	
	const FVector DynamicStart = InCfg->DynamicSpawnAreaScaling.GetStartExtents();
	if (InCfg->TargetConfig.BoundsScalingPolicy == EBoundsScalingPolicy::Dynamic)
	{
		AbsMaxExtents.X = FMath::Max(DynamicStart.X, InStaticExtents.X);
		AbsMaxExtents.Y = FMath::Max(DynamicStart.Y, InStaticExtents.Y);
		AbsMaxExtents.Z = FMath::Max(DynamicStart.Z, InStaticExtents.Z);
	}
	
	const float MaxRadius = GetMaxTargetRadius(InCfg->TargetConfig);

	// Account for target scale, use radius since this is half-size
	AbsMaxExtents.X += MaxRadius + DirBoxPadding;
	AbsMaxExtents.Y += MaxRadius + 2.f;
	AbsMaxExtents.Z += MaxRadius + 2.f;

	// Adjust Max Origin Location (X only)
	AbsMaxOrigin.X = AbsMaxOrigin.X - AbsMaxExtents.X;

	const float MinX = AbsMaxOrigin.X - AbsMaxExtents.X;
	const float MaxX = AbsMaxOrigin.X + AbsMaxExtents.X;
	const float MinY = AbsMaxOrigin.Y - AbsMaxExtents.Y;
	const float MaxY = AbsMaxOrigin.Y + AbsMaxExtents.Y;
	const float MinZ = AbsMaxOrigin.Z - AbsMaxExtents.Z;
	const float MaxZ = AbsMaxOrigin.Z + AbsMaxExtents.Z;
	
	return FExtrema(FVector(MinX, MinY, MinZ), FVector(MaxX, MaxY, MaxZ));
}

/* ------------------------------------------------- */
/* -- SpawnBox and SpawnVolume - Getters/Updaters -- */
/* ------------------------------------------------- */

FVector ATargetManager::GetSpawnBoxOrigin() const
{
	return SpawnBox->Bounds.Origin;
}

FVector ATargetManager::GetSpawnBoxExtents() const
{
	return SpawnBox->Bounds.BoxExtent;
}

FExtrema ATargetManager::GetSpawnBoxExtrema() const
{
	return FExtrema(SpawnBox->Bounds.GetBoxExtrema(0), SpawnBox->Bounds.GetBoxExtrema(1));
}

FVector ATargetManager::GetSpawnVolumeLocation() const
{
	return SpawnVolume->Bounds.Origin;
}

FVector ATargetManager::GetSpawnVolumeExtents() const
{
	return SpawnVolume->Bounds.BoxExtent;
}

FExtrema ATargetManager::GetSpawnVolumeExtrema() const
{
	return FExtrema(SpawnVolume->Bounds.GetBoxExtrema(0), SpawnVolume->Bounds.GetBoxExtrema(1));
}

void ATargetManager::UpdateSpawnBoxExtents(const float Factor) const
{
	const FBS_TargetConfig& Cfg = GetBSConfig()->TargetConfig;
	const FVector Start = GetBSConfig()->DynamicSpawnAreaScaling.GetStartExtents();

	const FVector LastExtents = GetSpawnBoxExtents();

	const float LerpY = UKismetMathLibrary::Lerp(Start.Y, StaticExtents.Y, Factor);
	const float LerpZ = UKismetMathLibrary::Lerp(Start.Z, StaticExtents.Z, Factor);
	
	const int32 IncY = SpawnAreaManager->GetSpawnAreaInc().Y;
	const int32 IncZ = SpawnAreaManager->GetSpawnAreaInc().Z;
	
	const int32 SnapY = FMath::Max(IncY * FMath::FloorToInt(LerpY / IncY), MinValue_HorizontalSpread);
	const int32 SnapZ = Cfg.TargetDistributionPolicy == ETargetDistributionPolicy::HeadshotHeightOnly
	    ? HeadshotHeight_VerticalSpread * 0.5f
	    : FMath::Max(IncZ * FMath::FloorToInt(LerpZ / IncZ), MinValue_VerticalSpread);
	
	const bool bGrid = Cfg.TargetDistributionPolicy == ETargetDistributionPolicy::Grid;
	const FVector NewExtents = FVector(0, bGrid ? LerpY : SnapY, bGrid ? LerpZ : SnapZ);

	if (LastExtents == NewExtents) return;
	
	SpawnBox->SetBoxExtent(NewExtents);
	SpawnAreaManager->OnExtremaChanged(GetSpawnBoxExtrema());
}

void ATargetManager::UpdateSpawnVolume(const float Factor) const
{
	const FVector Origin = GetSpawnBoxOrigin();
	const FVector VolumeLocation = GenerateSpawnVolumeLocation(Origin, GetSpawnBoxExtents(), StaticExtents, Factor);
	const FVector VolumeExtents = GenerateSpawnVolumeExtents(GetBSConfig().Get(), GetSpawnBoxExtents(), StaticExtents, Factor);

	SpawnVolume->SetRelativeLocation(VolumeLocation);
	SpawnVolume->SetBoxExtent(VolumeExtents);

	TopBox->SetRelativeLocation(FVector(VolumeLocation.X, Origin.Y, Origin.Z + VolumeExtents.Z));
	BottomBox->SetRelativeLocation(FVector(VolumeLocation.X, Origin.Y, Origin.Z - VolumeExtents.Z));
	LeftBox->SetRelativeLocation(FVector(VolumeLocation.X, Origin.Y - VolumeExtents.Y, Origin.Z));
	RightBox->SetRelativeLocation(FVector(VolumeLocation.X, Origin.Y + VolumeExtents.Y, Origin.Z));
	ForwardBox->SetRelativeLocation(FVector(VolumeLocation.X + VolumeExtents.X, Origin.Y, Origin.Z));
	BackwardBox->SetRelativeLocation(FVector(VolumeLocation.X - VolumeExtents.X, Origin.Y, Origin.Z));

	TopBox->SetBoxExtent(FVector(VolumeExtents.X, VolumeExtents.Y, 0));
	BottomBox->SetBoxExtent(FVector(VolumeExtents.X, VolumeExtents.Y, 0));
	LeftBox->SetBoxExtent(FVector(VolumeExtents.X, 0, VolumeExtents.Z));
	RightBox->SetBoxExtent(FVector(VolumeExtents.X, 0, VolumeExtents.Z));
	ForwardBox->SetBoxExtent(FVector(0, VolumeExtents.Y, VolumeExtents.Z));
	BackwardBox->SetBoxExtent(FVector(0, VolumeExtents.Y, VolumeExtents.Z));
}

/* ------------------------------ */
/* -- Misc or Helper functions -- */
/* ------------------------------ */

void ATargetManager::ChangeTargetDirection(ATarget* InTarget, const uint8 InSpawnActivationDeactivation) const
{
	// Alternate directions of spawned targets, while also alternating directions of individual target direction changes
	bool bLastDirectionChangeHorizontal;

	if (InSpawnActivationDeactivation == 0)
	{
		bLastDirectionChangeHorizontal = bLastSpawnedTargetDirectionChangeHorizontal;
		bLastSpawnedTargetDirectionChangeHorizontal = !bLastDirectionChangeHorizontal;
	}
	else if (InSpawnActivationDeactivation == 1)
	{
		bLastDirectionChangeHorizontal = bLastActivatedTargetDirectionChangeHorizontal;
		bLastActivatedTargetDirectionChangeHorizontal = !bLastDirectionChangeHorizontal;
	}
	else
	{
		bLastDirectionChangeHorizontal = InTarget->GetLastDirectionChangeHorizontal();
		InTarget->SetLastDirectionChangeHorizontal(!bLastDirectionChangeHorizontal);
	}

	InTarget->SetTargetDirection(GetNewTargetDirection(InTarget->GetActorLocation(), bLastDirectionChangeHorizontal));
}

FVector ATargetManager::GetNewTargetDirection(const FVector& LocationBeforeChange,
	const bool bLastDirectionChangeHorizontal) const
{
	switch (GetBSConfig()->TargetConfig.MovingTargetDirectionMode)
	{
	case EMovingTargetDirectionMode::HorizontalOnly:
		{
			return FMath::RandBool() ? FVector(0, 1, 0) : FVector(0, -1, 0);
		}
	case EMovingTargetDirectionMode::VerticalOnly:
		{
			return FMath::RandBool() ? FVector(0, 0, 1) : FVector(0, 0, -1);
		}
	case EMovingTargetDirectionMode::AlternateHorizontalVertical:
		{
			if (bLastDirectionChangeHorizontal)
			{
				return FMath::RandBool() ? FVector(0, 0, 1) : FVector(0, 0, -1);
			}
			return FMath::RandBool() ? FVector(0, 1, 0) : FVector(0, -1, 0);
		}
	case EMovingTargetDirectionMode::Any:
		{
			FVector Origin = GetSpawnBoxOrigin();
			const FVector SBExtents = GetSpawnBoxExtents();

			// Randomize the X location
			Origin.X = FMath::FRandRange(Origin.X, Origin.X - GetBSConfig()->TargetConfig.BoxBounds.X);

			// 1/4 of the total Spawn Volume
			const FVector Extent = FVector(SBExtents.X * 0.5f, SBExtents.Y * 0.5f, SBExtents.Z * 0.5f);
			const FVector Offset = FVector(0, Extent.Y, Extent.Z);

			const FVector BotLeft = RandBoxPoint(Origin + FVector(0, -1, -1) * Offset, Extent);
			const FVector BotRight = RandBoxPoint(Origin + FVector(0, 1, -1) * Offset, Extent);
			const FVector TopLeft = RandBoxPoint(Origin + FVector(0, -1, 1) * Offset, Extent);
			const FVector TopRight = RandBoxPoint(Origin + FVector(0, 1, 1) * Offset, Extent);
			DrawDebugBox(GetWorld(), Origin + FVector(0, -1, -1) * Offset, Extent, FColor::Green, false, 1.f);
			DrawDebugBox(GetWorld(), Origin + FVector(0, 1, -1) * Offset, Extent, FColor::Green, false, 1.f);
			DrawDebugBox(GetWorld(), Origin + FVector(0, -1, 1) * Offset, Extent, FColor::Green, false, 1.f);
			DrawDebugBox(GetWorld(), Origin + FVector(0, 1, 1) * Offset, Extent, FColor::Green, false, 1.f);
			TArray PossibleLocations = {BotLeft, BotRight, TopLeft, TopRight};

			if (LocationBeforeChange.Y < 0)
			{
				PossibleLocations.Remove(LocationBeforeChange.Z < Origin.Z ? BotLeft : TopLeft);
			}
			else
			{
				PossibleLocations.Remove(LocationBeforeChange.Z < Origin.Z ? BotRight : TopRight);
			}
			const FVector NewLocation = PossibleLocations[UKismetMathLibrary::RandomIntegerInRange(0, 2)];
			DrawDebugPoint(GetWorld(), NewLocation, 4.f, FColor::Red, false, 2.f);
			return UKismetMathLibrary::GetDirectionUnitVector(LocationBeforeChange, NewLocation);
		}
	case EMovingTargetDirectionMode::ForwardOnly:
		{
			return FVector(-1.f, 0.f, 0.f);
		}
	case EMovingTargetDirectionMode::None:
		break;
	}
	return FVector::ZeroVector;
}

void ATargetManager::UpdateTotalPossibleDamage()
{
	TotalPossibleDamage++;
	for (const ATarget* Target : GetManagedTargets())
	{
		if (Target) SpawnAreaManager->UpdateTotalTrackingDamagePossible(Target->GetActorLocation());
	}
}

bool ATargetManager::TrackingTargetIsDamageable() const
{
	if (!GetBSConfig()) return false;
	if (GetBSConfig()->TargetConfig.TargetDamageType == ETargetDamageType::Hit || ManagedTargets.IsEmpty())
	{
		return false;
	}
	for (const ATarget* Target : GetManagedTargets())
	{
		if (Target && !Target->IsImmuneToTrackingDamage()) return true;
	}
	return false;
}

TArray<ATarget*> ATargetManager::GetManagedTargets() const
{
	TArray<ATarget*> Out;
	ManagedTargets.GenerateValueArray(Out);
	return Out;
}

float ATargetManager::GetCurveTableValue(const bool bIsSpawnArea, const int32 InTime) const
{
	UCompositeCurveTable* Table = bIsSpawnArea ? CCT_SpawnArea : CCT_TargetScale;
	const bool bThresholdMet = bIsSpawnArea
		? InTime > GetBSConfig()->DynamicSpawnAreaScaling.StartThreshold
		: InTime > GetBSConfig()->DynamicTargetScaling.StartThreshold;
	const bool bIsCubicInterpolation = bIsSpawnArea
		? GetBSConfig()->DynamicSpawnAreaScaling.bIsCubicInterpolation
		: GetBSConfig()->DynamicTargetScaling.bIsCubicInterpolation;

	float OutXY = -1.f;
	TEnumAsByte<EEvaluateCurveTableResult::Type> OutResult;

	FName Name = bIsCubicInterpolation ? FName("Cubic_ThresholdMet") : FName("Linear_ThresholdMet");
	Name = bThresholdMet ? Name : FName("Linear_PreThreshold");
	UDataTableFunctionLibrary::EvaluateCurveTableRow(Table, Name, InTime, OutResult, OutXY, "");

	return OutXY;
}

float ATargetManager::GetMaxTargetRadius(const FBS_TargetConfig& InTargetCfg)
{
	const float MaxScale = FMath::Max(InTargetCfg.MinSpawnedTargetScale, InTargetCfg.MaxSpawnedTargetScale);
	return MaxScale * SphereTargetRadius;
}

float ATargetManager::GetMaxTargetDiameter(const FBS_TargetConfig& InTargetCfg)
{
	const float MaxScale = FMath::Max(InTargetCfg.MinSpawnedTargetScale, InTargetCfg.MaxSpawnedTargetScale);
	return MaxScale * SphereTargetDiameter;
}

void ATargetManager::UpdatePlayerSettings(const FPlayerSettings_Game& InPlayerSettings)
{
	if (!ManagedTargets.IsEmpty())
	{
		for (const auto TargetPair : ManagedTargets)
		{
			TargetPair.Value->UpdatePlayerSettings(InPlayerSettings);
		}
	}
}

/* --------------------- */
/* -- CommonScoreInfo -- */
/* --------------------- */

FAccuracyData ATargetManager::GetLocationAccuracy() const
{
	return SpawnAreaManager->GetLocationAccuracy();
}

void ATargetManager::UpdateCommonScoreInfoQTable(FCommonScoreInfo& InCommonScoreInfo) const
{
	if (RLComponent->GetReinforcementLearningMode() != EReinforcementLearningMode::None)
	{
		RLComponent->ClearCachedTargetPairs();
		InCommonScoreInfo.UpdateQTable(RLComponent->GetTArray_FromNdArray_QTable(), RLComponent->GetNumQTableRows(),
			RLComponent->GetNumQTableColumns(), RLComponent->GetTArray_FromNdArray_TrainingSamples(),
			RLComponent->GetTotalTrainingSamples());
	}
}