// Fill out your copyright notice in the Description page of Project Settings.


#include "PostGameMenuWidget.h"

#include "AASettings.h"
#include "DefaultGameMode.h"
#include "DefaultPlayerController.h"
#include "SettingsMenuWidget.h"
#include "GameModesWidget.h"
#include "SlideRightButton.h"
#include "WebBrowserOverlay.h"
#include "QuitMenuWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"

void UPostGameMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	MenuWidgets.Add(ScoresButton, Scores);
	MenuWidgets.Add(GameModesButton, GameModes);
	MenuWidgets.Add(SettingsButton, Settings);
	MenuWidgets.Add(FAQButton, FAQ);
	
	ScoresButton->Button->OnClicked.AddDynamic(this, &UPostGameMenuWidget::OnScoresButtonClicked);
	PlayAgainButton->Button->OnClicked.AddDynamic(this, &UPostGameMenuWidget::OnPlayAgainButtonClicked);
	GameModesButton->Button->OnClicked.AddDynamic(this, &UPostGameMenuWidget::OnGameModesButtonClicked);
	SettingsButton->Button->OnClicked.AddDynamic(this, &UPostGameMenuWidget::OnSettingsButtonClicked);
	FAQButton->Button->OnClicked.AddDynamic(this, &UPostGameMenuWidget::OnFAQButtonClicked);
	QuitButton->Button->OnClicked.AddDynamic(this, &UPostGameMenuWidget::OnQuitButtonClicked);

	SettingsMenuWidget->AASettingsWidget->OnRestartButtonClicked.BindDynamic(this, &UPostGameMenuWidget::HandleRestart);
	
	FadeInWidgetDelegate.BindDynamic(this, &UPostGameMenuWidget::SetScoresWidgetVisibility);
	BindToAnimationFinished(FadeInWidget, FadeInWidgetDelegate);
	PlayFadeInWidget();
}

void UPostGameMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UPostGameMenuWidget::Restart()
{
	ADefaultPlayerController* Controller = Cast<ADefaultPlayerController>(
	UGameplayStatics::GetPlayerController(GetWorld(), 0));
	Controller->OnScreenFadeToBlackFinish.AddDynamic(this, &UPostGameMenuWidget::HandleRestart);
	Controller->FadeScreenToBlack();
}

void UPostGameMenuWidget::HandleRestart()
{
	Cast<ADefaultGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->InitializeGameMode();
	ADefaultPlayerController* Controller = Cast<ADefaultPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0));
	Controller->HandlePause();
	Controller->HidePostGameMenu();
}

void UPostGameMenuWidget::SetScoresWidgetVisibility()
{
	ScoresWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SlideButtons(ScoresButton);
}

void UPostGameMenuWidget::SlideButtons(const USlideRightButton* ActiveButton)
{
	for (TTuple<USlideRightButton*, UVerticalBox*>& Elem : MenuWidgets)
	{
		if (Elem.Key != ActiveButton)
		{
			Elem.Key->SlideButton(false);
			continue;
		}
		Elem.Key->SlideButton(true);
		MenuSwitcher->SetActiveWidget(Elem.Value);
	}
}

void UPostGameMenuWidget::OnPlayAgainButtonClicked()
{
	SlideButtons(PlayAgainButton);
	QuitMenuWidget->SetVisibility(ESlateVisibility::Visible);
	QuitMenuWidget->PlayFadeInRestartMenu();
}

void UPostGameMenuWidget::OnGameModesButtonClicked()
{
	SlideButtons(GameModesButton);
}

void UPostGameMenuWidget::OnSettingsButtonClicked()
{
	SlideButtons(SettingsButton);
}

void UPostGameMenuWidget::OnFAQButtonClicked()
{
	SlideButtons(FAQButton);
}

void UPostGameMenuWidget::OnQuitButtonClicked()
{
	SlideButtons(QuitButton);
	QuitMenuWidget->SetVisibility(ESlateVisibility::Visible);
	QuitMenuWidget->PlayInitialFadeInMenu();
}

void UPostGameMenuWidget::SlideQuitMenuButtonsLeft()
{
	PlayAgainButton->SlideButton(false);
	QuitButton->SlideButton(false);
}

void UPostGameMenuWidget::OnScoresButtonClicked()
{
}
