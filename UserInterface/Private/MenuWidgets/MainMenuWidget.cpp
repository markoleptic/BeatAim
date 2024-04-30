// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuWidgets/MainMenuWidget.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "OverlayWidgets/PopupWidgets/LoginWidget.h"
#include "OverlayWidgets/PopupWidgets/FeedbackWidget.h"
#include "SubMenuWidgets/ScoreBrowserWidget.h"
#include "Styles/MenuStyle.h"
#include "BSWidgetInterface.h"
#include "SaveGamePlayerSettings.h"
#include "SubmenuWidgets/GameModesWidgets/GameModesWidget.h"
#include "WidgetComponents/Buttons/MenuButton.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Button_Login_Register->SetVisibility(ESlateVisibility::Collapsed);

	SetStyles();

	ScoresWidget->OnURLChangedResult.AddUObject(this, &UMainMenuWidget::OnURLChangedResult_ScoresWidget);
	LoginWidget->OnLoginButtonClicked.AddUObject(this, &UMainMenuWidget::OnButtonClicked_Login);

	LoginWidget->OnExitAnimationCompletedDelegate.AddUObject(this, &ThisClass::OnWidgetExitAnimationCompleted,
		Button_Login_Register);
	FeedbackWidget->OnExitAnimationCompletedDelegate.AddUObject(this, &ThisClass::OnWidgetExitAnimationCompleted,
		Button_Feedback);

	LoginWidget->Button_RetrySteamLogin->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	Button_Login_Register->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);

	MenuButton_PatchNotes->SetDefaults(Box_PatchNotes, MenuButton_GameModes);
	MenuButton_GameModes->SetDefaults(Box_GameModes, MenuButton_Scores);
	MenuButton_Scores->SetDefaults(Box_Scores, MenuButton_Settings);
	MenuButton_Settings->SetDefaults(Box_Settings, MenuButton_FAQ);
	MenuButton_FAQ->SetDefaults(Box_FAQ, Button_Feedback);
	Button_Feedback->SetDefaults(nullptr, MenuButton_Quit);
	MenuButton_Quit->SetDefaults(Box_PatchNotes, MenuButton_PatchNotes);

	MenuButton_PatchNotes->OnBSButtonPressed.AddUObject(this, &ThisClass::OnMenuButtonClicked_BSButton);
	MenuButton_GameModes->OnBSButtonPressed.AddUObject(this, &ThisClass::OnMenuButtonClicked_BSButton);
	MenuButton_Scores->OnBSButtonPressed.AddUObject(this, &ThisClass::OnMenuButtonClicked_BSButton);
	MenuButton_Settings->OnBSButtonPressed.AddUObject(this, &ThisClass::OnMenuButtonClicked_BSButton);
	MenuButton_FAQ->OnBSButtonPressed.AddUObject(this, &ThisClass::OnMenuButtonClicked_BSButton);
	Button_Feedback->OnBSButtonPressed.AddUObject(this, &ThisClass::OnMenuButtonClicked_BSButton);
	MenuButton_Quit->OnBSButtonPressed.AddUObject(this, &ThisClass::OnMenuButtonClicked_BSButton);

	WebBrowserOverlayPatchNotes->InitScoreBrowser(EScoreBrowserType::PatchNotes);
	ScoresWidget->InitScoreBrowser(EScoreBrowserType::MainMenuScores);

	MenuButton_PatchNotes->SetActive();
	MainMenuSwitcher->SetActiveWidget(MenuButton_PatchNotes->GetBox());
}

void UMainMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	SetStyles();
}

void UMainMenuWidget::SetStyles()
{
	MenuStyle = IBSWidgetInterface::GetStyleCDO<UMenuStyle>(MenuStyleClass);
}

void UMainMenuWidget::OnPlayerSettingsChanged(const FPlayerSettings_User& UserSettings)
{
}

void UMainMenuWidget::LoginScoresWidgetWithSteam(const FString SteamAuthTicket)
{
	CurrentLoginMethod = ELoginMethod::Steam;
	ScoresWidget->LoginUserBrowser(FString(SteamAuthTicket));
}

void UMainMenuWidget::LoginScoresWidgetSubsequent()
{
	CurrentLoginMethod = ELoginMethod::Steam;
	const FPlayerSettings_User PlayerSettings = LoadPlayerSettings().User;
	if (IsRefreshTokenValid(PlayerSettings.RefreshCookie) && !PlayerSettings.UserID.IsEmpty())
	{
		ScoresWidget->LoadProfile(PlayerSettings.UserID);
	}
}

void UMainMenuWidget::TryFallbackLogin()
{
	CurrentLoginMethod = ELoginMethod::Legacy;
	const FPlayerSettings_User PlayerSettings = LoadPlayerSettings().User;
	if (IsRefreshTokenValid(PlayerSettings.RefreshCookie) && !PlayerSettings.UserID.IsEmpty())
	{
		ScoresWidget->LoadProfile(PlayerSettings.UserID);
	}
}

