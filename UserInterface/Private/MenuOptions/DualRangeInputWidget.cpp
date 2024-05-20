// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/DualRangeInputWidget.h"
#include "CommonTextBlock.h"
#include "Components/BorderSlot.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Slider.h"
#include "Components/Spacer.h"
#include "Styles/MenuOptionStyle.h"
#include "Utilities/BSHorizontalBox.h"
#include "Utilities/BSWidgetInterface.h"

void UDualRangeInputWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TextBlock_Max->SetText(MaxText);

	Slider_Min->OnValueChanged.AddUniqueDynamic(this, &ThisClass::OnSliderChanged_Min);
	Slider_Max->OnValueChanged.AddUniqueDynamic(this, &ThisClass::OnSliderChanged_Max);
	EditableTextBox_Min->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnTextCommitted_Min);
	EditableTextBox_Max->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnTextCommitted_Max);
	CheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::OnCheckStateChanged_CheckBox);
}

void UDualRangeInputWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (TextBlock_Max)
	{
		TextBlock_Max->SetText(MaxText);
	}
}

void UDualRangeInputWidget::SetStyling()
{
	Super::SetStyling();
	if (MenuOptionStyle)
	{
		if (EditableTextBox_Min)
		{
			EditableTextBox_Min->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		}
		if (EditableTextBox_Max)
		{
			EditableTextBox_Max->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		}
		if (TextBlock_Min)
		{
			TextBlock_Min->SetFont(MenuOptionStyle->Font_DescriptionText);
		}
		if (TextBlock_Max)
		{
			TextBlock_Max->SetFont(MenuOptionStyle->Font_DescriptionText);
		}
		if (TextBlock_Description_Min)
		{
			TextBlock_Description_Min->SetFont(MenuOptionStyle->Font_DescriptionText);
			UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(TextBlock_Description_Min->Slot);
			if (HorizontalBoxSlot)
			{
				HorizontalBoxSlot->SetPadding(MenuOptionStyle->Padding_DescriptionText);
			}
		}
		if (TextBlock_Description_Max)
		{
			TextBlock_Description_Max->SetFont(MenuOptionStyle->Font_DescriptionText);
			UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(TextBlock_Description_Max->Slot);
			if (HorizontalBoxSlot)
			{
				HorizontalBoxSlot->SetPadding(MenuOptionStyle->Padding_DescriptionText);
			}
		}
		if (Indent_Left_Min)
		{
			Indent_Left_Min->SetSize(FVector2d(IndentLevel * MenuOptionStyle->IndentAmount, 0.f));
		}
		if (Indent_Left_Max)
		{
			Indent_Left_Max->SetSize(FVector2d(IndentLevel * MenuOptionStyle->IndentAmount, 0.f));
		}
		if (Box_Left_Max)
		{
			if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(Box_Left_Max->Slot))
			{
				BorderSlot->SetPadding(MenuOptionStyle->Padding_LeftBox);
			}
		}
		if (Box_Left_Min)
		{
			if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(Box_Left_Min->Slot))
			{
				BorderSlot->SetPadding(MenuOptionStyle->Padding_LeftBox);
			}
		}
		if (Box_Right_Min)
		{
			if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(Box_Right_Min->Slot))
			{
				BorderSlot->SetPadding(MenuOptionStyle->Padding_RightBox);
			}
		}
		if (Box_Right_Max)
		{
			if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(Box_Right_Max->Slot))
			{
				BorderSlot->SetPadding(MenuOptionStyle->Padding_RightBox);
			}
		}
	}
}

void UDualRangeInputWidget::SetValues(const float Min, const float Max, const float SnapSize)
{
	Slider_Min->SetMinValue(Min);
	Slider_Max->SetMinValue(Min);
	Slider_Min->SetMaxValue(Max);
	Slider_Max->SetMaxValue(Max);
	Slider_Min->SetStepSize(SnapSize);
	Slider_Max->SetStepSize(SnapSize);
	GridSnapSize = SnapSize;
}

void UDualRangeInputWidget::SetValue_Min(const float Value) const
{
	IBSWidgetInterface::SetSliderAndEditableTextBoxValues(Value, EditableTextBox_Min, Slider_Min, GridSnapSize,
		Slider_Min->GetMinValue(), Slider_Min->GetMaxValue());
}

