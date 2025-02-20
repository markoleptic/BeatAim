// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "BSGameInstance.h"
#include "BSGameMode.h"
#include "BSGameUserSettings.h"
#include "MetasoundGeneratorHandle.h"
#include "MetasoundOutput.h"
#include "MetasoundOutputSubsystem.h"
#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "DeveloperSettings/BSLoadingScreenSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BSPlayerController.h"
#include "SaveGames/SaveGamePlayerSettings.h"
#include "Slate/SLoadingScreenWidget.h"
#include "System/SteamManager.h"
#include "Utilities/GameModeTransitionState.h"
#include "Utilities/TooltipWidget.h"

void UBSGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnPostLoadMapWithWorld);
#if !WITH_EDITOR
	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &ThisClass::OnPreLoadMapWithContext);
#endif // !WITH_EDITOR

	GetMoviePlayer()->OnPrepareLoadingScreen().AddUObject(this, &ThisClass::PrepareLoadingScreen);

	SetBSConfig(FBSConfig());
	SteamManager = NewObject<USteamManager>(this);
	SteamManager->AssignGameInstance(this);
	SteamManager->InitializeSteamManager();
	OnFadeCompleted.BindDynamic(this, &UBSGameInstance::HandleFadeCompleted);
	PlaybackTime.BindDynamic(this, &UBSGameInstance::HandlePlaybackTimeChanged);
}

#if WITH_EDITOR
FGameInstancePIEResult UBSGameInstance::PostCreateGameModeForPIE(const FGameInstancePIEParameters& Params,
	AGameModeBase* GameMode)
{
	FGameInstancePIEResult Result = Super::PostCreateGameModeForPIE(Params, GameMode);
	UBSGameUserSettings::Get()->Initialize(WorldContext->World());
	InitializeAudioComponent(WorldContext->World());
	return Result;
}

FGameInstancePIEResult UBSGameInstance::StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer,
	const FGameInstancePIEParameters& Params)
{
	UTooltipWidget::InitializeTooltipWidget(TooltipClass);
	return Super::StartPlayInEditorGameInstance(LocalPlayer, Params);
}

#else // WITH_EDITOR
void UBSGameInstance::OnPreLoadMapWithContext(const FWorldContext& InWorldContext, const FString& /*MapName*/)
{
	UBSGameUserSettings::Get()->Initialize(InWorldContext.World());
	UTooltipWidget::InitializeTooltipWidget(TooltipClass);
	InitializeAudioComponent(InWorldContext.World());
}
#endif // WITH_EDITOR

void UBSGameInstance::OnPostLoadMapWithWorld(UWorld* /*World*/)
{
	RemoveLoadingScreen();
}

void UBSGameInstance::OnLoadingScreenFadeOutComplete()
{
	// Widget has completed fade out, so can be removed
	GetMoviePlayer()->StopMovie();
	if (LoadingScreenWidget.IsValid())
	{
		LoadingScreenWidget.Reset();
	}

	// No longer the initial loading screen
	bIsInitialLoadingScreen = false;

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Equals(MainMenuLevelName.ToString()))
	{
		if (ABSPlayerController* PC = Cast<ABSPlayerController>(GetFirstLocalPlayerController(GetWorld())))
		{
			PC->FadeScreenFromBlack();
		}
	}
}

