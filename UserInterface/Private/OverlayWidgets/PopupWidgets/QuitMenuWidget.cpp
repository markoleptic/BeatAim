// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "OverlayWidgets/PopupWidgets/QuitMenuWidget.h"

#include "BSGameModeDataAsset.h"
#include "OverlayWidgets/LoadingScreenWidgets/ScreenFadeWidget.h"
#include "WidgetComponents/Buttons/BSButton.h"

void UQuitMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (bIsPostGameMenuChild)
	{
		bShouldSaveScores = false;
		Button_QuitMainMenu->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
		Button_QuitDesktop->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
		Button_QuitBack->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	}
	else
	{
		Button_QuitMainMenu->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
		Button_QuitDesktop->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
		Button_QuitBack->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
		Button_QuitAndSave->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
		Button_QuitWithoutSave->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
		Button_SaveBack->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
		Button_RestartAndSave->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
		Button_RestartWithoutSave->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
		Button_RestartBack->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	}
}

void UQuitMenuWidget::OnButtonClicked_BSButton(const UBSButton* Button)
{
	if (bIsPostGameMenuChild)
	{
		if (Button == Button_QuitMainMenu)
		{
			PlayFadeOutMenu();
			SetGotoMainMenuTrue();
			Quit();
		}
		else if (Button == Button_QuitDesktop)
		{
			PlayFadeOutMenu();
			SetGotoMainMenuFalse();
			Quit();
		}
		else if (Button == Button_QuitBack)
		{
			PlayFadeOutMenu();
			InitializeExit();
		}
		return;
	}

	if (Button == Button_QuitMainMenu)
	{
		PlayFadeOutMenu();
		PlayFadeInSaveMenu();
		SetGotoMainMenuTrue();
		SetSaveMenuTitleMainMenu();
	}
	else if (Button == Button_QuitDesktop)
	{
		PlayFadeOutMenu();
		PlayFadeInSaveMenu();
		SetGotoMainMenuFalse();
		SetSaveMenuTitleDesktop();
	}
	else if (Button == Button_QuitBack)
	{
		PlayFadeOutMenu();
		InitializeExit();
	}
	else if (Button == Button_QuitAndSave)
	{
		PlayFadeOutSaveMenu();
		SetShouldSaveScoresTrue();
		Quit();
	}
	else if (Button == Button_QuitWithoutSave)
	{
		PlayFadeOutSaveMenu();
		SetShouldSaveScoresFalse();
		Quit();
	}
	else if (Button == Button_SaveBack)
	{
		PlayFadeOutSaveMenu();
		PlayFadeInMenu();
	}
	else if (Button == Button_RestartAndSave)
	{
		SetShouldSaveScoresTrue();
		OnRestart();
	}
	else if (Button == Button_RestartWithoutSave)
	{
		SetShouldSaveScoresFalse();
		OnRestart();
	}
	else if (Button == Button_RestartBack)
	{
		PlayFadeOutRestartMenu();
		InitializeExit();
	}
}

void UQuitMenuWidget::Quit()
{
	if (bGotoMainMenu)
	{
		OnQuitToMainMenu();
		return;
	}
	OnQuitToDesktop();
}

void UQuitMenuWidget::OnQuitToMainMenu()
{
	FGameModeTransitionState TransitionState;
	TransitionState.TransitionState = ETransitionState::QuitToMainMenu;
	TransitionState.bSaveCurrentScores = bShouldSaveScores;
	OnGameModeStateChanged.Broadcast(TransitionState);
}

void UQuitMenuWidget::OnQuitToDesktop()
{
	if (bShouldSaveScores)
	{
		PlayFadeInSaveInProgress();
	}
	FGameModeTransitionState TransitionState;
	TransitionState.TransitionState = ETransitionState::QuitToDesktop;
	TransitionState.bSaveCurrentScores = bShouldSaveScores;
	OnGameModeStateChanged.Broadcast(TransitionState);
}

void UQuitMenuWidget::OnRestart()
{
	FGameModeTransitionState TransitionState;
	TransitionState.TransitionState = ETransitionState::Restart;
	TransitionState.bSaveCurrentScores = bShouldSaveScores;
	OnGameModeStateChanged.Broadcast(TransitionState);
}

void UQuitMenuWidget::CollapseWidget()
{
	SetVisibility(ESlateVisibility::Collapsed);
	UnbindFromAnimationFinished(FadeOutBackgroundBlur, FadeOutWidgetDelegate);
}

void UQuitMenuWidget::InitializeExit()
{
	PlayAnimationForward(FadeOutBackgroundBlur);
	FadeOutWidgetDelegate.BindDynamic(this, &UQuitMenuWidget::CollapseWidget);
	BindToAnimationFinished(FadeOutBackgroundBlur, FadeOutWidgetDelegate);
	if (!OnExitQuitMenu.ExecuteIfBound())
	{
		UE_LOG(LogTemp, Display, TEXT("OnExitQuitMenu not bound."));
	}
}
