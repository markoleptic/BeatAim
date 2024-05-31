// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/BSWidgetInterface.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Kismet/KismetStringLibrary.h"
#include "Utilities/TooltipIcon.h"
#include "Utilities/TooltipWidget.h"
#include "Utilities/ComboBox/BSComboBoxEntry.h"
#include "Utilities/ComboBox/BSComboBoxString.h"

float IBSWidgetInterface::OnEditableTextBoxChanged(const FText& NewTextValue, UEditableTextBox* TextBoxToChange,
	USlider* SliderToChange, const float GridSnapSize, const float Min, const float Max)
{
	const FString StringTextValue = UKismetStringLibrary::Replace(NewTextValue.ToString(), ",", "");
	const float ClampedValue = FMath::Clamp(FCString::Atof(*StringTextValue), Min, Max);
	const float SnappedValue = FMath::GridSnap(ClampedValue, GridSnapSize);
	TextBoxToChange->SetText(FText::AsNumber(SnappedValue));
	SliderToChange->SetValue(SnappedValue);
	return SnappedValue;
}

float IBSWidgetInterface::OnSliderChanged(const float NewValue, UEditableTextBox* TextBoxToChange,
	const float GridSnapSize)
{
	const float ReturnValue = FMath::GridSnap(NewValue, GridSnapSize);
	TextBoxToChange->SetText(FText::AsNumber(ReturnValue));
	return ReturnValue;
}

void IBSWidgetInterface::SetSliderAndEditableTextBoxValues(const float NewValue, UEditableTextBox* TextBoxToChange,
	USlider* SliderToChange, const float GridSnapSize, const float Min, const float Max)
{
	const float ClampedValue = FMath::Clamp(NewValue, Min, Max);
	const float SnappedValue = FMath::GridSnap(ClampedValue, GridSnapSize);
	TextBoxToChange->SetText(FText::AsNumber(SnappedValue));
	SliderToChange->SetValue(SnappedValue);
}

UWidget* IBSWidgetInterface::OnGenerateWidgetEvent(const UBSComboBoxString* ComboBoxString, FString Method)
{
	const FText EntryText = Method.IsEmpty() ? FText::FromString("None Selected") : FText::FromString(Method);
	FText TooltipText = FText::GetEmpty();
	if (const FString Key = GetStringTableKeyFromComboBox(ComboBoxString, Method); !Key.IsEmpty())
	{
		TooltipText = GetTooltipTextFromKey(Key);
	}
	const bool bShowTooltipIcon = !TooltipText.IsEmpty();

	if (UBSComboBoxEntry* Entry = ConstructComboBoxEntryWidget())
	{
		ComboBoxString->InitializeComboBoxEntry(Entry, EntryText, bShowTooltipIcon, MoveTemp(TooltipText));
		return Entry;
	}
	return nullptr;
}

UWidget* IBSWidgetInterface::OnSelectionChanged_GenerateMultiSelectionItem(const UBSComboBoxString* ComboBoxString,
	const TArray<FString>& SelectedOptions)
{
	FString EntryString = FString();

	if (!SelectedOptions.IsEmpty())
	{
		for (int i = 0; i < SelectedOptions.Num(); i++)
		{
			if (!SelectedOptions[i].IsEmpty())
			{
				EntryString.Append(SelectedOptions[i]);
				if (i < SelectedOptions.Num() - 1)
				{
					EntryString.Append(", ");
				}
			}
		}
	}
	FText TooltipText = FText::GetEmpty();
	if (SelectedOptions.Num() == 1)
	{
		if (const FString Key = GetStringTableKeyFromComboBox(ComboBoxString, SelectedOptions[0]); !Key.IsEmpty())
		{
			TooltipText = GetTooltipTextFromKey(Key);
		}
	}

	const FText EntryText = FText::FromString(EntryString);
	const bool bShowTooltipIcon = !TooltipText.IsEmpty();

	if (UBSComboBoxEntry* Entry = ConstructComboBoxEntryWidget())
	{
		ComboBoxString->InitializeComboBoxEntry(Entry, EntryText, bShowTooltipIcon, MoveTemp(TooltipText));
		return Entry;
	}

	return nullptr;
}

FString IBSWidgetInterface::GetStringTableKeyFromComboBox(const UBSComboBoxString* ComboBoxString,
	const FString& EnumString)
{
	return FString();
}

UTooltipWidget* IBSWidgetInterface::GetTooltipWidget()
{
	return UTooltipWidget::Get();
}

void IBSWidgetInterface::SetupTooltip(UTooltipIcon* TooltipIcon, const FText& TooltipText, const bool bInAllowTextWrap)
{
	if (!TooltipIcon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid TooltipIcon."));
	}
	if (TooltipText.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Empty Tooltip Text for %s."), *TooltipIcon->GetParent()->GetParent()->GetName());
	}

	UTooltipWidget* TooltipWidget = GetTooltipWidget();
	TooltipIcon->SetTooltipText(TooltipText, bInAllowTextWrap);
	if (!TooltipIcon->OnTooltipIconHovered.IsBound())
	{
		TooltipIcon->OnTooltipIconHovered.AddUObject(TooltipWidget, &UTooltipWidget::HandleTooltipIconHovered);
	}
}
