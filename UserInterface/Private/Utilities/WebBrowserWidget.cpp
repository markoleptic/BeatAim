// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/WebBrowserWidget.h"
#include "TimerManager.h"
#include "WebBrowser.h"

using namespace Constants;

void UWebBrowserWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UWebBrowserWidget::NativeDestruct()
{
	Super::NativeDestruct();
	GetWorld()->GetTimerManager().ClearTimer(CheckURLTimer);
	CheckURLDelegate.Unbind();
}

void UWebBrowserWidget::LoadAuthenticateSteamUserURL(const FString& AuthTicket)
{
	IntendedDestinationURL = Segment_Profile;
	Browser->LoadURL(Endpoint_AuthenticateUserTicket + AuthTicket);
	CheckNewURL();
}

void UWebBrowserWidget::LoadCustomGameModesURL(const FString& UserID)
{
	IntendedDestinationURL = Segment_Profile + UserID + Segment_CustomModes;
	Browser->LoadURL(Segment_Profile + UserID + Segment_CustomModes);
	CheckNewURL();
}

void UWebBrowserWidget::LoadDefaultGameModesURL(const FString& UserID)
{
	IntendedDestinationURL = Segment_Profile + UserID;
	Browser->LoadURL(Segment_Profile + UserID + Segment_DefaultModes);
	CheckNewURL();
}

void UWebBrowserWidget::LoadPatchNotesURL() const
{
	Browser->LoadURL(Endpoint_PatchNotes);
}

void UWebBrowserWidget::LoadProfileURL(const FString& UserID)
{
	IntendedDestinationURL = Segment_Profile + UserID;
	Browser->LoadURL(Segment_Profile + UserID);
	CheckNewURL();
}

void UWebBrowserWidget::LoginUserToBeatShotWebsite(const FLoginPayload LoginPayload, const FString UserID)
{
	// fill in the forms with text
	if (!ExecuteJS_LoginFormEntries(LoginPayload))
	{
		return;
	}

	IntendedDestinationURL = Segment_Profile + UserID;

	// This happens second
	CheckboxDelegate.BindLambda([this]
	{
		Browser->ExecuteJavascript(CheckPersistScript);
		GetWorld()->GetTimerManager().SetTimer(ClickLoginDelay, ClickLoginDelegate, 0.1f, false);
	});

	// This happens third
	ClickLoginDelegate.BindLambda([this]
	{
		Browser->ExecuteJavascript(ClickLoginScript);
		CheckNewURL();
	});

	// This happens first
	GetWorld()->GetTimerManager().SetTimer(CheckCheckboxDelay, CheckboxDelegate, 0.1f, false);
}

bool UWebBrowserWidget::ExecuteJS_LoginFormEntries(const FLoginPayload LoginPayload) const
{
	if ((LoginPayload.Username.IsEmpty() && LoginPayload.Email.IsEmpty()) || LoginPayload.Password.IsEmpty())
	{
		return false;
	}
	Browser->ExecuteJavascript(InitialInputEventScript);
	if (!LoginPayload.Username.IsEmpty())
	{
		Browser->ExecuteJavascript(
			SetElementUsernameScript + LoginPayload.Username + "');" + DispatchUsernameChangeEventScript);
	}
	else
	{
		Browser->ExecuteJavascript(SetElementEmailScript + LoginPayload.Email + "');" + DispatchEmailChangeEventScript);
	}
	Browser->ExecuteJavascript(
		SetElementPasswordScript + LoginPayload.Password + "');" + DispatchPasswordChangeEventScript);
	return true;
}

void UWebBrowserWidget::CheckNewURL()
{
	CheckURLDelegate.BindLambda([this]
	{
		const FString URL = Browser->GetUrl();
		URLCheckAttempts++;
		if (URLCheckAttempts >= MaxNumURLCheckAttempts)
		{
			IntendedDestinationURL = "";
			URLCheckAttempts = 0;
			OnURLLoaded.Broadcast(false);
			GetWorld()->GetTimerManager().ClearTimer(CheckURLTimer);
			return;
		}
		if (URL.Contains(IntendedDestinationURL, ESearchCase::IgnoreCase))
		{
			IntendedDestinationURL = "";
			URLCheckAttempts = 0;
			OnURLLoaded.Broadcast(true);
			GetWorld()->GetTimerManager().ClearTimer(CheckURLTimer);
		}
	});
	GetWorld()->GetTimerManager().SetTimer(CheckURLTimer, CheckURLDelegate, 0.5f, true);
}
