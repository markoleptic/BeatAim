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
#include "Target/SpawnAreaManagerComponent.h"

FVector (&RandBoxPoint)(const FVector Center, const FVector Extents) = UKismetMathLibrary::RandomPointInBoundingBox;
DEFINE_LOG_CATEGORY(LogTargetManager);
using namespace Constants;

ATargetManager::ATargetManager()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Spawn Box"));
	RootComponent = SpawnBox;

	SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Spawn Volume"));
	TopBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Top Box"));
	BottomBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bottom Box"));
	LeftBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Box"));
	RightBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Box"));
	ForwardBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Forward Box"));
	BackwardBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Backward Box"));

	SpawnBox->SetLineThickness(5.f);
	TopBox->SetLineThickness(5.f);
	BottomBox->SetLineThickness(5.f);
	LeftBox->SetLineThickness(5.f);
	RightBox->SetLineThickness(5.f);
	ForwardBox->SetLineThickness(5.f);
	BackwardBox->SetLineThickness(5.f);

	SpawnAreaManager = CreateDefaultSubobject<USpawnAreaManagerComponent>(TEXT("Spawn Area Manager Component"));
	RLComponent = CreateDefaultSubobject<UReinforcementLearningComponent>(TEXT("Reinforcement Learning Component"));

	CurrentStreak = 0;
	BSConfigLocal = FBSConfig();
	BSConfig = nullptr;
	PlayerSettings = FPlayerSettings_Game();
	ShouldSpawn = false;
	bPrintDebug_NumRecentNumActive = false;
	bPrintDebug_SpawnAreaInfo = false;
	bPrintDebug_SpawnAreaDistance = false;
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
}

void ATargetManager::Destroyed()
{
	if (ShouldSpawn)
	{
		SetShouldSpawn(false);
	}

	if (!GetManagedTargets().IsEmpty())
	{
		for (const TPair<FGuid, ATarget*> Pair : GetManagedTargets())
		{
			if (Pair.Value)
			{
				Pair.Value->Destroy();
			}
		}
		ManagedTargets.Empty();
	}

	BSConfig = nullptr;

	Super::Destroyed();
}

void ATargetManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TrackingTargetIsDamageable())
	{
		UpdateTotalPossibleDamage();
	}
}

void ATargetManager::Init(const FBSConfig& InBSConfig, const FPlayerSettings_Game& InPlayerSettings)
{
	if (ShouldSpawn)
	{
		SetShouldSpawn(false);
	}

	// Initialize local copy of BSConfig
	BSConfigLocal = InBSConfig;
	BSConfig = &BSConfigLocal;
	PlayerSettings = InPlayerSettings;

	Init_Internal();
}

void ATargetManager::Init(FBSConfig* InBSConfig, const FPlayerSettings_Game& InPlayerSettings)
{
	if (ShouldSpawn)
	{
		SetShouldSpawn(false);
	}
	// Initialize pointer to another BSConfig
	BSConfig = InBSConfig;
	PlayerSettings = InPlayerSettings;

	Init_Internal();
}

void ATargetManager::TestRLComponent(int32 NumIterations)
{
	while (NumIterations > 0)
	{
		if (!ShouldSpawn) return;
		HandleRuntimeSpawning();
		HandleTargetActivation();
		SpawnAreaManager->RefreshRecentFlags();
		NumIterations--;
	}
}