void UDualRangeInputWidget::SetValue_Max(const float Value) const
{
	IBSWidgetInterface::SetSliderAndEditableTextBoxValues(Value, EditableTextBox_Max, Slider_Max, GridSnapSize,
		Slider_Max->GetMinValue(), Slider_Max->GetMaxValue());
}

void UDualRangeInputWidget::SetValue_Constant(const float Value) const
{
	SetValue_Min(Value);
	SetValue_Max(Value);
}

void UDualRangeInputWidget::SetConstantMode(const bool bUseConstantMode) const
{
	CheckBox->SetIsChecked(bUseConstantMode);
	UpdateMinMaxDependencies(bUseConstantMode);
}

void UDualRangeInputWidget::SetConstantModeLocked(const bool bLock) const
{
	CheckBox->SetIsEnabled(!bLock);
}

void UDualRangeInputWidget::SetMenuOptionEnabledState(const EMenuOptionEnabledState EnabledState)
{
	Super::SetMenuOptionEnabledState(EnabledState);

	switch (EnabledState)
	{
	case EMenuOptionEnabledState::Enabled:
		Box_Right_Min->SetIsEnabled(true);
		Box_Right_Max->SetIsEnabled(true);
		break;
	case EMenuOptionEnabledState::DependentMissing:
		Box_Right_Min->SetIsEnabled(false);
		Box_Right_Max->SetIsEnabled(false);
		break;
	case EMenuOptionEnabledState::Disabled:
		break;
	}
}

UWidget* UDualRangeInputWidget::SetSubMenuOptionEnabledState(const TSubclassOf<UWidget> SubWidget,
	const EMenuOptionEnabledState State)
{
	if (SubWidget->GetDefaultObject()->IsA<UCheckBox>())
	{
		switch (State)
		{
		case EMenuOptionEnabledState::Enabled:
			CheckBox->SetIsEnabled(true);
			break;
		case EMenuOptionEnabledState::DependentMissing:
			CheckBox->SetIsEnabled(false);
			break;
		case EMenuOptionEnabledState::Disabled:
			break;
		}
		return Box_CheckBox_Tooltip;
	}
	return nullptr;
}

float UDualRangeInputWidget::GetMinSliderValue(const bool bClamped) const
{
	if (!bClamped)
	{
		return Slider_Min->GetValue();
	}

	const float ClampedValue = FMath::Clamp(Slider_Min->GetValue(), Slider_Min->GetMinValue(),
		Slider_Min->GetMaxValue());
	const float SnappedValue = FMath::GridSnap(ClampedValue, GridSnapSize);
	return SnappedValue;
}

float UDualRangeInputWidget::GetMaxSliderValue(const bool bClamped) const
{
	if (!bClamped)
	{
		return Slider_Max->GetValue();
	}

	const float ClampedValue = FMath::Clamp(Slider_Max->GetValue(), Slider_Max->GetMinValue(),
		Slider_Max->GetMaxValue());
	const float SnappedValue = FMath::GridSnap(ClampedValue, GridSnapSize);
	return SnappedValue;
}

float UDualRangeInputWidget::GetConstantSliderValue(const bool bClamped) const
{
	if (bUseMinAsConstant)
	{
		return GetMinSliderValue(bClamped);
	}
	return GetMaxSliderValue(bClamped);
}

float UDualRangeInputWidget::GetMinEditableTextBoxValue(const bool bClamped) const
{
	const FString StringTextValue = EditableTextBox_Min->GetText().ToString().Replace(*FString(","), *FString(),
		ESearchCase::IgnoreCase);

	if (!bClamped)
	{
		return FCString::Atof(*StringTextValue);
	}

	const float ClampedValue = FMath::Clamp(FCString::Atof(*StringTextValue), Slider_Min->GetMinValue(),
		Slider_Min->GetMaxValue());
	const float SnappedValue = FMath::GridSnap(ClampedValue, GridSnapSize);
	return SnappedValue;
}

float UDualRangeInputWidget::GetMaxEditableTextBoxValue(const bool bClamped) const
{
	const FString StringTextValue = EditableTextBox_Max->GetText().ToString().Replace(*FString(","), *FString(),
		ESearchCase::IgnoreCase);

	if (!bClamped)
	{
		return FCString::Atof(*StringTextValue);
	}

	const float ClampedValue = FMath::Clamp(FCString::Atof(*StringTextValue), Slider_Max->GetMinValue(),
		Slider_Max->GetMaxValue());
	const float SnappedValue = FMath::GridSnap(ClampedValue, GridSnapSize);
	return SnappedValue;
}