void UMainMenuWidget::OnMenuButtonClicked_BSButton(const UBSButton* Button)
{
	const UMenuButton* MenuButton = Cast<UMenuButton>(Button);
	if (!MenuButton) return;

	// Always stop the game mode preview if game modes widget is not visible
	if (MenuButton != MenuButton_GameModes)
	{
		GameModesWidget->StopGameModePreview();
	}

	// Manually fade out browser overlay to avoid abrupt change
	if (MenuButton == MenuButton_Scores)
	{
		if (bFadeInScoreBrowserOnButtonPress)
		{
			ScoresWidget->FadeOutLoadingOverlay();
			bFadeInScoreBrowserOnButtonPress = false;
		}
		if (bFadeInOverlayTextOnButtonPress)
		{
			ScoresWidget->FadeOutLoadingIconAndShowText();
			bFadeInOverlayTextOnButtonPress = false;
		}
	}
	// Feedback button
	else if (Button == Button_Feedback)
	{
		FeedbackWidget->ShowFeedbackWidget();
	}

	// Menu button
	if (MenuButton->GetBox())
	{
		MainMenuSwitcher->SetActiveWidget(MenuButton->GetBox());
	}

	// Quit button
	if (Button == MenuButton_Quit)
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0),
			EQuitPreference::Quit, false);
	}
}

void UMainMenuWidget::OnButtonClicked_BSButton(const UBSButton* Button)
{
	// Login/Register button
	if (Button == Button_Login_Register)
	{
		LoginWidget->ShowSteamLoginScreen();
	}
	else if (Button == LoginWidget->Button_RetrySteamLogin)
	{
		CurrentLoginMethod = ELoginMethod::Steam;
		TextBlock_SignInState->SetText(FText());
		if (OnSteamLoginRequest.IsBound()) OnSteamLoginRequest.Execute();
	}
}

void UMainMenuWidget::OnURLChangedResult_ScoresWidget(const bool bSuccess)
{
	UpdateLoginState(bSuccess);
}

void UMainMenuWidget::UpdateLoginState(const bool bSuccessfulLogin, const FString OptionalStringTableKey)
{
	if (!bSuccessfulLogin)
	{
		if (OptionalStringTableKey.IsEmpty())
		{
			TextBlock_SignInState->SetText(IBSWidgetInterface::GetWidgetTextFromKey("Login_NotSignedIn"));
		}
		else
		{
			TextBlock_SignInState->SetText(IBSWidgetInterface::GetWidgetTextFromKey(OptionalStringTableKey));
		}
		// Collapse username
		TextBlock_Username->SetText(FText());
		TextBlock_Username->SetVisibility(ESlateVisibility::Collapsed);

		// Show Login Button
		Button_Login_Register->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		bFadeInOverlayTextOnButtonPress = true;
		CurrentLoginMethod = ELoginMethod::None;
		return;
	}


	if (CurrentLoginMethod == ELoginMethod::Steam)
	{
		TextBlock_SignInState->SetText(IBSWidgetInterface::GetWidgetTextFromKey("Login_SignedInSteam"));
		// Only collapse login button if signed in through steam
		Button_Login_Register->SetVisibility(ESlateVisibility::Collapsed);
	}
	else if (CurrentLoginMethod == ELoginMethod::Legacy)
	{
		TextBlock_SignInState->SetText(IBSWidgetInterface::GetWidgetTextFromKey("Login_SignedInAs"));
		Button_Login_Register->
			SetButtonText(IBSWidgetInterface::GetWidgetTextFromKey("Login_Register_SteamButtonText"));
		LoginWidget->SetIsLegacySignedIn(true);
	}
	else
	{
		TextBlock_SignInState->SetText(FText::FromString("Unhandled Login Method"));
		Button_Login_Register->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Warning, TEXT("Unhandled Login Method in Main Menu"));
	}

	// Show username
	TextBlock_Username->SetText(FText::FromString(LoadPlayerSettings().User.DisplayName));
	TextBlock_Username->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	bFadeInScoreBrowserOnButtonPress = true;
	CurrentLoginMethod = ELoginMethod::None;
}

void UMainMenuWidget::OnWidgetExitAnimationCompleted(UMenuButton* ButtonToSetInactive)
{
	ButtonToSetInactive->SetInActive();
}

// ReSharper disable once CppPassValueParameterByConstReference
void UMainMenuWidget::OnButtonClicked_Login(const FLoginPayload LoginPayload)
{
	CurrentLoginMethod = ELoginMethod::Legacy;
	TSharedPtr<FLoginResponse> LoginResponse(new FLoginResponse);
	LoginResponse->OnHttpResponseReceived.BindLambda([this, LoginResponse, LoginPayload]
	{
		if (LoginResponse->HttpStatus != 200)
		{
			if (LoginResponse->HttpStatus == 401)
			{
				LoginWidget->ShowLoginScreen("Login_InvalidCredentialsText");
			}
			else if (LoginResponse->HttpStatus == 408 || LoginResponse->HttpStatus == 504)
			{
				LoginWidget->ShowLoginScreen("Login_TimeOutErrorText");
			}
			else
			{
				LoginWidget->ShowLoginScreen("Login_LoginErrorText");
			}
			UpdateLoginState(false);
		}
		else
		{
			FPlayerSettings_User PlayerSettingsToSave = LoadPlayerSettings().User;
			PlayerSettingsToSave.UserID = LoginResponse->UserID;
			PlayerSettingsToSave.DisplayName = LoginResponse->DisplayName;
			PlayerSettingsToSave.RefreshCookie = LoginResponse->RefreshToken;
			SavePlayerSettings(PlayerSettingsToSave);
			TextBlock_SignInState->SetText(IBSWidgetInterface::GetWidgetTextFromKey("SignInState_LoggingWebBrowser"));

			// Callback function for this is OnURLChangedResult_ScoresWidget
			ScoresWidget->LoginUserBrowser(LoginPayload, PlayerSettingsToSave.UserID);
		}
	});
	TextBlock_SignInState->SetText(IBSWidgetInterface::GetWidgetTextFromKey("SignInState_SendingRequest"));
	LoginUser(LoginPayload, LoginResponse);
}
