// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerSettingsInterface.h"
#include "SaveGamePlayerSettings.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

struct FBSConfig;
class UGameModesWidget;
class ATargetManagerPreview;
class ABSPlayerController;
class UTargetWidget;

/** Game mode used for the Main Menu level. Responsible for spawning a Target Manager for game mode preview purposes,
 *  which is bound to the GameModesWidget. */
UCLASS()
class BEATSHOT_API AMainMenuGameMode : public AGameModeBase, public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

public:
	AMainMenuGameMode();

	virtual void BeginPlay() override;
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings) override;

	/** Enables communication between GameModesWidget and TargetManager. */
	void SetupTargetManager(UGameModesWidget* GameModesWidget);

	/** Called when the GameModesWidget wants to start or stop previewing a game mode. */
	void OnRequestSimulationStateChange(const bool bSimulate);

	/** Starts timers, binds timer delegates, restarts TargetManager. */
	void StartSimulation();

	/** Manually calls OnAudioAnalyzerTick in TargetManager at fixed intervals. */
	void OnSimulationInterval();

	/** Clears the timers and calls FinishSimulation on TargetManager. */
	void FinishSimulation();

	/** Returns whether or not the SimulationTimer is active. */
	bool IsSimulating() const;

	/** Called when the GameModesWidget contains at least one breaking game mode option, or none. */
	void OnGameModeBreakingChange(const bool bIsGameModeBreakingChange);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<ATargetManagerPreview> TargetManagerClass;

	/** The spawned TargetManager that provides preview logic for CustomGameModesWidget_Preview. */
	UPROPERTY()
	TObjectPtr<ATargetManagerPreview> TargetManager;

	/** Pointer to the game mode config, shared between GameModesWidget and TargetManager. */
	TSharedPtr<FBSConfig> BSConfig;

	/** Game settings to pass to the TargetManager. */
	FPlayerSettings_Game PlayerSettings_Game;

	/** Looping timer that executes every TargetSpawnCD duration. */
	FTimerHandle SimulationIntervalTimer;

	/** Timer that spans the length of the simulation. */
	FTimerHandle SimulationTimer;

	/** Whether or not GameModesWidget has at least one breaking game mode option, or none.
	 *  Prevents simulation if true. */
	bool bGameModeBreakingChangePresent = false;

	/** The duration of a game mode simulation. */
	const float SimulationTimerDuration = 15.f;

	/** The initial delay of the simulation interval timer. */
	const float SimulationIntervalTimerInitialDelay = 1.f;
};
