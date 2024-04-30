// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "SubMenuWidgets/SettingsWidgets/SettingsMenuWidget_CrossHair.h"
#include "BSWidgetInterface.h"
#include "Components/CheckBox.h"
#include "Math/Color.h"
#include "OverlayWidgets/HUDWidgets/CrossHairWidget.h"
#include "WidgetComponents/Buttons/BSButton.h"
#include "WidgetComponents/SavedTextWidget.h"
#include "WidgetComponents/MenuOptionWidgets/CheckBoxOptionWidget.h"
#include "WidgetComponents/MenuOptionWidgets/ColorSelectOptionWidget.h"
#include "WidgetComponents/MenuOptionWidgets/SliderTextBoxOptionWidget.h"

void USettingsMenuWidget_CrossHair::NativeConstruct()
{
	Super::NativeConstruct();

	MenuOption_InnerOffset->SetValues(0, 30, 1);
	MenuOption_LineLength->SetValues(0, 100, 1);
	MenuOption_LineWidth->SetValues(0, 100, 1);
	MenuOption_OutlineSize->SetValues(0, 50, 1);
	MenuOption_CrossHairDotSize->SetValues(0, 50, 1);

	MenuOption_InnerOffset->OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);
	MenuOption_LineLength->OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);
	MenuOption_LineWidth->OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);
	MenuOption_OutlineSize->OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);
	MenuOption_CrossHairDotSize->OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);

	Button_Reset->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	Button_Revert->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	Button_Save->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);

	Button_Reset->SetDefaults(static_cast<uint8>(ESettingButtonType::Reset));
	Button_Revert->SetDefaults(static_cast<uint8>(ESettingButtonType::Revert));
	Button_Save->SetDefaults(static_cast<uint8>(ESettingButtonType::Save));

	MenuOption_ColorSelect->OnColorChanged.BindUObject(this, &USettingsMenuWidget_CrossHair::OnColorChanged_CrossHair);
	MenuOption_CrossHairDotColorSelect->OnColorChanged.BindUObject(this,
		&USettingsMenuWidget_CrossHair::OnColorChanged_CrossHairDot);
	MenuOption_OutlineColorSelect->OnColorChanged.BindUObject(this,
		&USettingsMenuWidget_CrossHair::OnColorChanged_CrossHairOutline);

	MenuOption_ShowCrossHairDot->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&USettingsMenuWidget_CrossHair::OnCheckStateChanged_MenuOption_ShowCrossHairDot);

	SavedTextWidget->SetSavedText(FText::FromString("CrossHair Settings Saved"));
	InitialCrossHairSettings = LoadPlayerSettings().CrossHair;
	NewCrossHairSettings = InitialCrossHairSettings;
	SetCrossHairOptions(InitialCrossHairSettings);

	MenuOption_CrossHairDotSize->SetVisibility(ESlateVisibility::Collapsed);
	MenuOption_CrossHairDotColorSelect->SetVisibility(ESlateVisibility::Collapsed);

	UpdateBrushColors();
}

