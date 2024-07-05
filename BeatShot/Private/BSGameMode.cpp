// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "BSGameMode.h"
#include "AudioAnalyzerManager.h"
#include "BSGameInstance.h"
#include "BSGameUserSettings.h"
#include "BSPlayerScoreInterface.h"
#include "RuntimeAudioImporterLibrary.h"
#include "AbilitySystem/BSAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/BSGA_AimBot.h"
#include "AbilitySystem/Abilities/BSGA_TrackGun.h"
#include "AbilitySystem/Globals/BSAttributeSetBase.h"
#include "Character/BSCharacter.h"
#include "Components/AudioComponent.h"
#include "Equipment/BSGun.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BSPlayerController.h"
#include "Sound/CapturableSoundWave.h"
#include "System/SteamManager.h"
#include "Target/Target.h"
#include "Target/TargetManager.h"
#include "Utilities/GameModeTransitionState.h"
#include "Visualizers/VisualizerManager.h"

DEFINE_LOG_CATEGORY(LogBSGameMode);

ABSGameMode::ABSGameMode(): AATracker(nullptr), AAPlayer(nullptr), TrackGunAbilitySet(nullptr), AudioImporter(nullptr),
                            AudioCapturer(nullptr), bLastTargetOnSet(false), bShouldTick(false), Elapsed(0),
                            MaxScorePerTarget(0), TimePlayedGameMode(0)
{
	PrimaryActorTick.bCanEverTick = true;
	AudioComponent = CreateDefaultSubobject<UAudioComponent>("Audio Component");
	RootComponent = AudioComponent;
}

void ABSGameMode::BeginPlay()
{
	Super::BeginPlay();

	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	GI->RegisterPlayerSettingsSubscriber<
		ABSGameMode, FPlayerSettings_Game>(this, &ABSGameMode::OnPlayerSettingsChanged);
	GI->RegisterPlayerSettingsSubscriber<ABSGameMode, FPlayerSettings_AudioAnalyzer>(this,
		&ABSGameMode::OnPlayerSettingsChanged);

	InitializeGameMode(GI->GetBSConfig());
}

void ABSGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bShouldTick && GetWorldTimerManager().IsTimerActive(GameModeLengthTimer))
	{
		OnTick_AudioAnalyzers(DeltaSeconds);
	}
}

void ABSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (ABSPlayerController* NewBSPlayer = Cast<ABSPlayerController>(NewPlayer))
	{
		Controllers.Add(NewBSPlayer);
		CurrentPlayerScores.Add(NewBSPlayer, FPlayerScore());
	}
	SpawnPlayer(NewPlayer);
}

void ABSGameMode::PostLoad()
{
	Super::PostLoad();
}

void ABSGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	Controllers.Remove(Cast<ABSPlayerController>(Exiting));
	CurrentPlayerScores.Remove(Cast<ABSPlayerController>(Exiting));
}

ACharacter* ABSGameMode::SpawnPlayer(APlayerController* PlayerController)
{
	if (ACharacter* Character = PlayerController->GetCharacter())
	{
		Character->Destroy();
	}
	const APlayerStart* ChosenPlayerStart = Cast<APlayerStart>(ChoosePlayerStart(PlayerController));
	ACharacter* SpawnedCharacter = GetWorld()->SpawnActor<
		ACharacter>(CharacterClass, ChosenPlayerStart->GetTransform());
	PlayerController->Possess(SpawnedCharacter);
	return SpawnedCharacter;
}

void ABSGameMode::HandleAudioImporterResult(URuntimeAudioImporterLibrary* Importer, UImportedSoundWave* SoundWave,
	ERuntimeImportStatus Status)
{
	if (Status == ERuntimeImportStatus::SuccessfulImport)
	{
		AudioComponent->SetSound(SoundWave);
	}
}

void ABSGameMode::HandleGetAvailableAudioInputDevices(const TArray<FRuntimeAudioInputDeviceInfo>& DeviceInfo)
{
	for (int i = 0; i < DeviceInfo.Num(); i++)
	{
		if (DeviceInfo[i].DeviceName.Equals(BSConfig->AudioConfig.InAudioDevice, ESearchCase::IgnoreCase))
		{
			if (AudioCapturer->StartCapture(i))
			{
				AudioComponent->SetSound(AudioCapturer);
			}
			break;
		}
	}
}

