// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/SingleRangeInputWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Styles/MenuOptionStyle.h"
#include "Utilities/BSWidgetInterface.h"


void USingleRangeInputWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Slider->OnValueChanged.AddUniqueDynamic(this, &ThisClass::OnSliderChanged_Slider);
	EditableTextBox->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnTextCommitted_EditableTextBox);
}

void USingleRangeInputWidget::SetStyling()
{
	Super::SetStyling();
	if (MenuOptionStyle)
	{
		EditableTextBox->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
	}
}

void USingleRangeInputWidget::SetMenuOptionEnabledState(const EMenuOptionEnabledState EnabledState)
{
	Super::SetMenuOptionEnabledState(EnabledState);
}

void USingleRangeInputWidget::OnSliderChanged_Slider(const float Value)
{
	const float ClampedValue = IBSWidgetInterface::OnSliderChanged(Value, EditableTextBox, GridSnapSize);
	OnSliderTextBoxValueChanged.Broadcast(this, ClampedValue);
}

void USingleRangeInputWidget::OnTextCommitted_EditableTextBox(const FText& Text, ETextCommit::Type CommitType)
{
	const float ClampedValue = IBSWidgetInterface::OnEditableTextBoxChanged(Text, EditableTextBox, Slider, GridSnapSize,
		Slider->GetMinValue(), Slider->GetMaxValue());
}

void USingleRangeInputWidget::SetValues(const float Min, const float Max, const float SnapSize)
{
	Slider->SetMinValue(Min);
	Slider->SetMaxValue(Max);
	Slider->SetStepSize(SnapSize);
	GridSnapSize = SnapSize;
}

void USingleRangeInputWidget::SetValue(const float Value) const
{
	IBSWidgetInterface::SetSliderAndEditableTextBoxValues(Value, EditableTextBox, Slider, GridSnapSize,
		Slider->GetMinValue(), Slider->GetMaxValue());
}

float USingleRangeInputWidget::GetSliderValue() const
{
	return Slider->GetValue();
}

float USingleRangeInputWidget::GetEditableTextBoxValue() const
{
	const FString StringTextValue = EditableTextBox->GetText().ToString().Replace(*FString(","), *FString(),
		ESearchCase::IgnoreCase);
	const float ClampedValue = FMath::Clamp(FCString::Atof(*StringTextValue), Slider->GetMinValue(),
		Slider->GetMaxValue());
	const float SnappedValue = FMath::GridSnap(ClampedValue, GridSnapSize);
	return SnappedValue;
}

float USingleRangeInputWidget::GetSliderValueSnapped() const
{
	return FMath::GridSnap(Slider->GetValue(), GridSnapSize);
}

void USingleRangeInputWidget::SetSliderAndTextBoxEnabledStates(const bool bEnabled) const
{
	Slider->SetLocked(!bEnabled);
	EditableTextBox->SetIsReadOnly(!bEnabled);
}
