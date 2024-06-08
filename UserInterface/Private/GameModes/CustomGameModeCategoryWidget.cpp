// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModeCategoryWidget.h"
#include "Blueprint/WidgetTree.h"
#include "BSGameModeConfig/BSConfig.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Mappings/GameModeCategoryTagMap.h"
#include "MenuOptions/CheckBoxWidget.h"
#include "MenuOptions/ComboBoxWidget.h"
#include "MenuOptions/DualRangeInputWidget.h"
#include "MenuOptions/SingleRangeInputWidget.h"
#include "MenuOptions/TextInputWidget.h"
#include "MenuOptions/ToggleableSingleRangeInputWidget.h"
#include "Utilities/GameModeCategoryTagWidget.h"
#include "Utilities/TooltipWidget.h"
#include "Utilities/ComboBox/BSComboBoxString.h"


void UCustomGameModeCategoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (UMenuOptionWidget* MenuOption = Cast<UMenuOptionWidget>(Widget))
		{
			MenuOptionWidgets.Add(MenuOption);

			AddGameModeCategoryTagWidgets(MenuOption);

			if (UComboBoxWidget* ComboBoxOptionWidget = Cast<UComboBoxWidget>(MenuOption))
			{
				ComboBoxOptionWidget->SetGameplayTagWidgetMap(GameModeCategoryTagMap);
				ComboBoxOptionWidget->SetEnumTagMap(EnumTagMap);
			}
		}
	});
}

void UCustomGameModeCategoryWidget::NativeDestruct()
{
	Super::NativeDestruct();
	BSConfig = nullptr;
}

void UCustomGameModeCategoryWidget::InitComponent(const TSharedPtr<FBSConfig>& InConfig)
{
	BSConfig = InConfig;
}

void UCustomGameModeCategoryWidget::UpdateOptionsFromConfig()
{
}

void UCustomGameModeCategoryWidget::HandlePropertyValidation(
	TSet<FValidationCheckResult, FValidationCheckKeyFuncs>& CheckResults)
{
	for (const auto& Elem : CheckResults)
	{
		for (const auto& [Property, Data] : Elem.PropertyData)
		{
			if (Data.IsEmpty())
			{
				continue;
			}
			if (const TWeakObjectPtr<UMenuOptionWidget>* Found = PropertyMenuOptionWidgetMap.Find(Property->Hash))
			{
				UE_LOG(LogTemp, Display, TEXT("Found widget %s for property %s"), *Found->Get()->GetName(),
					*Property.Get()->PropertyName);
				(*Found)->UpdateDynamicTooltipIcon(Elem.bSuccess, Data, Elem.CalculatedValues);
			}
		}
	}
}

int32 UCustomGameModeCategoryWidget::GetNumberOfDynamicTooltipIcons(const ETooltipIconType Type)
{
	int32 Count = 0;
	for (const auto& [Hash, MenuOptionWidget] : PropertyMenuOptionWidgetMap)
	{
		Count += MenuOptionWidget->GetNumberOfDynamicTooltipIcons(Type);
	}
	return Count;
}

void UCustomGameModeCategoryWidget::AddGameModeCategoryTagWidgets(UMenuOptionWidget* MenuOptionWidget)
{
	if (!GameModeCategoryTagMap)
	{
		return;
	}
	FGameplayTagContainer Container;
	MenuOptionWidget->GetGameModeCategoryTags(Container);
	TArray<UGameModeCategoryTagWidget*> GameModeCategoryTagWidgets;

	for (const FGameplayTag& Tag : Container)
	{
		const TSubclassOf<UUserWidget> SubClass = GameModeCategoryTagMap->GetWidgetByGameModeCategoryTag(Tag);
		if (!SubClass)
		{
			continue;
		}
		UGameModeCategoryTagWidget* TagWidget = CreateWidget<UGameModeCategoryTagWidget>(this, SubClass);
		GameModeCategoryTagWidgets.Add(TagWidget);
	}
	if (GameModeCategoryTagWidgets.Num() > 0)
	{
		MenuOptionWidget->AddGameModeCategoryTagWidgets(GameModeCategoryTagWidgets);
	}
}

bool UCustomGameModeCategoryWidget::UpdateValueIfDifferent(const USingleRangeInputWidget* Widget, const float Value)
{
	if (FMath::IsNearlyEqual(Widget->GetSliderValue(), Value) && FMath::IsNearlyEqual(Widget->GetEditableTextBoxValue(),
		Value))
	{
		return false;
	}

	Widget->SetValue(Value);
	return true;
}

bool UCustomGameModeCategoryWidget::UpdateValueIfDifferent(const UComboBoxWidget* Widget, const FString& NewOption)
{
	if (NewOption.IsEmpty())
	{
		if (Widget->ComboBox->GetSelectedOptionCount() == 0)
		{
			return false;
		}
		Widget->ComboBox->ClearSelection();
		return true;
	}
	if (Widget->ComboBox->GetSelectedOption() == NewOption)
	{
		return false;
	}
	Widget->ComboBox->SetSelectedOption(NewOption);
	return true;
}

