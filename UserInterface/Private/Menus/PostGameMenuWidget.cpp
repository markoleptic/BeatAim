// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Menus/PostGameMenuWidget.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Menus/GameModeMenuWidget.h"
#include "Menus/ScoreBrowserWidget.h"
#include "Overlays/AudioSelectWidget.h"
#include "Overlays/QuitMenuWidget.h"
#include "Styles/MenuStyle.h"
#include "Utilities/Buttons/MenuButton.h"

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

	MenuButton_Scores->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_PlayAgain->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_GameModes->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_Settings->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_FAQ->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_Quit->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);

	QuitMenuWidget->OnExitQuitMenu.BindDynamic(this, &ThisClass::UPostGameMenuWidget::SetQuitMenuButtonsInActive);
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

void UPostGameMenuWidget::ShowAudioFormatSelect()
{
	AudioSelectWidget = CreateWidget<UAudioSelectWidget>(this, AudioSelectClass);

	AudioSelectWidget->OnStartButtonClickedDelegate.BindLambda([this](const FBS_AudioConfig& AudioConfig)
	{
		FGameModeTransitionState GameModeTransitionState;

		GameModeTransitionState.TransitionState = ETransitionState::PlayAgain;
		GameModeTransitionState.bSaveCurrentScores = false;
		GameModeTransitionState.BSConfig.AudioConfig.SongTitle = AudioConfig.SongTitle;
		GameModeTransitionState.BSConfig.AudioConfig.SongLength = AudioConfig.SongLength;
		GameModeTransitionState.BSConfig.AudioConfig.InAudioDevice = AudioConfig.InAudioDevice;
		GameModeTransitionState.BSConfig.AudioConfig.SongPath = AudioConfig.SongPath;
		GameModeTransitionState.BSConfig.AudioConfig.bPlaybackAudio = AudioConfig.bPlaybackAudio;
		GameModeTransitionState.BSConfig.AudioConfig.AudioFormat = AudioConfig.AudioFormat;

		GameModesWidget->OnGameModeStateChanged.Broadcast(GameModeTransitionState);
		AudioSelectWidget->FadeOut();
	});

	AudioSelectWidget->AddToViewport();
	AudioSelectWidget->FadeIn();
}

void UPostGameMenuWidget::SetScoresWidgetVisibility()
{
	ScoresWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	MenuButton_Scores->SetActive();
	MenuSwitcher->SetActiveWidget(MenuButton_Scores->GetBox());
}

void UPostGameMenuWidget::OnButtonClicked_BSButton(const UBSButton* Button)
{
	const UMenuButton* MenuButton = Cast<UMenuButton>(Button);
	if (!MenuButton)
	{
		return;
	}

	// Always stop the game mode preview if game modes widget is not visible
	if (MenuButton != MenuButton_GameModes)
	{
		GameModesWidget->StopGameModePreview();
	}

	if (Button == MenuButton_Quit)
	{
		QuitMenuWidget->SetVisibility(ESlateVisibility::Visible);
		QuitMenuWidget->PlayInitialFadeInMenu();
	}
	else if (Button == MenuButton_PlayAgain)
	{
		ShowAudioFormatSelect();
	}

	if (MenuButton->GetBox())
	{
		MenuSwitcher->SetActiveWidget(MenuButton->GetBox());
	}
}

void UPostGameMenuWidget::SetQuitMenuButtonsInActive()
{
	MenuButton_PlayAgain->SetInActive();
	MenuButton_Quit->SetInActive();
}