void ATargetManager::Init_Internal()
{
	// Clean Up
	if (!GetManagedTargets().IsEmpty())
	{
		for (const TPair<FGuid, ATarget*> Pair : GetManagedTargets())
		{
			if (Pair.Value)
			{
				Pair.Value->Destroy();
			}
		}
		ManagedTargets.Empty();
	}
	CurrentStreak = 0;
	LastTargetDamageType = ETargetDamageType::Tracking;
	CurrentTargetScale = FVector(1.f);
	StaticExtrema = FExtrema();
	StaticExtents = FVector();
	CurrentStreak = 0;
	DynamicLookUpValue_TargetScale = 0;
	DynamicLookUpValue_SpawnAreaScale = 0;
	TotalPossibleDamage = 0.f;
	SpawnAreaManager->Clear();

	// Initialize target colors
	GetBSConfig()->InitColors(PlayerSettings.bUseSeparateOutlineColor, PlayerSettings.InactiveTargetColor,
		PlayerSettings.TargetOutlineColor, PlayerSettings.StartTargetColor, PlayerSettings.PeakTargetColor,
		PlayerSettings.EndTargetColor, PlayerSettings.TakingTrackingDamageColor,
		PlayerSettings.NotTakingTrackingDamageColor);

	// Set SpawnBox location & BoxExtent, StaticExtents, and StaticExtrema
	SpawnBox->SetRelativeLocation(GenerateStaticLocation(GetBSConfig()));
	StaticExtents = GenerateStaticExtents(GetBSConfig());
	SpawnBox->SetBoxExtent(StaticExtents);
	StaticExtrema = GenerateStaticExtrema(GetBSConfig(), GetSpawnBoxOrigin(), StaticExtents);

	// Initialize the CompositeCurveTables in case they need to be modified
	Init_Tables();

	// Initialize the SpawnAreaManager
	SpawnAreaManager->Init(GetBSConfig(), GetSpawnBoxOrigin(), StaticExtents, StaticExtrema);
	SpawnAreaManager->GetSpawnAreaRequestDelegate().BindUObject(this, &ThisClass::GetNextSpawnAreaFromRLC);

	// Initialize SpawnBox extents and the SpawnVolume extents & location
	const bool bDynamic = GetBSConfig()->TargetConfig.BoundsScalingPolicy == EBoundsScalingPolicy::Dynamic;
	const float Factor = bDynamic ? GetCurveTableValue(true, DynamicLookUpValue_SpawnAreaScale) : 1.f;
	UpdateSpawnBoxExtents(Factor);
	UpdateSpawnVolume(Factor);
	SpawnAreaManager->OnExtremaChanged(GetSpawnBoxExtrema());

	// Init RLC
	if (GetBSConfig()->IsCompatibleWithReinforcementLearning())
	{
		const FCommonScoreInfo CommonScoreInfo = FindCommonScoreInfo(GetBSConfig()->DefiningConfig);
		const FRLAgentParams Params(GetBSConfig()->AIConfig, CommonScoreInfo, SpawnAreaManager->GetSpawnAreaSize());
		RLComponent->Init(Params);

		#if !UE_BUILD_SHIPPING
		// Print loaded QTable
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = 2;
		Options.MaximumFractionalDigits = 2;
		Options.MaximumIntegralDigits = 1;
		Options.MinimumIntegralDigits = 1;
		UE_LOG(LogTargetManager, Display, TEXT("Loaded QTable:"));
		USaveGamePlayerScore::PrintQTable(GetBSConfig()->DefiningConfig, CommonScoreInfo, Options);
		#endif
	}

	SpawnAreaManager->SetShouldAskRLCForSpawnAreas(RLComponent->GetReinforcementLearningMode() == EReinforcementLearningMode::Exploration || RLComponent->
		GetReinforcementLearningMode() == EReinforcementLearningMode::ActiveAgent);

	// Spawn any targets if needed
	if (GetBSConfig()->TargetConfig.TargetSpawningPolicy == ETargetSpawningPolicy::UpfrontOnly)
	{
		HandleUpfrontSpawning();
	}
}

void ATargetManager::Init_Tables()
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

void ATargetManager::SetShouldSpawn(const bool bShouldSpawn)
{
	if (bShouldSpawn)
	{
		if (GetBSConfig()->TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::HideTarget))
		{
			for (const TPair<FGuid, ATarget*>& Pair : GetManagedTargets())
			{
				Pair.Value->SetActorHiddenInGame(true);
			}
		}
	}
	ShouldSpawn = bShouldSpawn;
}

void ATargetManager::OnPlayerStopTrackingTarget()
{
	for (const TPair<FGuid, ATarget*>& Pair : GetManagedTargets())
	{
		if (Pair.Value && !Pair.Value->IsImmuneToTrackingDamage())
		{
			Pair.Value->SetTargetColor(GetBSConfig()->TargetConfig.NotTakingTrackingDamageColor);
		}
	}
}

/* ------------------------------------ */
/* -- Target spawning and activation -- */
/* ------------------------------------ */

void ATargetManager::OnAudioAnalyzerBeat()
{
	if (!ShouldSpawn) return;

	HandleRuntimeSpawning();
	HandleTargetActivation();
	SpawnAreaManager->RefreshRecentFlags();
	
	#if !UE_BUILD_SHIPPING
	if (bPrintDebug_NumRecentNumActive)
	{
		PrintDebug_NumRecentNumActive();
	}
	#endif
}

