// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "Target/TargetManager.h"
#include "BSConstants.h"
#include "BSGameMode.h"
#include "Components/BoxComponent.h"
#include "Engine/CompositeCurveTable.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Target/ReinforcementLearningComponent.h"
#include "Target/SpawnAreaManagerComponent.h"
#include "Target/Target.h"
#include "Utilities/BSCommon.h"

FVector (&RandBoxPoint)(const FVector Center, const FVector Extents) = UKismetMathLibrary::RandomPointInBoundingBox;
DEFINE_LOG_CATEGORY(LogTargetManager);

using namespace Constants;
using namespace BSCommon;

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
	SpawnAreaDimensions = FIntVector3();
	CurrentStreak = 0;
	DynamicLookUpValue_TargetScale = 0;
	DynamicLookUpValue_SpawnAreaScale = 0;
	ManagedTargets = TMap<FGuid, ATarget*>();
	TotalPossibleDamage = 0.f;
	bLastSpawnedTargetDirectionHorizontal = false;
	bLastActivatedTargetDirectionHorizontal = false;

#if !UE_BUILD_SHIPPING
	bShowDebug_AnyMovingTargetDirectionMode = false;
#endif
}

void ATargetManager::BeginPlay()
{
	Super::BeginPlay();

	TargetSpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	TargetSpawnInfo.Owner = this;
	TargetSpawnInfo.CustomPreSpawnInitalization = [this](AActor* SpawnedActor)
	{
		if (ATarget* Target = Cast<ATarget>(SpawnedActor))
		{
			Target->Init(BSConfig->TargetConfig);
		}
	};
}

void ATargetManager::Destroyed()
{
	SetShouldSpawn(false);
	DestroyTargets();
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

void ATargetManager::Init(const TSharedPtr<FBSConfig>& InConfig, const FCommonScoreInfo& InCommonScoreInfo,
	const FPlayerSettings_Game& InPlayerSettings)
{
	Clear();
	BSConfig = InConfig;

	// Initialize target colors
	BSConfig->InitColors(InPlayerSettings.bUseSeparateOutlineColor, InPlayerSettings.InactiveTargetColor,
		InPlayerSettings.TargetOutlineColor, InPlayerSettings.StartTargetColor, InPlayerSettings.PeakTargetColor,
		InPlayerSettings.EndTargetColor, InPlayerSettings.TakingTrackingDamageColor,
		InPlayerSettings.NotTakingTrackingDamageColor);

	// Set SpawnBox location, BoxExtent, StaticExtents, and StaticExtrema
	SpawnBox->SetRelativeLocation(GenerateStaticLocation(BSConfig.Get()));
	StaticExtents = GenerateStaticExtents(BSConfig.Get());
	SpawnBox->SetBoxExtent(StaticExtents);
	StaticExtentsBox->SetBoxExtent(StaticExtents);
	StaticExtrema = GenerateStaticExtrema(BSConfig.Get(), GetSpawnBoxOrigin(), StaticExtents);

	// Initialize the CompositeCurveTables in case they need to be modified
	Init_Tables();

	// Initialize the SpawnAreaManager
	SpawnAreaDimensions = SpawnAreaManager->Init(BSConfig, GetSpawnBoxOrigin(), StaticExtents, StaticExtrema);

	// Initialize SpawnBox extents and the SpawnVolume extents & location
	const bool bDynamic = BSConfig->TargetConfig.BoundsScalingPolicy == EBoundsScalingPolicy::Dynamic;
	const float Factor = bDynamic ? GetCurveTableValue(true, DynamicLookUpValue_SpawnAreaScale) : 1.f;
	UpdateSpawnBoxExtents(Factor);
	UpdateSpawnVolume(Factor);
	SpawnAreaManager->OnExtremaChanged(GetSpawnBoxExtrema());

	if (BSConfig->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::HorizontalOnly || BSConfig->
		TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::VerticalOnly)
	{
		SpawnAreaManager->GetRequestMovingTargetLocationsDelegate().BindUObject(this,
			&ATargetManager::GetMovingTargetLocations);
	}

	// Init RLC
	if (BSConfig->IsCompatibleWithReinforcementLearning())
	{
		const FRLAgentParams Params(BSConfig->AIConfig, InCommonScoreInfo, SpawnAreaManager->GetSpawnAreaSize());
		RLComponent->Init(Params);

		// Bind the SpawnAreaManager to the RLComponent if it should request activation locations
		const EReinforcementLearningMode Mode = RLComponent->GetRLMode();
		if (Or(Mode, EReinforcementLearningMode::Exploration, EReinforcementLearningMode::ActiveAgent))
		{
			SpawnAreaManager->GetSpawnAreaRequestDelegate().BindUObject(RLComponent,
				&UReinforcementLearningComponent::ChooseNextActionIndex);
		}

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
			USaveGamePlayerScore::PrintQTable(BSConfig->DefiningConfig, InCommonScoreInfo, Options);
		}
#endif
	}

	RandomNumToActivateStream.Initialize(FMath::Rand());

	// Spawn any targets if needed
	if (BSConfig->TargetConfig.TargetSpawningPolicy == ETargetSpawningPolicy::UpfrontOnly)
	{
		HandleUpfrontSpawning();
	}
}

