// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "BeatShotGameModeFunctionalTest.h"

#include "SaveGamePlayerScore.h"
#include "Algo/RandomShuffle.h"
#include "Target/SpawnArea.h"
#include "Target/SpawnAreaManagerComponent.h"
#include "Target/Target.h"
#include "Target/TargetManager.h"

ABeatShotGameModeFunctionalTest::ABeatShotGameModeFunctionalTest()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlayerSettings_Game = FPlayerSettings_Game();
}

void ABeatShotGameModeFunctionalTest::PrepareTest()
{
	Super::PrepareTest();
	if (!TargetManagerClass)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("No TargetManagerClass."));
	}

	ImportDefaultGameModes();
	ImportCustomGameModes();

	if (GameModesToTest.IsEmpty())
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("No Games Modes to test."));
	}

	GameModeConfig = MakeShareable(new FBSConfig());
	TargetManager = GetWorld()->SpawnActor<ATargetManager>(TargetManagerClass);
	if (bRecordGetSpawnableSpawnAreasExecutionTime)
	{
		TargetManager->ExecutionTimeDelegate.BindUObject(this, &ThisClass::OnSpawnableSpawnAreasExecution);
	}
}

void ABeatShotGameModeFunctionalTest::ImportDefaultGameModes()
{
	if (!GameModeDataAsset) return;

	const auto Map = GameModeDataAsset->GetDefaultGameModesMap();
	for (const auto& GameMode : DefaultGameModesToTest)
	{
		for (const auto& Difficulty : DefaultGameModeDifficultiesToTest)
		{
			if (const auto Found = Map.Find(FBS_DefiningConfig(EGameModeType::Preset, GameMode, "", Difficulty)))
			{
				GameModesToTest.Add(*Found);
			}
			else
			{
				const FString GameModeString = UEnum::GetDisplayValueAsText(GameMode).ToString();
				const FString DifString = UEnum::GetDisplayValueAsText(Difficulty).ToString();
				AddWarning(FString::Printf(TEXT("Failed to import default game mode: %s %s"), *GameModeString,
					*DifString));
			}
		}
	}
}

void ABeatShotGameModeFunctionalTest::ImportCustomGameModes()
{
	for (const auto& ExportString : CustomGameModesToTest)
	{
		FBSConfig Config;
		FText FailureReason;
		if (FBSConfig::DecodeFromString(ExportString, Config, &FailureReason))
		{
			GameModesToTest.Add(Config);
		}
		else
		{
			AddWarning(FString::Printf(TEXT("Failed to import custom game mode: %s"), *FailureReason.ToString()));
		}
	}
}

bool ABeatShotGameModeFunctionalTest::IsReady_Implementation()
{
	return TargetManager && GameModeConfig.IsValid();
}

void ABeatShotGameModeFunctionalTest::StartTest()
{
	Super::StartTest();
	StartGameMode();
}

void ABeatShotGameModeFunctionalTest::FinishTest(EFunctionalTestResult TestResult, const FString& Message)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	TargetManager->Clear();
	TargetManager->Destroy();
	GameModeConfig.Reset();
	GameModeConfig = nullptr;
	GameModesToTest.Empty();
	Super::FinishTest(TestResult, Message);
}

void ABeatShotGameModeFunctionalTest::CleanUp()
{
	Super::CleanUp();
}

void ABeatShotGameModeFunctionalTest::OnAudioAnalyzerBeat()
{
	TargetManager->OnAudioAnalyzerBeat();

	const int32 NumActivated = TargetManager->SpawnAreaManager->GetNumActivated();
	const int32 NumManaged = TargetManager->SpawnAreaManager->GetNumManaged();
	AssertEqual_Bool(NumActivated <= NumManaged, true, "Activated <= Managed");

	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [&]
	{
		DestroyTargets();
	}));
}

