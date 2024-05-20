// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "BSGameModeConfig/BSGameModeDataAsset.h"
#include "SaveGames/SaveGamePlayerSettings.h"
#include "BeatShotGameModeFunctionalTest.generated.h"

class UBSGameModeDataAsset;
class ATargetManager;

/** Functional testing class used to speed run through game modes. Runs the game mode for the specified duration
 *  and BeatFrequency, destroying the targets on the next tick after activating. */
UCLASS()
class BEATSHOTTESTING_API ABeatShotGameModeFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	ABeatShotGameModeFunctionalTest();

protected:
	// AFunctionalTest
	virtual void PrepareTest() override;
	virtual bool IsReady_Implementation() override;
	virtual void StartTest() override;
	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;
	virtual void CleanUp() override;
	// ~AFunctionalTest

	/** Imports Default Game Modes using the GameModeDataAsset, DefaultGameModesToTest,
	 *  and DefaultGameModeDifficultiesToTest. */
	void ImportDefaultGameModes();

	/** Imports Custom Game Modes using CustomGameModesToTest, which are game modes exported from within the game. */
	void ImportCustomGameModes();

	/** Calls OnAudioAnalyzerBeat function of target manager. Callback function for BeatTimer. */
	void OnAudioAnalyzerBeat();

	/** Initializes target manager and starts timers. */
	void StartGameMode();

	/** Clears target manager, clears timers, calls StartGameMode if not finished.
	 *  Callback function for GameModeTimer. */
	void StopGameMode();

	/** Calls DamageSelf on targets. Callback function for DestroyTimer. */
	void DestroyTargets();

	/** Gathers data for a single game mode, and adds info to the current test step. */
	void GatherData();

	/** Callback function for TargetManager's SpawnableSpawnAreasExecutionTimeDelegate. */
	void OnSpawnableSpawnAreasExecution(const double ElapsedTime);

	/** Default Game Modes data asset to pull configs from. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameModes|Default")
	TObjectPtr<UBSGameModeDataAsset> GameModeDataAsset;

	/** Default Game Modes to test. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameModes|Default",
		meta = (EditCondition = "GameModeDataAsset != nullptr"))
	TSet<EBaseGameMode> DefaultGameModesToTest;

	/** Default Game Mode Difficulties to test. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameModes|Default",
		meta = (EditCondition = "GameModeDataAsset != nullptr"))
	TSet<EGameModeDifficulty> DefaultGameModeDifficultiesToTest;

	/** Custom Game modes to test that have been exported as a string from within the game. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameModes|Custom")
	TArray<FString> CustomGameModesToTest;

	/** Blueprint class to use for the TargetManager. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	TSubclassOf<ATargetManager> TargetManagerClass;

	/** Game settings for the spawned targets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	FPlayerSettings_Game PlayerSettings_Game;

	/** The length of time to test each individual game mode and difficulty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float GameModeDuration = 10.f;

	/** How frequently to call the OnAudioAnalyzerBeat function of target manager, effectively spawning or activating
	 *  targets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float BeatFrequency = 0.1f;

	/** Initial offset applied to the BeatTimer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float InitialDelay = 0.1f;

	/** whether to destroy all activated targets at each time step, or defer to NumActivatedToDestroy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	bool bDestroyAllActivatedTargetsOnTimeStep = true;

	/** The number of activated targets to destroy at each time step. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration",
		meta = (EditCondition = "!bDestroyAllActivatedTargetsOnTimeStep", ClampMin = 1))
	int32 NumActivatedTargetsToDestroy = 1;

	/** whether to record execution times for calls to SpawnAreaManager's GetSpawnableSpawnAreas function. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	bool bRecordGetSpawnableSpawnAreasExecutionTime = false;

	/** All game modes to test. */
	UPROPERTY()
	TArray<FBSConfig> GameModesToTest;

	/** The spawned target manager. */
	UPROPERTY()
	TObjectPtr<ATargetManager> TargetManager;

	/** Timer that spans the game mode duration. */
	FTimerHandle GameModeTimer;

	/** Looping timer that spawns or activates targets. */
	FTimerHandle BeatTimer;

	/** The current game mode config shared with the target manager. */
	TSharedPtr<FBSConfig> GameModeConfig;

	/** The current index inside GameModeConfigs. */
	int32 CurrentIndex = 0;

	/** An array of execution times for calls to SpawnAreaManager's GetSpawnableSpawnAreas function. */
	TArray<double> SpawnableSpawnAreasExecutionTimes;
};
