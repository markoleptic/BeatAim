// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "SubMenuWidgets/SettingsWidgets/SettingsMenuWidget_Game.h"
#include "BSWidgetInterface.h"
#include "Components/CheckBox.h"
#include "WidgetComponents/Buttons/BSButton.h"
#include "WidgetComponents/SavedTextWidget.h"
#include "WidgetComponents/MenuOptionWidgets/CheckBoxOptionWidget.h"
#include "WidgetComponents/MenuOptionWidgets/ColorSelectOptionWidget.h"
#include "WidgetComponents/MenuOptionWidgets/SliderTextBoxOptionWidget.h"

void USettingsMenuWidget_Game::NativeConstruct()
{
	Super::NativeConstruct();

	MenuOption_StartTargetColor->OnColorChanged.BindUObject(this, &ThisClass::OnColorChanged_StartTarget);
	MenuOption_PeakTargetColor->OnColorChanged.BindUObject(this, &ThisClass::OnOnColorChanged_PeakTarget);
	MenuOption_EndTargetColor->OnColorChanged.BindUObject(this, &ThisClass::OnOnColorChanged_EndTarget);
	MenuOption_InactiveColor->OnColorChanged.BindUObject(this, &ThisClass::OnColorChanged_BeatGridInactive);
	MenuOption_TakingTrackingDamageColor->OnColorChanged.BindUObject(this,
		&ThisClass::OnColorChanged_TakingTrackingDamageColor);
	MenuOption_NotTakingTrackingDamageColor->OnColorChanged.BindUObject(this,
		&ThisClass::OnColorChanged_NotTakingTrackingDamageColor);
	MenuOption_TargetOutlineColor->OnColorChanged.BindUObject(this, &ThisClass::OnColorChanged_TargetOutline);

	MenuOption_CombatTextFrequency->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	MenuOption_CombatTextFrequency->SetValues(0, 100, 1);

	MenuOption_UseSeparateOutlineColor->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_UseSeparateOutlineColor);
	MenuOption_ShowStreakCombatText->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_ShowCombatText);
	MenuOption_Recoil->CheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::OnCheckStateChanged_Recoil);
	MenuOption_AutomaticFire->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_AutomaticFire);
	MenuOption_ShowBulletDecals->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_ShowBulletDecals);
	MenuOption_ShowBulletTracers->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_ShowBulletTracers);
	MenuOption_ShowMuzzleFlash->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_ShowMuzzleFlash);
	MenuOption_ShowMesh->CheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::OnCheckStateChanged_ShowMesh);
	MenuOption_ShowHitTimingWidget->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_ShowHitTimingWidget);

	Button_Reset->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonClicked_BSButton);
	Button_Revert->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonClicked_BSButton);
	Button_Save->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonClicked_BSButton);

	Button_Reset->SetDefaults(static_cast<uint8>(ESettingButtonType::Reset));
	Button_Revert->SetDefaults(static_cast<uint8>(ESettingButtonType::Revert));
	Button_Save->SetDefaults(static_cast<uint8>(ESettingButtonType::Save));

	InitialGameSettings = LoadPlayerSettings().Game;
	NewGameSettings = InitialGameSettings;
	InitializeGameSettings(NewGameSettings);
}

