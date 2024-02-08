// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Player/BSPlayerController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/BSAbilitySystemComponent.h"
#include "Character/BSCharacterBase.h"
#include "BSGameInstance.h"
#include "BSGameMode.h"
#include "FloatingTextActor.h"
#include "MainMenuGameMode.h"
#include "BeatShot/BSGameplayTags.h"
#include "Player/BSPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "MenuWidgets/MainMenuWidget.h"
#include "MenuWidgets/PauseMenuWidget.h"
#include "MenuWidgets/PostGameMenuWidget.h"
#include "OverlayWidgets/LoadingScreenWidgets/CountdownWidget.h"
#include "OverlayWidgets/HUDWidgets/CrossHairWidget.h"
#include "OverlayWidgets/HUDWidgets/FPSCounterWidget.h"
#include "OverlayWidgets/HUDWidgets/PlayerHUD.h"
#include "OverlayWidgets/HUDWidgets/RLAgentWidget.h"
#include "OverlayWidgets/LoadingScreenWidgets/ScreenFadeWidget.h"
#include "OverlayWidgets/PopupWidgets/QuitMenuWidget.h"
#include "SubMenuWidgets/ScoreBrowserWidget.h"
#include "SubMenuWidgets/SettingsWidgets/SettingsMenuWidget.h"
#include "SubMenuWidgets/GameModesWidgets/CGMW_CreatorView.h"
#include "System/SteamManager.h"

void ABSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	/*if (HasAuthority())
	{
		if (IsLocalController())
		{
			UE_LOG(LogTemp, Display, TEXT("Local Controller %s: Has Authority: true, %s %s"), *GetNameSafe(this),
				*UEnum::GetValueAsString(GetLocalRole()), *UEnum::GetValueAsString(GetRemoteRole()));
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("NotLocal Controller %s: Has Authority: true, %s %s"), *GetNameSafe(this),
				*UEnum::GetValueAsString(GetLocalRole()), *UEnum::GetValueAsString(GetRemoteRole()));
		}
	}
	else
	{
		if (IsLocalController())
		{
			UE_LOG(LogTemp, Display, TEXT("Local Controller %s: Has Authority: false, %s %s"), *GetNameSafe(this),
				*UEnum::GetValueAsString(GetLocalRole()), *UEnum::GetValueAsString(GetRemoteRole()));
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("NotLocal Controller %s: Has Authority: false, %s %s"), *GetNameSafe(this),
				*UEnum::GetValueAsString(GetLocalRole()), *UEnum::GetValueAsString(GetRemoteRole()));
		}
	}*/

	const FPlayerSettings Settings = LoadPlayerSettings();
	if (Settings.VideoAndSound.bShowFPSCounter)
	{
		ShowFPSCounter();
	}
	CombatTextFrequency = Settings.Game.CombatTextFrequency;
	bShowStreakCombatText = Settings.Game.bShowStreakCombatText;
	bNightModeUnlocked = Settings.User.bNightModeUnlocked;

	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	GI->AddDelegateToOnPlayerSettingsChanged(OnPlayerSettingsChangedDelegate_VideoAndSound);
	GI->GetPublicVideoAndSoundSettingsChangedDelegate().AddUObject(this, &ABSPlayerController::OnPlayerSettingsChanged);
	GI->GetPublicGameSettingsChangedDelegate().AddUObject(this, &ABSPlayerController::OnPlayerSettingsChanged);

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("MainMenu"))
	{
		ShowMainMenu();
	}
}

void ABSPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PreProcessInput(DeltaTime, bGamePaused);
}

void ABSPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);
	if (!IsLocalController() || !HasAuthority())
	{
		return;
	}
	GetBSAbilitySystemComponent()->ProcessAbilityInput(DeltaTime, bGamePaused);
}

