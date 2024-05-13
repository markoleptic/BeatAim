// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/ToggleableSingleRangeInputWidget.h"
#include "CommonTextBlock.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Slider.h"
#include "Styles/MenuOptionStyle.h"

void UToggleableSingleRangeInputWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TextBlock_CheckBox)
	{
		TextBlock_CheckBox->SetText(CheckBoxText);
	}
	CheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::OnCheckStateChanged_CheckBox);
	OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxOptionChanged);
}

void UToggleableSingleRangeInputWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (TextBlock_CheckBox)
	{
		TextBlock_CheckBox->SetText(CheckBoxText);
	}
}

void UToggleableSingleRangeInputWidget::SetStyling()
{
	Super::SetStyling();
	if (MenuOptionStyle)
	{
		if (TextBlock_CheckBox)
		{
			TextBlock_CheckBox->SetFont(MenuOptionStyle->Font_DescriptionText);
		}
	}
}

void UToggleableSingleRangeInputWidget::OnSliderTextBoxOptionChanged(USingleRangeInputWidget* Widget, const float Value)
{
	OnSliderTextBoxCheckBoxOptionChanged.Broadcast(this, CheckBox->IsChecked(), Value);
}

bool UToggleableSingleRangeInputWidget::GetIsChecked() const
{
	return CheckBox->IsChecked();
}

void UToggleableSingleRangeInputWidget::SetIsChecked(const bool bIsChecked) const
{
	CheckBox->SetIsChecked(bIsChecked);
	UpdateCheckBoxDependencies(bIsChecked);
}

void UToggleableSingleRangeInputWidget::UpdateCheckBoxDependencies(const bool bConstant) const
{
	if (bConstant)
	{
		Slider->SetLocked(true);
		EditableTextBox->SetIsReadOnly(true);
		//Slider->SetVisibility(ESlateVisibility::Collapsed);
		//EditableTextBox->SetVisibility(ESlateVisibility::Collapsed);
		//if (Box_CheckBox)
		//{
		//	UHorizontalBoxSlot* BoxSlot = Cast<UHorizontalBoxSlot>(Box_CheckBox->Slot);
		//	if (BoxSlot) BoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Type::Fill));
		//}
	}
	else
	{
		Slider->SetLocked(false);
		EditableTextBox->SetIsReadOnly(false);
		//Slider->SetVisibility(ESlateVisibility::Visible);
		//EditableTextBox->SetVisibility(ESlateVisibility::Visible);
		//if (Box_CheckBox)
		//{
		//	UHorizontalBoxSlot* BoxSlot = Cast<UHorizontalBoxSlot>(Box_CheckBox->Slot);
		//	if (BoxSlot) BoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Type::Automatic));
		//}
	}
}

void UToggleableSingleRangeInputWidget::OnCheckStateChanged_CheckBox(const bool bChecked)
{
	UpdateCheckBoxDependencies(bChecked);
	OnSliderTextBoxCheckBoxOptionChanged.Broadcast(this, bChecked, GetSliderValue());
}
