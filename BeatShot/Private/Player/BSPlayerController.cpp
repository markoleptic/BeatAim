// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Player/BSPlayerController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/BSAbilitySystemComponent.h"
#include "Character/BSCharacter.h"
#include "BSGameInstance.h"
#include "BSGameMode.h"
#include "FloatingTextActor.h"
#include "MainMenuGameMode.h"
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

	if (LoadPlayerSettings().VideoAndSound.bShowFPSCounter)
	{
		ShowFPSCounter();
	}

	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	GI->AddDelegateToOnPlayerSettingsChanged(OnPlayerSettingsChangedDelegate_VideoAndSound);
	GI->GetPublicVideoAndSoundSettingsChangedDelegate().AddUObject(this, &ABSPlayerController::OnPlayerSettingsChanged);
	
	PostGameMenuActive = false;
	if (ABSGameMode* GameMode = Cast<ABSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->OnGameModeStarted.AddLambda([this]
		{
			ShowCrossHair();
			ShowPlayerHUD();
			HideCountdown();
		});
	}
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

void ABSPlayerController::SetPlayerEnabledState(const bool bPlayerEnabled)
{
	if (GetWorld()->GetMapName().Contains("Range"))
	{
		if (bPlayerEnabled)
		{
			GetPawn()->EnableInput(this);
		}
		else
		{
			GetPawn()->DisableInput(this);
		}
	}
}