ATarget* ATargetManager::SpawnTarget(USpawnArea* InSpawnArea)
{
	if (!InSpawnArea) return nullptr;

	const FVector Loc = InSpawnArea->GetChosenPoint();
	const FVector Scale = InSpawnArea->GetTargetScale();
	const FTransform TForm = FTransform(FRotator(0), Loc, Scale);
	ATarget* Target = GetWorld()->SpawnActorDeferred<ATarget>(TargetToSpawn, TForm, this, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	Target->Init(GetBSConfig()->TargetConfig);
	Target->SetTargetDamageType(FindNextTargetDamageType());
	Target->OnTargetDamageEvent.AddDynamic(this, &ATargetManager::OnTargetDamageEvent);
	Target->OnDeactivationResponse_ChangeDirection.AddUObject(this, &ATargetManager::ChangeTargetDirection);
	Target->OnDeactivationResponse_Reactivate.AddUObject(this, &ATargetManager::OnReactivationRequested);
	AddToManagedTargets(Target, InSpawnArea);
	
	Target->FinishSpawning(FTransform(), true);

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
	// TargetManager handles all TargetActivationResponses
	// Each target handles their own TargetDeactivationResponses & TargetDestructionConditions

	if (!InTarget || !SpawnAreaManager->IsSpawnAreaValid(SpawnAreaManager->FindSpawnAreaFromGuid(InTarget->GetGuid())))
	{
		return false;
	}
	
	// Only perform some Activation Responses if already activated
	const bool bPreTargetActivationState = InTarget->IsActivated();

	// Make sure it isn't hidden
	if (!bPreTargetActivationState && InTarget->IsHidden())
	{
		InTarget->SetActorHiddenInGame(false);
	}
	if (!bPreTargetActivationState && GetBSConfig()->TargetConfig.TargetActivationResponses.Contains(
		ETargetActivationResponse::AddImmunity))
	{
		InTarget->ApplyImmunityEffect();
	}
	if (!bPreTargetActivationState && GetBSConfig()->TargetConfig.TargetActivationResponses.Contains(
		ETargetActivationResponse::RemoveImmunity))
	{
		InTarget->RemoveImmunityEffect();
	}
	if (!bPreTargetActivationState && GetBSConfig()->TargetConfig.TargetActivationResponses.Contains(
		ETargetActivationResponse::ToggleImmunity))
	{
		InTarget->IsImmuneToDamage() ? InTarget->RemoveImmunityEffect() : InTarget->ApplyImmunityEffect();
	}
	if (GetBSConfig()->TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeVelocity))
	{
		InTarget->SetTargetSpeed(FMath::FRandRange(GetBSConfig()->TargetConfig.MinActivatedTargetSpeed,
			GetBSConfig()->TargetConfig.MaxActivatedTargetSpeed));
		if (!GetBSConfig()->TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeDirection)
			&& GetBSConfig()->TargetConfig.MovingTargetDirectionMode != EMovingTargetDirectionMode::None)
		{
			ChangeTargetDirection(InTarget, 1);
		}
	}
	if (GetBSConfig()->TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeDirection))
	{
		ChangeTargetDirection(InTarget, 1);
	}

	if (!bPreTargetActivationState && InTarget->HasBeenActivatedBefore() && GetBSConfig()->TargetConfig.
		TargetActivationResponses.Contains(ETargetActivationResponse::ApplyConsecutiveTargetScale))
	{
		InTarget->SetTargetScale(FindNextSpawnedTargetScale());
	}
	const bool bPostTargetActivationState = InTarget->ActivateTarget(GetBSConfig()->TargetConfig.TargetMaxLifeSpan);
	const bool bIsReactivation = bPreTargetActivationState && bPostTargetActivationState;

	// Don't continue if failed to activate
	if (!bPostTargetActivationState) return false;

	const USpawnArea* Previous = SpawnAreaManager->GetMostRecentSpawnArea();

	SpawnAreaManager->FlagSpawnAreaAsActivated(InTarget->GetGuid(),
		GetBSConfig()->TargetConfig.bAllowActivationWhileActivated);
	SpawnAreaManager->SetMostRecentSpawnArea(SpawnAreaManager->FindSpawnAreaFromGuid(InTarget->GetGuid()));

	if (bIsReactivation)
	{
		UE_LOG(LogTemp, Display, TEXT("Reactivated Target"));
	}

	// Don't continue if the target was already activated and succeeded the reactivation
	if (bIsReactivation) return true;

	OnTargetActivated.Broadcast(InTarget->GetTargetDamageType());
	OnTargetActivated_AimBot.Broadcast(InTarget);
	if (RLComponent->GetReinforcementLearningMode() != EReinforcementLearningMode::None && Previous)
	{
		const USpawnArea* Current = SpawnAreaManager->FindSpawnAreaFromGuid(InTarget->GetGuid());
		if (Current) RLComponent->AddToActiveTargetPairs(Previous->GetIndex(), Current->GetIndex());
	}
	return true;
}

