// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModeTargetSizingWidget.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "GameModes/CustomGameModeWidget.h"
#include "MenuOptions/ComboBoxWidget.h"
#include "MenuOptions/DualRangeInputWidget.h"
#include "MenuOptions/SingleRangeInputWidget.h"
#include "Utilities/ComboBox/BSComboBoxString.h"

UCustomGameModeTargetSizingWidget::UCustomGameModeTargetSizingWidget() :
	SliderTextBoxOption_DeactivatedTargetScaleMultiplier(nullptr), MenuOption_TargetScale(nullptr),
	SliderTextBoxOption_StartThreshold(nullptr), SliderTextBoxOption_EndThreshold(nullptr),
	SliderTextBoxOption_DecrementAmount(nullptr), SliderTextBoxOption_LifetimeTargetScaleMultiplier(nullptr),
	ComboBoxOption_ConsecutiveTargetScalePolicy(nullptr),
	LastSelectedConsecutiveTargetScalePolicy(EConsecutiveTargetScalePolicy::None)
{
	GameModeCategory = EGameModeCategory::TargetSizing;
}

void UCustomGameModeTargetSizingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SliderTextBoxOption_DeactivatedTargetScaleMultiplier->SetValues(
		Constants::MinValue_ConsecutiveChargeScaleMultiplier, Constants::MaxValue_ConsecutiveChargeScaleMultiplier,
		Constants::SnapSize_ConsecutiveChargeScaleMultiplier);
	SliderTextBoxOption_StartThreshold->SetValues(Constants::MinValue_DynamicStartThreshold,
		Constants::MaxValue_DynamicStartThreshold, Constants::SnapSize_DynamicStartThreshold);
	SliderTextBoxOption_EndThreshold->SetValues(Constants::MinValue_DynamicEndThreshold,
		Constants::MaxValue_DynamicEndThreshold, Constants::SnapSize_DynamicEndThreshold);
	SliderTextBoxOption_DecrementAmount->SetValues(Constants::MinValue_DynamicDecrementAmount,
		Constants::MaxValue_DynamicDecrementAmount, Constants::SnapSize_DynamicDecrementAmount);
	SliderTextBoxOption_LifetimeTargetScaleMultiplier->SetValues(Constants::MinValue_TargetScale,
		Constants::MaxValue_TargetScale, Constants::SnapSize_TargetScale);
	MenuOption_TargetScale->SetValues(Constants::MinValue_TargetScale, Constants::MaxValue_TargetScale,
		Constants::SnapSize_TargetScale);

	SliderTextBoxOption_DeactivatedTargetScaleMultiplier->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_StartThreshold->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_EndThreshold->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_DecrementAmount->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_LifetimeTargetScaleMultiplier->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	MenuOption_TargetScale->OnMinMaxMenuOptionChanged.AddUObject(this, &ThisClass::OnMinMaxMenuOptionChanged);

	ComboBoxOption_ConsecutiveTargetScalePolicy->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_ConsecutiveTargetScalePolicy);

	ComboBoxOption_ConsecutiveTargetScalePolicy->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_ConsecutiveTargetScalePolicy);

	ComboBoxOption_ConsecutiveTargetScalePolicy->ComboBox->ClearOptions();

	TArray<FString> Options;
	for (const EConsecutiveTargetScalePolicy& Method : TEnumRange<EConsecutiveTargetScalePolicy>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_ConsecutiveTargetScalePolicy->SortAddOptionsAndSetEnumType<EConsecutiveTargetScalePolicy>(Options);
	Options.Empty();

	SliderTextBoxOption_StartThreshold->SetVisibility(ESlateVisibility::Collapsed);
	SliderTextBoxOption_EndThreshold->SetVisibility(ESlateVisibility::Collapsed);
	SliderTextBoxOption_DecrementAmount->SetVisibility(ESlateVisibility::Collapsed);

	SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_LifetimeTargetScaleMultiplier,
		EMenuOptionEnabledState::Enabled);
	SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_DeactivatedTargetScaleMultiplier,
		EMenuOptionEnabledState::Enabled);

	SetupWarningTooltipCallbacks();
	UpdateBrushColors();
}