void ABSGameMode::InitializeGameMode(const TSharedPtr<FBSConfig>& InConfig)
{
	Elapsed = 0.f;
	bLastTargetOnSet = false;
	TimePlayedGameMode = 0.f;
	bShouldTick = false;

	for (ABSPlayerController* Controller : Controllers)
	{
		check(Controller);
		Controller->FadeScreenFromBlack();
		Controller->ShowCountdown();
	}

	const FPlayerSettings PlayerSettings = LoadPlayerSettings();
	OnPlayerSettingsChanged(PlayerSettings.Game);
	OnPlayerSettingsChanged(PlayerSettings.AudioAnalyzer);

	check(InConfig);
	BSConfig = InConfig;

	if (!TargetManager)
	{
		TargetManager = GetWorld()->SpawnActor<ATargetManager>(TargetManagerClass, FVector::Zero(),
			FRotator::ZeroRotator, SpawnParameters);
		TargetManager->OnTargetActivated.AddUObject(this, &ABSGameMode::UpdateTargetsSpawned);
		TargetManager->PostTargetDamageEvent.AddUObject(this, &ABSGameMode::HandlePostTargetDamageEvent);
	}
	const FCommonScoreInfo CommonScoreInfo =
		IBSPlayerScoreInterface::FindOrAddCommonScoreInfo(BSConfig->DefiningConfig);
	TargetManager->Init(BSConfig, CommonScoreInfo, PlayerSettings.Game);

	if (!VisualizerManager)
	{
		VisualizerManager = GetWorld()->SpawnActor<AVisualizerManager>(VisualizerManagerClass);
		VisualizerManager->InitializeVisualizers(PlayerSettings.Game, PlayerSettings.AudioAnalyzer);
	}

	for (const ABSPlayerController* Controller : Controllers)
	{
		const ABSCharacterBase* Character = Controller->GetBSCharacter();
		check(Character);

		UBSAbilitySystemComponent* ASC = Character->GetBSAbilitySystemComponent();
		check(ASC);

		const UBSAttributeSetBase* Set = ASC->GetSet<UBSAttributeSetBase>();
		check(Set);

		ASC->SetNumericAttributeBase(Set->GetHitDamageAttribute(), BSConfig->TargetConfig.BasePlayerHitDamage);
		ASC->SetNumericAttributeBase(Set->GetTrackingDamageAttribute(),
			BSConfig->TargetConfig.BasePlayerTrackingDamage);

		if (BSConfig->TargetConfig.TargetDamageType == ETargetDamageType::Tracking)
		{
			if (TrackGunAbilityGrantedHandles.IsEmpty())
			{
				TrackGunAbilitySet->GiveToAbilitySystem(ASC, &TrackGunAbilityGrantedHandles);
			}
			FGameplayAbilitySpec* TrackGunSpec = TrackGunAbilityGrantedHandles.FindAbilitySpecFromHandle(ASC);
			if (UBSGA_TrackGun* TrackAbility = Cast<UBSGA_TrackGun>(TrackGunSpec->GetPrimaryInstance()))
			{
				TrackAbility->OnPlayerStopTrackingTarget.AddUniqueDynamic(TargetManager.Get(),
					&ATargetManager::OnPlayerStopTrackingTarget);
				ASC->MarkAbilitySpecDirty(*TrackGunSpec);
			}
		}
		else
		{
			if (!TrackGunAbilityGrantedHandles.IsEmpty())
			{
				TrackGunAbilityGrantedHandles.TakeFromAbilitySystem(Character->GetBSAbilitySystemComponent());
			}
		}
	}

	LoadMatchingPlayerScores();
	bShouldTick = InitializeAudioManagers();
	if (!bShouldTick)
	{
		UE_LOG(LogBSGameMode, Warning, TEXT("Audio Analyzer Initialization Error"));
	}
}

void ABSGameMode::StartGameMode()
{
	for (auto& CurrentPlayerScore : CurrentPlayerScores)
	{
		CurrentPlayerScore.Key->ShowCrossHair();
		CurrentPlayerScore.Key->ShowPlayerHUD();
		CurrentPlayerScore.Key->HideCountdown();
		CurrentPlayerScore.Key->UpdatePlayerHUD(CurrentPlayerScore.Value, -1.f, -1.f);
	}

	StartGameModeTimers();
	TargetManager->SetShouldSpawn(true);
}