void ABeatShotGameModeFunctionalTest::StartGameMode()
{
	if (GameModesToTest[CurrentIndex].DefiningConfig.GameModeType == EGameModeType::Preset)
	{
		const FText DiffText = UEnum::GetDisplayValueAsText(GameModesToTest[CurrentIndex].DefiningConfig.Difficulty);
		const FText Preset = UEnum::GetDisplayValueAsText(GameModesToTest[CurrentIndex].DefiningConfig.BaseGameMode);
		StartStep(Preset.ToString() + "." + DiffText.ToString());
	}
	else
	{
		const FText Custom = FText::FromString(GameModesToTest[CurrentIndex].DefiningConfig.CustomGameModeName);
		StartStep(Custom.ToString());
	}

	GameModeConfig = MakeShareable(new FBSConfig(GameModesToTest[CurrentIndex]));
	TargetManager->Init(GameModeConfig, FCommonScoreInfo(), PlayerSettings_Game);
	TargetManager->SetShouldSpawn(true);
	GetWorldTimerManager().SetTimer(GameModeTimer, this, &ThisClass::StopGameMode, GameModeDuration, false);
	GetWorldTimerManager().SetTimer(BeatTimer, this, &ThisClass::OnAudioAnalyzerBeat, BeatFrequency, true,
		InitialDelay);
}

void ABeatShotGameModeFunctionalTest::StopGameMode()
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	if (!TargetManager) FinishTest(EFunctionalTestResult::Failed, TEXT("Null Target Manager"));

	GatherData();

	GameModeConfig.Reset();
	TargetManager->Clear();
	FinishStep();
	CurrentIndex++;

	if (GameModesToTest.IsValidIndex(CurrentIndex))
	{
		StartGameMode();
	}
	else
	{
		FinishTest(EFunctionalTestResult::Succeeded, TEXT("End of Game Mode Configs"));
	}
}

void ABeatShotGameModeFunctionalTest::DestroyTargets()
{
	if (TargetManager)
	{
		if (bDestroyAllActivatedTargetsOnTimeStep)
		{
			for (const USpawnArea* SpawnArea : TargetManager->SpawnAreaManager->GetActivatedSpawnAreas())
			{
				if (ATarget* Target = TargetManager->ManagedTargets.FindRef(SpawnArea->GetGuid()))
				{
					Target->DamageSelf(true);
				}
			}
		}
		else
		{
			TArray<USpawnArea*> Activated = TargetManager->SpawnAreaManager->GetActivatedSpawnAreas().Array();
			Algo::RandomShuffle(Activated);
			Activated.SetNum(NumActivatedTargetsToDestroy);
			for (const USpawnArea* SpawnArea : Activated)
			{
				if (ATarget* Target = TargetManager->ManagedTargets.FindRef(SpawnArea->GetGuid()))
				{
					Target->DamageSelf(true);
				}
			}
		}
	}
}

void ABeatShotGameModeFunctionalTest::GatherData()
{
	if (bRecordGetSpawnableSpawnAreasExecutionTime && !SpawnableSpawnAreasExecutionTimes.IsEmpty())
	{
		double Sum = 0.f;
		double WorstCase = 0.f;
		const int32 Num = SpawnableSpawnAreasExecutionTimes.Num();
		for (auto It = SpawnableSpawnAreasExecutionTimes.CreateConstIterator(); It; ++It)
		{
			const double Value = *It;
			Sum += Value;
			if (Value > WorstCase)
			{
				WorstCase = Value;
			}
		}
		const double Avg = Sum / Num;
		SpawnableSpawnAreasExecutionTimes.Empty();
		AddInfo(FString::Printf(TEXT("Avg Execution Time to get SpawnArea for spawning: %lf"), Avg));
		AddInfo(FString::Printf(TEXT("Worst Case Execution Time to get SpawnArea for spawning: %lf"), WorstCase));
	}
}

void ABeatShotGameModeFunctionalTest::OnSpawnableSpawnAreasExecution(const double ElapsedTime)
{
	SpawnableSpawnAreasExecutionTimes.Emplace(ElapsedTime);
}