void ATargetManager::HandleUpfrontSpawning()
{
	if (GetBSConfig()->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid)
	{
		for (int i = 0; i < SpawnAreaManager->GetSpawnAreaSize().Y * SpawnAreaManager->GetSpawnAreaSize().Z; i++)
		{
			SpawnAreaManager->GetSpawnAreasRef()[i]->SetTargetScale(FindNextSpawnedTargetScale());
			SpawnTarget(SpawnAreaManager->GetSpawnAreasRef()[i]);
		}
	}
	else
	{
		const int32 NumToSpawn = GetBSConfig()->TargetConfig.NumUpfrontTargetsToSpawn;
		for (USpawnArea* SpawnArea : FindNextSpawnAreasForSpawn(NumToSpawn))
		{
			if (SpawnArea) SpawnTarget(SpawnArea);
		}
	}
}

int32 ATargetManager::HandleRuntimeSpawning()
{
	if (GetBSConfig()->TargetConfig.TargetSpawningPolicy != ETargetSpawningPolicy::RuntimeOnly) return 0;
	
	int32 NumberToSpawn = GetNumberOfRuntimeTargetsToSpawn();

	// Only spawn targets that can be activated unless allowed
	if (!GetBSConfig()->TargetConfig.bAllowSpawnWithoutActivation)
	{
		const int32 NumberToActivate = GetNumberOfTargetsToActivateAtOnce(NumberToSpawn);
		if (NumberToSpawn > NumberToActivate)
		{
			NumberToSpawn = NumberToActivate;
		}
	}

	int32 NumSpawned = 0;
	for (USpawnArea* SpawnArea : FindNextSpawnAreasForSpawn(NumberToSpawn))
	{
		if (SpawnArea && SpawnTarget(SpawnArea)) NumSpawned++;
	}
	if (bPrintDebug_NumRecentNumActive)
	{
		if (NumberToSpawn > 0) UE_LOG(LogTemp, Display, TEXT("Spawned %d/%d targets."), NumSpawned, NumberToSpawn);
	}
	return NumSpawned;
}

int32 ATargetManager::HandleTargetActivation()
{
	if (GetManagedTargets().IsEmpty()) return 0;

	// Persistent Targets are the only type that can always receive continuous activation
	if (GetBSConfig()->TargetConfig.TargetDeactivationConditions.Contains(ETargetDeactivationCondition::Persistent))
	{
		HandlePermanentlyActiveTargetActivation();
		return 0;
	}
	
	// Check to see if theres any targets available to activate
	const int32 NumAvailableToActivate = SpawnAreaManager->GetDeactivatedManagedSpawnAreas().Num();
	const int32 NumToActivate = GetNumberOfTargetsToActivateAtOnce(NumAvailableToActivate);

	// Scuffed temporary solution to make tracking more interesting
	if (NumAvailableToActivate == 0 && NumToActivate == 0 && GetBSConfig()->TargetConfig.bAllowActivationWhileActivated)
	{
		HandleActivateAlreadyActivated();
		return 0;
	}

	int32 NumActivated = 0;
	TArray<USpawnArea*> SpawnAreas = FindNextSpawnAreasForActivation(NumToActivate);
	for (const USpawnArea* SpawnArea : SpawnAreas)
	{
		ATarget* Target = FindManagedTargetByGuid(SpawnArea->GetGuid());
		if (!Target)
		{
			UE_LOG(LogTargetManager, Warning, TEXT("Failed to find target guid for activation."));
			continue;
		}
		if (ActivateTarget(Target))
		{
			NumActivated++;
		}
		else
		{
			UE_LOG(LogTargetManager, Warning, TEXT("Failed to activate target."));
		}
	}
	
	if (bPrintDebug_NumRecentNumActive)
	{
		if (NumToActivate > 0) UE_LOG(LogTargetManager, Display, TEXT("Activated %d/%d targets."), NumActivated, NumToActivate);
	}
	return NumActivated;
}

void ATargetManager::HandlePermanentlyActiveTargetActivation() const
{
	// Handle initial activation
	TSet<USpawnArea*> SpawnAreas = SpawnAreaManager->GetActivatedSpawnAreas();
	if (SpawnAreas.IsEmpty())
	{
		SpawnAreas = SpawnAreaManager->GetDeactivatedManagedSpawnAreas();
	}

	for (const USpawnArea* SpawnArea : SpawnAreas)
	{
		if (ATarget* Target = FindManagedTargetByGuid(SpawnArea->GetGuid()))
			ActivateTarget(Target);
	}
}

