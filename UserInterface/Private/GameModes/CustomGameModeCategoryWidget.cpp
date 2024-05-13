// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModeCategoryWidget.h"
#include "GameModeCategoryTagMap.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Utilities/ComboBox/BSComboBoxString.h"
#include "Utilities/GameModeCategoryTagWidget.h"
#include "MenuOptions/CheckBoxWidget.h"
#include "MenuOptions/ComboBoxWidget.h"
#include "MenuOptions/DualRangeInputWidget.h"
#include "MenuOptions/TextInputWidget.h"
#include "MenuOptions/ToggleableSingleRangeInputWidget.h"
#include "MenuOptions/SingleRangeInputWidget.h"
#include "Utilities/TooltipWidget.h"

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

void UCustomGameModeCategoryWidget::UpdateAllOptionsValid()
{
	if (!UpdateWarningTooltips())
	{
		RequestComponentUpdate.Broadcast();
	}
}

void UCustomGameModeCategoryWidget::InitComponent(TSharedPtr<FBSConfig> InConfig, const int32 InIndex)
{
	BSConfig = InConfig;
	UpdateOptionsFromConfig();
	bIsInitialized = true;
	Index = InIndex;
}

void UCustomGameModeCategoryWidget::UpdateOptionsFromConfig()
{
}

void UCustomGameModeCategoryWidget::AddGameModeCategoryTagWidgets(UMenuOptionWidget* MenuOptionWidget)
{
	if (!GameModeCategoryTagMap) return;
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
	if (bDifferent) Widget->SetConstantMode(bIsChecked);

	const bool bMinDifferent = !FMath::IsNearlyEqual(Widget->GetMinSliderValue(false), Min) || !FMath::IsNearlyEqual(
		Widget->GetMinEditableTextBoxValue(false), Min);
	if (bMinDifferent) Widget->SetValue_Min(Min);
	bDifferent = bMinDifferent || bDifferent;

	const bool bMaxDifferent = !FMath::IsNearlyEqual(Widget->GetMaxSliderValue(false), Max) || !FMath::IsNearlyEqual(
		Widget->GetMaxEditableTextBoxValue(false), Max);
	if (bMaxDifferent) Widget->SetValue_Max(Max);
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

	if (bValueDiff) Widget->SetValue(Value);
	bDifferent = bValueDiff || bDifferent;

	return bDifferent;
}

bool UCustomGameModeCategoryWidget::UpdateWarningTooltips()
{
	bool bAllClean = true;
	for (const TWeakObjectPtr<UMenuOptionWidget>& Widget : MenuOptionWidgets)
	{
		Widget->UpdateAllWarningTooltips();
		TArray<FTooltipData>& TooltipData = Widget->GetTooltipWarningData();
		for (FTooltipData& Data : TooltipData)
		{
			if (Data.IsDirty())
			{
				bAllClean = false;
				if (Data.ShouldShowTooltipImage())
				{
					Widget->ConstructTooltipWarningImageIfNeeded(Data);
					SetupTooltip(Data);
				}
				else
				{
					Data.RemoveTooltipImage();
				}
				Data.SetIsClean(true);
			}
		}
	}
	UpdateCustomGameModeCategoryInfo();
	return bAllClean;
}

void UCustomGameModeCategoryWidget::UpdateCustomGameModeCategoryInfo()
{
	int32 NumWarnings = 0;
	int32 NumCautions = 0;
	for (const TWeakObjectPtr<UMenuOptionWidget>& Widget : MenuOptionWidgets)
	{
		NumWarnings += Widget->GetNumberOfWarnings();
		NumCautions += Widget->GetNumberOfCautions();
	}
	CustomGameModeCategoryInfo.Update(NumCautions, NumWarnings);
}

