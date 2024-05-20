// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModeGeneralWidget.h"
#include "Components/CheckBox.h"
#include "MenuOptions/CheckBoxWidget.h"
#include "MenuOptions/ComboBoxWidget.h"
#include "MenuOptions/SingleRangeInputWidget.h"
#include "MenuOptions/ToggleableSingleRangeInputWidget.h"
#include "Utilities/ComboBox/BSComboBoxString.h"

using namespace Constants;

void UCustomGameModeGeneralWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SliderTextBoxOption_SpawnBeatDelay->SetValues(MinValue_PlayerDelay, MaxValue_PlayerDelay, SnapSize_PlayerDelay);
	SliderTextBoxOption_TargetSpawnCD->
		SetValues(MinValue_TargetSpawnCD, MaxValue_TargetSpawnCD, SnapSize_TargetSpawnCD);
	SliderTextBoxOption_MaxNumRecentTargets->SetValues(MinValue_MaxNumRecentTargets, MaxValue_MaxNumRecentTargets,
		SnapSize_MaxNumRecentTargets);
	SliderTextBoxOption_RecentTargetTimeLength->SetValues(MinValue_RecentTargetTimeLength,
		MaxValue_RecentTargetTimeLength, SnapSize_RecentTargetTimeLength);
	SliderTextBoxOption_ExpirationHealthPenalty->SetValues(MinValue_ExpirationHealthPenalty,
		MaxValue_ExpirationHealthPenalty, SnapSize_ExpirationHealthPenalty);
	SliderTextBoxOption_DeactivationHealthLostThreshold->SetValues(MinValue_SpecificHealthLost,
		MaxValue_SpecificHealthLost, SnapSize_SpecificHealthLost);
	SliderTextBoxOption_Alpha->SetValues(MinValue_Alpha, MaxValue_Alpha, SnapSize_Alpha);
	SliderTextBoxOption_Epsilon->SetValues(MinValue_Epsilon, MaxValue_Epsilon, SnapSize_Epsilon);
	SliderTextBoxOption_Gamma->SetValues(MinValue_Gamma, MaxValue_Gamma, SnapSize_Gamma);
	MenuOption_TargetHealth->SetValues(MinValue_MaxHealth, MaxValue_MaxHealth, SnapSize_MaxHealth);
	MenuOption_TargetLifespan->SetValues(MinValue_Lifespan, MaxValue_Lifespan, SnapSize_Lifespan);

	SliderTextBoxOption_SpawnBeatDelay->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_TargetSpawnCD->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_MaxNumRecentTargets->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_RecentTargetTimeLength->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_ExpirationHealthPenalty->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_DeactivationHealthLostThreshold->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_Alpha->OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_Epsilon->OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_Gamma->OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);
	MenuOption_TargetHealth->OnSliderTextBoxCheckBoxOptionChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxCheckBoxOptionChanged);
	MenuOption_TargetLifespan->OnSliderTextBoxCheckBoxOptionChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxCheckBoxOptionChanged);

	CheckBoxOption_EnableAI->CheckBox->OnCheckStateChanged.AddUniqueDynamic(this,
		&ThisClass::OnCheckStateChanged_EnableAI);

	ComboBoxOption_RecentTargetMemoryPolicy->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_RecentTargetMemoryPolicy);
	ComboBoxOption_DamageType->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_DamageType);
	ComboBoxOption_HyperParameterMode->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_HyperParameterMode);

	ComboBoxOption_RecentTargetMemoryPolicy->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_TargetActivationSelectionPolicy);
	ComboBoxOption_DamageType->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_DamageType);
	ComboBoxOption_HyperParameterMode->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_HyperParameterMode);

	ComboBoxOption_RecentTargetMemoryPolicy->ComboBox->ClearOptions();
	ComboBoxOption_DamageType->ComboBox->ClearOptions();
	ComboBoxOption_HyperParameterMode->ComboBox->ClearOptions();

	TArray<FString> Options;
	for (const ERecentTargetMemoryPolicy& Method : TEnumRange<ERecentTargetMemoryPolicy>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_RecentTargetMemoryPolicy->SortAddOptionsAndSetEnumType<ERecentTargetMemoryPolicy>(Options);
	Options.Empty();

	for (const ETargetDamageType& Method : TEnumRange<ETargetDamageType>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_DamageType->SortAddOptionsAndSetEnumType<ETargetDamageType>(Options);
	Options.Empty();

	for (const EReinforcementLearningHyperParameterMode& Method : TEnumRange<
		     EReinforcementLearningHyperParameterMode>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_HyperParameterMode->SortAddOptionsAndSetEnumType<EReinforcementLearningHyperParameterMode>(Options);
	Options.Empty();

	SliderTextBoxOption_RecentTargetTimeLength->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
	SliderTextBoxOption_MaxNumRecentTargets->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
	SliderTextBoxOption_Alpha->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
	SliderTextBoxOption_Epsilon->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
	SliderTextBoxOption_Gamma->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
	ComboBoxOption_HyperParameterMode->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
	SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_DeactivationHealthLostThreshold,
		EMenuOptionEnabledState::Enabled);

	SetupWarningTooltipCallbacks();
	UpdateBrushColors();
}

void UCustomGameModeGeneralWidget::UpdateAllOptionsValid()
{
	Super::UpdateAllOptionsValid();
}

void UCustomGameModeGeneralWidget::UpdateOptionsFromConfig()
{
	UpdateValueIfDifferent(SliderTextBoxOption_SpawnBeatDelay, BSConfig->TargetConfig.SpawnBeatDelay);
	UpdateValueIfDifferent(SliderTextBoxOption_TargetSpawnCD, BSConfig->TargetConfig.TargetSpawnCD);
	UpdateValueIfDifferent(SliderTextBoxOption_MaxNumRecentTargets, BSConfig->TargetConfig.MaxNumRecentTargets);
	UpdateValueIfDifferent(SliderTextBoxOption_RecentTargetTimeLength, BSConfig->TargetConfig.RecentTargetTimeLength);
	UpdateValueIfDifferent(SliderTextBoxOption_ExpirationHealthPenalty, BSConfig->TargetConfig.ExpirationHealthPenalty);
	UpdateValueIfDifferent(SliderTextBoxOption_DeactivationHealthLostThreshold,
		BSConfig->TargetConfig.DeactivationHealthLostThreshold);
	UpdateValueIfDifferent(SliderTextBoxOption_Alpha, BSConfig->AIConfig.Alpha);
	UpdateValueIfDifferent(SliderTextBoxOption_Epsilon, BSConfig->AIConfig.Epsilon);
	UpdateValueIfDifferent(SliderTextBoxOption_Gamma, BSConfig->AIConfig.Gamma);
	UpdateValuesIfDifferent(MenuOption_TargetHealth, BSConfig->TargetConfig.MaxHealth <= 0.f,
		BSConfig->TargetConfig.MaxHealth);
	UpdateValuesIfDifferent(MenuOption_TargetLifespan, BSConfig->TargetConfig.TargetMaxLifeSpan <= 0.f,
		BSConfig->TargetConfig.TargetMaxLifeSpan);

	UpdateValueIfDifferent(ComboBoxOption_HyperParameterMode,
		GetStringFromEnum_FromTagMap(BSConfig->AIConfig.HyperParameterMode));
	UpdateValueIfDifferent(ComboBoxOption_RecentTargetMemoryPolicy,
		GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.RecentTargetMemoryPolicy));
	UpdateValueIfDifferent(ComboBoxOption_DamageType,
		GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.TargetDamageType));

	UpdateValueIfDifferent(CheckBoxOption_EnableAI, BSConfig->AIConfig.bEnableReinforcementLearning);
	UpdateDependentOptions_RecentTargetMemoryPolicy(BSConfig->TargetConfig.RecentTargetMemoryPolicy);
	UpdateDependentOptions_EnableAI(BSConfig->AIConfig.bEnableReinforcementLearning,
		BSConfig->AIConfig.HyperParameterMode);

	UpdateDependentOptions_DeactivationConditions(BSConfig->TargetConfig.TargetDeactivationConditions);

	UpdateBrushColors();
}