void ATargetManager::HandleActivateAlreadyActivated()
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
			ActivateTarget(FindManagedTargetByGuid(SpawnAreas[Index]->GetGuid()));
	}
}

int32 ATargetManager::GetNumberOfRuntimeTargetsToSpawn() const
{
	// Depends on: MaxNumTargetsAtOnce, NumRuntimeTargetsToSpawn, ManagedTargets, bUseBatchSpawning

	// Batch spawning waits until there are no more Activated and Deactivated target(s)
	if (GetBSConfig()->TargetConfig.bUseBatchSpawning)
	{
		if (GetManagedTargets().Num() > 0 || SpawnAreaManager->GetActivatedSpawnAreas().Num() > 0 || SpawnAreaManager->
			GetDeactivatedManagedSpawnAreas().Num() > 0)
		{
			return 0;
		}
	}

	// Set default value to number of runtime targets to spawn to NumRuntimeTargetsToSpawn
	int32 NumAllowedToSpawn = (GetBSConfig()->TargetConfig.NumRuntimeTargetsToSpawn == -1)
		? 1
		: GetBSConfig()->TargetConfig.NumRuntimeTargetsToSpawn;

	// Return NumRuntimeTargetsToSpawn if no Max
	if (GetBSConfig()->TargetConfig.MaxNumTargetsAtOnce == -1)
	{
		return NumAllowedToSpawn;
	}

	NumAllowedToSpawn = GetBSConfig()->TargetConfig.MaxNumTargetsAtOnce - GetManagedTargets().Num();

	// Don't let NumAllowedToSpawn exceed NumRuntimeTargetsToSpawn
	if (NumAllowedToSpawn > GetBSConfig()->TargetConfig.NumRuntimeTargetsToSpawn)
	{
		return GetBSConfig()->TargetConfig.NumRuntimeTargetsToSpawn;
	}

	return NumAllowedToSpawn;
}

int32 ATargetManager::GetNumberOfTargetsToActivateAtOnce(const int32 MaxPossibleToActivate) const
{
	// Depends on: MaxNumActivatedTargetsAtOnce, MinNumTargetsToActivateAtOnce, MaxNumTargetsToActivateAtOnce, ActivatedSpawnAreas

	// Default to very high number if less than 1
	const int32 MaxAllowed = GetBSConfig()->TargetConfig.MaxNumActivatedTargetsAtOnce < 1
		? 100
		: GetBSConfig()->TargetConfig.MaxNumActivatedTargetsAtOnce;

	// Constraints: Max Possible & Max Allowed (both must be satisfied, so pick min)
	const int32 ConstrainedLimit = FMath::Min(MaxAllowed - SpawnAreaManager->GetActivatedSpawnAreas().Num(),
		MaxPossibleToActivate);
	if (ConstrainedLimit <= 0)
	{
		return 0;
	}

	// Can activate at least 1 at this point

	int32 MinToActivate = FMath::Min(GetBSConfig()->TargetConfig.MinNumTargetsToActivateAtOnce,
		GetBSConfig()->TargetConfig.MaxNumTargetsToActivateAtOnce);
	int32 MaxToActivate = FMath::Max(GetBSConfig()->TargetConfig.MinNumTargetsToActivateAtOnce,
		GetBSConfig()->TargetConfig.MaxNumTargetsToActivateAtOnce);

	// Allow 0, but don't default to it
	const int32 MinClampValue = MinToActivate == 0 ? 0 : 1;
	MinToActivate = FMath::Clamp(MinToActivate, MinClampValue, ConstrainedLimit);
	MaxToActivate = FMath::Clamp(MaxToActivate, 1, ConstrainedLimit);

	return FMath::RandRange(MinToActivate, MaxToActivate);
}

FVector ATargetManager::FindNextSpawnedTargetScale() const
{
	if (GetBSConfig()->TargetConfig.ConsecutiveTargetScalePolicy == EConsecutiveTargetScalePolicy::SkillBased)
	{
		const float NewFactor = GetCurveTableValue(false, DynamicLookUpValue_TargetScale);
		return FVector(UKismetMathLibrary::Lerp(GetBSConfig()->TargetConfig.MaxSpawnedTargetScale,
			GetBSConfig()->TargetConfig.MinSpawnedTargetScale, NewFactor));
	}
	return FVector(FMath::FRandRange(GetBSConfig()->TargetConfig.MinSpawnedTargetScale,
		GetBSConfig()->TargetConfig.MaxSpawnedTargetScale));
}