void ATargetManager::Init_Tables() const
{
	FRealCurve* PreThresholdCurve = CCT_SpawnArea->GetCurves()[0].CurveToEdit;
	int32 CurveIndex = BSConfig->DynamicSpawnAreaScaling.bIsCubicInterpolation ? 2 : 1;
	FRealCurve* ThresholdMetCurve = CCT_SpawnArea->GetCurves()[CurveIndex].CurveToEdit;

	PreThresholdCurve->SetKeyTime(PreThresholdCurve->GetFirstKeyHandle(), 0);
	PreThresholdCurve->SetKeyTime(PreThresholdCurve->GetLastKeyHandle(),
		BSConfig->DynamicSpawnAreaScaling.StartThreshold);
	ThresholdMetCurve->SetKeyTime(ThresholdMetCurve->GetFirstKeyHandle(),
		BSConfig->DynamicSpawnAreaScaling.StartThreshold);
	ThresholdMetCurve->SetKeyTime(ThresholdMetCurve->GetLastKeyHandle(),
		BSConfig->DynamicSpawnAreaScaling.EndThreshold);

	PreThresholdCurve = CCT_TargetScale->GetCurves()[0].CurveToEdit;
	CurveIndex = BSConfig->DynamicTargetScaling.bIsCubicInterpolation ? 2 : 1;
	ThresholdMetCurve = CCT_TargetScale->GetCurves()[CurveIndex].CurveToEdit;

	PreThresholdCurve->SetKeyTime(PreThresholdCurve->GetFirstKeyHandle(), 0);
	PreThresholdCurve->SetKeyTime(PreThresholdCurve->GetLastKeyHandle(), BSConfig->DynamicTargetScaling.StartThreshold);
	ThresholdMetCurve->SetKeyTime(ThresholdMetCurve->GetFirstKeyHandle(),
		BSConfig->DynamicTargetScaling.StartThreshold);
	ThresholdMetCurve->SetKeyTime(ThresholdMetCurve->GetLastKeyHandle(), BSConfig->DynamicTargetScaling.EndThreshold);
}

void ATargetManager::Clear()
{
	ShouldSpawn = false;
	DestroyTargets();
	CurrentStreak = 0;
	BSConfig.Reset();
	BSConfig = nullptr;
	LastTargetDamageType = ETargetDamageType::Tracking;
	CurrentTargetScale = FVector(1.f);
	StaticExtrema = FExtrema();
	StaticExtents = FVector();
	SpawnAreaDimensions = FIntVector3();
	CurrentStreak = 0;
	DynamicLookUpValue_TargetScale = 0;
	DynamicLookUpValue_SpawnAreaScale = 0;
	TotalPossibleDamage = 0.f;
	bLastSpawnedTargetDirectionHorizontal = false;
	bLastActivatedTargetDirectionHorizontal = false;

	RLComponent->Clear();
	SpawnAreaManager->Clear();
}