void UBSGameInstance::PrepareLoadingScreen()
{
	FLoadingScreenAttributes Attributes;
	Attributes.bAutoCompleteWhenLoadingCompletes = false;
	Attributes.bAllowEngineTick = true;
	Attributes.bMoviesAreSkippable = false;
	Attributes.bWaitForManualStop = true;
	const UBSLoadingScreenSettings* LoadingScreenSettings = GetDefault<UBSLoadingScreenSettings>();
	Attributes.MinimumLoadingScreenDisplayTime = LoadingScreenSettings->MinimumLoadingScreenDisplayTime;
	if (LoadingScreenStyle && LoadingScreenStyle->CustomStyle)
	{
		if (const FLoadingScreenStyle* Style = static_cast<const struct FLoadingScreenStyle*>(LoadingScreenStyle->
			CustomStyle->GetStyle()))
		{
			SAssignNew(LoadingScreenWidget, SLoadingScreenWidget)
			.LoadingScreenStyle(Style)
			.OnFadeOutComplete(BIND_UOBJECT_DELEGATE(FOnFadeOutComplete, OnLoadingScreenFadeOutComplete));
		}
	}
	Attributes.WidgetLoadingScreen = LoadingScreenWidget;
	GetMoviePlayer()->SetupLoadingScreen(Attributes);
}

void UBSGameInstance::OnStart()
{
	Super::OnStart();
}

void UBSGameInstance::Shutdown()
{
	UTooltipWidget::Cleanup();
	Super::Shutdown();
}

void UBSGameInstance::SetBSConfig(const FBSConfig& InConfig)
{
	if (!BSConfig.IsValid())
	{
		BSConfig = MakeShareable(new FBSConfig(InConfig));
	}
	else
	{
		*BSConfig = InConfig;
	}
}

void UBSGameInstance::HandleGameModeTransition(const FGameModeTransitionState& NewGameModeTransitionState)
{
	switch (NewGameModeTransitionState.TransitionState)
	{
	case ETransitionState::StartFromMainMenu:
		{
			SetBSConfig(NewGameModeTransitionState.BSConfig);
			if (ABSPlayerController* PC = Cast<ABSPlayerController>(GetFirstLocalPlayerController(GetWorld())))
			{
				PC->OnScreenFadeToBlackFinish.BindLambda([this]
				{
					UGameplayStatics::OpenLevel(GetWorld(), RangeLevelName);
				});
				PC->FadeScreenToBlack();
				SetLoadingScreenAudioComponentState(0.75f, 0.25f);
			}
		}
		break;
	case ETransitionState::QuitToMainMenu:
		{
			EndBSGameMode(NewGameModeTransitionState);
			SetLoadingScreenAudioComponentState(0.75f, 0.25f);
		}
		break;
	case ETransitionState::StartFromPostGameMenu:
		{
			// Update Game Mode configuration
			SetBSConfig(NewGameModeTransitionState.BSConfig);
			EndBSGameMode(NewGameModeTransitionState);
		}
		break;
	case ETransitionState::Restart:
		{
			// Nothing else to do here
			EndBSGameMode(NewGameModeTransitionState);
		}
		break;
	case ETransitionState::QuitToDesktop:
		{
			EndBSGameMode(NewGameModeTransitionState);
			// Can exit immediately if not saving scores, otherwise SavePlayerScoresToDatabase will handle it
			if (!NewGameModeTransitionState.bSaveCurrentScores)
			{
				UKismetSystemLibrary::QuitGame(GetWorld(), GetFirstLocalPlayerController(GetWorld()),
					EQuitPreference::Quit, false);
			}
		}
		break;
	case ETransitionState::PlayAgain:
		{
			BSConfig->AudioConfig = NewGameModeTransitionState.BSConfig.AudioConfig;
			BSConfig->OnCreate();
			EndBSGameMode(NewGameModeTransitionState);
		}
		break;
	case ETransitionState::None:
		break;
	}
}

void UBSGameInstance::EndBSGameMode(const FGameModeTransitionState& NewGameModeTransitionState) const
{
	if (ABSGameMode* GM = Cast<ABSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->EndGameMode(NewGameModeTransitionState.bSaveCurrentScores, NewGameModeTransitionState.TransitionState);
	}
}

void UBSGameInstance::InitializeAudioComponent(const UWorld* World)
{
	if (LoadingScreenAudioComponent)
	{
		return;
	}
	if (LoadingScreenSound)
	{
		LoadingScreenAudioComponent = UGameplayStatics::CreateSound2D(World, LoadingScreenSound, 1, 1, 0, nullptr, true,
			true);
	}
}