void ABSGameMode::StartGameModeTimers()
{
	if (BSConfig->AudioConfig.SongLength == 0.f)
	{
		GetWorldTimerManager().SetTimer(GameModeLengthTimer, 31536000, false);
	}
	else
	{
		GameModeLengthTimerDelegate.BindUObject(this, &ABSGameMode::EndGameMode, true, ETransitionState::None);
		GetWorldTimerManager().SetTimer(GameModeLengthTimer, GameModeLengthTimerDelegate,
			BSConfig->AudioConfig.SongLength, false);
	}
	GetWorldTimerManager().SetTimer(OnSecondPassedTimer, this, &ABSGameMode::HandleSecondPassed, 1.f, true);
}

void ABSGameMode::EndGameMode(const bool bSaveScores, const ETransitionState TransitionState)
{
	bShouldTick = false;
	FTimerManager& TimerManager = GetWorldTimerManager();
	TimePlayedGameMode = TimerManager.GetTimerElapsed(GameModeLengthTimer);
	TimerManager.ClearAllTimersForObject(this);
	GameModeLengthTimerDelegate.Unbind();

	if (TargetManager)
	{
		TargetManager->SetShouldSpawn(false);

		if (TargetManager->OnTargetActivated.IsBoundToObject(this))
		{
			TargetManager->OnTargetActivated.RemoveAll(this);
		}
		if (TargetManager->PostTargetDamageEvent.IsBoundToObject(this))
		{
			TargetManager->PostTargetDamageEvent.RemoveAll(this);
		}
	}

	if (VisualizerManager)
	{
		VisualizerManager->DeactivateVisualizers();
	}

	if (AATracker)
	{
		AATracker->UnloadCapturerAudio();
		AATracker->UnloadPlayerAudio();
		AATracker->MarkAsGarbage();
	}

	if (AAPlayer)
	{
		AAPlayer->UnloadCapturerAudio();
		AAPlayer->UnloadPlayerAudio();
		AAPlayer->MarkAsGarbage();
	}

	if (AudioImporter)
	{
		AudioImporter->MarkAsGarbage();
	}
	if (AudioCapturer)
	{
		AudioCapturer->StopCapture();
		AudioCapturer->MarkAsGarbage();
	}
	AudioComponent->Stop();
	AudioComponent->SetSound(nullptr);

	bool bQuitToDesktopAfterSave = false;
	for (ABSPlayerController* Controller : Controllers)
	{
		if (Controller->IsPaused())
		{
			Controller->HandlePause();
		}

		// Always try to hide the countdown
		Controller->HideCountdown();

		switch (TransitionState)
		{
		case ETransitionState::StartFromPostGameMenu:
		case ETransitionState::Restart:
		case ETransitionState::PlayAgain:
			{
				// Fade screen to black and reinitialize
				Controller->OnScreenFadeToBlackFinish.BindLambda([this, Controller]
				{
					Controller->HidePostGameMenu();
					Controller->HidePlayerHUD();
					Controller->HideCrossHair();
					InitializeGameMode(BSConfig);
				});
				Controller->FadeScreenToBlack();
			}
			break;
		case ETransitionState::QuitToMainMenu:
			{
				Controller->FadeScreenToBlack();
			}
			break;
		case ETransitionState::None:
			{
				Controller->HidePlayerHUD();
				Controller->HideCrossHair();
				Controller->ShowPostGameMenu();
			}
		case ETransitionState::QuitToDesktop:
			bQuitToDesktopAfterSave = true;
			break;
		case ETransitionState::StartFromMainMenu:
			break;
		}
	}

	// Handle saving scores before resetting Target Manager
	HandleScoreSaving(bSaveScores, bQuitToDesktopAfterSave);

	TargetManager->Clear();

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	if (TransitionState == ETransitionState::QuitToMainMenu && !Controllers.IsEmpty())
	{
		GetWorldTimerManager().SetTimer(GoToMainMenuTimer, this, &ABSGameMode::GoToMainMenu,
			Controllers[0]->ScreenFadeWidgetAnimationDuration, false);
	}
}