void USettingsMenuWidget_CrossHair::SetCrossHairOptions(const FPlayerSettings_CrossHair& CrossHairSettings)
{
	MenuOption_ColorSelect->InitializeColor(CrossHairSettings.CrossHairColor);
	MenuOption_OutlineColorSelect->InitializeColor(CrossHairSettings.OutlineColor);
	MenuOption_CrossHairDotColorSelect->InitializeColor(CrossHairSettings.CrossHairDotColor);

	MenuOption_InnerOffset->SetValue(CrossHairSettings.InnerOffset);
	MenuOption_LineLength->SetValue(CrossHairSettings.LineLength);
	MenuOption_LineWidth->SetValue(CrossHairSettings.LineWidth);
	MenuOption_OutlineSize->SetValue(CrossHairSettings.OutlineSize);
	MenuOption_CrossHairDotSize->SetValue(CrossHairSettings.CrossHairDotSize);

	MenuOption_ShowCrossHairDot->CheckBox->SetIsChecked(CrossHairSettings.bShowCrossHairDot);

	MenuOption_CrossHairDotSize->SetVisibility(CrossHairSettings.bShowCrossHairDot
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
	MenuOption_CrossHairDotColorSelect->SetVisibility(CrossHairSettings.bShowCrossHairDot
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);

	CrossHairWidget->InitializeCrossHair(CrossHairSettings);
}

void USettingsMenuWidget_CrossHair::OnSliderTextBoxValueChanged(USliderTextBoxOptionWidget* Widget, const float Value)
{
	if (Widget == MenuOption_InnerOffset)
	{
		NewCrossHairSettings.InnerOffset = Value;
		CrossHairWidget->SetInnerOffset(NewCrossHairSettings.InnerOffset);
	}
	else if (Widget == MenuOption_LineLength)
	{
		NewCrossHairSettings.LineLength = Value;
		CrossHairWidget->SetLineSize(FVector2d(NewCrossHairSettings.LineWidth, NewCrossHairSettings.LineLength));
		CrossHairWidget->SetOutlineSize(NewCrossHairSettings.OutlineSize,
			FVector2d(NewCrossHairSettings.LineWidth, NewCrossHairSettings.LineLength));
	}
	else if (Widget == MenuOption_LineWidth)
	{
		NewCrossHairSettings.LineWidth = Value;
		CrossHairWidget->SetLineSize(FVector2d(NewCrossHairSettings.LineWidth, NewCrossHairSettings.LineLength));
		CrossHairWidget->SetOutlineSize(NewCrossHairSettings.OutlineSize,
			FVector2d(NewCrossHairSettings.LineWidth, NewCrossHairSettings.LineLength));
	}
	else if (Widget == MenuOption_OutlineSize)
	{
		NewCrossHairSettings.OutlineSize = Value;
		CrossHairWidget->SetOutlineSize(NewCrossHairSettings.OutlineSize,
			FVector2d(NewCrossHairSettings.LineWidth, NewCrossHairSettings.LineLength));
		CrossHairWidget->SetCrossHairDotSize(NewCrossHairSettings.CrossHairDotSize, NewCrossHairSettings.OutlineSize);
	}
	else if (Widget == MenuOption_CrossHairDotSize)
	{
		NewCrossHairSettings.CrossHairDotSize = Value;
		CrossHairWidget->SetCrossHairDotSize(NewCrossHairSettings.CrossHairDotSize, NewCrossHairSettings.OutlineSize);
	}
}

void USettingsMenuWidget_CrossHair::OnColorChanged_CrossHair(const FLinearColor& NewColor)
{
	NewCrossHairSettings.CrossHairColor = NewColor;
	CrossHairWidget->SetLineColor(NewCrossHairSettings.CrossHairColor);
}

void USettingsMenuWidget_CrossHair::OnColorChanged_CrossHairDot(const FLinearColor& NewColor)
{
	NewCrossHairSettings.CrossHairDotColor = NewColor;
	CrossHairWidget->SetCrossHairDotColor(NewCrossHairSettings.CrossHairDotColor);
}

void USettingsMenuWidget_CrossHair::OnColorChanged_CrossHairOutline(const FLinearColor& NewColor)
{
	NewCrossHairSettings.OutlineColor = NewColor;
	CrossHairWidget->SetOutlineColor(NewCrossHairSettings.OutlineColor);
	CrossHairWidget->SetCrossHairDotOutlineColor(NewCrossHairSettings.OutlineColor);
}

void USettingsMenuWidget_CrossHair::OnCheckStateChanged_MenuOption_ShowCrossHairDot(const bool bIsChecked)
{
	MenuOption_CrossHairDotSize->SetVisibility(bIsChecked
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
	MenuOption_CrossHairDotColorSelect->SetVisibility(bIsChecked
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);

	NewCrossHairSettings.bShowCrossHairDot = bIsChecked;
	CrossHairWidget->SetShowCrossHairDot(NewCrossHairSettings.bShowCrossHairDot);
}

void USettingsMenuWidget_CrossHair::OnButtonClicked_Reset()
{
	NewCrossHairSettings = FPlayerSettings_CrossHair();
	SetCrossHairOptions(NewCrossHairSettings);
}

void USettingsMenuWidget_CrossHair::OnButtonClicked_Revert()
{
	NewCrossHairSettings = InitialCrossHairSettings;
	SetCrossHairOptions(NewCrossHairSettings);
}

void USettingsMenuWidget_CrossHair::OnButtonClicked_Save()
{
	SavePlayerSettings(NewCrossHairSettings);
	SavedTextWidget->PlayFadeInFadeOut();
	InitialCrossHairSettings = NewCrossHairSettings;
}

void USettingsMenuWidget_CrossHair::OnButtonClicked_BSButton(const UBSButton* Button)
{
	switch (static_cast<ESettingButtonType>(Button->GetEnumValue()))
	{
	case ESettingButtonType::Save:
		OnButtonClicked_Save();
		break;
	case ESettingButtonType::Reset:
		OnButtonClicked_Reset();
		break;
	case ESettingButtonType::Revert:
		OnButtonClicked_Revert();
		break;
	default:
		break;
	}
}
