﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "WidgetComponents/MenuOptionWidgets/SliderTextBoxWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "BSWidgetInterface.h"

void USliderTextBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Slider->OnValueChanged.AddUniqueDynamic(this, &ThisClass::OnSliderChanged_Slider);
	EditableTextBox->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnTextCommitted_EditableTextBox);
}

void USliderTextBoxWidget::OnSliderChanged_Slider(const float Value)
{
	const float ClampedValue = IBSWidgetInterface::OnSliderChanged(Value, EditableTextBox, GridSnapSize);
	OnSliderTextBoxValueChanged.Broadcast(this, ClampedValue);
}

void USliderTextBoxWidget::OnTextCommitted_EditableTextBox(const FText& Text, ETextCommit::Type CommitType)
{
	const float ClampedValue = IBSWidgetInterface::OnEditableTextBoxChanged(Text, EditableTextBox, Slider, GridSnapSize, Slider->GetMinValue(), Slider->GetMaxValue());
	OnSliderTextBoxValueChanged.Broadcast(this, ClampedValue);
}

void USliderTextBoxWidget::SetValues(const float Min, const float Max, const float SnapSize)
{
	Slider->SetMinValue(Min);
	Slider->SetMaxValue(Max);
	Slider->SetStepSize(SnapSize);
	GridSnapSize = SnapSize;
}

void USliderTextBoxWidget::SetValue(const float Value)
{
	const float ClampedValue = IBSWidgetInterface::OnSliderChanged(Value, EditableTextBox, GridSnapSize);
	Slider->SetValue(ClampedValue);
}

float USliderTextBoxWidget::GetValue() const
{
	return Slider->GetValue();
}