void ABSGameMode::SpawnNewTarget(const bool bNewTargetState)
{
	if (bNewTargetState && !bLastTargetOnSet)
	{
		bLastTargetOnSet = true;
		if (Elapsed > BSConfig->TargetConfig.TargetSpawnCD)
		{
			Elapsed = 0.f;
			TargetManager->OnAudioAnalyzerBeat();
		}
	}
	else if (!bNewTargetState && bLastTargetOnSet)
	{
		bLastTargetOnSet = false;
	}
}

void ABSGameMode::StartAAManagerPlayback()
{
	switch (BSConfig->AudioConfig.AudioFormat)
	{
	case EAudioFormat::File:
		AATracker->Play();
		if (AAPlayer)
		{
			auto HandleTimerFinished = [this]()
			{
				AudioComponent->Play();
				UE_LOG(LogBSGameMode, Display, TEXT("Now Playing AudioComponent"));
			};
			GetWorldTimerManager().SetTimer(PlayerDelayTimer, HandleTimerFinished, BSConfig->AudioConfig.PlayerDelay,
				false);
		}
		else
		{
			AudioComponent->Play();
		}
		break;
	case EAudioFormat::Capture:
		{
			AATracker->StartCapture(false, false);
			AudioComponent->Play();
			break;
		}
	case EAudioFormat::Loopback:
		{
			AATracker->StartLoopback(false);
			AudioComponent->Play();
			break;
		}
	default:
		break;
	}
	UE_LOG(LogBSGameMode, Display, TEXT("Now Playing AATracker"));
}

void ABSGameMode::PauseAAManager(const bool ShouldPause)
{
	if (!AATracker)
	{
		return;
	}
	switch (BSConfig->AudioConfig.AudioFormat)
	{
	case EAudioFormat::File:
		AATracker->SetPaused(ShouldPause);
		if (AAPlayer)
		{
			AAPlayer->SetPaused(ShouldPause);
		}
		break;
	case EAudioFormat::Capture:
		ShouldPause ? AATracker->StopCapture() : AATracker->StartCapture(false, false);
		break;
	case EAudioFormat::Loopback:
		ShouldPause ? AATracker->StopLoopback() : AATracker->StartLoopback(false);
		break;
	default:
		break;
	}
}

void ABSGameMode::RegisterGun(ABSGun* InGun)
{
	if (InGun)
	{
		InGun->OnShotFired.BindUObject(this, &ABSGameMode::UpdateShotsFired);
	}
}