void UCustomGameModeGeneralWidget::SetupWarningTooltipCallbacks()
{
	CheckBoxOption_EnableAI->AddWarningTooltipData(FTooltipData("Invalid_HeadshotHeightOnly_AI",
		ETooltipImageType::Warning)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid && BSConfig->AIConfig.
			bEnableReinforcementLearning && BSConfig->TargetConfig.TargetDistributionPolicy ==
			ETargetDistributionPolicy::HeadshotHeightOnly;
	});
	CheckBoxOption_EnableAI->AddWarningTooltipData(FTooltipData("Invalid_Tracking_AI", ETooltipImageType::Warning)).
	                         BindLambda([this]()
	                         {
		                         return BSConfig->TargetConfig.TargetDistributionPolicy ==
			                         ETargetDistributionPolicy::Grid && BSConfig->AIConfig.bEnableReinforcementLearning
			                         && BSConfig->TargetConfig.TargetDamageType == ETargetDamageType::Tracking;
	                         });
	ComboBoxOption_DamageType->AddWarningTooltipData(FTooltipData("Invalid_Tracking_AI", ETooltipImageType::Warning)).
	                           BindLambda([this]()
	                           {
		                           return BSConfig->AIConfig.bEnableReinforcementLearning && BSConfig->TargetConfig.
			                           TargetDamageType == ETargetDamageType::Tracking;
	                           });
}

