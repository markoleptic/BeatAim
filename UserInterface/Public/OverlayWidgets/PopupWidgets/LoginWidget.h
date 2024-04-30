// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerSettingsInterface.h"
#include "HttpRequestInterface.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "BSWidgetInterface.h"
#include "LoginWidget.generated.h"

class UBackgroundBlur;
class UOverlay;
class UEditableTextBox;
class UTextBlock;
class UBSButton;
class UHorizontalBox;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoginButtonClicked, const FLoginPayload LoginPayload);

/** Widget used to prompt a user to login and handle login through HTTP requests */
UCLASS()
class USERINTERFACE_API ULoginWidget : public UUserWidget, public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

	friend class UMainMenuWidget;
	virtual void NativeConstruct() override;

public:
	/** Shows the login screen */
	UFUNCTION()
	void ShowLoginScreen(const FString& Key);

	void ShowSteamLoginScreen();

	/** Sets the Text in the Box_Error given a key for the ST_Login string table */
	void SetErrorText(const FString& Key);

	void SetIsLegacySignedIn(const bool bIsSignedIn);

	/** Broadcasts the user's login info to WebBrowserOverlay in order to log them into the web browser */
	FOnLoginButtonClicked OnLoginButtonClicked;

	/** Broadcast when the widget is hidden after animations have completed */
	FOnExitAnimationCompleted OnExitAnimationCompletedDelegate;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBackgroundBlur* BackgroundBlur;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_Steam;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_SteamBody;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_RetrySteamLogin;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_FromSteam_ToLegacyLogin;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_NoSteamLogin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_ContinueWithout;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_ContinueWithoutTitle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_ContinueWithoutBody;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_NoLoginConfirm;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_NoLoginCancel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_Login;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* Box_Error;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_Error;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBox* Value_UsernameEmail;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBox* Value_Password;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Register;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Login;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_FromLogin_ToSteam;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_NoLogin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutLogin;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutContinueWithout;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutSteam;

	FWidgetAnimationDynamicEvent FadeOutDelegate;

	UFUNCTION()
	void ClearErrorText(const FText& Text);
	UFUNCTION()
	void InitializeExit();
	UFUNCTION()
	void OnExitAnimationCompleted();

	static void LaunchRegisterURL() { UKismetSystemLibrary::LaunchURL("https://beatshot.gg/register"); }

	void OnButtonClicked_BSButton(const UBSButton* Button);

	UFUNCTION()
	void LoginButtonClicked();

	UFUNCTION()
	void PlayFadeInLogin() { PlayAnimationReverse(FadeOutLogin); }

	UFUNCTION()
	void PlayFadeOutLogin() { PlayAnimationForward(FadeOutLogin); }

	UFUNCTION()
	void PlayFadeInContinueWithout() { PlayAnimationReverse(FadeOutContinueWithout); }

	UFUNCTION()
	void PlayFadeOutContinueWithout() { PlayAnimationForward(FadeOutContinueWithout); }

	UFUNCTION()
	void PlayFadeInSteamLogin() { PlayAnimationReverse(FadeOutSteam); }

	UFUNCTION()
	void PlayFadeOutSteamLogin() { PlayAnimationForward(FadeOutSteam); }

	bool bIsLegacySignedIn = false;
};