bool ABSGameMode::InitializeAudioManagers()
{
	AATracker = NewObject<UAudioAnalyzerManager>(this);
	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();

	switch (BSConfig->AudioConfig.AudioFormat)
	{
	case EAudioFormat::File:
		{
			if (!AATracker->InitPlayerAudio(BSConfig->AudioConfig.SongPath))
			{
				return false;
			}
			if (!AudioImporter)
			{
				AudioImporter = URuntimeAudioImporterLibrary::CreateRuntimeAudioImporter();
				AudioImporter->OnResultNative.AddUObject(this, &ABSGameMode::HandleAudioImporterResult);
			}
			AudioImporter->ImportAudioFromFile(BSConfig->AudioConfig.SongPath, ERuntimeAudioFormat::Auto);
		}
		break;
	case EAudioFormat::Capture:
		{
			AATracker->SetDefaultDevicesCapturerAudio(*BSConfig->AudioConfig.InAudioDevice,
				*GameUserSettings->GetAudioOutputDeviceId());
			if (!AATracker->InitCapturerAudioEx(48000, EAA_AudioDepth::B_16, EAA_AudioFormat::Signed_Int, 1.f, false))
			{
				return false;
			}
			if (!AudioCapturer)
			{
				AudioCapturer = UCapturableSoundWave::CreateCapturableSoundWave();
			}
			FOnGetAvailableAudioInputDevicesResultNative Result;
			Result.BindUObject(this, &ABSGameMode::HandleGetAvailableAudioInputDevices);
			AudioCapturer->GetAvailableAudioInputDevices(Result);
		}
		break;
	case EAudioFormat::Loopback:
		{
			AATracker->SetDefaultDevicesCapturerAudio(*BSConfig->AudioConfig.InAudioDevice,
				*GameUserSettings->GetAudioOutputDeviceId());
			AATracker->SetDefaultDeviceLoopbackAudio(*GameUserSettings->GetAudioOutputDeviceId());
			if (!AATracker->InitLoopbackAudio())
			{
				return false;
			}
			if (!AudioCapturer)
			{
				AudioCapturer = UCapturableSoundWave::CreateCapturableSoundWave();
			}
			FOnGetAvailableAudioInputDevicesResultNative Result;
			Result.BindUObject(this, &ABSGameMode::HandleGetAvailableAudioInputDevices);
			AudioCapturer->GetAvailableAudioInputDevices(Result);
		}
		break;
	case EAudioFormat::None: default:
		break;
	}

	AATracker->InitBeatTrackingConfigWLimits(EAA_ChannelSelectionMode::All_in_one, 0, AASettings.BandLimits,
		AASettings.TimeWindow, 10 / AASettings.TimeWindow, false, 100, 2.1);
	AATracker->InitSpectrumConfigWLimits(EAA_ChannelSelectionMode::All_in_one, -1, AASettings.BandLimits,
		AASettings.TimeWindow, 10 / AASettings.TimeWindow, true, AASettings.NumBandChannels);

	AATracker->SetPlaybackVolume(0.0);

	// AAPlayer will only be used if AudioFormat is File and PlayerDelay > 0.01f
	if (BSConfig->AudioConfig.PlayerDelay >= 0.01f)
	{
		AAPlayer = NewObject<UAudioAnalyzerManager>(this);
		if (!AAPlayer->InitPlayerAudio(BSConfig->AudioConfig.SongPath))
		{
			return false;
		}
		AAPlayer->InitBeatTrackingConfigWLimits(EAA_ChannelSelectionMode::All_in_one, 0, AASettings.BandLimits,
			AASettings.TimeWindow, AASettings.HistorySize, false, 100, 2.1);
		AAPlayer->InitSpectrumConfigWLimits(EAA_ChannelSelectionMode::All_in_one, -1, AASettings.BandLimits,
			AASettings.TimeWindow, 10 / AASettings.TimeWindow, true, AASettings.NumBandChannels);
		AAPlayer->SetPlaybackVolume(0.0);
	}

	return true;
}

void ABSGameMode::OnTick_AudioAnalyzers(const float DeltaSeconds)
{
	Elapsed += DeltaSeconds;

	AATracker->GetBeatTrackingWLimitsWThreshold(Beats, SpectrumValues, BpmCurrent, BpmTotal,
		AASettings.BandLimitsThreshold);
	for (const bool Beat : Beats)
	{
		SpawnNewTarget(Beat);
	}

	if (AAPlayer)
	{
		AAPlayer->GetBeatTrackingWLimitsWThreshold(Beats, SpectrumValues, BpmCurrent, BpmTotal,
			AASettings.BandLimitsThreshold);
		AAPlayer->GetBeatTrackingAverageAndVariance(SpectrumVariance, VisualizerManager->AvgSpectrumValues);
	}
	else
	{
		AATracker->GetBeatTrackingAverageAndVariance(SpectrumVariance, VisualizerManager->AvgSpectrumValues);
	}
	VisualizerManager->UpdateVisualizers(SpectrumValues);
}

void ABSGameMode::HandleSecondPassed() const
{
	OnSecondPassed.Broadcast(GetWorldTimerManager().GetTimerElapsed(GameModeLengthTimer));
}

void ABSGameMode::GoToMainMenu()
{
	for (ABSPlayerController* Controller : Controllers)
	{
		Controller->HidePostGameMenu();
		Controller->HidePlayerHUD();
		Controller->HideCrossHair();
	}
	const UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	UGameplayStatics::OpenLevel(GetWorld(), GI->GetMainMenuLevelName());
}

