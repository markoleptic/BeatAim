// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MainMenuGameMode.h"
#include "Components/AudioComponent.h"
#include "Player/BSPlayerController.h"
#include "SubMenuWidgets/GameModesWidgets/GameModesWidget.h"
#include "SubMenuWidgets/GameModesWidgets/CGMW_CreatorView.h"
#include "Target/TargetManagerPreview.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	MainMenuMusicComp = CreateDefaultSubobject<UAudioComponent>(TEXT("MainMenuMusicComp"));
	MainMenuMusicComp->SetAutoActivate(false);
	SetRootComponent(MainMenuMusicComp);
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AMainMenuGameMode::SetupTargetManager(UGameModesWidget* GameModesWidget)
{
	TargetManager = GetWorld()->SpawnActor<ATargetManagerPreview>(TargetManagerClass, FVector::Zero(),
		FRotator::ZeroRotator);
	TargetManager->InitBoxBoundsWidget(GameModesWidget->CustomGameModesWidget_CreatorView->Widget_Preview);
	TargetManager->Init(GameModesWidget->GetConfigPointer(), LoadPlayerSettings().Game);
	TargetManager->CreateTargetWidget.BindUObject(GameModesWidget->CustomGameModesWidget_CreatorView->Widget_Preview,
		&UCGMWC_Preview::ConstructTargetWidget);

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

	TargetManager->RestartSimulation();
	TargetManager->SetSimulatePlayerDestroyingTargets(true);
	TargetManager->SetShouldSpawn(true);

	// Bind the simulation timer
	SimulationTimerDelegate.BindUObject(this, &ThisClass::FinishSimulation);
	SimulationIntervalDelegate.BindUObject(this, &ThisClass::OnSimulationInterval);

	// Start timers
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(SimulationTimer, SimulationTimerDelegate, SimulationTimerDuration, false);
	TimerManager.SetTimer(SimulationIntervalTimer, SimulationIntervalDelegate,
		TargetManager->GetSimulation_TargetSpawnCD(), true, SimulationIntervalTimerInitialDelay);
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
	// Unbind delegates
	if (SimulationTimerDelegate.IsBound())
	{
		SimulationTimerDelegate.Unbind();
	}
	if (SimulationIntervalDelegate.IsBound())
	{
		SimulationIntervalDelegate.Unbind();
	}

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	// Clear Timers
	TimerManager.ClearTimer(SimulationIntervalTimer);
	TimerManager.ClearTimer(SimulationTimer);

	if (TargetManager)
	{
		TargetManager->SetShouldSpawn(false);
		TargetManager->FinishSimulation();
	}
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

void AMainMenuGameMode::FadeInMainMenuMusic(const float FadeInDuration)
{
	MainMenuMusicComp->FadeIn(FadeInDuration, 1.f, 0.f, EAudioFaderCurve::SCurve);
}

void AMainMenuGameMode::FadeOutMainMenuMusic(const float FadeOutDuration)
{
	MainMenuMusicComp->FadeOut(FadeOutDuration, 0.f, EAudioFaderCurve::Linear);
}