TArray<USpawnArea*> ATargetManager::FindNextSpawnAreasForSpawn(int32 NumToSpawn) const
{
	if (NumToSpawn == 0) return TArray<USpawnArea*>();
	
	// Change the BoxExtent of the SpawnBox if dynamic
	const bool bDynamic = GetBSConfig()->TargetConfig.BoundsScalingPolicy == EBoundsScalingPolicy::Dynamic;
	if (bDynamic)
	{
		const float Factor = bDynamic ? GetCurveTableValue(true, DynamicLookUpValue_SpawnAreaScale) : 1.f;
		UpdateSpawnBoxExtents(Factor);
		UpdateSpawnVolume(Factor);
	}
	
	TArray<FVector> Scales;
	for (int i = 0; i < NumToSpawn; i++) Scales.Add(FindNextSpawnedTargetScale());
	
	TArray<USpawnArea*> Out = SpawnAreaManager->GetSpawnableSpawnAreas(Scales, NumToSpawn);
	if (Out.IsEmpty()) UE_LOG(LogTargetManager, Warning, TEXT("ValidSpawnableSpawnAreas is empty."));
	
	return Out;
}

TArray<USpawnArea*> ATargetManager::FindNextSpawnAreasForActivation(const int32 NumToActivate) const
{
	TArray<USpawnArea*> Out;

	if (NumToActivate <= 0) return Out;

	Out = SpawnAreaManager->GetActivatableSpawnAreas(NumToActivate);
	
	if (Out.IsEmpty()) UE_LOG(LogTargetManager, Warning, TEXT("ValidActivatableSpawnAreas is empty."));
	
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

USpawnArea* ATargetManager::GetNextSpawnAreaFromRLC(const TArray<USpawnArea*>& ValidSpawnAreas, const USpawnArea* Previous) const
{
	TArray<int32> Indices;
	for (const USpawnArea* SpawnArea : ValidSpawnAreas)
	{
		Indices.Add(SpawnArea->GetIndex());
	}
	if (Indices.IsEmpty())
	{
		UE_LOG(LogTargetManager, Warning, TEXT("No targets in OpenLocations or No targets in TargetPairs"));
		return nullptr;
	}
	
	if (!Previous) return nullptr;

	const int32 ChosenIndex = RLComponent->ChooseNextActionIndex(Indices, Previous->GetIndex());
	if (!SpawnAreaManager->IsSpawnAreaValid(ChosenIndex)) return nullptr;

	USpawnArea* NextSpawnArea = SpawnAreaManager->GetSpawnAreasRef()[ChosenIndex];
	return NextSpawnArea;
}

/* ---------------------------------- */
/* -- Deactivation and Destruction -- */
/* ---------------------------------- */

void ATargetManager::OnTargetDamageEvent(FTargetDamageEvent Event)
{
	// Needs to be called first so that CurrentStreak is up to date
	UpdateCurrentStreak(Event);
	UpdateDynamicLookUpValues(Event);

	// Set TargetManagerData and broadcast to GameMode
	Event.SetTargetManagerData(CurrentStreak, TotalPossibleDamage);
	const USpawnArea* Found = SpawnAreaManager->FindSpawnAreaFromGuid(Event.Guid);
	PostTargetDamageEvent.Broadcast(Event);
	
	SpawnAreaManager->HandleTargetDamageEvent(Event);
	if (Event.bWillDestroy) RemoveFromManagedTargets(Event.Guid);

	// Update RLC
	if (RLComponent->GetReinforcementLearningMode() == EReinforcementLearningMode::None) return;
	
	if (!Found) return;
	RLComponent->SetActiveTargetPairReward(Found->GetIndex(), !Event.bDamagedSelf);

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
	FVector SpawnBoxCenter = DefaultTargetManagerLocation;
	const bool bDynamic = InCfg->TargetConfig.BoundsScalingPolicy == EBoundsScalingPolicy::Dynamic;

	switch (InCfg->TargetConfig.TargetDistributionPolicy)
	{
	case ETargetDistributionPolicy::HeadshotHeightOnly:
		{
			SpawnBoxCenter.Z = HeadshotHeight;
		}
		break;
	case ETargetDistributionPolicy::None:
	case ETargetDistributionPolicy::EdgeOnly:
	case ETargetDistributionPolicy::FullRange:
		{
			float HalfHeight;
			if (bDynamic)
			{
				HalfHeight = FMath::Max(InCfg->TargetConfig.BoxBounds.Z,
					InCfg->DynamicSpawnAreaScaling.StartBounds.Z) / 2.f;
			}
			else
			{
				HalfHeight = InCfg->TargetConfig.BoxBounds.Z / 2.f;
			}
			SpawnBoxCenter.Z = HalfHeight + InCfg->TargetConfig.FloorDistance;
		}
		break;
	case ETargetDistributionPolicy::Grid:
		{
			const float MaxTargetDiameter = FMath::Max(InCfg->TargetConfig.MinSpawnedTargetScale,
				InCfg->TargetConfig.MaxSpawnedTargetScale) * SphereTargetDiameter;
			const float VSpacing = InCfg->GridConfig.GridSpacing.Y * (InCfg->GridConfig.
				NumVerticalGridTargets - 1);
			const float VTargetWidth = (InCfg->GridConfig.NumVerticalGridTargets - 1) * MaxTargetDiameter;
			const float HalfHeight = (VSpacing + VTargetWidth) * 0.5f;
			SpawnBoxCenter.Z = HalfHeight + InCfg->TargetConfig.FloorDistance;
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
		Out.Z = 1.f;
		break;
	case ETargetDistributionPolicy::None:
	case ETargetDistributionPolicy::EdgeOnly:
	case ETargetDistributionPolicy::FullRange:
		break;
	case ETargetDistributionPolicy::Grid:
		{
			const float MaxTargetDiameter = FMath::Max(InCfg->TargetConfig.MinSpawnedTargetScale, InCfg->TargetConfig.MaxSpawnedTargetScale) * SphereTargetDiameter;

			const float HSpacing = InCfg->GridConfig.GridSpacing.X * (InCfg->GridConfig.
				NumHorizontalGridTargets - 1);
			const float VSpacing = InCfg->GridConfig.GridSpacing.Y * (InCfg->GridConfig.
				NumVerticalGridTargets - 1);

			const float HTargetWidth = (InCfg->GridConfig.NumHorizontalGridTargets - 1) * MaxTargetDiameter;
			const float VTargetWidth = (InCfg->GridConfig.NumVerticalGridTargets - 1) * MaxTargetDiameter;

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
	const float MaxTargetDiameter = FMath::Max(InCfg->TargetConfig.MinSpawnedTargetScale,
		InCfg->TargetConfig.MaxSpawnedTargetScale) * SphereTargetDiameter;

	const float MinX = InOrigin.X - 2 * InStaticExtents.X - MaxTargetDiameter;
	const float MaxX = InOrigin.X + MaxTargetDiameter;

	if (InCfg->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid)
	{
		const float HSpacing = InCfg->GridConfig.GridSpacing.X * (InCfg->GridConfig.
			NumHorizontalGridTargets - 1);
		const float VSpacing = InCfg->GridConfig.GridSpacing.Y * (InCfg->GridConfig.
			NumVerticalGridTargets - 1);

		const float HTargetWidth = (InCfg->GridConfig.NumHorizontalGridTargets - 1) * MaxTargetDiameter;
		const float VTargetWidth = (InCfg->GridConfig.NumVerticalGridTargets - 1) * MaxTargetDiameter;

		const float HalfWidth = (HSpacing + HTargetWidth) * 0.5f;
		const float HalfHeight = (VSpacing + VTargetWidth) * 0.5f;

		const float MinY = InOrigin.Y - HalfWidth;
		const float MaxY = InOrigin.Y + HalfWidth;
		const float MinZ = InOrigin.Z - HalfHeight;
		const float MaxZ = InOrigin.Z + HalfHeight;

		return FExtrema(FVector(MinX, MinY, MinZ), FVector(MaxX, MaxY, MaxZ));
	}

	const float MinY = InOrigin.Y - InStaticExtents.Y;
	const float MaxY = InOrigin.Y + InStaticExtents.Y;
	const float MinZ = InOrigin.Z - InStaticExtents.Z;
	const float MaxZ = InOrigin.Z + InStaticExtents.Z;

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
	const float MaxHalfTargetSize = FMath::Max(InCfg->TargetConfig.MinSpawnedTargetScale,
		InCfg->TargetConfig.MaxSpawnedTargetScale) * SphereTargetRadius;
	
	// Account for target scale, use radius since this is half-size
	Out.X = LerpX + MaxHalfTargetSize + DirBoxPadding;
	Out.Y += MaxHalfTargetSize + 2.f;
	Out.Z += MaxHalfTargetSize + 2.f;

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

	const float MaxHalfTargetSize = FMath::Max(InCfg->TargetConfig.MinSpawnedTargetScale,
		InCfg->TargetConfig.MaxSpawnedTargetScale) * SphereTargetRadius;

	// Account for target scale, use radius since this is half-size
	AbsMaxExtents.X += MaxHalfTargetSize + DirBoxPadding;
	AbsMaxExtents.Y += MaxHalfTargetSize + 2.f;
	AbsMaxExtents.Z += MaxHalfTargetSize + 2.f;

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
	const FVector Start = GetBSConfig()->DynamicSpawnAreaScaling.GetStartExtents();

	const FVector LastExtents = GetSpawnBoxExtents();

	const float LerpY = UKismetMathLibrary::Lerp(Start.Y, StaticExtents.Y, Factor);
	const float LerpZ = UKismetMathLibrary::Lerp(Start.Z, StaticExtents.Z, Factor);
	
	const int32 IncY = SpawnAreaManager->GetSpawnAreaInc().Y;
	const int32 IncZ = SpawnAreaManager->GetSpawnAreaInc().Z;

	const int32 SnapY = IncY * FMath::FloorToInt(LerpY / IncY);
	const int32 SnapZ = IncZ * FMath::FloorToInt(LerpZ / IncZ);
	
	const bool bGrid = GetBSConfig()->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid;
	const FVector NewExtents = FVector(0, bGrid ? LerpY : SnapY, bGrid ? LerpZ : SnapZ);

	if (LastExtents == NewExtents) return;
	
	SpawnBox->SetBoxExtent(NewExtents);
	SpawnAreaManager->OnExtremaChanged(GetSpawnBoxExtrema());
}

void ATargetManager::UpdateSpawnVolume(const float Factor) const
{
	const FVector Origin = GetSpawnBoxOrigin();
	const FVector VolumeLocation = GenerateSpawnVolumeLocation(Origin, GetSpawnBoxExtents(), StaticExtents, Factor);
	const FVector VolumeExtents = GenerateSpawnVolumeExtents(GetBSConfig(), GetSpawnBoxExtents(), StaticExtents, Factor);

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

void ATargetManager::OnReactivationRequested(ATarget* Target)
{
	ActivateTarget(Target);
}

void ATargetManager::UpdateTotalPossibleDamage()
{
	TotalPossibleDamage++;
	for (const TPair<FGuid, ATarget*>& Pair : GetManagedTargets())
	{
		SpawnAreaManager->UpdateTotalTrackingDamagePossible(Pair.Value->GetActorLocation());
	}
}

bool ATargetManager::TrackingTargetIsDamageable() const
{
	if (GetBSConfig()->TargetConfig.TargetDamageType == ETargetDamageType::Hit || GetManagedTargets().IsEmpty())
	{
		return false;
	}
	for (const TPair<FGuid, ATarget*>& Pair : GetManagedTargets())
	{
		if (!Pair.Value->IsImmuneToTrackingDamage()) return true;
	}
	return false;
}

ATarget* ATargetManager::FindManagedTargetByGuid(const FGuid Guid) const
{
	const auto Found = ManagedTargets.Find(Guid);
	return Found ? *Found : nullptr;
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

void ATargetManager::UpdatePlayerSettings(const FPlayerSettings_Game& InPlayerSettings)
{
	PlayerSettings = InPlayerSettings;
	if (!ManagedTargets.IsEmpty())
	{
		for (const TPair<FGuid, ATarget*>& Pair : GetManagedTargets())
		{
			Pair.Value->UpdatePlayerSettings(PlayerSettings);
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

void ATargetManager::SaveQTable(FCommonScoreInfo& InCommonScoreInfo) const
{
	if (RLComponent->GetReinforcementLearningMode() != EReinforcementLearningMode::None)
	{
		RLComponent->ClearCachedTargetPairs();
		InCommonScoreInfo.UpdateQTable(RLComponent->GetTArray_FromNdArray_QTable(), RLComponent->GetNumQTableRows(),
			RLComponent->GetNumQTableColumns(), RLComponent->GetTArray_FromNdArray_TrainingSamples(),
			RLComponent->GetTotalTrainingSamples());
	}
}

/* ----------- */
/* -- Debug -- */
/* ----------- */

void ATargetManager::PrintDebug_NumRecentNumActive() const
{
	const int NumRecent = SpawnAreaManager->GetRecentSpawnAreas().Num();
	const int NumAct = SpawnAreaManager->GetActivatedSpawnAreas().Num();
	const int NumManaged = SpawnAreaManager->GetManagedSpawnAreas().Num();
	UE_LOG(LogTargetManager, Display, TEXT("NumRecent: %d NumActivated: %d NumManaged: %d"), NumRecent, NumAct,
		NumManaged);
}