void ABSGameMode::LoadMatchingPlayerScores()
{
	if (BSConfig->AudioConfig.SongLength == 0.f)
	{
		MaxScorePerTarget = 1000.f;
	}
	else
	{
		MaxScorePerTarget = 100000.f / ((BSConfig->AudioConfig.SongLength - 1.f) / BSConfig->TargetConfig.
			TargetSpawnCD);
	}

	for (auto& CurrentPlayerScore : CurrentPlayerScores)
	{
		CurrentPlayerScore.Value = FPlayerScore();
		CurrentPlayerScore.Value.DefiningConfig = BSConfig->DefiningConfig;
		CurrentPlayerScore.Value.SongTitle = BSConfig->AudioConfig.SongTitle;
		CurrentPlayerScore.Value.SongLength = BSConfig->AudioConfig.SongLength;
		CurrentPlayerScore.Value.TotalPossibleDamage = 0.f;

		const TArray<FPlayerScore> PlayerScores = CurrentPlayerScore.Key->LoadPlayerScores().FilterByPredicate(
			[&](const FPlayerScore& PlayerScore)
			{
				if (PlayerScore == CurrentPlayerScore.Value)
				{
					return true;
				}
				return false;
			});

		for (const FPlayerScore& ScoreObject : PlayerScores)
		{
			if (ScoreObject.Score > CurrentPlayerScore.Value.HighScore)
			{
				CurrentPlayerScore.Value.HighScore = ScoreObject.Score;
			}
		}
	}
}

void ABSGameMode::HandleScoreSaving(const bool bExternalSaveScores, const bool bQuitToDesktopAfterSave)
{
	if (!bExternalSaveScores)
	{
		for (auto& CurrentPlayerScore : CurrentPlayerScores)
		{
			CurrentPlayerScore.Value = FPlayerScore();
		}
		return;
	}

	// Get location accuracy from Target Manager
	const FAccuracyData AccuracyData = TargetManager->GetLocationAccuracy();

	for (auto& CurrentPlayerScore : CurrentPlayerScores)
	{
		// Update location accuracy for the current player score
		CurrentPlayerScore.Value.LocationAccuracy = AccuracyData.AccuracyRows;

		// Find or add a Common Score Info instance
		FCommonScoreInfo ScoreInfoInst = CurrentPlayerScore.Key->FindOrAddCommonScoreInfo(BSConfig->DefiningConfig);

		// Update the Common Score Info Accuracy Data
		ScoreInfoInst.UpdateAccuracy(AccuracyData);

		// Update the Common Score Info QTable if settings permit
		if (BSConfig->AIConfig.ReinforcementLearningMode != EReinforcementLearningMode::None)
		{
			TargetManager->UpdateCommonScoreInfoQTable(ScoreInfoInst);
		}

		UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

		const bool bValidToSave = CurrentPlayerScore.Value.IsValidToSave();
		if (bValidToSave)
		{
			// Update Steam Stat for Game Mode
#if UE_BUILD_SHIPPING
			if (TimePlayedGameMode > Constants::MinStatRequirement_Duration_NumGamesPlayed)
			{
				GI->GetSteamManager()->UpdateStat_NumGamesPlayed(
					CurrentPlayerScore.Value.DefiningConfig.GameModeType == EGameModeType::Custom
					? EBaseGameMode::None
					: CurrentPlayerScore.Value.DefiningConfig.BaseGameMode, 1);
			}
#else // !UE_BUILD_SHIPPING
			GI->GetSteamManager()->UpdateStat_NumGamesPlayed(
				CurrentPlayerScore.Value.DefiningConfig.GameModeType == EGameModeType::Custom
				? EBaseGameMode::None
				: CurrentPlayerScore.Value.DefiningConfig.BaseGameMode, 1);
#endif // UE_BUILD_SHIPPING

			// Save common score info and completed scores locally
			CurrentPlayerScore.Key->SaveCommonScoreInfo(BSConfig->DefiningConfig, ScoreInfoInst);
			FinalizePlayerScore(CurrentPlayerScore.Value);
			CurrentPlayerScore.Key->SavePlayerScoreInstance(CurrentPlayerScore.Value);
		}

		CurrentPlayerScore.Value = FPlayerScore();

		// Let game instance handle posting scores to db
		GI->SavePlayerScoresToDatabase(CurrentPlayerScore.Key, bValidToSave, bQuitToDesktopAfterSave);
	}
}

