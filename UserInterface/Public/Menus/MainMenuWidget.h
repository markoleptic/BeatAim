// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerSettingsInterface.h"
#include "HttpRequestInterface.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UBSButton;
class UMenuStyle;
class UFeedbackWidget;
class UScoreBrowserWidget;
class UVerticalBox;
class UWidgetSwitcher;
class UGameModeMenuWidget;
class USettingsMenuWidget;
class UFAQWidget;
class ULoginWidget;
class UHorizontalBox;
class UOverlay;
class UEditableTextBox;
class UTextBlock;
class UMenuButton;

/** The login method used to access a BeatShot account. */
UENUM()
enum class ELoginMethod : uint8
{
	None UMETA(DisplayName="None"),
	Steam UMETA(DisplayName="Steam"),
	Legacy UMETA(DisplayName="Legacy"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ELoginMethod, ELoginMethod::None, ELoginMethod::Legacy);

DECLARE_DELEGATE(FOnSteamLoginRequest);

/** Widget that is the entry point into the game, holding most other widgets that aren't MenuWidgets. */
UCLASS()
class USERINTERFACE_API UMainMenuWidget : public UUserWidget, public IBSPlayerSettingsInterface,
                                          public IHttpRequestInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UGameModeMenuWidget* GameModesWidget;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	USettingsMenuWidget* SettingsMenuWidget;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UScoreBrowserWidget* ScoresWidget;

	UFUNCTION()
	void UpdateLoginState(const bool bSuccessfulLogin, const FString OptionalStringTableKey = "");

	/** Returns OnPlayerSettingsChangedDelegate_User, the delegate that is broadcast when this class saves User
	 *  settings. */
	FOnPlayerSettingsChanged_User& GetUserDelegate() { return OnPlayerSettingsChangedDelegate_User; }

	/** Called when another class saves User settings. */
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_User& UserSettings) override;

	/** Executed when the player requests to try to log in through Steam after a failed attempt. */
	FOnSteamLoginRequest OnSteamLoginRequest;

	void LoginScoresWidgetWithSteam(const FString SteamAuthTicket);
	void LoginScoresWidgetSubsequent();
	void TryFallbackLogin();

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

	UPROPERTY(EditDefaultsOnly, Category="MainMenuWidget")
	TSubclassOf<UMenuStyle> MenuStyleClass;

	UPROPERTY()
	const UMenuStyle* MenuStyle;

	void SetStyles();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* MainMenuSwitcher;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_PatchNotes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_GameModes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_Scores;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_Settings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_FAQ;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* Button_Feedback;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* Button_Login_Register;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UScoreBrowserWidget* WebBrowserOverlayPatchNotes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UFAQWidget* FAQWidget;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	ULoginWidget* LoginWidget;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UFeedbackWidget* FeedbackWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_PatchNotes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_GameModes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_Settings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_Scores;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_FAQ;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_Quit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_SignInState;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_Username;

private:
	void OnMenuButtonClicked_BSButton(const UBSButton* Button);
	void OnButtonClicked_BSButton(const UBSButton* Button);
	UFUNCTION()
	void OnButtonClicked_Login(const FLoginPayload LoginPayload);
	UFUNCTION()
	void OnURLChangedResult_ScoresWidget(const bool bSuccess);
	UFUNCTION()
	void OnWidgetExitAnimationCompleted(UMenuButton* ButtonToSetInactive);

	ELoginMethod CurrentLoginMethod;

	bool bFadeInScoreBrowserOnButtonPress = false;
	bool bFadeInOverlayTextOnButtonPress = false;
};