float UDualRangeInputWidget::GetConstantEditableTextBoxValue(const bool bClamped) const
{
	if (bUseMinAsConstant)
	{
		return GetMinEditableTextBoxValue(bClamped);
	}
	return GetMaxEditableTextBoxValue(bClamped);
}

bool UDualRangeInputWidget::IsInConstantMode() const
{
	return CheckBox->IsChecked();
}

void UDualRangeInputWidget::SetSliderAndTextBoxEnabledStates(const bool bEnabled) const
{
	Slider_Min->SetLocked(!bEnabled);
	Slider_Max->SetLocked(!bEnabled);
	EditableTextBox_Min->SetIsReadOnly(!bEnabled);
	EditableTextBox_Max->SetIsReadOnly(!bEnabled);
}

void UDualRangeInputWidget::UpdateMinMaxDependencies(const bool bConstant) const
{
	if (bConstant)
	{
		if (bUseMinAsConstant)
		{
			TextBlock_Min->SetVisibility(ESlateVisibility::Collapsed);
			BSBox_Max->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			TextBlock_Max->SetVisibility(ESlateVisibility::Collapsed);
			BSBox_Min->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		TextBlock_Min->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		TextBlock_Min->SetText(MinText);
		BSBox_Max->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		TextBlock_Max->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		TextBlock_Max->SetText(MaxText);
		BSBox_Min->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UDualRangeInputWidget::OnSliderChanged_Min(const float Value)
{
	const float ClampedValue = IBSWidgetInterface::OnSliderChanged(Value, EditableTextBox_Min, GridSnapSize);
	if (IsInConstantMode())
	{
		if (bUseMinAsConstant)
		{
			OnMinMaxMenuOptionChanged.Broadcast(this, true, ClampedValue, ClampedValue);
		}
	}
	else
	{
		OnMinMaxMenuOptionChanged.Broadcast(this, false, ClampedValue, GetMaxSliderValue(true));
	}
}

void UDualRangeInputWidget::OnSliderChanged_Max(const float Value)
{
	const float ClampedValue = IBSWidgetInterface::OnSliderChanged(Value, EditableTextBox_Max, GridSnapSize);
	if (IsInConstantMode())
	{
		if (!bUseMinAsConstant)
		{
			OnMinMaxMenuOptionChanged.Broadcast(this, true, ClampedValue, ClampedValue);
		}
	}
	else
	{
		OnMinMaxMenuOptionChanged.Broadcast(this, false, GetMinSliderValue(true), ClampedValue);
	}
}

void UDualRangeInputWidget::OnTextCommitted_Min(const FText& Text, ETextCommit::Type CommitType)
{
	const float ClampedValue = IBSWidgetInterface::OnEditableTextBoxChanged(Text, EditableTextBox_Min, Slider_Min,
		GridSnapSize, Slider_Min->GetMinValue(), Slider_Min->GetMaxValue());
	if (IsInConstantMode())
	{
		if (bUseMinAsConstant)
		{
			OnMinMaxMenuOptionChanged.Broadcast(this, true, ClampedValue, ClampedValue);
		}
	}
	else
	{
		OnMinMaxMenuOptionChanged.Broadcast(this, false, ClampedValue, GetMaxSliderValue(true));
	}
}

void UDualRangeInputWidget::OnTextCommitted_Max(const FText& Text, ETextCommit::Type CommitType)
{
	const float ClampedValue = IBSWidgetInterface::OnEditableTextBoxChanged(Text, EditableTextBox_Max, Slider_Max,
		GridSnapSize, Slider_Max->GetMinValue(), Slider_Max->GetMaxValue());
	if (IsInConstantMode())
	{
		if (!bUseMinAsConstant)
		{
			OnMinMaxMenuOptionChanged.Broadcast(this, true, ClampedValue, ClampedValue);
		}
	}
	else
	{
		OnMinMaxMenuOptionChanged.Broadcast(this, false, GetMinSliderValue(true), ClampedValue);
	}
}

void UDualRangeInputWidget::OnCheckStateChanged_CheckBox(const bool bChecked)
{
	UpdateMinMaxDependencies(bChecked);
	OnMinMaxMenuOptionChanged.Broadcast(this, bChecked, GetMinSliderValue(true), GetMaxSliderValue(true));
}