void UBSGameInstance::SetLoadingScreenAudioComponentState(const float FadeTarget, const float FadeDuration)
{
	if (!LoadingScreenAudioComponent)
	{
		InitializeAudioComponent(GetWorld());
	}
	if (LoadingScreenAudioComponent)
	{
		LoadingScreenAudioComponent->SetFloatParameter(FName("FadeTarget"), FadeTarget);
		LoadingScreenAudioComponent->SetFloatParameter(FName("FadeDuration"), FadeDuration);

		if (!LoadingScreenAudioComponent->IsPlaying())
		{
			if (LastPlaybackPosition > 0.f)
			{
				LoadingScreenAudioComponent->SetFloatParameter(FName("StartTime"), LastPlaybackPosition);
				LastPlaybackPosition = -1.f;
			}
			LoadingScreenAudioComponent->Play();
		}
		else
		{
			LoadingScreenAudioComponent->SetTriggerParameter(FName("Fade"));
		}
		if (UMetaSoundOutputSubsystem* Subsystem = GetWorld()->GetSubsystem<UMetaSoundOutputSubsystem>())
		{
			if (!Subsystem->IsTickable())
			{
				Subsystem->WatchOutput(LoadingScreenAudioComponent.Get(), FName("PlaybackTime"), PlaybackTime);
				Subsystem->WatchOutput(LoadingScreenAudioComponent.Get(), FName("OnFadeCompleted"), OnFadeCompleted);
			}
		}
	}
}

void UBSGameInstance::SavePlayerScoresToDatabase(ABSPlayerController* PlayerController, const bool bWasValidToSave,
	const bool bQuitToDesktopAfterSave) const
{
	const FPlayerSettings_User PlayerSettings = PlayerController->GetPlayerSettings().User;
	// If game mode encountered a reason not to save to database
	if (!bWasValidToSave)
	{
		PlayerController->OnPostScoresResponseReceived("SBW_DidNotSaveScores");
		if (bQuitToDesktopAfterSave)
		{
			UKismetSystemLibrary::QuitGame(GetWorld(), GetFirstLocalPlayerController(GetWorld()), EQuitPreference::Quit,
				false);
		}
		return;
	}

	// No account
	if (PlayerSettings.RefreshCookie.IsEmpty())
	{
		if (bQuitToDesktopAfterSave)
		{
			UKismetSystemLibrary::QuitGame(GetWorld(), GetFirstLocalPlayerController(GetWorld()), EQuitPreference::Quit,
				false);
		}
		PlayerController->OnPostScoresResponseReceived("SBW_NoAccount");
		return;
	}

	// Acquire access token
	TSharedPtr<FAccessTokenResponse> AccessTokenResponse = MakeShareable(new FAccessTokenResponse());
	AccessTokenResponse->OnHttpResponseReceived.BindLambda(
		[this, AccessTokenResponse, UserID = PlayerSettings.UserID, bQuitToDesktopAfterSave]
		{
			if (AccessTokenResponse->OK) // Successful access token retrieval
			{
				TSharedPtr<FBSHttpResponse> PostScoresResponse = MakeShareable(new FBSHttpResponse());
				PostScoresResponse->OnHttpResponseReceived.BindLambda(
					[this, PostScoresResponse, bQuitToDesktopAfterSave]
					{
						if (ABSPlayerController* Controller = Cast<ABSPlayerController>(
							GetFirstLocalPlayerController(GetWorld())))
						{
							if (PostScoresResponse->OK) // Successful scores post
							{
								SetAllPlayerScoresSavedToDatabase();
								Controller->OnPostScoresResponseReceived();
							}
							else // Unsuccessful scores post
							{
								Controller->OnPostScoresResponseReceived("SBW_SavedScoresLocallyOnly");
							}

							if (bQuitToDesktopAfterSave)
							{
								UKismetSystemLibrary::QuitGame(GetWorld(), Controller, EQuitPreference::Quit, false);
							}
						}
					});
				PostPlayerScores(LoadPlayerScores_UnsavedToDatabase(), UserID, AccessTokenResponse->AccessToken,
					PostScoresResponse);
			}
			else // Unsuccessful access token retrieval
			{
				if (ABSPlayerController* Controller = Cast<ABSPlayerController>(
					GetFirstLocalPlayerController(GetWorld())))
				{
					Controller->OnPostScoresResponseReceived("SBW_SavedScoresLocallyOnly");
					if (bQuitToDesktopAfterSave)
					{
						UKismetSystemLibrary::QuitGame(GetWorld(), Controller, EQuitPreference::Quit, false);
					}
				}
			}
		});
	RequestAccessToken(PlayerSettings.RefreshCookie, AccessTokenResponse);
}

