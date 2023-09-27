// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuWidgets/PostGameMenuWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/VerticalBox.h"
#include "OverlayWidgets/PopupWidgets/QuitMenuWidget.h"
#include "Styles/MenuStyle.h"
#include "SubMenuWidgets/SettingsWidgets/SettingsMenuWidget.h"
#include "SubMenuWidgets/ScoreBrowserWidget.h"
#include "SubMenuWidgets/GameModesWidgets/GameModesWidget.h"
#include "WidgetComponents/Buttons/MenuButton.h"

void UPostGameMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetStyles();
	MenuButton_Scores->SetDefaults(Box_Scores, MenuButton_PlayAgain);
	MenuButton_PlayAgain->SetDefaults(nullptr, MenuButton_GameModes);
	MenuButton_GameModes->SetDefaults(Box_GameModes, MenuButton_Settings);
	MenuButton_Settings->SetDefaults(Box_Settings, MenuButton_FAQ);
	MenuButton_FAQ->SetDefaults(Box_FAQ, MenuButton_Quit);
	MenuButton_Quit->SetDefaults(nullptr, MenuButton_Scores);
	
	MenuButton_Scores->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_PlayAgain->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_GameModes->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_Settings->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_FAQ->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_Quit->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonClicked_BSButton);

	QuitMenuWidget->OnExitQuitMenu.BindUFunction(this, "SetQuitMenuButtonsInActive");
	SettingsMenuWidget->OnRestartButtonClicked.BindUFunction(this, "Restart");
	FadeInWidgetDelegate.BindDynamic(this, &UPostGameMenuWidget::SetScoresWidgetVisibility);
	
	BindToAnimationFinished(FadeInWidget, FadeInWidgetDelegate);
	PlayFadeInWidget();
}

void UPostGameMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	SetStyles();
}

void UPostGameMenuWidget::SetStyles()
{
	MenuStyle = IBSWidgetInterface::GetStyleCDO(MenuStyleClass);
}

void UPostGameMenuWidget::Restart()
{
	FGameModeTransitionState GameModeTransitionState;
	GameModeTransitionState.TransitionState = ETransitionState::Restart;
	GameModeTransitionState.bSaveCurrentScores = false;
	GameModesWidget->OnGameModeStateChanged.Broadcast(GameModeTransitionState);
}

void UPostGameMenuWidget::SetScoresWidgetVisibility()
{
	ScoresWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	MenuButton_Scores->SetActive();
	MenuSwitcher->SetActiveWidget(MenuButton_Scores->GetBox());
}

void UPostGameMenuWidget::OnButtonClicked_BSButton(const UBSButton* Button)
{
	if (Button == MenuButton_Quit)
	{
		QuitMenuWidget->SetVisibility(ESlateVisibility::Visible);
		QuitMenuWidget->PlayInitialFadeInMenu();
	}
	else if (Button == MenuButton_PlayAgain)
	{
		Restart();
		return;
	}
	if (const UMenuButton* MenuButton = Cast<UMenuButton>(Button))
	{
		if (MenuButton->GetBox())
		{
			MenuSwitcher->SetActiveWidget(MenuButton->GetBox());
		}
	}
}

void UPostGameMenuWidget::SetQuitMenuButtonsInActive()
{
	MenuButton_PlayAgain->SetInActive();
	MenuButton_Quit->SetInActive();
}