void USettingsMenuWidget_Game::InitializeGameSettings(const FPlayerSettings_Game& PlayerSettings_Game)
{
	MenuOption_StartTargetColor->InitializeColor(PlayerSettings_Game.StartTargetColor);
	MenuOption_PeakTargetColor->InitializeColor(PlayerSettings_Game.PeakTargetColor);
	MenuOption_EndTargetColor->InitializeColor(PlayerSettings_Game.EndTargetColor);
	MenuOption_InactiveColor->InitializeColor(PlayerSettings_Game.InactiveTargetColor);
	MenuOption_TargetOutlineColor->InitializeColor(PlayerSettings_Game.TargetOutlineColor);
	MenuOption_TakingTrackingDamageColor->InitializeColor(PlayerSettings_Game.TakingTrackingDamageColor);
	MenuOption_NotTakingTrackingDamageColor->InitializeColor(PlayerSettings_Game.NotTakingTrackingDamageColor);
	MenuOption_TargetOutlineColor->InitializeColor(PlayerSettings_Game.TargetOutlineColor);

	MenuOption_CombatTextFrequency->SetValue(PlayerSettings_Game.CombatTextFrequency);

	MenuOption_UseSeparateOutlineColor->CheckBox->SetIsChecked(PlayerSettings_Game.bUseSeparateOutlineColor);
	MenuOption_ShowStreakCombatText->CheckBox->SetIsChecked(PlayerSettings_Game.bShowStreakCombatText);
	MenuOption_Recoil->CheckBox->SetIsChecked(PlayerSettings_Game.bShouldRecoil);
	MenuOption_AutomaticFire->CheckBox->SetIsChecked(PlayerSettings_Game.bAutomaticFire);
	MenuOption_ShowBulletDecals->CheckBox->SetIsChecked(PlayerSettings_Game.bShowBulletDecals);
	MenuOption_ShowBulletTracers->CheckBox->SetIsChecked(PlayerSettings_Game.bShowBulletTracers);
	MenuOption_ShowMuzzleFlash->CheckBox->SetIsChecked(PlayerSettings_Game.bShowMuzzleFlash);
	MenuOption_ShowMesh->CheckBox->SetIsChecked(
		PlayerSettings_Game.bShowCharacterMesh && PlayerSettings_Game.bShowWeaponMesh);
	MenuOption_ShowHitTimingWidget->CheckBox->SetIsChecked(PlayerSettings_Game.bShowHitTimingWidget);

	if (PlayerSettings_Game.bShowStreakCombatText)
	{
		MenuOption_CombatTextFrequency->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		MenuOption_CombatTextFrequency->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (PlayerSettings_Game.bUseSeparateOutlineColor)
	{
		MenuOption_TargetOutlineColor->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		MenuOption_TargetOutlineColor->SetVisibility(ESlateVisibility::Collapsed);
	}
	UpdateBrushColors();
}

void USettingsMenuWidget_Game::OnColorChanged_StartTarget(const FLinearColor& NewColor)
{
	NewGameSettings.StartTargetColor = NewColor;
}

void USettingsMenuWidget_Game::OnOnColorChanged_PeakTarget(const FLinearColor& NewColor)
{
	NewGameSettings.PeakTargetColor = NewColor;
}

void USettingsMenuWidget_Game::OnOnColorChanged_EndTarget(const FLinearColor& NewColor)
{
	NewGameSettings.EndTargetColor = NewColor;
}

void USettingsMenuWidget_Game::OnCheckStateChanged_UseSeparateOutlineColor(const bool bIsChecked)
{
	NewGameSettings.bUseSeparateOutlineColor = bIsChecked;
	if (bIsChecked)
	{
		MenuOption_TargetOutlineColor->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		MenuOption_TargetOutlineColor->SetVisibility(ESlateVisibility::Collapsed);
	}
	UpdateBrushColors();
}

void USettingsMenuWidget_Game::OnColorChanged_TargetOutline(const FLinearColor& NewColor)
{
	NewGameSettings.TargetOutlineColor = NewColor;
}

void USettingsMenuWidget_Game::OnColorChanged_BeatGridInactive(const FLinearColor& NewColor)
{
	NewGameSettings.InactiveTargetColor = NewColor;
}

void USettingsMenuWidget_Game::OnColorChanged_TakingTrackingDamageColor(const FLinearColor& NewColor)
{
	NewGameSettings.TakingTrackingDamageColor = NewColor;
}

void USettingsMenuWidget_Game::OnColorChanged_NotTakingTrackingDamageColor(const FLinearColor& NewColor)
{
	NewGameSettings.NotTakingTrackingDamageColor = NewColor;
}

void USettingsMenuWidget_Game::OnSliderTextBoxValueChanged(USliderTextBoxOptionWidget* Widget, const float Value)
{
	if (Widget == MenuOption_CombatTextFrequency)
	{
		NewGameSettings.CombatTextFrequency = Value;
	}
}

void USettingsMenuWidget_Game::OnCheckStateChanged_ShowCombatText(const bool bIsChecked)
{
	NewGameSettings.bShowStreakCombatText = bIsChecked;
	if (bIsChecked)
	{
		MenuOption_CombatTextFrequency->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		MenuOption_CombatTextFrequency->SetVisibility(ESlateVisibility::Collapsed);
	}
	UpdateBrushColors();
}

void USettingsMenuWidget_Game::OnCheckStateChanged_Recoil(const bool bIsChecked)
{
	NewGameSettings.bShouldRecoil = bIsChecked;
}

void USettingsMenuWidget_Game::OnCheckStateChanged_AutomaticFire(const bool bIsChecked)
{
	NewGameSettings.bAutomaticFire = bIsChecked;
}

void USettingsMenuWidget_Game::OnCheckStateChanged_ShowBulletDecals(const bool bIsChecked)
{
	NewGameSettings.bShowBulletDecals = bIsChecked;
}

void USettingsMenuWidget_Game::OnCheckStateChanged_ShowBulletTracers(const bool bIsChecked)
{
	NewGameSettings.bShowBulletTracers = bIsChecked;
}

void USettingsMenuWidget_Game::OnCheckStateChanged_ShowMuzzleFlash(const bool bIsChecked)
{
	NewGameSettings.bShowMuzzleFlash = bIsChecked;
}

void USettingsMenuWidget_Game::OnCheckStateChanged_ShowMesh(const bool bIsChecked)
{
	NewGameSettings.bShowWeaponMesh = bIsChecked;
	NewGameSettings.bShowCharacterMesh = bIsChecked;
}

void USettingsMenuWidget_Game::OnCheckStateChanged_ShowHitTimingWidget(const bool bIsChecked)
{
	NewGameSettings.bShowHitTimingWidget = bIsChecked;
}

void USettingsMenuWidget_Game::OnButtonClicked_BSButton(const UBSButton* Button)
{
	switch (static_cast<ESettingButtonType>(Button->GetEnumValue()))
	{
	case ESettingButtonType::Save:
		OnButtonClicked_Save();
		break;
	case ESettingButtonType::Revert:
		OnButtonClicked_Revert();
		break;
	case ESettingButtonType::Reset:
		OnButtonClicked_Reset();
		break;
	default:
		break;
	}
}

void USettingsMenuWidget_Game::OnButtonClicked_Save()
{
	SavePlayerSettings(NewGameSettings);
	SavedTextWidget->SetSavedText(GetWidgetTextFromKey("SM_Saved_Game"));
	SavedTextWidget->PlayFadeInFadeOut();
	InitialGameSettings = NewGameSettings;
}

void USettingsMenuWidget_Game::OnButtonClicked_Reset()
{
	NewGameSettings.ResetToDefault();
	InitializeGameSettings(NewGameSettings);
}

void USettingsMenuWidget_Game::OnButtonClicked_Revert()
{
	NewGameSettings = InitialGameSettings;
	InitializeGameSettings(NewGameSettings);
}