void UCustomGameModeTargetSizingWidget::UpdateOptionsFromConfig()
{
	UpdateValueIfDifferent(SliderTextBoxOption_DeactivatedTargetScaleMultiplier,
		BSConfig->TargetConfig.ConsecutiveChargeScaleMultiplier);
	UpdateValueIfDifferent(SliderTextBoxOption_StartThreshold, BSConfig->DynamicTargetScaling.StartThreshold);
	UpdateValueIfDifferent(SliderTextBoxOption_EndThreshold, BSConfig->DynamicTargetScaling.EndThreshold);
	UpdateValueIfDifferent(SliderTextBoxOption_DecrementAmount, BSConfig->DynamicTargetScaling.DecrementAmount);
	UpdateValueIfDifferent(SliderTextBoxOption_LifetimeTargetScaleMultiplier,
		BSConfig->TargetConfig.LifetimeTargetScaleMultiplier);
	UpdateValuesIfDifferent(MenuOption_TargetScale,
		BSConfig->TargetConfig.ConsecutiveTargetScalePolicy == EConsecutiveTargetScalePolicy::Static,
		BSConfig->TargetConfig.MinSpawnedTargetScale, BSConfig->TargetConfig.MaxSpawnedTargetScale);

	UpdateValueIfDifferent(ComboBoxOption_ConsecutiveTargetScalePolicy,
		GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.ConsecutiveTargetScalePolicy));

	UpdateDependentOptions_TargetActivationResponses(BSConfig->TargetConfig.TargetActivationResponses);
	UpdateDependentOptions_TargetDeactivationResponses(BSConfig->TargetConfig.TargetDeactivationConditions,
		BSConfig->TargetConfig.TargetDeactivationResponses);
	UpdateDependentOptions_ConsecutiveTargetScalePolicy(BSConfig->TargetConfig.ConsecutiveTargetScalePolicy);

	UpdateBrushColors();
}

void UCustomGameModeTargetSizingWidget::SetupWarningTooltipCallbacks()
{
	MenuOption_TargetScale->AddDynamicWarningTooltipData(
		FTooltipData("Invalid_Grid_MaxSpawnedTargetScale", ETooltipImageType::Warning),
		"Invalid_Grid_MaxSpawnedTargetScale_Fallback", Constants::MinValue_TargetScale, 2).BindLambda([this]()
	{
		const float Max = FMath::Max(BSConfig->TargetConfig.MaxSpawnedTargetScale,
			BSConfig->TargetConfig.MinSpawnedTargetScale);
		return FDynamicTooltipState(Max, GetMaxAllowedTargetScale());
	});
}

void UCustomGameModeTargetSizingWidget::UpdateDependentOptions_TargetActivationResponses(
	const TArray<ETargetActivationResponse>& InResponses)
{
	if (InResponses.Contains(ETargetActivationResponse::ApplyLifetimeTargetScaling))
	{
		SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_LifetimeTargetScaleMultiplier,
			EMenuOptionEnabledState::Enabled);
	}
	else
	{
		SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_LifetimeTargetScaleMultiplier,
			EMenuOptionEnabledState::DependentMissing, "DM_LifetimeTargetScaleMultiplier");
	}
}

void UCustomGameModeTargetSizingWidget::UpdateDependentOptions_TargetDeactivationResponses(
	const TArray<ETargetDeactivationCondition>& Conditions, const TArray<ETargetDeactivationResponse>& Responses)
{
	if (Responses.Contains(ETargetDeactivationResponse::ApplyDeactivatedTargetScaleMultiplier))
	{
		SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_DeactivatedTargetScaleMultiplier,
			EMenuOptionEnabledState::Enabled);
	}
	else
	{
		SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_DeactivatedTargetScaleMultiplier,
			EMenuOptionEnabledState::DependentMissing, "DM_DeactivatedTargetScaleMultiplier");
	}
}

void UCustomGameModeTargetSizingWidget::UpdateDependentOptions_ConsecutiveTargetScalePolicy(
	const EConsecutiveTargetScalePolicy InConsecutiveTargetScalePolicy)
{
	if (InConsecutiveTargetScalePolicy == EConsecutiveTargetScalePolicy::SkillBased)
	{
		SliderTextBoxOption_StartThreshold->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		SliderTextBoxOption_EndThreshold->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		SliderTextBoxOption_DecrementAmount->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		SliderTextBoxOption_StartThreshold->SetVisibility(ESlateVisibility::Collapsed);
		SliderTextBoxOption_EndThreshold->SetVisibility(ESlateVisibility::Collapsed);
		SliderTextBoxOption_DecrementAmount->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (InConsecutiveTargetScalePolicy == EConsecutiveTargetScalePolicy::Static)
	{
		MenuOption_TargetScale->SetConstantMode(true);
	}
	else
	{
		MenuOption_TargetScale->SetConstantMode(false);
		LastSelectedConsecutiveTargetScalePolicy = InConsecutiveTargetScalePolicy;
	}
}

void UCustomGameModeTargetSizingWidget::OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value)
{
	if (Widget == SliderTextBoxOption_DeactivatedTargetScaleMultiplier)
	{
		BSConfig->TargetConfig.ConsecutiveChargeScaleMultiplier = Value;
	}
	else if (Widget == SliderTextBoxOption_StartThreshold)
	{
		BSConfig->DynamicTargetScaling.StartThreshold = Value;
	}
	else if (Widget == SliderTextBoxOption_EndThreshold)
	{
		BSConfig->DynamicTargetScaling.EndThreshold = Value;
	}
	else if (Widget == SliderTextBoxOption_DecrementAmount)
	{
		BSConfig->DynamicTargetScaling.DecrementAmount = Value;
	}
	else if (Widget == SliderTextBoxOption_LifetimeTargetScaleMultiplier)
	{
		BSConfig->TargetConfig.LifetimeTargetScaleMultiplier = Value;
	}
	//UpdateAllOptionsValid();
}