void ABSGameMode::FinalizePlayerScore(FPlayerScore& InScore) const
{
	InScore.Time = FDateTime::UtcNow().ToIso8601();

	if (BSConfig->TargetConfig.TargetDamageType == ETargetDamageType::Tracking)
	{
		InScore.Accuracy = FloatDivide(InScore.Score, InScore.TotalPossibleDamage);
		InScore.Completion = FloatDivide(InScore.Score, InScore.TotalPossibleDamage);
	}
	else
	{
		InScore.AvgTimeOffset = FloatDivide(InScore.TotalTimeOffset, InScore.TargetsHit);
		InScore.Accuracy = FloatDivide(InScore.TargetsHit, InScore.ShotsFired);
		InScore.Completion = FloatDivide(InScore.TargetsHit, InScore.TargetsSpawned);
	}
}

void ABSGameMode::OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings)
{
	if (VisualizerManager)
	{
		VisualizerManager->UpdateVisualizerSettings(GameSettings);
	}
	if (TargetManager)
	{
		TargetManager->UpdatePlayerSettings(GameSettings);
	}
}

void ABSGameMode::OnPlayerSettingsChanged(const FPlayerSettings_AudioAnalyzer& NewAudioAnalyzerSettings)
{
	AASettings = NewAudioAnalyzerSettings;
	if (VisualizerManager)
	{
		VisualizerManager->UpdateAASettings(AASettings);
	}
}

void ABSGameMode::HandlePostTargetDamageEvent(const FTargetDamageEvent& Event)
{
	float NormalizedError = -1.f;
	float Error = -1.f;
	FPlayerScore* Score = nullptr;
	ABSPlayerController* Controller = nullptr;

	if (const ABSCharacterBase* Character = Cast<ABSCharacterBase>(Event.DamageCauser))
	{
		Controller = Character->GetBSPlayerController();
		if (Controller)
		{
			Score = CurrentPlayerScores.Find(Controller);
		}
	}
	if (!Score)
	{
		return;
	}

	switch (Event.DamageType)
	{
	case ETargetDamageType::Tracking:
		{
			Score->TotalPossibleDamage = Event.TotalPossibleTrackingDamage;
			Score->Score += Event.DamageDelta;
		}
		break;
	case ETargetDamageType::Hit:
		{
			Score->Score += GetScoreFromTimeAlive(Event.TimeAlive);
			Score->TotalTimeOffset += GetAbsHitTimingError(Event.TimeAlive);
			Score->TargetsHit++;
			UpdateStreak(Controller, *Score, Event.Streak, Event.Transform);
			// UpdateTimeOffset(TimeOffset, Transform);
			NormalizedError = GetNormalizedHitTimingError(Event.TimeAlive);
			Error = GetHitTimingError(Event.TimeAlive);
		}
		break;
	case ETargetDamageType::Combined:
	case ETargetDamageType::None:
		{
			UE_LOG(LogBSGameMode, Warning,
				TEXT("TargetDamageType of Combined/None received in OnPostTargetDamageEvent."));
		}
		return;
	case ETargetDamageType::Self:
		return;
	}

	// Update high score
	if (Score->Score > Score->HighScore)
	{
		Score->HighScore = Score->Score;
	}

	// Update PlayerHUD
	Controller->UpdatePlayerHUD(*Score, NormalizedError, Error);
}

void ABSGameMode::UpdateTargetsSpawned(const ETargetDamageType& DamageType)
{
	if (DamageType == ETargetDamageType::Hit)
	{
		for (auto& CurrentPlayerScore : CurrentPlayerScores)
		{
			CurrentPlayerScore.Value.TargetsSpawned++;
			CurrentPlayerScore.Key->UpdatePlayerHUD(CurrentPlayerScore.Value, -1.f, -1.f);
		}
	}
}

void ABSGameMode::UpdateShotsFired(ABSPlayerController* Controller)
{
	if (Controller)
	{
		FPlayerScore* Score = CurrentPlayerScores.Find(Controller);
		if (Score)
		{
			Score->ShotsFired++;
			Controller->UpdatePlayerHUD(*Score, -1.f, -1.f);
		}
	}
}

void ABSGameMode::UpdateStreak(ABSPlayerController* Controller, FPlayerScore& InScore, const int32 Streak,
	const FTransform& Transform) const
{
	if (Streak > InScore.Streak)
	{
		InScore.Streak = Streak;
	}
	Controller->ShowCombatText(Streak, Transform);
	FPlayerSettings_User Settings = Controller->GetPlayerSettings().User;
	if (Streak > StreakThreshold && !Settings.bNightModeUnlocked)
	{
		Settings.bNightModeUnlocked = true;
		Controller->SavePlayerSettings(Settings);
	}
}

