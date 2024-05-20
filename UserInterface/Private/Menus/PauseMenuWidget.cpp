// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Menus/PauseMenuWidget.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Menus/SettingsMenuWidget.h"
#include "Overlays/QuitMenuWidget.h"
#include "Styles/MenuStyle.h"
#include "Utilities/BSWidgetInterface.h"
#include "Utilities/Buttons/BSButton.h"
#include "Utilities/Buttons/MenuButton.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetStyles();
	MenuButton_Resume->SetDefaults(Box_PauseScreen, MenuButton_Settings);
	MenuButton_Settings->SetDefaults(Box_SettingsMenu, MenuButton_FAQ);
	MenuButton_FAQ->SetDefaults(Box_FAQ, MenuButton_RestartCurrentMode);
	MenuButton_RestartCurrentMode->SetDefaults(Box_PauseScreen, MenuButton_Quit);
	MenuButton_Quit->SetDefaults(Box_PauseScreen, MenuButton_Resume);

	MenuButton_Resume->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_Settings->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_FAQ->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_RestartCurrentMode->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	MenuButton_Quit->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);

	QuitMenuWidget->OnExitQuitMenu.BindUFunction(this, "SetQuitMenuButtonsInActive");
	SettingsMenuWidget->OnRestartButtonClicked.BindUFunction(this, "OnButtonClicked_RestartCurrentMode");
	FadeInWidget();
}

void UPauseMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	SetStyles();
}

void UPauseMenuWidget::SetStyles()
{
	MenuStyle = IBSWidgetInterface::GetStyleCDO(MenuStyleClass);
}

void UPauseMenuWidget::SetQuitMenuButtonsInActive()
{
	MenuButton_RestartCurrentMode->SetInActive();
	MenuButton_Quit->SetInActive();
}

void UPauseMenuWidget::OnButtonClicked_BSButton(const UBSButton* Button)
{
	if (Button == MenuButton_Resume)
	{
		if (!ResumeGame.ExecuteIfBound())
		{
			UE_LOG(LogTemp, Display, TEXT("ResumeGame not bound."));
		}
	}
	else if (Button == MenuButton_Quit)
	{
		QuitMenuWidget->SetVisibility(ESlateVisibility::Visible);
		QuitMenuWidget->PlayInitialFadeInMenu();
	}
	else if (Button == MenuButton_RestartCurrentMode)
	{
		QuitMenuWidget->SetVisibility(ESlateVisibility::Visible);
		QuitMenuWidget->PlayFadeInRestartMenu();
	}

	if (const UMenuButton* MenuButton = Cast<UMenuButton>(Button))
	{
		if (MenuButton->GetBox())
		{
			PauseMenuSwitcher->SetActiveWidget(MenuButton->GetBox());
		}
	}
}

void UPauseMenuWidget::OnButtonClicked_RestartCurrentMode()
{
	QuitMenuWidget->SetVisibility(ESlateVisibility::Visible);
	QuitMenuWidget->PlayFadeInRestartMenu();

	MenuButton_RestartCurrentMode->SetActive();
	PauseMenuSwitcher->SetActiveWidget(MenuButton_RestartCurrentMode);
}