void ATargetManager::SetShouldSpawn(const bool bShouldSpawn)
{
	if (bShouldSpawn)
	{
		if (BSConfig->TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::HideTarget))
		{
			for (const TTuple<FGuid, ATarget*>& Pair : ManagedTargets)
			{
				Pair.Value->SetActorHiddenInGame(true);
			}
		}
	}
	ShouldSpawn = bShouldSpawn;
}

void ATargetManager::OnPlayerStopTrackingTarget()
{
	for (const TTuple<FGuid, ATarget*>& Pair : ManagedTargets)
	{
		if (Pair.Value && !Pair.Value->IsImmuneToTrackingDamage())
		{
			Pair.Value->SetTargetColor(BSConfig->TargetConfig.NotTakingTrackingDamageColor);
		}
	}
}

/* ------------------------------------ */
/* -- Target spawning and activation -- */
/* ------------------------------------ */

void ATargetManager::OnAudioAnalyzerBeat()
{
	if (!ShouldSpawn)
	{
		return;
	}

	const int32 LastSeed = RandomNumToActivateStream.GetCurrentSeed();
	HandleRuntimeSpawning();
	RandomNumToActivateStream.Initialize(LastSeed);
	HandleTargetActivation();

#if !UE_BUILD_SHIPPING
	DrawDebug();
#endif
}

ATarget* ATargetManager::SpawnTarget(const FTargetSpawnParams& Params)
{
	ATarget* Target = GetWorld()->SpawnActor<ATarget>(TargetToSpawn, Params.Transform(), TargetSpawnInfo);

#if !UE_BUILD_SHIPPING
	if (GIsAutomationTesting)
	{
		Target->DispatchBeginPlay();
	}
#endif

	Target->SetTargetDamageType(FindNextTargetDamageType());
	Target->OnTargetDamageEvent.AddUObject(this, &ATargetManager::OnTargetDamageEvent);
	AddToManagedTargets(Target, Params.SpawnAreaIndex);

	if (!Target)
	{
		return nullptr;
	}

	// Handle spawn responses
	if (BSConfig->TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::AddImmunity))
	{
		Target->ApplyImmunityEffect();
	}
	if (BSConfig->TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeVelocity))
	{
		const float SpawnVelocity = FMath::FRandRange(BSConfig->TargetConfig.MinSpawnedTargetSpeed,
			BSConfig->TargetConfig.MaxSpawnedTargetSpeed);
		Target->SetTargetSpeed(SpawnVelocity);

		// In case no direction change is provided
		if (!BSConfig->TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeDirection) && BSConfig->
			TargetConfig.MovingTargetDirectionMode != EMovingTargetDirectionMode::None)
		{
			ChangeTargetDirection(Target, 0);
		}
	}
	if (BSConfig->TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeDirection))
	{
		ChangeTargetDirection(Target, 0);
	}
	return Target;
}

void ATargetManager::AddToManagedTargets(ATarget* SpawnTarget, const int32 SpawnAreaIndex)
{
	ManagedTargets.Add(SpawnTarget->GetGuid(), SpawnTarget);
	SpawnAreaManager->FlagSpawnAreaAsManaged(SpawnAreaIndex, SpawnTarget->GetGuid());
}