void ABSPlayerController::ShowMainMenu()
{
	if (!IsLocalController())
	{
		return;
	}
	
	SetInputMode(FInputModeUIOnly());
	SetShowMouseCursor(true);
	
	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	AMainMenuGameMode* GameMode = Cast<AMainMenuGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	MainMenu = CreateWidget<UMainMenuWidget>(this, MainMenuClass);
	MainMenu->AddToViewport();
	MainMenu->GameModesWidget->OnGameModeStateChanged.AddUObject(GI, &UBSGameInstance::HandleGameModeTransition);
	MainMenu->OnSteamLoginRequest.BindUObject(this, &ThisClass::InitiateSteamLogin);

	if (GameMode)
	{
		GameMode->BindGameModesWidgetToTargetManager(MainMenu->GameModesWidget);
	}

	GI->AddDelegateToOnPlayerSettingsChanged(MainMenu->SettingsMenuWidget->GetGameDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(MainMenu->SettingsMenuWidget->GetVideoAndSoundDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(MainMenu->SettingsMenuWidget->GetCrossHairDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(MainMenu->SettingsMenuWidget->GetAudioAnalyzerDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(MainMenu->SettingsMenuWidget->GetUserDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(MainMenu->GetUserDelegate());

	UGameUserSettings::GetGameUserSettings()->SetFrameRateLimit(LoadPlayerSettings().VideoAndSound.FrameRateLimitMenu);
	UGameUserSettings::GetGameUserSettings()->ApplySettings(false);

	if (!bIsLoggedIn)
	{
		LoginUser();
	}
	else
	{
		MainMenu->LoginScoresWidgetSubsequent();
	}
}

void ABSPlayerController::HideMainMenu()
{
	if (MainMenu)
	{
		MainMenu->RemoveFromParent();
		MainMenu = nullptr;
	}
}

void ABSPlayerController::ShowPauseMenu()
{
	if (!IsLocalController())
	{
		return;
	}
	PauseMenu = CreateWidget<UPauseMenuWidget>(this, PauseMenuClass);
	PauseMenu->ResumeGame.BindLambda([&]
	{
		HandlePause();
		HidePauseMenu();
	});

	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	GI->AddDelegateToOnPlayerSettingsChanged(PauseMenu->SettingsMenuWidget->GetGameDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PauseMenu->SettingsMenuWidget->GetVideoAndSoundDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PauseMenu->SettingsMenuWidget->GetCrossHairDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PauseMenu->SettingsMenuWidget->GetAudioAnalyzerDelegate());
	GI->AddDelegateToOnPlayerSettingsChanged(PauseMenu->SettingsMenuWidget->GetUserDelegate());

	PauseMenu->QuitMenuWidget->OnGameModeStateChanged.AddUObject(GI, &UBSGameInstance::HandleGameModeTransition);
	PauseMenu->AddToViewport();

	UGameUserSettings::GetGameUserSettings()->SetFrameRateLimit(LoadPlayerSettings().VideoAndSound.FrameRateLimitMenu);
	UGameUserSettings::GetGameUserSettings()->ApplySettings(false);
}

void ABSPlayerController::HidePauseMenu()
{
	if (!IsLocalController())
	{
		return;
	}
	if (PauseMenu)
	{
		PauseMenu->RemoveFromParent();
		PauseMenu = nullptr;
		UGameUserSettings::GetGameUserSettings()->SetFrameRateLimit(
			LoadPlayerSettings().VideoAndSound.FrameRateLimitGame);
		UGameUserSettings::GetGameUserSettings()->ApplySettings(false);
	}
}

void ABSPlayerController::ShowCrossHair()
{
	if (!IsLocalController())
	{
		return;
	}
	CrossHair = CreateWidget<UCrossHairWidget>(this, CrossHairClass);

	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	GI->GetPublicCrossHairSettingsChangedDelegate().AddUObject(CrossHair,
		&UCrossHairWidget::OnPlayerSettingsChanged_CrossHair);

	CrossHair->AddToViewport();
}

void ABSPlayerController::HideCrossHair()
{
	if (CrossHair)
	{
		CrossHair->RemoveFromParent();
		CrossHair = nullptr;
	}
}

void ABSPlayerController::ShowPlayerHUD()
{
	if (!IsLocalController())
	{
		return;
	}

	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	
	PlayerHUD = CreateWidget<UPlayerHUD>(this, PlayerHUDClass);
	check(GI->GetBSConfig())
	PlayerHUD->Init(GI->GetBSConfig());
	
	GI->AddDelegateToOnPlayerSettingsChanged(PlayerHUD->GetGameDelegate());
	GI->GetPublicGameSettingsChangedDelegate().AddUObject(PlayerHUD, &UPlayerHUD::OnPlayerSettingsChanged_Game);

	ABSGameMode* GameMode = Cast<ABSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->UpdateScoresToHUD.AddUObject(PlayerHUD, &UPlayerHUD::UpdateAllElements);
	GameMode->OnSecondPassed.AddUObject(PlayerHUD, &UPlayerHUD::UpdateSongProgress);

	PlayerHUD->AddToViewport();
}

void ABSPlayerController::HidePlayerHUD()
{
	if (PlayerHUD)
	{
		PlayerHUD->RemoveFromParent();
		PlayerHUD = nullptr;
	}
	HideRLAgentWidget();
}

void ABSPlayerController::ShowCountdown(const bool bIsRestart)
{
	if (!IsLocalController())
	{
		return;
	}
	SetControlRotation(FRotator(0, 0, 0));
	if (GetPawn() != nullptr)
	{
		Cast<ABSCharacter>(GetPawn())->SetActorLocationAndRotation(FVector(1580, 0, 102), FRotator(0, 0, 0));
	}
	if (bIsRestart)
	{
		FadeScreenFromBlack();
	}

	if (GetBSCharacter())
	{
		GetBSCharacter()->BindLeftClick();
	}
	
	Countdown = CreateWidget<UCountdownWidget>(this, CountdownClass);
	ABSGameMode* GameMode = Cast<ABSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	Countdown->OnCountdownCompleted.BindUObject(GameMode, &ABSGameMode::StartGameMode);
	Countdown->StartAAManagerPlayback.BindUObject(GameMode, &ABSGameMode::StartAAManagerPlayback);
	Countdown->AddToViewport();
	UGameUserSettings::GetGameUserSettings()->SetFrameRateLimit(LoadPlayerSettings().VideoAndSound.FrameRateLimitGame);
	UGameUserSettings::GetGameUserSettings()->ApplySettings(false);
}

void ABSPlayerController::HideCountdown()
{
	if (!IsLocalController())
	{
		return;
	}
	if (Countdown)
	{
		Countdown->RemoveFromParent();
		Countdown = nullptr;
	}
}

void ABSPlayerController::ShowPostGameMenu()
{
	if (!IsLocalController())
	{
		return;
	}

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
	PostGameMenuActive = true;

	SetInputMode(FInputModeUIOnly());
	SetShowMouseCursor(true);
	SetPlayerEnabledState(false);

	UGameUserSettings::GetGameUserSettings()->SetFrameRateLimit(LoadPlayerSettings().VideoAndSound.FrameRateLimitMenu);
	UGameUserSettings::GetGameUserSettings()->ApplySettings(false);
}

void ABSPlayerController::OnPostScoresResponseReceived(const FString& StringTableKey)
{
	if (!IsLocalController())
	{
		return;
	}
	if (!PostGameMenuWidget)
	{
		return;
	}
	PostGameMenuWidget->ScoresWidget->InitScoreBrowser(EScoreBrowserType::PostGameModeMenuScores, StringTableKey);
}

void ABSPlayerController::HandlePause()
{
	if (IsPostGameMenuActive())
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
		GameMode->PauseAAManager(false);
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
	}
	else
	{
		ShowPauseMenu();
		UGameplayStatics::SetGamePaused(World, true);
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
	if (Countdown)
	{
		UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		if (GI && GI->GetBSConfig().IsValid())
		{
			Countdown->StartCountdown(Constants::CountdownTimerLength, GI->GetBSConfig()->AudioConfig.PlayerDelay);
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

ABSCharacter* ABSPlayerController::GetBSCharacter() const
{
	return Cast<ABSCharacter>(GetPawn());
}

void ABSPlayerController::LoginUser()
{
	if (!MainMenu) return;

	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	USteamManager* SteamManager = GI->GetSteamManager();
	if (!SteamManager) return;

	TSharedPtr<FOnAuthTicketForWebApiResponseCallbackHandler> CallbackHandler(
		new FOnAuthTicketForWebApiResponseCallbackHandler());

	// First get auth ticket for web api
	CallbackHandler->OnAuthTicketForWebApiReady.BindLambda([this, CallbackHandler]
	{
		if (CallbackHandler->Result != k_EResultOK)
		{
			if (SteamUser()) SteamUser()->CancelAuthTicket(CallbackHandler->Handle);
			MainMenu->UpdateLoginState(false, "SignInState_SteamSignInFailed");
			MainMenu->TryFallbackLogin();
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
				
				AsyncTask(ENamedThreads::GameThread, [this, CallbackHandler]()
				{
					TryResetAuthTicketHandle(CallbackHandler->Handle);
				});
			});
			AuthenticateSteamUser(CallbackHandler->Ticket, SteamAuthTicketResponse);
			
			// This will be OnlineAsyncTaskThreadSteam, need GameThread for TimerManager later on
			AsyncTask(ENamedThreads::GameThread, [this, CallbackHandler]()
			{
				FDelegateHandle Handle = MainMenu->ScoresWidget->OnURLChangedResult.AddLambda(
				[this, &Handle, CallbackHandler](const bool bSuccess)
					{
						TryResetAuthTicketHandle(CallbackHandler->Handle);
						Handle.Reset();
					});
				// Login to the in-game web browser using the redirect url from the auth ticket for web api
				MainMenu->LoginScoresWidgetWithSteam(CallbackHandler->Ticket);
			});
		}
	});
	// Could fail if not logged in to Steam
	if (!SteamManager->CreateAuthTicketForWebApi(CallbackHandler))
	{
		MainMenu->UpdateLoginState(false, "SignInState_SteamSignInFailed");
		MainMenu->TryFallbackLogin();
	}
}

void ABSPlayerController::InitiateSteamLogin()
{
	LoginUser();
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
	GetBSAbilitySystemComponent()->ProcessAbilityInput(DeltaTime, IsPaused());
}

void ABSPlayerController::OnLoadingScreenVisibilityChanged(bool bIsVisible)
{
	if (!bIsVisible)
	{
		FadeScreenFromBlack();
	}
}

void ABSPlayerController::HidePostGameMenu()
{
	if (PostGameMenuWidget)
	{
		PostGameMenuWidget->RemoveFromParent();
		PostGameMenuWidget = nullptr;
		PostGameMenuActive = false;
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
		SetPlayerEnabledState(true);
	}
}

void ABSPlayerController::ShowFPSCounter()
{
	if (FPSCounter == nullptr)
	{
		FPSCounter = CreateWidget<UFPSCounterWidget>(this, FPSCounterClass);
		FPSCounter->AddToViewport(ZOrderFPSCounter);
	}
}

void ABSPlayerController::HideFPSCounter()
{
	if (FPSCounter)
	{
		FPSCounter->RemoveFromParent();
		FPSCounter = nullptr;
	}
}

void ABSPlayerController::CreateScreenFadeWidget(const float StartOpacity)
{
	if (!ScreenFadeWidget)
	{
		ScreenFadeWidget = CreateWidget<UScreenFadeWidget>(this, ScreenFadeClass);
		ScreenFadeWidget->OnFadeToBlackFinish.AddLambda([&]
		{
			if (!OnScreenFadeToBlackFinish.ExecuteIfBound())
			{
				UE_LOG(LogTemp, Display, TEXT("OnScreenFadeToBlackFinish not bound."));
			}
		});
		ScreenFadeWidget->OnFadeFromBlackFinish.AddUObject(this, &ABSPlayerController::OnFadeScreenFromBlackFinish);
		ScreenFadeWidget->SetStartOpacity(StartOpacity);
		ScreenFadeWidget->AddToViewport(ZOrderFadeScreen);
	}
}

void ABSPlayerController::FadeScreenToBlack()
{
	if (!IsLocalController())
	{
		return;
	}
	if (!ScreenFadeWidget)
	{
		CreateScreenFadeWidget(0.f);
	}
	if (!GetWorld()->GetMapName().Contains("Range"))
	{
		if (AMainMenuGameMode* MainMenuGameMode = Cast<AMainMenuGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			MainMenuGameMode->FadeOutMainMenuMusic(LoadingScreenWidgetFadeOutTime);
		}
	}
	ScreenFadeWidget->FadeToBlack(LoadingScreenWidgetFadeOutTime);
}

void ABSPlayerController::FadeScreenFromBlack()
{
	if (!IsLocalController())
	{
		return;
	}
	if (!ScreenFadeWidget)
	{
		CreateScreenFadeWidget(1.f);
	}
	if (!GetWorld()->GetMapName().Contains("Range"))
	{
		if (AMainMenuGameMode* MainMenuGameMode = Cast<AMainMenuGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			MainMenuGameMode->FadeInMainMenuMusic(LoadingScreenWidgetFadeOutTime);
		}
	}
	ScreenFadeWidget->FadeFromBlack(LoadingScreenWidgetFadeOutTime);
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
	if (!IsLocalController())
	{
		return;
	}
	if (!InteractInfoWidget)
	{
		InteractInfoWidget = CreateWidget<UUserWidget>(this, InteractInfoWidgetClass);
		InteractInfoWidget->AddToViewport(ZOrderFPSCounter);
	}
}

void ABSPlayerController::HideInteractInfo()
{
	if (InteractInfoWidget)
	{
		InteractInfoWidget->RemoveFromParent();
		InteractInfoWidget = nullptr;
	}
}

void ABSPlayerController::ShowRLAgentWidget(FOnQTableUpdate& OnQTableUpdate, const int32 Rows, const int32 Columns,
	const TArray<float>& QTable)
{
	if (!IsLocalController())
	{
		return;
	}
	if (!RLAgentWidget)
	{
		RLAgentWidget = CreateWidget<URLAgentWidget>(this, RLAgentWidgetClass);
		OnQTableUpdate.AddUObject(RLAgentWidget, &URLAgentWidget::UpdatePanel);
		RLAgentWidget->InitQTable(Rows, Columns, QTable);
		RLAgentWidget->AddToViewport();
	}
}

void ABSPlayerController::HideRLAgentWidget()
{
	if (RLAgentWidget)
	{
		RLAgentWidget->RemoveFromParent();
		RLAgentWidget = nullptr;
	}
}

void ABSPlayerController::ShowCombatText(const int32 Streak, const FTransform& Transform)
{
	AFloatingTextActor* CombatText = GetWorld()->SpawnActorDeferred<AFloatingTextActor>(FloatingTextActorClass,
		FTransform(), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	CombatText->SetText(FText::FromString(FString::FromInt(Streak)));
	CombatText->FinishSpawning(CombatText->GetTextTransform(Transform, true), false);
}

void ABSPlayerController::ShowAccuracyText(const float TimeOffset, const FTransform& Transform)
{
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
		if (!FPSCounter)
		{
			ShowFPSCounter();
		}
	}
	else
	{
		if (FPSCounter)
		{
			HideFPSCounter();
		}
	}
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