void UCustomGameModeCategoryWidget::SetMenuOptionEnabledStateAndAddTooltip(UMenuOptionWidget* Widget,
	const EMenuOptionEnabledState State, const FString& Key)
{
	Widget->SetMenuOptionEnabledState(State);
	if (State == EMenuOptionEnabledState::DependentMissing && !Key.IsEmpty())
	{
		UTooltipWidget* TooltipWidget = ConstructTooltipWidget();
		TooltipWidget->TooltipDescriptor->SetText(GetTooltipTextFromKey(Key));
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
	if (!SubWidget) return;

	if (State == EMenuOptionEnabledState::DependentMissing && !Key.IsEmpty())
	{
		UTooltipWidget* TooltipWidget = ConstructTooltipWidget();
		TooltipWidget->TooltipDescriptor->SetText(GetTooltipTextFromKey(Key));
		SubWidget->SetToolTip(TooltipWidget);
	}
	else
	{
		SubWidget->SetToolTip(nullptr);
	}
}

float UCustomGameModeCategoryWidget::GetMinRequiredHorizontalSpread() const
{
	return (BSConfig->GridConfig.GridSpacing.X + GetMaxTargetDiameter()) * (BSConfig->GridConfig.
		NumHorizontalGridTargets - 1);
}

float UCustomGameModeCategoryWidget::GetMinRequiredVerticalSpread() const
{
	return (BSConfig->GridConfig.GridSpacing.Y + GetMaxTargetDiameter()) * (BSConfig->GridConfig.NumVerticalGridTargets
		- 1);
}

float UCustomGameModeCategoryWidget::GetMaxTargetDiameter() const
{
	return FMath::Max(BSConfig->TargetConfig.MinSpawnedTargetScale, BSConfig->TargetConfig.MaxSpawnedTargetScale) *
		Constants::SphereTargetDiameter;
}

int32 UCustomGameModeCategoryWidget::GetMaxAllowedNumHorizontalTargets() const
{
	// Total = GridSpacing.X * (NumHorizontalGridTargets - 1) + (NumHorizontalGridTargets - 1) * MaxTargetDiameter;
	// Total = (NumHorizontalGridTargets - 1) * (GridSpacing.X + MaxTargetDiameter);
	// Total / (GridSpacing.X + MaxTargetDiameter) = NumHorizontalGridTargets - 1
	// NumHorizontalGridTargets = Total / (GridSpacing.X + MaxTargetDiameter) + 1
	return Constants::MaxValue_HorizontalSpread / (BSConfig->GridConfig.GridSpacing.X + GetMaxTargetDiameter()) + 1;
}

int32 UCustomGameModeCategoryWidget::GetMaxAllowedNumVerticalTargets() const
{
	// Total = GridSpacing.Y * (NumVerticalGridTargets - 1) + (NumVerticalGridTargets - 1) * MaxTargetDiameter;
	// Total = (NumVerticalGridTargets - 1) * (GridSpacing.Y + MaxTargetDiameter);
	// Total / (GridSpacing.Y + MaxTargetDiameter) = NumVerticalGridTargets - 1
	// NumVerticalGridTargets = Total / (GridSpacing.Y * MaxTargetDiameter) + 1
	return Constants::MaxValue_VerticalSpread / (BSConfig->GridConfig.GridSpacing.Y + GetMaxTargetDiameter()) + 1;
}

float UCustomGameModeCategoryWidget::GetMaxAllowedHorizontalSpacing() const
{
	// Total = GridSpacing.X * (NumHorizontalGridTargets - 1) + (NumHorizontalGridTargets - 1) * MaxTargetDiameter;
	// Total = (NumHorizontalGridTargets - 1) * (GridSpacing.X + MaxTargetDiameter);
	// Total / (NumHorizontalGridTargets - 1) = GridSpacing.X + MaxTargetDiameter;
	// Total / (NumHorizontalGridTargets - 1) - MaxTargetDiameter = GridSpacing.X;
	return Constants::MaxValue_HorizontalSpread / (BSConfig->GridConfig.NumHorizontalGridTargets - 1) -
		GetMaxTargetDiameter();
}

float UCustomGameModeCategoryWidget::GetMaxAllowedVerticalSpacing() const
{
	// Total = GridSpacing.Y * (NumVerticalGridTargets - 1) + (NumVerticalGridTargets - 1) * MaxTargetDiameter;
	// Total = (NumVerticalGridTargets - 1) * (GridSpacing.Y + MaxTargetDiameter);
	// Total / (NumVerticalGridTargets - 1) = GridSpacing.Y + MaxTargetDiameter;
	// Total / (NumVerticalGridTargets - 1) - MaxTargetDiameter = GridSpacing.Y;
	return Constants::MaxValue_VerticalSpread / (BSConfig->GridConfig.NumVerticalGridTargets - 1) -
		GetMaxTargetDiameter();
}

float UCustomGameModeCategoryWidget::GetMaxAllowedTargetScale() const
{
	// Total = GridSpacing.X * (NumHorizontalGridTargets - 1) + (NumHorizontalGridTargets - 1) * SphereTargetDiameter * Scale;
	// Total - (GridSpacing.X * (NumHorizontalGridTargets - 1)) = (NumHorizontalGridTargets - 1) * SphereTargetDiameter * Scale;
	// Total - (GridSpacing.X * (NumHorizontalGridTargets - 1)) = (NumHorizontalGridTargets - 1) * SphereTargetDiameter * Scale;
	// (Total - (GridSpacing.X * (NumHorizontalGridTargets - 1))) / ((NumHorizontalGridTargets - 1) * SphereTargetDiameter) = Scale;
	// Scale = (Total - (GridSpacing.X * (NumHorizontalGridTargets - 1))) / ((NumHorizontalGridTargets - 1) * SphereTargetDiameter)

	const float Horizontal = (Constants::MaxValue_HorizontalSpread - (BSConfig->GridConfig.GridSpacing.X * (BSConfig->
		GridConfig.NumHorizontalGridTargets - 1))) / ((BSConfig->GridConfig.NumHorizontalGridTargets - 1) *
		Constants::SphereTargetDiameter);

	const float Vertical = (Constants::MaxValue_VerticalSpread - (BSConfig->GridConfig.GridSpacing.Y * (BSConfig->
		GridConfig.NumVerticalGridTargets - 1))) / ((BSConfig->GridConfig.NumVerticalGridTargets - 1) *
		Constants::SphereTargetDiameter);
	return FMath::Min(Horizontal, Vertical);
}