void UCustomGameModeGeneralWidget::UpdateDependentOptions_RecentTargetMemoryPolicy(
	const ERecentTargetMemoryPolicy& InRecentTargetMemoryPolicy)
{
	switch (InRecentTargetMemoryPolicy)
	{
	case ERecentTargetMemoryPolicy::CustomTimeBased:
		SliderTextBoxOption_RecentTargetTimeLength->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
		SliderTextBoxOption_MaxNumRecentTargets->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		break;
	case ERecentTargetMemoryPolicy::NumTargetsBased:
		SliderTextBoxOption_RecentTargetTimeLength->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		SliderTextBoxOption_MaxNumRecentTargets->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
		break;
	case ERecentTargetMemoryPolicy::None:
	case ERecentTargetMemoryPolicy::UseTargetSpawnCD: default:
		SliderTextBoxOption_RecentTargetTimeLength->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		SliderTextBoxOption_MaxNumRecentTargets->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		break;
	}
}

void UCustomGameModeGeneralWidget::UpdateDependentOptions_DeactivationConditions(
	const TArray<ETargetDeactivationCondition>& Conditions)
{
	if (Conditions.Contains(ETargetDeactivationCondition::OnSpecificHealthLost))
	{
		SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_DeactivationHealthLostThreshold,
			EMenuOptionEnabledState::Enabled);
	}
	else
	{
		SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_DeactivationHealthLostThreshold,
			EMenuOptionEnabledState::DependentMissing, "DM_DeactivationHealthLostThreshold");
	}
}

void UCustomGameModeGeneralWidget::UpdateDependentOptions_EnableAI(const bool bInEnableReinforcementLearning,
	const EReinforcementLearningHyperParameterMode HyperParameterMode)
{
	// TODO: Implement Auto Reinforcement Learning HyperParameter Mode
	/*if (bInEnableReinforcementLearning)
	{
		ComboBoxOption_HyperParameterMode->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
	}
	else
	{
		ComboBoxOption_HyperParameterMode->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
	}*/
	UpdateDependentOptions_HyperParameterMode(bInEnableReinforcementLearning, HyperParameterMode);
}

void UCustomGameModeGeneralWidget::UpdateDependentOptions_HyperParameterMode(const bool bInEnableReinforcementLearning,
	const EReinforcementLearningHyperParameterMode HyperParameterMode)
{
	if (bInEnableReinforcementLearning)
	{
		SliderTextBoxOption_Alpha->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
		SliderTextBoxOption_Epsilon->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
		SliderTextBoxOption_Gamma->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
	}
	else
	{
		SliderTextBoxOption_Alpha->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		SliderTextBoxOption_Epsilon->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		SliderTextBoxOption_Gamma->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
	}

	// TODO: Implement Auto Reinforcement Learning HyperParameter Mode
	/*if (!bInEnableReinforcementLearning)
	{
		SliderTextBoxOption_Alpha->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		SliderTextBoxOption_Epsilon->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		SliderTextBoxOption_Gamma->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		return;
	}
	switch (HyperParameterMode)
	{
	case EReinforcementLearningHyperParameterMode::None:
	case EReinforcementLearningHyperParameterMode::Auto:
		SliderTextBoxOption_Alpha->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		SliderTextBoxOption_Epsilon->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		SliderTextBoxOption_Gamma->SetMenuOptionEnabledState(EMenuOptionEnabledState::Disabled);
		break;
	case EReinforcementLearningHyperParameterMode::Custom:
		SliderTextBoxOption_Alpha->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
		SliderTextBoxOption_Epsilon->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
		SliderTextBoxOption_Gamma->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
		break;
	}*/
}

void UCustomGameModeGeneralWidget::OnCheckStateChanged_EnableAI(const bool bChecked)
{
	BSConfig->AIConfig.bEnableReinforcementLearning = bChecked;
	UpdateDependentOptions_EnableAI(BSConfig->AIConfig.bEnableReinforcementLearning,
		BSConfig->AIConfig.HyperParameterMode);

	UpdateBrushColors();
	UpdateAllOptionsValid();
}