void UCustomGameModeTargetSizingWidget::OnMinMaxMenuOptionChanged(UDualRangeInputWidget* Widget, const bool bChecked,
	const float MinOrConstant, const float Max)
{
	if (Widget == MenuOption_TargetScale)
	{
		BSConfig->TargetConfig.MinSpawnedTargetScale = MinOrConstant;
		BSConfig->TargetConfig.MaxSpawnedTargetScale = bChecked ? MinOrConstant : Max;

		// Constant has been checked while Random or Skill-Based
		if (bChecked && BSConfig->TargetConfig.ConsecutiveTargetScalePolicy != EConsecutiveTargetScalePolicy::Static)
		{
			BSConfig->TargetConfig.ConsecutiveTargetScalePolicy = EConsecutiveTargetScalePolicy::Static;
			UpdateValueIfDifferent(ComboBoxOption_ConsecutiveTargetScalePolicy,
				GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.ConsecutiveTargetScalePolicy));
			UpdateDependentOptions_ConsecutiveTargetScalePolicy(BSConfig->TargetConfig.ConsecutiveTargetScalePolicy);
		}
		// Constant has been unchecked while Static
		else if (!bChecked && BSConfig->TargetConfig.ConsecutiveTargetScalePolicy ==
			EConsecutiveTargetScalePolicy::Static)
		{
			if (LastSelectedConsecutiveTargetScalePolicy != EConsecutiveTargetScalePolicy::None)
			{
				BSConfig->TargetConfig.ConsecutiveTargetScalePolicy = LastSelectedConsecutiveTargetScalePolicy;
			}
			else
			{
				BSConfig->TargetConfig.ConsecutiveTargetScalePolicy = EConsecutiveTargetScalePolicy::Random;
			}
			UpdateValueIfDifferent(ComboBoxOption_ConsecutiveTargetScalePolicy,
				GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.ConsecutiveTargetScalePolicy));
			UpdateDependentOptions_ConsecutiveTargetScalePolicy(BSConfig->TargetConfig.ConsecutiveTargetScalePolicy);
		}
	}
	UpdateBrushColors();
	//UpdateAllOptionsValid();
}

void UCustomGameModeTargetSizingWidget::OnSelectionChanged_ConsecutiveTargetScalePolicy(const TArray<FString>& Selected,
	const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}

	BSConfig->TargetConfig.ConsecutiveTargetScalePolicy = GetEnumFromString_FromTagMap<
		EConsecutiveTargetScalePolicy>(Selected[0]);
	UpdateDependentOptions_ConsecutiveTargetScalePolicy(BSConfig->TargetConfig.ConsecutiveTargetScalePolicy);

	if (BSConfig->TargetConfig.ConsecutiveTargetScalePolicy == EConsecutiveTargetScalePolicy::Static)
	{
		BSConfig->TargetConfig.MinSpawnedTargetScale = MenuOption_TargetScale->GetMinSliderValue(true);
		BSConfig->TargetConfig.MaxSpawnedTargetScale = BSConfig->TargetConfig.MinSpawnedTargetScale;
	}
	else
	{
		BSConfig->TargetConfig.MinSpawnedTargetScale = MenuOption_TargetScale->GetMinSliderValue(true);
		BSConfig->TargetConfig.MaxSpawnedTargetScale = MenuOption_TargetScale->GetMaxSliderValue(true);
	}
	UpdateBrushColors();
	//UpdateAllOptionsValid();
}

FString UCustomGameModeTargetSizingWidget::GetComboBoxEntryTooltipStringTableKey_ConsecutiveTargetScalePolicy(
	const FString& EnumString)
{
	const EConsecutiveTargetScalePolicy EnumValue = GetEnumFromString_FromTagMap<
		EConsecutiveTargetScalePolicy>(EnumString);
	return GetStringTableKeyNameFromEnum(EnumValue);
}
