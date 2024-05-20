// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpRequestInterface.h"
#include "Blueprint/UserWidget.h"
#include "WebBrowserWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnURLLoaded, const bool bLoadedSuccessfully);

class UWebBrowser;

/** Wrapper around UWebBrowser providing specific functionality. */
UCLASS()
class USERINTERFACE_API UWebBrowserWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class UScoreBrowserWidget;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** The WebBrowser widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UWebBrowser* Browser;

	/** Load the steam authenticate URL to receive refresh token as cookie and get redirected to profile. */
	UFUNCTION()
	void LoadAuthenticateSteamUserURL(const FString& AuthTicket);

	/** Load Custom GameModes for the user. */
	UFUNCTION()
	void LoadCustomGameModesURL(const FString& UserID);

	/** Load Default GameModes for the user. */
	UFUNCTION()
	void LoadDefaultGameModesURL(const FString& UserID);

	/** Profile for a user. */
	UFUNCTION()
	void LoadProfileURL(const FString& UserID);

	/** Load Patch Notes for any user. */
	UFUNCTION()
	void LoadPatchNotesURL() const;

	/** Executes a series of Javascript scripts to login a user. */
	UFUNCTION()
	void LoginUserToBeatShotWebsite(const FLoginPayload LoginPayload, const FString UserID);

	/** Broadcast true when the new WebBrowser URL matches the IntendedDestinationURL set by a LoadURL function,
	 *  otherwise false. */
	FOnURLLoaded OnURLLoaded;

private:
	/** Fills the login form on the login screen of the website using Javascript. */
	bool ExecuteJS_LoginFormEntries(const FLoginPayload LoginPayload) const;

	/** Compares the browser's current URL address against the IntendedDestinationURL. */
	UFUNCTION()
	void CheckNewURL();

	FTimerDelegate CheckboxDelegate;
	FTimerDelegate ClickLoginDelegate;
	FTimerDelegate CheckURLDelegate;

	/** Timer to delay clicking the Checkbox_SyncSlidersAndValues on the login page. */
	FTimerHandle CheckCheckboxDelay;

	/** Timer to delay clicking the login button on the login page. */
	FTimerHandle ClickLoginDelay;

	/** Timer to delay URL address comparisons. */
	FTimerHandle CheckURLTimer;

	/** The intended destination URL set by a LoadURL function used to check the current browser URL against. */
	FString IntendedDestinationURL;

	/** The number of CheckNewURL function calls, or number of profile URL checks. */
	int32 URLCheckAttempts = 0;

	const int32 MaxNumURLCheckAttempts = 10;

#pragma region SCRIPTS

	/* Sets up the nativeInputValueSetter so the browser recognizes the changes. */
	const FString InitialInputEventScript =
		"var nativeInputValueSetter = Object.getOwnPropertyDescriptor(window.HTMLInputElement.prototype, 'value').set; "
		"var event = new Event('input', { bubbles: true});";

	/* Javascript to check the persist checkbox. */
	const FString CheckPersistScript =
		"document.getElementById('persist').checked=true; document.getElementById('persist').click();";

	/* Javascript to click the login button. */
	const FString ClickLoginScript = "document.getElementById('login-button').click()";

	/* Sets the username form element text. */
	const FString SetElementUsernameScript = "nativeInputValueSetter.call(username, '";

	/* Sets the email form element text. */
	const FString SetElementEmailScript = "nativeInputValueSetter.call(email, '";

	/* Sets the password form element text. */
	const FString SetElementPasswordScript = "nativeInputValueSetter.call(password, '";

	/* Dispatches the username so the browser recognizes the change. */
	const FString DispatchUsernameChangeEventScript = "username.dispatchEvent(event);";

	/* Dispatches the email so the browser recognizes the change. */
	const FString DispatchEmailChangeEventScript = "email.dispatchEvent(event);";

	/* Dispatches the password so the browser recognizes the change. */
	const FString DispatchPasswordChangeEventScript = "password.dispatchEvent(event);";

#pragma endregion
};