void UCustomGameModeGeneralWidget::OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value)
{
	if (Widget == SliderTextBoxOption_SpawnBeatDelay)
	{
		BSConfig->TargetConfig.SpawnBeatDelay = Value;
	}
	else if (Widget == SliderTextBoxOption_TargetSpawnCD)
	{
		BSConfig->TargetConfig.TargetSpawnCD = Value;
	}
	else if (Widget == SliderTextBoxOption_MaxNumRecentTargets)
	{
		BSConfig->TargetConfig.MaxNumRecentTargets = Value;
	}
	else if (Widget == SliderTextBoxOption_RecentTargetTimeLength)
	{
		BSConfig->TargetConfig.RecentTargetTimeLength = Value;
	}
	else if (Widget == SliderTextBoxOption_Alpha)
	{
		BSConfig->AIConfig.Alpha = Value;
	}
	else if (Widget == SliderTextBoxOption_Epsilon)
	{
		BSConfig->AIConfig.Epsilon = Value;
	}
	else if (Widget == SliderTextBoxOption_Gamma)
	{
		BSConfig->AIConfig.Gamma = Value;
	}
	else if (Widget == SliderTextBoxOption_ExpirationHealthPenalty)
	{
		BSConfig->TargetConfig.ExpirationHealthPenalty = Value;
	}
	else if (Widget == SliderTextBoxOption_DeactivationHealthLostThreshold)
	{
		BSConfig->TargetConfig.DeactivationHealthLostThreshold = Value;
	}
	UpdateAllOptionsValid();
}

void UCustomGameModeGeneralWidget::OnSliderTextBoxCheckBoxOptionChanged(UToggleableSingleRangeInputWidget* Widget,
	const bool bChecked, const float Value)
{
	if (Widget == MenuOption_TargetHealth)
	{
		BSConfig->TargetConfig.MaxHealth = bChecked ? -1.f : Value;
	}
	else if (Widget == MenuOption_TargetLifespan)
	{
		BSConfig->TargetConfig.TargetMaxLifeSpan = bChecked ? -1.f : Value;
	}
	UpdateAllOptionsValid();
}

void UCustomGameModeGeneralWidget::OnSelectionChanged_RecentTargetMemoryPolicy(const TArray<FString>& Selected,
	const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}

	BSConfig->TargetConfig.RecentTargetMemoryPolicy = GetEnumFromString_FromTagMap<
		ERecentTargetMemoryPolicy>(Selected[0]);
	UpdateDependentOptions_RecentTargetMemoryPolicy(BSConfig->TargetConfig.RecentTargetMemoryPolicy);
	UpdateBrushColors();
	UpdateAllOptionsValid();
}

void UCustomGameModeGeneralWidget::OnSelectionChanged_DamageType(const TArray<FString>& Selected,
	const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}

	BSConfig->TargetConfig.TargetDamageType = GetEnumFromString_FromTagMap<ETargetDamageType>(Selected[0]);
	UpdateAllOptionsValid();
}

void UCustomGameModeGeneralWidget::OnSelectionChanged_HyperParameterMode(const TArray<FString>& Selected,
	const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}

	BSConfig->AIConfig.HyperParameterMode = GetEnumFromString_FromTagMap<
		EReinforcementLearningHyperParameterMode>(Selected[0]);
	UpdateDependentOptions_EnableAI(BSConfig->AIConfig.bEnableReinforcementLearning,
		BSConfig->AIConfig.HyperParameterMode);

	UpdateBrushColors();
	UpdateAllOptionsValid();
}

FString UCustomGameModeGeneralWidget::GetComboBoxEntryTooltipStringTableKey_HyperParameterMode(
	const FString& EnumString)
{
	const EReinforcementLearningHyperParameterMode EnumValue = GetEnumFromString_FromTagMap<
		EReinforcementLearningHyperParameterMode>(EnumString);
	return GetStringTableKeyNameFromEnum(EnumValue);
}

FString UCustomGameModeGeneralWidget::GetComboBoxEntryTooltipStringTableKey_TargetActivationSelectionPolicy(
	const FString& EnumString)
{
	return GetStringTableKeyNameFromEnum(GetEnumFromString_FromTagMap<ERecentTargetMemoryPolicy>(EnumString));
}

FString UCustomGameModeGeneralWidget::GetComboBoxEntryTooltipStringTableKey_DamageType(const FString& EnumString)
{
	const ETargetDamageType EnumValue = GetEnumFromString_FromTagMap<ETargetDamageType>(EnumString);
	return GetStringTableKeyNameFromEnum(EnumValue);
}
