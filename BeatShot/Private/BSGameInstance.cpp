// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "BSGameInstance.h"

#include "BSGameMode.h"
#include "BSGameUserSettings.h"
#include "Player/BSPlayerController.h"
#include "System/SteamManager.h"
#include "Kismet/GameplayStatics.h"
#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"
#include "SaveGamePlayerSettings.h"
#include "OverlayWidgets/LoadingScreenWidgets/SLoadingScreenWidget.h"
#include "BSLoadingScreenSettings.h"

void UBSGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnPostLoadMapWithWorld);
	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &ThisClass::OnPreLoadMapWithContext);

	UE_LOG(LogTemp, Display, TEXT("UBSGameInstance::Init"));
	InitializeLoadingScreen();
	InitializeSteamManager();
}

#if WITH_EDITOR
FGameInstancePIEResult UBSGameInstance::PostCreateGameModeForPIE(const FGameInstancePIEParameters& Params,
	AGameModeBase* GameMode)
{
	UBSGameUserSettings::Get()->Initialize(WorldContext->World());
	const FBSConfig LocalConfig = FBSConfig();
	SetBSConfig(LocalConfig);
	return Super::PostCreateGameModeForPIE(Params, GameMode);
}

FGameInstancePIEResult UBSGameInstance::StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer,
	const FGameInstancePIEParameters& Params)
{
	return Super::StartPlayInEditorGameInstance(LocalPlayer, Params);
}
#endif WITH_EDITOR

void UBSGameInstance::OnPreLoadMapWithContext(const FWorldContext& InWorldContext, const FString& /*MapName*/)
{
}

void UBSGameInstance::OnPostLoadMapWithWorld(UWorld* World)
{
	// Fade out the loading screen when map is ready
	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->SetLoadingScreenState(ELoadingScreenState::FadingOut);
	}
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

	if (GetWorld()->GetMapName().Contains(MainMenuLevelName.ToString()))
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
			.OnFadeOutComplete(BIND_UOBJECT_DELEGATE(FOnFadeOutComplete, OnLoadingScreenFadeOutComplete))
			.bIsInitialLoadingScreen(bIsInitialLoadingScreen);
		}
	}
	if (!LoadingScreenWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadingScreenWidget is null"));
	}
	Attributes.WidgetLoadingScreen = LoadingScreenWidget;
	GetMoviePlayer()->SetupLoadingScreen(Attributes);
}

void UBSGameInstance::OnStart()
{
	Super::OnStart();
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

void UBSGameInstance::InitializeLoadingScreen()
{
	GetMoviePlayer()->OnMoviePlaybackStarted().AddLambda([this]()
	{
		const auto GameUserSettings = UBSGameUserSettings::Get();
		GameUserSettings->SetLoadingScreenMixActivationState(true);
	});
	GetMoviePlayer()->OnMoviePlaybackFinished().AddLambda([this]()
	{
		const auto GameUserSettings = UBSGameUserSettings::Get();
		GameUserSettings->SetLoadingScreenMixActivationState(false);
	});
	GetMoviePlayer()->OnPrepareLoadingScreen().AddUObject(this, &ThisClass::PrepareLoadingScreen);

	#if WITH_EDITOR
	OnLoadingScreenFadeOutComplete();
	#endif
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
			}
		}
		break;
	case ETransitionState::QuitToMainMenu:
		{
			// Nothing else to do here
			EndBSGameMode(NewGameModeTransitionState);
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
				UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0),
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

void UBSGameInstance::InitializeSteamManager()
{
	SteamManager = NewObject<USteamManager>(this);
	SteamManager->AssignGameInstance(this);
	SteamManager->InitializeSteamManager();
}

void UBSGameInstance::SavePlayerScoresToDatabase(ABSPlayerController* PC, const bool bWasValidToSave)
{
	// If game mode encountered a reason not to save to database
	if (!bWasValidToSave)
	{
		PC->OnPostScoresResponseReceived("SBW_DidNotSaveScores");
		if (bQuitToDesktopAfterSave)
		{
			UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0),
				EQuitPreference::Quit, false);
		}
		return;
	}

	// No account
	if (PC->GetPlayerSettings().User.RefreshCookie.IsEmpty())
	{
		if (bQuitToDesktopAfterSave)
		{
			UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0),
				EQuitPreference::Quit, false);
		}
		PC->OnPostScoresResponseReceived("SBW_NoAccount");
		return;
	}

	// Acquire access token
	TSharedPtr<FAccessTokenResponse> AccessTokenResponse = MakeShareable(new FAccessTokenResponse());
	AccessTokenResponse->OnHttpResponseReceived.BindLambda([this, AccessTokenResponse, PC]
	{
		if (AccessTokenResponse->OK) // Successful access token retrieval
		{
			TSharedPtr<FBSHttpResponse> PostScoresResponse = MakeShareable(new FBSHttpResponse());
			PostScoresResponse->OnHttpResponseReceived.BindLambda([this, PostScoresResponse, PC]
			{
				check(PC);
				if (PostScoresResponse->OK) // Successful scores post
				{
					SetAllPlayerScoresSavedToDatabase();
					PC->OnPostScoresResponseReceived();
				}
				else // Unsuccessful scores post
				{
					PC->OnPostScoresResponseReceived("SBW_SavedScoresLocallyOnly");
				}

				if (bQuitToDesktopAfterSave)
				{
					UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0),
						EQuitPreference::Quit, false);
				}
			});
			PostPlayerScores(LoadPlayerScores_UnsavedToDatabase(), PC->GetPlayerSettings().User.UserID,
				AccessTokenResponse->AccessToken, PostScoresResponse);
		}
		else // Unsuccessful access token retrieval
		{
			check(PC);
			PC->OnPostScoresResponseReceived("SBW_SavedScoresLocallyOnly");

			if (bQuitToDesktopAfterSave)
			{
				UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0),
					EQuitPreference::Quit, false);
			}
		}
	});
	RequestAccessToken(PC->GetPlayerSettings().User.RefreshCookie, AccessTokenResponse);
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