ABSPlayerState* ABSPlayerController::GetBSPlayerState() const
{
	return CastChecked<ABSPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UBSAbilitySystemComponent* ABSPlayerController::GetBSAbilitySystemComponent() const
{
	const ABSPlayerState* PS = GetBSPlayerState();
	return (PS ? PS->GetBSAbilitySystemComponent() : nullptr);
}

ABSCharacterBase* ABSPlayerController::GetBSCharacter() const
{
	return GetCharacter() ? Cast<ABSCharacterBase>(GetCharacter()) : nullptr;
}

void ABSPlayerController::SetPlayerEnabledState(const bool bPlayerEnabled)
{
	if (GetWorld()->GetMapName().Contains("Range"))
	{
		if (bPlayerEnabled)
		{
			GetPawn()->EnableInput(this);
			if (GetBSAbilitySystemComponent())
			{
				GetBSAbilitySystemComponent()->RemoveLooseGameplayTag(FBSGameplayTags::Get().Ability_InputBlocked);
			}
		}
		else
		{
			GetPawn()->DisableInput(this);
			if (GetBSAbilitySystemComponent())
			{
				GetBSAbilitySystemComponent()->AddLooseGameplayTag(FBSGameplayTags::Get().Ability_InputBlocked);
			}
		}
	}
}

void ABSPlayerController::ShowMainMenu()
{
	if (!IsLocalController()) return;
	SetInputMode(FInputModeUIOnly());
	SetShowMouseCursor(true);
	
	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	
	MainMenuWidget = CreateWidget<UMainMenuWidget>(this, MainMenuClass);
	MainMenuWidget->AddToViewport();
	MainMenuWidget->GameModesWidget->OnGameModeStateChanged.AddUObject(GI, &UBSGameInstance::HandleGameModeTransition);
	MainMenuWidget->OnSteamLoginRequest.BindUObject(this, &ThisClass::InitiateSteamLogin);

	if (AMainMenuGameMode* GameMode = Cast<AMainMenuGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->SetupTargetManager(MainMenuWidget->GameModesWidget);
	}

	GI->AddDelegateToOnPlayerSettingsChanged(MainMenuWidget->SettingsMenuWidget->GetGameDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(MainMenuWidget->SettingsMenuWidget->GetVideoAndSoundDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(MainMenuWidget->SettingsMenuWidget->GetCrossHairDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(MainMenuWidget->SettingsMenuWidget->GetAudioAnalyzerDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(MainMenuWidget->SettingsMenuWidget->GetUserDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(MainMenuWidget->GetUserDelegate());

	UGameUserSettings::GetGameUserSettings()->SetFrameRateLimit(LoadPlayerSettings().VideoAndSound.FrameRateLimitMenu);
	UGameUserSettings::GetGameUserSettings()->ApplySettings(false);

	if (!bIsLoggedIn)
	{
		LoginUser();
	}
	else
	{
		MainMenuWidget->LoginScoresWidgetSubsequent();
	}
}

void ABSPlayerController::HideMainMenu()
{
	if (MainMenuWidget && IsLocalController())
	{
		MainMenuWidget->RemoveFromParent();
		MainMenuWidget = nullptr;
	}
}

void ABSPlayerController::ShowPauseMenu()
{
	if (!IsLocalController()) return;
	PauseMenuWidget = CreateWidget<UPauseMenuWidget>(this, PauseMenuClass);
	PauseMenuWidget->ResumeGame.BindLambda([this]
	{
		HandlePause();
	});

	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	GI->AddDelegateToOnPlayerSettingsChanged(PauseMenuWidget->SettingsMenuWidget->GetGameDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PauseMenuWidget->SettingsMenuWidget->GetVideoAndSoundDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PauseMenuWidget->SettingsMenuWidget->GetCrossHairDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PauseMenuWidget->SettingsMenuWidget->GetAudioAnalyzerDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PauseMenuWidget->SettingsMenuWidget->GetUserDelegate());

	PauseMenuWidget->QuitMenuWidget->OnGameModeStateChanged.AddUObject(GI, &UBSGameInstance::HandleGameModeTransition);
	PauseMenuWidget->AddToViewport();

	UGameUserSettings::GetGameUserSettings()->SetFrameRateLimit(LoadPlayerSettings().VideoAndSound.FrameRateLimitMenu);
	UGameUserSettings::GetGameUserSettings()->ApplySettings(false);
}

void ABSPlayerController::HidePauseMenu()
{
	if (PauseMenuWidget && IsLocalController())
	{
		PauseMenuWidget->RemoveFromParent();
		PauseMenuWidget = nullptr;
		UGameUserSettings::GetGameUserSettings()->SetFrameRateLimit(
			LoadPlayerSettings().VideoAndSound.FrameRateLimitGame);
		UGameUserSettings::GetGameUserSettings()->ApplySettings(false);
	}
}

void ABSPlayerController::ShowCrossHair()
{
	if (!IsLocalController()) return;
	CrossHairWidget = CreateWidget<UCrossHairWidget>(this, CrossHairClass);

	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	GI->GetPublicCrossHairSettingsChangedDelegate().AddUObject(CrossHairWidget,
		&UCrossHairWidget::OnPlayerSettingsChanged_CrossHair);

	CrossHairWidget->AddToViewport();
}

void ABSPlayerController::HideCrossHair()
{
	if (CrossHairWidget && IsLocalController())
	{
		CrossHairWidget->RemoveFromParent();
		CrossHairWidget = nullptr;
	}
}

void ABSPlayerController::ShowPlayerHUD()
{
	if (!IsLocalController()) return;
	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	
	PlayerHUDWidget = CreateWidget<UPlayerHUD>(this, PlayerHUDClass);
	check(GI->GetBSConfig())
	PlayerHUDWidget->Init(GI->GetBSConfig());
	
	GI->AddDelegateToOnPlayerSettingsChanged(PlayerHUDWidget->GetGameDelegate());
	GI->GetPublicGameSettingsChangedDelegate().AddUObject(PlayerHUDWidget, &UPlayerHUD::OnPlayerSettingsChanged_Game);

	ABSGameMode* GameMode = Cast<ABSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->OnSecondPassed.AddUObject(PlayerHUDWidget, &UPlayerHUD::UpdateSongProgress);

	PlayerHUDWidget->AddToViewport();
}

void ABSPlayerController::HidePlayerHUD()
{
	if (PlayerHUDWidget && IsLocalController())
	{
		PlayerHUDWidget->RemoveFromParent();
		PlayerHUDWidget = nullptr;
	}
	HideRLAgentWidget();
}

void ABSPlayerController::UpdatePlayerHUD(const FPlayerScore& PlayerScore, const float TimeOffsetNormalized,
	const float TimeOffsetRaw)
{
	if (PlayerHUDWidget && IsLocalController())
	{
		PlayerHUDWidget->UpdateAllElements(PlayerScore, TimeOffsetNormalized, TimeOffsetRaw);
	}
}

void ABSPlayerController::ShowCountdown()
{
	if (!IsLocalController()) return;
	SetControlRotation(FRotator(0, 0, 0));
	ABSCharacterBase* BSCharacter = GetBSCharacter();
	check(BSCharacter);
	
	BSCharacter->SetActorLocationAndRotation(FVector(1580, 0, 102), FRotator(0, 0, 0));
	
	CountdownWidget = CreateWidget<UCountdownWidget>(this, CountdownClass);
	ABSGameMode* GameMode = Cast<ABSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	CountdownWidget->OnCountdownCompleted.BindUObject(GameMode, &ABSGameMode::StartGameMode);
	CountdownWidget->StartAAManagerPlayback.BindUObject(GameMode, &ABSGameMode::StartAAManagerPlayback);
	CountdownWidget->AddToViewport();
	UGameUserSettings::GetGameUserSettings()->SetFrameRateLimit(LoadPlayerSettings().VideoAndSound.FrameRateLimitGame);
	UGameUserSettings::GetGameUserSettings()->ApplySettings(false);

	BSCharacter->BindLeftClick();
}

void ABSPlayerController::HideCountdown()
{
	if (CountdownWidget && IsLocalController())
	{
		CountdownWidget->RemoveFromParent();
		CountdownWidget = nullptr;
	}
}

void ABSPlayerController::ShowPostGameMenu()
{
	if (!IsLocalController()) return;
	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	PostGameMenuWidget = CreateWidget<UPostGameMenuWidget>(this, PostGameMenuWidgetClass);
	PostGameMenuWidget->GameModesWidget->OnGameModeStateChanged.AddUObject(GI,
		&UBSGameInstance::HandleGameModeTransition);
	PostGameMenuWidget->QuitMenuWidget->OnGameModeStateChanged.AddUObject(GI,
		&UBSGameInstance::HandleGameModeTransition);

	GI->AddDelegateToOnPlayerSettingsChanged(PostGameMenuWidget->SettingsMenuWidget->GetGameDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PostGameMenuWidget->SettingsMenuWidget->GetVideoAndSoundDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PostGameMenuWidget->SettingsMenuWidget->GetCrossHairDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PostGameMenuWidget->SettingsMenuWidget->GetAudioAnalyzerDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PostGameMenuWidget->SettingsMenuWidget->GetUserDelegate());
	
	PostGameMenuWidget->AddToViewport();

	SetInputMode(FInputModeUIOnly());
	SetShowMouseCursor(true);
	SetPlayerEnabledState(false);

	UGameUserSettings::GetGameUserSettings()->SetFrameRateLimit(LoadPlayerSettings().VideoAndSound.FrameRateLimitMenu);
	UGameUserSettings::GetGameUserSettings()->ApplySettings(false);
}

void ABSPlayerController::OnPostScoresResponseReceived(const FString& StringTableKey)
{
	if (!PostGameMenuWidget) return;
	
	PostGameMenuWidget->ScoresWidget->InitScoreBrowser(EScoreBrowserType::PostGameModeMenuScores, StringTableKey);
}

void ABSPlayerController::HandlePause()
{
	if (PostGameMenuWidget)
	{
		return;
	}

	const UWorld* World = GetWorld();
	ABSGameMode* GameMode = Cast<ABSGameMode>(UGameplayStatics::GetGameMode(World));

	if (!World || !GameMode)
	{
		return;
	}

	if (UGameplayStatics::IsGamePaused(World))
	{
		HidePauseMenu();
		UGameplayStatics::SetGamePaused(World, false);
		if (CountdownWidget)
		{
			CountdownWidget->SetCountdownPaused(false);
		}
		GameMode->PauseAAManager(false);
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
	}
	else
	{
		ShowPauseMenu();
		UGameplayStatics::SetGamePaused(World, true);
		if (CountdownWidget)
		{
			CountdownWidget->SetCountdownPaused(true);
		}
		GameMode->PauseAAManager(true);
		SetInputMode(FInputModeGameAndUI());
		SetShowMouseCursor(true);
	}
}

void ABSPlayerController::HandleLeftClick()
{
	if (GetBSCharacter())
	{
		GetBSCharacter()->UnbindLeftClick();
	}
	if (CountdownWidget)
	{
		UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		if (GI && GI->GetBSConfig().IsValid())
		{
			CountdownWidget->StartCountdown(Constants::CountdownTimerLength, GI->GetBSConfig()->AudioConfig.PlayerDelay);
		}
	}
}

void ABSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABSPlayerState* PS = GetPlayerState<ABSPlayerState>();
	if (PS)
	{
		// Init ASC with PS (Owner) and our new Pawn (AvatarActor)
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, InPawn);
	}
}

void ABSPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void ABSPlayerController::LoginUser()
{
	if (!MainMenuWidget) return;

	const UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	check(GI);

	const TObjectPtr<USteamManager> SteamManager = GI->GetSteamManager();
	check(SteamManager);

	TSharedPtr<FOnAuthTicketForWebApiResponseCallbackHandler> CallbackHandler(
		new FOnAuthTicketForWebApiResponseCallbackHandler());

	// First get auth ticket for web api
	CallbackHandler->OnAuthTicketForWebApiReady.BindLambda([this, CallbackHandler]
	{
		if (CallbackHandler->Result != k_EResultOK)
		{
			if (SteamUser()) SteamUser()->CancelAuthTicket(CallbackHandler->Handle);
			MainMenuWidget->UpdateLoginState(false, "SignInState_SteamSignInFailed");
			MainMenuWidget->TryFallbackLogin();
		}
		else
		{
			// Get display name, user id, and refresh token from BeatShot api request
			TSharedPtr<FSteamAuthTicketResponse> SteamAuthTicketResponse(new FSteamAuthTicketResponse());
			SteamAuthTicketResponse->OnHttpResponseReceived.BindLambda([this, SteamAuthTicketResponse, CallbackHandler]
			{
				if (!SteamAuthTicketResponse->bConnectedSuccessfully) return;
				const uint64 LocalSteamID = SteamUser()->GetSteamID().ConvertToUint64();
				const uint64 ResponseSteamID = FCString::Atoi64(*SteamAuthTicketResponse->SteamID);

				if (LocalSteamID == ResponseSteamID)
				{
					FPlayerSettings_User PlayerSettings = LoadPlayerSettings().User;
					PlayerSettings.DisplayName = FString(SteamFriends()->GetPersonaName());
					PlayerSettings.UserID = SteamAuthTicketResponse->SteamID;
					PlayerSettings.RefreshCookie = SteamAuthTicketResponse->RefreshCookie;
					SavePlayerSettings(PlayerSettings);
					bIsLoggedIn = true;
				}
				
				AsyncTask(ENamedThreads::GameThread, [this, CallbackHandler]
				{
					TryResetAuthTicketHandle(CallbackHandler->Handle);
				});
			});
			AuthenticateSteamUser(CallbackHandler->Ticket, SteamAuthTicketResponse);
			
			// This will be OnlineAsyncTaskThreadSteam, need GameThread for TimerManager later on
			AsyncTask(ENamedThreads::GameThread, [this, CallbackHandler]
			{
				FDelegateHandle Handle = MainMenuWidget->ScoresWidget->OnURLChangedResult.AddLambda(
				[this, &Handle, CallbackHandler](const bool bSuccess)
					{
						TryResetAuthTicketHandle(CallbackHandler->Handle);
						Handle.Reset();
					});
				// Login to the in-game web browser using the redirect url from the auth ticket for web api
				MainMenuWidget->LoginScoresWidgetWithSteam(CallbackHandler->Ticket);
			});
		}
	});
	// Could fail if not logged in to Steam
	if (!SteamManager->CreateAuthTicketForWebApi(CallbackHandler))
	{
		MainMenuWidget->UpdateLoginState(false, "SignInState_SteamSignInFailed");
		MainMenuWidget->TryFallbackLogin();
	}
}

void ABSPlayerController::InitiateSteamLogin()
{
	LoginUser();
}

void ABSPlayerController::HidePostGameMenu()
{
	if (PostGameMenuWidget && IsLocalController())
	{
		PostGameMenuWidget->RemoveFromParent();
		PostGameMenuWidget = nullptr;
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
		SetPlayerEnabledState(true);
	}
}

void ABSPlayerController::ShowFPSCounter()
{
	if (FPSCounterWidget == nullptr && IsLocalController())
	{
		FPSCounterWidget = CreateWidget<UFPSCounterWidget>(this, FPSCounterClass);
		FPSCounterWidget->AddToViewport(ZOrderFPSCounter);
	}
}

void ABSPlayerController::HideFPSCounter()
{
	if (FPSCounterWidget && IsLocalController())
	{
		FPSCounterWidget->RemoveFromParent();
		FPSCounterWidget = nullptr;
	}
}

void ABSPlayerController::CreateScreenFadeWidget(const float StartOpacity)
{
	if (!ScreenFadeWidget && IsLocalController())
	{
		ScreenFadeWidget = CreateWidget<UScreenFadeWidget>(this, ScreenFadeClass);
		ScreenFadeWidget->OnFadeToBlackFinish.AddLambda([this]
		{
			if (OnScreenFadeToBlackFinish.IsBound())
			{
				OnScreenFadeToBlackFinish.Execute();
			}
		});
		ScreenFadeWidget->OnFadeFromBlackFinish.AddUObject(this, &ABSPlayerController::OnFadeScreenFromBlackFinish);
		ScreenFadeWidget->SetStartOpacity(StartOpacity);
		ScreenFadeWidget->AddToViewport(ZOrderFadeScreen);
	}
}

void ABSPlayerController::FadeScreenToBlack()
{
	if (!IsLocalController()) return;
	if (!ScreenFadeWidget)
	{
		CreateScreenFadeWidget(0.f);
	}
	if (!GetWorld()->GetMapName().Contains("Range"))
	{
		if (AMainMenuGameMode* MainMenuGameMode = Cast<AMainMenuGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			MainMenuGameMode->FadeOutMainMenuMusic(ScreenFadeWidgetAnimationDuration);
		}
	}
	ScreenFadeWidget->FadeToBlack(ScreenFadeWidgetAnimationDuration);
}

void ABSPlayerController::FadeScreenFromBlack()
{
	if (!IsLocalController()) return;
	if (!ScreenFadeWidget)
	{
		CreateScreenFadeWidget(1.f);
	}
	if (GetWorld()->GetMapName().Contains("MainMenu"))
	{
		if (AMainMenuGameMode* MainMenuGameMode = Cast<AMainMenuGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			MainMenuGameMode->FadeInMainMenuMusic(2.f);
		}
	}
	ScreenFadeWidget->FadeFromBlack(ScreenFadeWidgetAnimationDuration);
}

void ABSPlayerController::OnFadeScreenFromBlackFinish()
{
	if (ScreenFadeWidget)
	{
		ScreenFadeWidget->OnFadeFromBlackFinish.RemoveAll(this);
		ScreenFadeWidget->RemoveFromParent();
		ScreenFadeWidget = nullptr;
	}
}

void ABSPlayerController::ShowInteractInfo()
{
	if (!InteractInfoWidget && IsLocalController())
	{
		InteractInfoWidget = CreateWidget<UUserWidget>(this, InteractInfoWidgetClass);
		InteractInfoWidget->AddToViewport(ZOrderFPSCounter);
	}
}

void ABSPlayerController::HideInteractInfo()
{
	if (InteractInfoWidget && IsLocalController())
	{
		InteractInfoWidget->RemoveFromParent();
		InteractInfoWidget = nullptr;
	}
}

void ABSPlayerController::ShowRLAgentWidget(FOnQTableUpdate& OnQTableUpdate, const int32 Rows, const int32 Columns,
	const TArray<float>& QTable)
{
	if (!RLAgentWidget && IsLocalController())
	{
		RLAgentWidget = CreateWidget<URLAgentWidget>(this, RLAgentWidgetClass);
		OnQTableUpdate.AddUObject(RLAgentWidget, &URLAgentWidget::UpdatePanel);
		RLAgentWidget->InitQTable(Rows, Columns, QTable);
		RLAgentWidget->AddToViewport();
	}
}

void ABSPlayerController::HideRLAgentWidget()
{
	if (RLAgentWidget && IsLocalController())
	{
		RLAgentWidget->RemoveFromParent();
		RLAgentWidget = nullptr;
	}
}

void ABSPlayerController::ShowCombatText(const int32 Streak, const FTransform& Transform)
{
	if (!IsLocalController()) return;
	if (!bShowStreakCombatText) return;

	if (Streak > 0 && CombatTextFrequency != 0 && Streak % CombatTextFrequency == 0)
	{
		AFloatingTextActor* CombatText = GetWorld()->SpawnActorDeferred<AFloatingTextActor>(FloatingTextActorClass,
		FTransform(), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		CombatText->SetText(FText::FromString(FString::FromInt(Streak)));
		CombatText->FinishSpawning(CombatText->GetTextTransform(Transform, true), false);
	}
}

void ABSPlayerController::ShowAccuracyText(const float TimeOffset, const FTransform& Transform)
{
	if (!IsLocalController()) return;
	FString AccuracyString;
	if (TimeOffset <= 0.1f)
	{
		AccuracyString = "Perfect";
	}
	else if (TimeOffset > 0.1f && TimeOffset <= 0.2f)
	{
		AccuracyString = "Good";
	}
	else
	{
		AccuracyString = "Oof";
	}
	AFloatingTextActor* CombatText = GetWorld()->SpawnActorDeferred<AFloatingTextActor>(FloatingTextActorClass,
		FTransform(), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	CombatText->SetText(FText::FromString(AccuracyString));
	CombatText->FinishSpawning(CombatText->GetTextTransform(Transform, false), false);
}

void ABSPlayerController::OnPlayerSettingsChanged(const FPlayerSettings_VideoAndSound& PlayerSettings)
{
	if (PlayerSettings.bShowFPSCounter)
	{
		if (!FPSCounterWidget)
		{
			ShowFPSCounter();
		}
	}
	else
	{
		if (FPSCounterWidget)
		{
			HideFPSCounter();
		}
	}
}

void ABSPlayerController::OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings)
{
	CombatTextFrequency = GameSettings.CombatTextFrequency;
	bShowStreakCombatText = GameSettings.bShowStreakCombatText;
}

void ABSPlayerController::OnPlayerSettingsChanged(const FPlayerSettings_User& UserSettings)
{
	bNightModeUnlocked = UserSettings.bNightModeUnlocked;
}

void ABSPlayerController::TryResetAuthTicketHandle(const uint32 Handle)
{
	NumAuthTicketFinishes++;
	if (NumAuthTicketFinishes < 2) return;
	// Cancel auth ticket after two uses (BeatShot API and MainMenuWidget)
	if (SteamUser() && Handle)
	{
		SteamUser()->CancelAuthTicket(Handle);
	}
	NumAuthTicketFinishes = 0;
}