void UBSGameInstance::RemoveLoadingScreen()
{
	// Fade out the loading screen when map is ready
	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->SetLoadingScreenState(ELoadingScreenState::FadingOut);
	}

	const FString MapName = UGameplayStatics::GetCurrentLevelName(WorldContext->World());
	const float FadeTarget = MapName.Equals(MainMenuLevelName.ToString()) ? 1.f : 0.f;
	const float FadeDuration = bIsInitialLoadingScreen ? 2.f : 0.75f;
	SetLoadingScreenAudioComponentState(FadeTarget, FadeDuration);
}

void UBSGameInstance::OnSteamOverlayIsOn()
{
	IsSteamOverlayActive = true;
	this->OnSteamOverlayIsActive(true);
}

void UBSGameInstance::OnSteamOverlayIsOff()
{
	IsSteamOverlayActive = false;
	this->OnSteamOverlayIsActive(false);
}

void UBSGameInstance::OnSteamOverlayIsActive(bool bIsOverlayActive) const
{
	if (bIsOverlayActive)
	{
		if (ABSPlayerController* PC = Cast<ABSPlayerController>(GetFirstLocalPlayerController(GetWorld())))
		{
			PC->HandlePause();
		}
	}
}

void UBSGameInstance::OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings)
{
	OnPlayerSettingsChangedDelegate_Game.Broadcast(GameSettings);
}

void UBSGameInstance::OnPlayerSettingsChanged(const FPlayerSettings_AudioAnalyzer& AudioAnalyzerSettings)
{
	OnPlayerSettingsChangedDelegate_AudioAnalyzer.Broadcast(AudioAnalyzerSettings);
}

void UBSGameInstance::OnPlayerSettingsChanged(const FPlayerSettings_User& UserSettings)
{
	OnPlayerSettingsChangedDelegate_User.Broadcast(UserSettings);
}

void UBSGameInstance::OnPlayerSettingsChanged(const FPlayerSettings_CrossHair& CrossHairSettings)
{
	OnPlayerSettingsChangedDelegate_CrossHair.Broadcast(CrossHairSettings);
}

void UBSGameInstance::HandleFadeCompleted(FName /*OutputName*/, const FMetaSoundOutput& Output)
{
	if (float FadeTarget; Output.Get(FadeTarget))
	{
		if (FadeTarget <= 0.f)
		{
			if (LoadingScreenAudioComponent)
			{
				LoadingScreenAudioComponent->Stop();
			}
		}
	}
}

void UBSGameInstance::HandlePlaybackTimeChanged(FName /*OutputName*/, const FMetaSoundOutput& Output)
{
	if (float PlaybackPosition; Output.Get(PlaybackPosition))
	{
		if (!LoadingScreenAudioComponent->IsPlaying())
		{
			LastPlaybackPosition = PlaybackPosition;
		}
		else
		{
			LastPlaybackPosition = -1.f;
		}
	}
}