bool UCustomGameModeCategoryWidget::UpdateValueIfDifferent(const UComboBoxWidget* Widget,
	const TArray<FString>& NewOptions)
{
	const TArray<FString> SelectedOptions = Widget->ComboBox->GetSelectedOptions();

	if (NewOptions.IsEmpty())
	{
		if (SelectedOptions.IsEmpty())
		{
			return false;
		}
		Widget->ComboBox->ClearSelection();
		return true;
	}

	if (SelectedOptions == NewOptions && SelectedOptions.Num() == NewOptions.Num())
	{
		return false;
	}

	Widget->ComboBox->SetSelectedOptions(NewOptions);
	return true;
}

bool UCustomGameModeCategoryWidget::UpdateValueIfDifferent(const UCheckBoxWidget* Widget, const bool bIsChecked)
{
	if (Widget->CheckBox->IsChecked() == bIsChecked)
	{
		return false;
	}
	Widget->CheckBox->SetIsChecked(bIsChecked);
	return true;
}

bool UCustomGameModeCategoryWidget::UpdateValueIfDifferent(const UTextInputWidget* Widget, const FText& NewText)
{
	if (Widget->EditableTextBox->GetText().EqualTo(NewText))
	{
		return false;
	}
	Widget->EditableTextBox->SetText(NewText);
	return true;
}

bool UCustomGameModeCategoryWidget::UpdateValuesIfDifferent(const UDualRangeInputWidget* Widget, const bool bIsChecked,
	const float Min, const float Max)
{
	bool bDifferent = Widget->IsInConstantMode() != bIsChecked;
	if (bDifferent)
	{
		Widget->SetConstantMode(bIsChecked);
	}

	const bool bMinDifferent = !FMath::IsNearlyEqual(Widget->GetMinSliderValue(false), Min) || !FMath::IsNearlyEqual(
		Widget->GetMinEditableTextBoxValue(false), Min);
	if (bMinDifferent)
	{
		Widget->SetValue_Min(Min);
	}
	bDifferent = bMinDifferent || bDifferent;

	const bool bMaxDifferent = !FMath::IsNearlyEqual(Widget->GetMaxSliderValue(false), Max) || !FMath::IsNearlyEqual(
		Widget->GetMaxEditableTextBoxValue(false), Max);
	if (bMaxDifferent)
	{
		Widget->SetValue_Max(Max);
	}
	bDifferent = bMaxDifferent || bDifferent;

	return bDifferent;
}

bool UCustomGameModeCategoryWidget::UpdateValuesIfDifferent(const UToggleableSingleRangeInputWidget* Widget,
	const bool bIsChecked, const float Value)
{
	// Don't consider the slider/text box value if checked
	if (bIsChecked)
	{
		if (Widget->GetIsChecked() != bIsChecked)
		{
			Widget->SetIsChecked(bIsChecked);
			return true;
		}
		return false;
	}

	bool bDifferent = Widget->GetIsChecked() != bIsChecked;
	if (bDifferent)
	{
		Widget->SetIsChecked(bIsChecked);
	}

	const bool bValueDiff = !FMath::IsNearlyEqual(Widget->GetSliderValue(), Value) || !FMath::IsNearlyEqual(
		Widget->GetEditableTextBoxValue(), Value);

	if (bValueDiff)
	{
		Widget->SetValue(Value);
	}
	bDifferent = bValueDiff || bDifferent;

	return bDifferent;
}

void UCustomGameModeCategoryWidget::SetMenuOptionEnabledStateAndAddTooltip(UMenuOptionWidget* Widget,
	const EMenuOptionEnabledState State, const FString& Key)
{
	Widget->SetMenuOptionEnabledState(State);
	if (State == EMenuOptionEnabledState::DependentMissing && !Key.IsEmpty())
	{
		UTooltipWidget* TooltipWidget = GetTooltipWidget();
		GetTooltipWidget()->SetText(GetTooltipTextFromKey(Key));
		Widget->SetToolTip(TooltipWidget);
	}
	else
	{
		Widget->SetToolTip(nullptr);
	}
}

void UCustomGameModeCategoryWidget::SetSubMenuOptionEnabledStateAndAddTooltip(UMenuOptionWidget* Widget,
	const TSubclassOf<UWidget> SubWidgetClass, const EMenuOptionEnabledState State, const FString& Key)
{
	UWidget* SubWidget = Widget->SetSubMenuOptionEnabledState(SubWidgetClass, State);
	if (!SubWidget)
	{
		return;
	}

	if (State == EMenuOptionEnabledState::DependentMissing && !Key.IsEmpty())
	{
		UTooltipWidget* TooltipWidget = GetTooltipWidget();
		TooltipWidget->SetText(GetTooltipTextFromKey(Key));
		SubWidget->SetToolTip(TooltipWidget);
	}
	else
	{
		SubWidget->SetToolTip(nullptr);
	}
}