void ABSGameMode::UpdateTimeOffset(const float TimeOffset, const FTransform& Transform)
{
	for (ABSPlayerController* Controller : Controllers)
	{
		Controller->ShowAccuracyText(TimeOffset, Transform);
	}
}

float ABSGameMode::FloatDivide(const float Numerator, const float Denominator)
{
	if (Denominator == 0)
	{
		return 0;
	}
	return Numerator / Denominator;
}

float ABSGameMode::GetScoreFromTimeAlive(const float InTimeAlive) const
{
	// Perfect shot
	if (GetAbsHitTimingError(InTimeAlive) < Constants::PerfectScoreTimeThreshold / 2.f)
	{
		return MaxScorePerTarget;
	}

	const float MinScorePerTarget = MaxScorePerTarget / 2.f;
	// Early shot
	if (InTimeAlive < BSConfig->TargetConfig.SpawnBeatDelay)
	{
		constexpr float MinEarlyShot = 0.f;
		const float MaxEarlyShot = BSConfig->TargetConfig.SpawnBeatDelay - Constants::PerfectScoreTimeThreshold / 2.f;
		const FVector2d InputRange = FVector2d(MinEarlyShot, MaxEarlyShot);
		const float LerpValue = FMath::GetMappedRangeValueClamped(InputRange, FVector2D(0.f, 1.f), InTimeAlive);

		// interp between half perfect score at MinEarlyShot to perfect score at MaxEarlyShot
		return MinScorePerTarget + LerpValue * (MaxScorePerTarget - MinScorePerTarget);
		//return FMath::Lerp<float>(MaxScorePerTarget / 2.f, MaxScorePerTarget, LerpValue);
	}

	// Late shot
	const float MinLateShot = BSConfig->TargetConfig.SpawnBeatDelay + Constants::PerfectScoreTimeThreshold / 2.f;
	const float MaxLateShot = BSConfig->TargetConfig.TargetMaxLifeSpan;
	const FVector2d InputRange = FVector2d(MinLateShot, MaxLateShot);
	const float LerpValue = FMath::GetMappedRangeValueClamped(InputRange, FVector2D(0.f, 1.f), InTimeAlive);

	// interp between perfect score at MinLateShot to half perfect score at MaxLateShot
	return MaxScorePerTarget + LerpValue * (MinScorePerTarget - MaxScorePerTarget);
	//return FMath::Lerp<float>(MaxScorePerTarget, MaxScorePerTarget / 2.f, LerpValue);
}

float ABSGameMode::GetHitTimingError(const float InTimeAlive) const
{
	return InTimeAlive - BSConfig->TargetConfig.SpawnBeatDelay;
}

float ABSGameMode::GetAbsHitTimingError(const float InTimeAlive) const
{
	return FMath::Abs<float>(InTimeAlive - BSConfig->TargetConfig.SpawnBeatDelay);
}

float ABSGameMode::GetNormalizedHitTimingError(const float InTimeAlive) const
{
	if (InTimeAlive == BSConfig->TargetConfig.SpawnBeatDelay)
	{
		return 0.5f;
	}

	// Early shot
	if (InTimeAlive < BSConfig->TargetConfig.SpawnBeatDelay)
	{
		constexpr float MinEarlyShot = 0.f;
		const float MaxEarlyShot = BSConfig->TargetConfig.SpawnBeatDelay;
		const FVector2d InputRange = FVector2d(MinEarlyShot, MaxEarlyShot);
		return FMath::GetMappedRangeValueClamped(InputRange, FVector2D(0.f, 0.5f), InTimeAlive);
	}

	// Late shot
	const float MinLateShot = BSConfig->TargetConfig.SpawnBeatDelay;
	const float MaxLateShot = BSConfig->TargetConfig.TargetMaxLifeSpan;
	const FVector2d InputRange = FVector2d(MinLateShot, MaxLateShot);
	return FMath::GetMappedRangeValueClamped(InputRange, FVector2D(0.5f, 1.f), InTimeAlive);
}
