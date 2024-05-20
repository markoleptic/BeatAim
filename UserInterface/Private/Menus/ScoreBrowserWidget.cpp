// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Menus/ScoreBrowserWidget.h"
#include "BSPlayerSettingsInterface.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "SaveGames/SaveGamePlayerSettings.h"
#include "Utilities/WebBrowserWidget.h"

void UScoreBrowserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindToAnimationFinished(FadeOutOverlay, FadeOutDelegate);
	FadeOutDelegate.BindUFunction(BrowserOverlay, FName("RemoveFromParent"));
	MID_LoadingIcon = LoadingIcon->GetDynamicMaterial();
	BrowserWidget->OnURLLoaded.AddUObject(this, &UScoreBrowserWidget::OnURLLoaded);
}

void UScoreBrowserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (BrowserOverlay)
	{
		TotalDeltaTime += InDeltaTime;
		MID_LoadingIcon->SetScalarParameterValue(FName("Time"), TotalDeltaTime);
	}
}

void UScoreBrowserWidget::InitScoreBrowser(const EScoreBrowserType InScoreBrowserType,
	const FString& ErrorStringTableKey)
{
	ScoreBrowserType = InScoreBrowserType;
	switch (InScoreBrowserType)
	{
	case EScoreBrowserType::MainMenuScores:
	case EScoreBrowserType::PostGameModeMenuScores:
		break;
	case EScoreBrowserType::PatchNotes:
		BrowserWidget->LoadPatchNotesURL();
		FadeOutLoadingOverlay();
		break;
	default:
		break;
	}

	if (ScoreBrowserType == EScoreBrowserType::PostGameModeMenuScores)
	{
		// Something went wrong
		if (!ErrorStringTableKey.IsEmpty())
		{
			SetOverlayTextAndFadeIn(ErrorStringTableKey);
			return;
		}

		int64 MinTimeDifference = FDateTime::MinValue().ToUnixTimestamp();
		FPlayerScore MinDateScore = FPlayerScore();
		// Could probably just use the last value in PlayerScoreArray, but just to be sure
		for (const FPlayerScore& Score : LoadPlayerScores())
		{
			FDateTime ParsedTime;
			FDateTime::ParseIso8601(*Score.Time, ParsedTime);
			if (ParsedTime.ToUnixTimestamp() > MinTimeDifference)
			{
				MinTimeDifference = ParsedTime.ToUnixTimestamp();
				MinDateScore = Score;
			}
		}
		if (MinDateScore.DefiningConfig.GameModeType == EGameModeType::Preset)
		{
			BrowserWidget->LoadDefaultGameModesURL(IBSPlayerSettingsInterface::LoadPlayerSettings().User.UserID);
		}
		else
		{
			BrowserWidget->LoadCustomGameModesURL(IBSPlayerSettingsInterface::LoadPlayerSettings().User.UserID);
		}
	}
}

void UScoreBrowserWidget::SetOverlayTextAndFadeIn(const FString& Key)
{
	OverlayText->SetText(FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", Key));
	PlayAnimationForward(FadeInOverlayText);
}

void UScoreBrowserWidget::FadeOutLoadingOverlay()
{
	PlayAnimationForward(FadeOutOverlay);
}

void UScoreBrowserWidget::FadeOutLoadingIconAndShowText()
{
	PlayAnimationForward(FadeInOverlayText);
}

void UScoreBrowserWidget::SetOverlayText(const FString& Key)
{
	OverlayText->SetText(FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", Key));
}

void UScoreBrowserWidget::LoginUserBrowser(const FLoginPayload LoginPayload, const FString UserID)
{
	BrowserWidget->LoginUserToBeatShotWebsite(LoginPayload, UserID);
}

void UScoreBrowserWidget::LoginUserBrowser(const FString SteamAuthTicket)
{
	BrowserWidget->LoadAuthenticateSteamUserURL(SteamAuthTicket);
}

void UScoreBrowserWidget::LoadProfile(const FString& UserID)
{
	BrowserWidget->LoadProfileURL(UserID);
}

void UScoreBrowserWidget::OnURLLoaded(const bool bLoadedSuccessfully)
{
	if (ScoreBrowserType == EScoreBrowserType::PostGameModeMenuScores)
	{
		bLoadedSuccessfully ? FadeOutLoadingOverlay() : SetOverlayTextAndFadeIn("SBW_SavedScoresButNotLoggedIn");
		return;
	}

	// let main menu handle fading out since it will be abrupt change if not already in view
	if (ScoreBrowserType == EScoreBrowserType::MainMenuScores)
	{
		OnURLChangedResult.Broadcast(bLoadedSuccessfully);
	}
}