bool ATargetManager::ActivateTarget(ATarget* InTarget) const
{
	if (!InTarget || SpawnAreaManager->GetSpawnAreaIndex(InTarget->GetGuid()) < 0)
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
		InTarget->SetTargetSpeed(FMath::FRandRange(BSConfig->TargetConfig.MinActivatedTargetSpeed,
			BSConfig->TargetConfig.MaxActivatedTargetSpeed));
		if (!Responses.Contains(ETargetActivationResponse::ChangeDirection) && BSConfig->TargetConfig.
			MovingTargetDirectionMode != EMovingTargetDirectionMode::None)
		{
			ChangeTargetDirection(InTarget, 1);
		}
	}
	if (Responses.Contains(ETargetActivationResponse::ChangeDirection))
	{
		ChangeTargetDirection(InTarget, 1);
	}

	// TODO: Overall weird and confusing setting, probably remove or refactor
	if (!bAlreadyActivated && InTarget->HasBeenActivatedBefore() && Responses.Contains(
		ETargetActivationResponse::ApplyConsecutiveTargetScale))
	{
		InTarget->SetTargetScale(FindNextSpawnedTargetScale());
	}

	const bool bActivated = InTarget->ActivateTarget(BSConfig->TargetConfig.TargetMaxLifeSpan);

	// Don't continue if failed to activate
	if (!bActivated)
	{
		return false;
	}

	// Cache the previous SpawnArea
	const int32 PreviousIndex = SpawnAreaManager->GetMostRecentSpawnAreaIndex();

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

	if (RLComponent->GetRLMode() != EReinforcementLearningMode::None && PreviousIndex >= 0)
	{
		const int32 CurrentIndex = SpawnAreaManager->GetSpawnAreaIndex(InTarget->GetGuid());
		if (CurrentIndex >= 0)
		{
			RLComponent->AddToActiveTargetPairs(PreviousIndex, CurrentIndex);
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

		if (!Responses.Contains(ETargetDeactivationResponse::ChangeDirection) && Config.MovingTargetDirectionMode !=
			EMovingTargetDirectionMode::None)
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
		const float SphereRadius = SphereTargetRadius * InTarget->GetActorScale().X;
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

int32 ATargetManager::HandleUpfrontSpawning()
{
	if (BSConfig->TargetConfig.TargetSpawningPolicy != ETargetSpawningPolicy::UpfrontOnly)
	{
		return 0;
	}

	int32 NumSpawned = 0;
	for (const FTargetSpawnParams& Params : GetTargetSpawnParams(GetNumberOfTargetsToSpawn()))
	{
		if (SpawnTarget(Params))
		{
			NumSpawned++;
		}
	}
	return NumSpawned;
}

int32 ATargetManager::HandleRuntimeSpawning()
{
	const auto& Cfg = BSConfig->TargetConfig;

	if (Cfg.TargetSpawningPolicy != ETargetSpawningPolicy::RuntimeOnly)
	{
		return 0;
	}

	int32 NumberToSpawn = GetNumberOfTargetsToSpawn();

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
	for (const FTargetSpawnParams& Params : GetTargetSpawnParams(NumberToSpawn))
	{
		if (SpawnTarget(Params))
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
	if (ManagedTargets.IsEmpty())
	{
		return 0;
	}

	const auto& Cfg = BSConfig->TargetConfig;

	// See if theres any Spawn Areas available to activate
	const int32 NumAvailableToActivate = SpawnAreaManager->GetNumDeactivated();
	const int32 NumCurrentlyActivated = SpawnAreaManager->GetNumActivated();

	// If not allowed to spawn without activation and runtime spawning,
	// HandleRuntimeSpawning spawned the number of targets that can be activated
	int32 NumToActivate = !Cfg.bAllowSpawnWithoutActivation && Cfg.TargetSpawningPolicy ==
		ETargetSpawningPolicy::RuntimeOnly
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
				UE_LOG(LogTemp, Display, TEXT("NumToActivateCheck < NumAvailableToActivate %d %d"), NumToActivateCheck,
					NumAvailableToActivate);
			}
			else
			{
				UE_LOG(LogTemp, Display, TEXT("NumToActivateCheck > NumAvailableToActivate %d %d"), NumToActivateCheck,
					NumAvailableToActivate);
			}
		}
	}
#endif

	// Scuffed temporary solution to make tracking more interesting
	if (NumAvailableToActivate == 0 && NumToActivate == 0 && Cfg.bAllowActivationWhileActivated)
	{
		NumToActivate = 1;
	}

	int32 NumActivated = 0;
	for (const FGuid& TargetGuid : SpawnAreaManager->GetActivatableTargets(NumToActivate))
	{
		if (const auto Target = ManagedTargets.Find(TargetGuid))
		{
			if (ActivateTarget(*Target))
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

int32 ATargetManager::GetNumberOfTargetsToSpawn() const
{
	const auto& Cfg = BSConfig->TargetConfig;
	const int32 NumManaged = ManagedTargets.Num();

	if (Cfg.TargetSpawningPolicy == ETargetSpawningPolicy::UpfrontOnly)
	{
		if (NumManaged > 0)
		{
			return 0;
		}
		if (Cfg.TargetDistributionPolicy == ETargetDistributionPolicy::Grid)
		{
			return BSConfig->GridConfig.NumHorizontalGridTargets * BSConfig->GridConfig.NumVerticalGridTargets;
		}
		return Cfg.NumUpfrontTargetsToSpawn;
	}

	// Batch spawning waits until there are no more Activated and Deactivated target(s)
	if (Cfg.bUseBatchSpawning)
	{
		if (NumManaged > 0)
		{
			return 0;
		}
		if (SpawnAreaManager->GetNumActivated() > 0)
		{
			return 0;
		}
		if (SpawnAreaManager->GetNumDeactivated() > 0)
		{
			return 0;
		}
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
	const auto& Cfg = BSConfig->TargetConfig;

	const int32 MaxAllowed = Cfg.MaxNumActivatedTargetsAtOnce >= 1
		? Cfg.MaxNumActivatedTargetsAtOnce
		: DefaultNumTargetsToActivate;

	// Constraints: Max Available & Max Allowed (both must be satisfied, so pick min)
	const int32 UpperLimit = FMath::Min(FMath::Max(0, MaxAllowed - NumActivated), MaxAvailable);

	if (UpperLimit <= 0)
	{
		return 0;
	}

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
	const auto& Cfg = BSConfig->TargetConfig;

	if (Cfg.ConsecutiveTargetScalePolicy == EConsecutiveTargetScalePolicy::SkillBased)
	{
		const float NewFactor = GetCurveTableValue(false, DynamicLookUpValue_TargetScale);
		return FVector(UKismetMathLibrary::Lerp(Cfg.MaxSpawnedTargetScale, Cfg.MinSpawnedTargetScale, NewFactor));
	}
	return FVector(FMath::FRandRange(Cfg.MinSpawnedTargetScale, Cfg.MaxSpawnedTargetScale));
}

TSet<FTargetSpawnParams> ATargetManager::GetTargetSpawnParams(const int32 NumToSpawn) const
{
	if (NumToSpawn == 0)
	{
		return {};
	}

	// Change the BoxExtent of the SpawnBox if dynamic
	if (BSConfig->TargetConfig.BoundsScalingPolicy == EBoundsScalingPolicy::Dynamic)
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

	TSet<FTargetSpawnParams> Out = SpawnAreaManager->GetTargetSpawnParams(Scales, NumToSpawn);

#if !UE_BUILD_SHIPPING
	const double EndTime = FPlatformTime::Seconds();
	const double ElapsedTime = EndTime - StartTime;

	if (Out.IsEmpty() && !GIsAutomationTesting)
	{
		UE_LOG(LogTargetManager, Display, TEXT("ValidSpawnableSpawnAreas is empty."));
	}
	if (ExecutionTimeDelegate.IsBound())
	{
		ExecutionTimeDelegate.Execute(ElapsedTime);
	}
#endif

	return Out;
}

ETargetDamageType ATargetManager::FindNextTargetDamageType()
{
	if (BSConfig->TargetConfig.TargetDamageType == ETargetDamageType::Combined)
	{
		LastTargetDamageType = (LastTargetDamageType == ETargetDamageType::Hit)
			? ETargetDamageType::Tracking
			: ETargetDamageType::Hit;
		return LastTargetDamageType;
	}
	return BSConfig->TargetConfig.TargetDamageType;
}

/* ---------------------------------- */
/* -- Deactivation and Destruction -- */
/* ---------------------------------- */

void ATargetManager::OnTargetDamageEvent(FTargetDamageEvent& Event)
{
	// Set TargetManagerData
	Event.SetTargetManagerData(
		ShouldDeactivateTarget(Event.bDamagedSelf, Event.CurrentHealth, Event.CurrentDeactivationHealthThreshold),
		ShouldDestroyTarget(Event.bDamagedSelf, Event.bOutOfHealth), CurrentStreak, TotalPossibleDamage);

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
	if (RLComponent->GetRLMode() != EReinforcementLearningMode::None)
	{
		const int32 FoundIndex = SpawnAreaManager->GetSpawnAreaIndex(Event.Guid);
		if (FoundIndex >= 0)
		{
			RLComponent->SetActiveTargetPairReward(FoundIndex, !Event.bDamagedSelf);
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
			DynamicLookUpValue_TargetScale - BSConfig->DynamicTargetScaling.DecrementAmount, 0,
			BSConfig->DynamicTargetScaling.EndThreshold);
		DynamicLookUpValue_SpawnAreaScale = FMath::Clamp(
			DynamicLookUpValue_SpawnAreaScale - BSConfig->DynamicSpawnAreaScaling.DecrementAmount, 0,
			BSConfig->DynamicSpawnAreaScaling.EndThreshold);
	}
	else if (Event.DamageType == ETargetDamageType::Hit)
	{
		DynamicLookUpValue_TargetScale = FMath::Clamp(DynamicLookUpValue_TargetScale + 1, 0,
			BSConfig->DynamicTargetScaling.EndThreshold);
		DynamicLookUpValue_SpawnAreaScale = FMath::Clamp(DynamicLookUpValue_SpawnAreaScale + 1, 0,
			BSConfig->DynamicSpawnAreaScaling.EndThreshold);
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

FVector ATargetManager::GenerateSpawnVolumeLocation(const FBSConfig* InCfg, const FVector& InOrigin,
	const FVector& InSpawnVolumeExtents)
{
	// Y and Z will be the same as SpawnBox
	FVector Out = InOrigin;

	// Align the back of the SpawnVolume to just behind the SpawnBox (MaxRadius + DirBoxPadding away from it)
	const float MaxRadius = GetMaxTargetRadius(InCfg->TargetConfig);
	Out.X -= InSpawnVolumeExtents.X - MaxRadius - DirBoxPadding;

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
	const FBS_TargetConfig& Cfg = BSConfig->TargetConfig;
	const FVector Start = BSConfig->DynamicSpawnAreaScaling.GetStartExtents();

	const FVector LastExtents = GetSpawnBoxExtents();

	const float LerpY = UKismetMathLibrary::Lerp(Start.Y, StaticExtents.Y, Factor);
	const float LerpZ = UKismetMathLibrary::Lerp(Start.Z, StaticExtents.Z, Factor);

	// Snap to nearest SpawnAreaDimensions
	int32 SnapY = FMath::FloorToInt(LerpY / SpawnAreaDimensions.Y) * SpawnAreaDimensions.Y;

	// But don't snap to zero
	SnapY = FMath::Max(SnapY, MinValue_HorizontalSpread);

	// Z snapping is slightly different since special case for HeadshotHeightOnly
	const int32 SnapZ = Cfg.TargetDistributionPolicy == ETargetDistributionPolicy::HeadshotHeightOnly
		? HeadshotHeight_VerticalSpread * 0.5f
		: FMath::FloorToInt(LerpZ / SpawnAreaDimensions.Z) * SpawnAreaDimensions.Z;

	// Don't snap at all if Grid
	const FVector NewExtents = Cfg.TargetDistributionPolicy == ETargetDistributionPolicy::Grid
		? FVector(0, LerpY, LerpZ)
		: FVector(0, SnapY, SnapZ);

	if (LastExtents == NewExtents)
	{
		return;
	}

	SpawnBox->SetBoxExtent(NewExtents);
	SpawnAreaManager->OnExtremaChanged(GetSpawnBoxExtrema());
}

void ATargetManager::UpdateSpawnVolume(const float Factor) const
{
	const FVector Origin = GetSpawnBoxOrigin();
	const FVector VolumeExtents = GenerateSpawnVolumeExtents(BSConfig.Get(), GetSpawnBoxExtents(), StaticExtents,
		Factor);
	const FVector VolumeLocation = GenerateSpawnVolumeLocation(BSConfig.Get(), Origin, VolumeExtents);

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
		bLastDirectionChangeHorizontal = bLastSpawnedTargetDirectionHorizontal;
		bLastSpawnedTargetDirectionHorizontal = !bLastDirectionChangeHorizontal;
	}
	else if (InSpawnActivationDeactivation == 1)
	{
		bLastDirectionChangeHorizontal = bLastActivatedTargetDirectionHorizontal;
		bLastActivatedTargetDirectionHorizontal = !bLastDirectionChangeHorizontal;
	}
	else
	{
		bLastDirectionChangeHorizontal = InTarget->GetLastDirectionChangeHorizontal();
		InTarget->SetLastDirectionChangeHorizontal(!bLastDirectionChangeHorizontal);
	}

	const FVector Direction = GetNewTargetDirection(InTarget->GetActorLocation(), bLastDirectionChangeHorizontal);
	InTarget->SetTargetDirection(Direction);
}

FVector ATargetManager::GetNewTargetDirection(const FVector& LocationBeforeChange,
	const bool bLastDirectionChangeHorizontal) const
{
	switch (BSConfig->TargetConfig.MovingTargetDirectionMode)
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
			Origin.X = FMath::FRandRange(Origin.X, Origin.X - BSConfig->TargetConfig.BoxBounds.X);

			// 1/4 of the total Spawn Volume
			const FVector Extent = FVector(SBExtents.X * 0.5f, SBExtents.Y * 0.5f, SBExtents.Z * 0.5f);
			const FVector Offset = FVector(0, Extent.Y, Extent.Z);
			TArray<FVector> PossibleLocations;

#if !UE_BUILD_SHIPPING
			LastAnyTargetDirectionModeSectors.Sectors.Empty();
#endif

			for (const FVector& Direction : GetAnyDirectionMultipliers(LocationBeforeChange, Origin))
			{
				PossibleLocations.Add(RandBoxPoint(Origin + Direction * Offset, Extent));
#if !UE_BUILD_SHIPPING
				LastAnyTargetDirectionModeSectors.Sectors.Add({Origin + Direction * Offset, Extent});
#endif
			}

			const FVector NewLocation = PossibleLocations[FMath::RandRange(0, PossibleLocations.Num() - 1)];

#if !UE_BUILD_SHIPPING
			LastAnyTargetDirectionModeSectors.LineStart = LocationBeforeChange;
			LastAnyTargetDirectionModeSectors.LineEnd = NewLocation;
#endif

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

TArray<FVector> ATargetManager::GetAnyDirectionMultipliers(const FVector& LocationBeforeChange, const FVector& Origin)
{
	TArray<FVector> Multipliers = AnyDirectionModeMultipliers;
	if (LocationBeforeChange.Y < 0)
	{
		if (LocationBeforeChange.Z < Origin.Z)
		{
			Multipliers.Remove(FVector(0, -1, -1));
		}
		else
		{
			Multipliers.Remove(FVector(0, -1, 1));
		}
	}
	else
	{
		if (LocationBeforeChange.Z < Origin.Z)
		{
			Multipliers.Remove(FVector(0, 1, -1));
		}
		else
		{
			Multipliers.Remove(FVector(0, 1, 1));
		}
	}
	return Multipliers;
}

void ATargetManager::UpdateTotalPossibleDamage()
{
	TotalPossibleDamage++;
	for (const auto Pair : ManagedTargets)
	{
		if (Pair.Value)
		{
			SpawnAreaManager->UpdateTotalTrackingDamagePossible(Pair.Value->GetActorLocation());
		}
	}
}

bool ATargetManager::TrackingTargetIsDamageable() const
{
	if (!BSConfig)
	{
		return false;
	}
	if (BSConfig->TargetConfig.TargetDamageType == ETargetDamageType::Hit || ManagedTargets.IsEmpty())
	{
		return false;
	}
	for (const auto Pair : ManagedTargets)
	{
		if (Pair.Value && !Pair.Value->IsImmuneToTrackingDamage())
		{
			return true;
		}
	}
	return false;
}

float ATargetManager::GetCurveTableValue(const bool bIsSpawnArea, const int32 InTime) const
{
	UCompositeCurveTable* Table = bIsSpawnArea ? CCT_SpawnArea : CCT_TargetScale;
	const bool bThresholdMet = bIsSpawnArea
		? InTime > BSConfig->DynamicSpawnAreaScaling.StartThreshold
		: InTime > BSConfig->DynamicTargetScaling.StartThreshold;
	const bool bIsCubicInterpolation = bIsSpawnArea
		? BSConfig->DynamicSpawnAreaScaling.bIsCubicInterpolation
		: BSConfig->DynamicTargetScaling.bIsCubicInterpolation;

	float OutXY = -1.f;
	TEnumAsByte<EEvaluateCurveTableResult::Type> OutResult;

	FName Name = bIsCubicInterpolation ? CurveTableRowName_Cubic_ThresholdMet : CurveTableRowName_Linear_ThresholdMet;
	Name = bThresholdMet ? Name : CurveTableRowName_Linear_PreThreshold;
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

void ATargetManager::DestroyTargets()
{
	if (ManagedTargets.IsEmpty())
	{
		return;
	}

	for (const auto Pair : ManagedTargets.Array())
	{
		if (Pair.Value)
		{
			Pair.Value->Destroy();
		}
	}
	ManagedTargets.Empty();
}

void ATargetManager::GetMovingTargetLocations(FMovingTargetLocations& MovingTargetLocations) const
{
	for (const TPair<FGuid, ATarget*>& Pair : ManagedTargets)
	{
		if (!FMath::IsNearlyZero(Pair.Value->GetTargetVelocity().Length(), UE_KINDA_SMALL_NUMBER))
		{
			MovingTargetLocations.Map.Add(Pair.Key, Pair.Value->GetActorLocation());
		}
	}
}

void ATargetManager::UpdatePlayerSettings(const FPlayerSettings_Game& InPlayerSettings)
{
	if (!ManagedTargets.IsEmpty())
	{
		for (const auto Pair : ManagedTargets)
		{
			Pair.Value->UpdatePlayerSettings(InPlayerSettings);
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
	if (RLComponent->GetRLMode() != EReinforcementLearningMode::None)
	{
		RLComponent->ClearCachedTargetPairs();
		InCommonScoreInfo.UpdateQTable(RLComponent->GetTArray_FromNdArray_QTable(), RLComponent->GetNumQTableRows(),
			RLComponent->GetNumQTableColumns(), RLComponent->GetTArray_FromNdArray_TrainingSamples(),
			RLComponent->GetTotalTrainingSamples());
	}
}

#if !UE_BUILD_SHIPPING

void ATargetManager::DrawDebug() const
{
	FlushPersistentDebugLines(GetWorld());
	if (bShowDebug_AnyMovingTargetDirectionMode)
	{
		for (const FAnyMovingTargetDirectionModeSector& Sector : LastAnyTargetDirectionModeSectors.Sectors)
		{
			DrawDebugBox(GetWorld(), Sector.Center, Sector.Extents, FColor::Green, true, -1, 0, 6);
		}
		DrawDebugLine(GetWorld(), LastAnyTargetDirectionModeSectors.LineStart,
			LastAnyTargetDirectionModeSectors.LineEnd, FColor::Red, true, -1, 0, 6.f);
	}
	SpawnAreaManager->DrawDebug();
}

#endif
