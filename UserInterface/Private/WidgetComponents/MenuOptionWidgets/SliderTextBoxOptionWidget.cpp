// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "WidgetComponents/MenuOptionWidgets/SliderTextBoxOptionWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "BSWidgetInterface.h"
#include "Styles/MenuOptionStyle.h"


void USliderTextBoxOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Slider->OnValueChanged.AddUniqueDynamic(this, &ThisClass::OnSliderChanged_Slider);
	EditableTextBox->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnTextCommitted_EditableTextBox);
}

void USliderTextBoxOptionWidget::SetStyling()
{
	Super::SetStyling();
	if (MenuOptionStyle)
	{
		EditableTextBox->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
	}
}

void USliderTextBoxOptionWidget::SetMenuOptionEnabledState(const EMenuOptionEnabledState EnabledState)
{
	Super::SetMenuOptionEnabledState(EnabledState);
}

void USliderTextBoxOptionWidget::OnSliderChanged_Slider(const float Value)
{
	const float ClampedValue = IBSWidgetInterface::OnSliderChanged(Value, EditableTextBox, GridSnapSize);
	OnSliderTextBoxValueChanged.Broadcast(this, ClampedValue);
}

void USliderTextBoxOptionWidget::OnTextCommitted_EditableTextBox(const FText& Text, ETextCommit::Type CommitType)
{
	const float ClampedValue = IBSWidgetInterface::OnEditableTextBoxChanged(Text, EditableTextBox, Slider, GridSnapSize,
		Slider->GetMinValue(), Slider->GetMaxValue());
}

void USliderTextBoxOptionWidget::SetValues(const float Min, const float Max, const float SnapSize)
{
	Slider->SetMinValue(Min);
	Slider->SetMaxValue(Max);
	Slider->SetStepSize(SnapSize);
	GridSnapSize = SnapSize;
}

void USliderTextBoxOptionWidget::SetValue(const float Value) const
{
	IBSWidgetInterface::SetSliderAndEditableTextBoxValues(Value, EditableTextBox, Slider, GridSnapSize,
		Slider->GetMinValue(), Slider->GetMaxValue());
}

float USliderTextBoxOptionWidget::GetSliderValue() const
{
	return Slider->GetValue();
}

float USliderTextBoxOptionWidget::GetEditableTextBoxValue() const
{
	const FString StringTextValue = EditableTextBox->GetText().ToString().Replace(*FString(","), *FString(),
		ESearchCase::IgnoreCase);
	const float ClampedValue = FMath::Clamp(FCString::Atof(*StringTextValue), Slider->GetMinValue(),
		Slider->GetMaxValue());
	const float SnappedValue = FMath::GridSnap(ClampedValue, GridSnapSize);
	return SnappedValue;
}

float USliderTextBoxOptionWidget::GetSliderValueSnapped() const
{
	return FMath::GridSnap(Slider->GetValue(), GridSnapSize);
}

void USliderTextBoxOptionWidget::SetSliderAndTextBoxEnabledStates(const bool bEnabled) const
{
	Slider->SetLocked(!bEnabled);
	EditableTextBox->SetIsReadOnly(!bEnabled);
}
