// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MainMenuGameMode.h"
#include "BSGameInstance.h"
#include "GameModes/CreatorViewWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Menus/GameModeMenuWidget.h"
#include "Target/TargetManagerPreview.h"

AMainMenuGameMode::AMainMenuGameMode()
{
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	GI->RegisterPlayerSettingsSubscriber<AMainMenuGameMode, FPlayerSettings_Game>(this,
		&AMainMenuGameMode::OnPlayerSettingsChanged);
}

void AMainMenuGameMode::OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings)
{
	PlayerSettings_Game = GameSettings;
}

void AMainMenuGameMode::SetupTargetManager(UGameModeMenuWidget* GameModesWidget)
{
	PlayerSettings_Game = LoadPlayerSettings().Game;
	BSConfig = GameModesWidget->GetBSConfig();
	TargetManager = GetWorld()->SpawnActor<ATargetManagerPreview>(TargetManagerClass, FVector::Zero(),
		FRotator::ZeroRotator);
	TargetManager->InitBoxBoundsWidget(GameModesWidget->CustomGameModesWidget_CreatorView->Widget_Preview);
	TargetManager->Init(BSConfig, FCommonScoreInfo(), PlayerSettings_Game);

	GameModesWidget->RequestSimulateTargetManagerStateChange.AddUObject(this,
		&ThisClass::OnRequestSimulationStateChange);
	GameModesWidget->OnGameModeBreakingChange.AddUObject(this, &ThisClass::OnGameModeBreakingChange);
}

void AMainMenuGameMode::OnRequestSimulationStateChange(const bool bSimulate)
{
	if (bSimulate)
	{
		StartSimulation();
	}
	else
	{
		FinishSimulation();
	}
}

void AMainMenuGameMode::StartSimulation()
{
	if (!TargetManager)
	{
		return;
	}

	if (IsSimulating())
	{
		FinishSimulation();
	}

	if (bGameModeBreakingChangePresent)
	{
		return;
	}

	TargetManager->Init(BSConfig, FCommonScoreInfo(), PlayerSettings_Game);
	TargetManager->SetSimulatePlayerDestroyingTargets(true);
	TargetManager->SetShouldSpawn(true);

	// Start timers
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(SimulationTimer, this, &ThisClass::FinishSimulation, SimulationTimerDuration, false);
	TimerManager.SetTimer(SimulationIntervalTimer, this, &ThisClass::OnSimulationInterval,
		BSConfig->TargetConfig.TargetSpawnCD, true, SimulationIntervalTimerInitialDelay);
}

void AMainMenuGameMode::OnSimulationInterval()
{
	if (TargetManager && !bGameModeBreakingChangePresent)
	{
		TargetManager->OnAudioAnalyzerBeat();
	}
}

void AMainMenuGameMode::FinishSimulation()
{
	if (TargetManager)
	{
		TargetManager->Clear();
	}

	// Clear Timers
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

bool AMainMenuGameMode::IsSimulating() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(SimulationTimer);
}

void AMainMenuGameMode::OnGameModeBreakingChange(const bool bIsGameModeBreakingChange)
{
	if (bIsGameModeBreakingChange == bGameModeBreakingChangePresent)
	{
		return;
	}
	bGameModeBreakingChangePresent = bIsGameModeBreakingChange;
	if (bIsGameModeBreakingChange && IsSimulating())
	{
		FinishSimulation();
	}
}
