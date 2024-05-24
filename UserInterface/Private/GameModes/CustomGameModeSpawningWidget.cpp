// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModeSpawningWidget.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "Components/CheckBox.h"
#include "GameModes/CustomGameModeWidget.h"
#include "MenuOptions/CheckBoxWidget.h"
#include "MenuOptions/ComboBoxWidget.h"
#include "MenuOptions/SingleRangeInputWidget.h"
#include "Utilities/ComboBox/BSComboBoxString.h"


UCustomGameModeSpawningWidget::UCustomGameModeSpawningWidget(): SliderTextBoxOption_MaxNumTargetsAtOnce(nullptr),
                                                                ComboBoxOption_TargetSpawningPolicy(nullptr),
                                                                ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode(
	                                                                nullptr),
                                                                CheckBoxOption_AllowSpawnWithoutActivation(nullptr),
                                                                CheckBoxOption_BatchSpawning(nullptr),
                                                                CheckBoxOption_SpawnAtOriginWheneverPossible(nullptr),
                                                                CheckBoxOption_SpawnEveryOtherTargetInCenter(nullptr),
                                                                SliderTextBoxOption_NumUpfrontTargetsToSpawn(nullptr),
                                                                SliderTextBoxOption_NumRuntimeTargetsToSpawn(nullptr)
{
	GameModeCategory = EGameModeCategory::TargetSpawning;
}

void UCustomGameModeSpawningWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SliderTextBoxOption_MaxNumTargetsAtOnce->SetValues(Constants::MinValue_MaxNumTargetsAtOnce,
		Constants::MaxValue_MaxNumTargetsAtOnce, Constants::SnapSize_MaxNumTargetsAtOnce);
	SliderTextBoxOption_NumUpfrontTargetsToSpawn->SetValues(Constants::MinValue_NumUpfrontTargetsToSpawn,
		Constants::MaxValue_NumUpfrontTargetsToSpawn, Constants::SnapSize_NumUpfrontTargetsToSpawn);
	SliderTextBoxOption_NumRuntimeTargetsToSpawn->SetValues(Constants::MinValue_NumRuntimeTargetsToSpawn,
		Constants::MaxValue_NumRuntimeTargetsToSpawn, Constants::SnapSize_NumRuntimeTargetsToSpawn);

	SliderTextBoxOption_MaxNumTargetsAtOnce->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_NumUpfrontTargetsToSpawn->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_NumRuntimeTargetsToSpawn->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);

	ComboBoxOption_TargetSpawningPolicy->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_TargetSpawningPolicy);
	ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_RuntimeTargetSpawningLocationSelectionMode);

	ComboBoxOption_TargetSpawningPolicy->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_TargetSpawningPolicy);
	ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_RuntimeTargetSpawningLocationSelectionMode);

	CheckBoxOption_AllowSpawnWithoutActivation->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_AllowSpawnWithoutActivation);
	CheckBoxOption_BatchSpawning->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_BatchSpawning);
	CheckBoxOption_SpawnAtOriginWheneverPossible->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_SpawnAtOriginWheneverPossible);
	CheckBoxOption_SpawnEveryOtherTargetInCenter->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_SpawnEveryOtherTargetInCenter);

	ComboBoxOption_TargetSpawningPolicy->ComboBox->ClearOptions();
	ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode->ComboBox->ClearOptions();

	TArray<FString> Options;
	for (const ETargetSpawningPolicy& Method : TEnumRange<ETargetSpawningPolicy>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_TargetSpawningPolicy->SortAddOptionsAndSetEnumType<ETargetSpawningPolicy>(Options);
	Options.Empty();

	for (const ERuntimeTargetSpawningLocationSelectionMode& Method : TEnumRange<
		     ERuntimeTargetSpawningLocationSelectionMode>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode->SortAddOptionsAndSetEnumType<
		ERuntimeTargetSpawningLocationSelectionMode>(Options);
	Options.Empty();

	SliderTextBoxOption_NumUpfrontTargetsToSpawn->SetVisibility(ESlateVisibility::Collapsed);
	SliderTextBoxOption_NumRuntimeTargetsToSpawn->SetVisibility(ESlateVisibility::Collapsed);

	SetupWarningTooltipCallbacks();
	UpdateBrushColors();
}


void UCustomGameModeSpawningWidget::UpdateOptionsFromConfig()
{
	UpdateValueIfDifferent(ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode,
		GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.RuntimeTargetSpawningLocationSelectionMode));
	UpdateValueIfDifferent(ComboBoxOption_TargetSpawningPolicy,
		GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.TargetSpawningPolicy));

	UpdateValueIfDifferent(SliderTextBoxOption_MaxNumTargetsAtOnce, BSConfig->TargetConfig.MaxNumTargetsAtOnce);
	UpdateValueIfDifferent(SliderTextBoxOption_NumUpfrontTargetsToSpawn,
		BSConfig->TargetConfig.NumUpfrontTargetsToSpawn);
	UpdateValueIfDifferent(SliderTextBoxOption_NumRuntimeTargetsToSpawn,
		BSConfig->TargetConfig.NumRuntimeTargetsToSpawn);

	UpdateValueIfDifferent(CheckBoxOption_AllowSpawnWithoutActivation,
		BSConfig->TargetConfig.bAllowSpawnWithoutActivation);
	UpdateValueIfDifferent(CheckBoxOption_BatchSpawning, BSConfig->TargetConfig.bUseBatchSpawning);
	UpdateValueIfDifferent(CheckBoxOption_SpawnAtOriginWheneverPossible,
		BSConfig->TargetConfig.bSpawnAtOriginWheneverPossible);
	UpdateValueIfDifferent(CheckBoxOption_SpawnEveryOtherTargetInCenter,
		BSConfig->TargetConfig.bSpawnEveryOtherTargetInCenter);

	UpdateDependentOptions_TargetSpawningPolicy(BSConfig->TargetConfig.TargetSpawningPolicy);

	UpdateBrushColors();
}

void UCustomGameModeSpawningWidget::SetupWarningTooltipCallbacks()
{
	CheckBoxOption_SpawnEveryOtherTargetInCenter->AddWarningTooltipData(
		FTooltipData("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning", ETooltipImageType::Warning)).BindLambda(
		[this]()
		{
			return BSConfig->TargetConfig.bSpawnEveryOtherTargetInCenter && BSConfig->TargetConfig.bUseBatchSpawning;
		});
	CheckBoxOption_SpawnEveryOtherTargetInCenter->AddWarningTooltipData(
		FTooltipData("Invalid_SpawnEveryOtherTargetInCenter_AllowSpawnWithoutActivation",
			ETooltipImageType::Warning)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.bSpawnEveryOtherTargetInCenter && BSConfig->TargetConfig.
			bAllowSpawnWithoutActivation;
	});
	CheckBoxOption_BatchSpawning->AddWarningTooltipData(
		FTooltipData("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning2", ETooltipImageType::Warning)).BindLambda(
		[this]()
		{
			return BSConfig->TargetConfig.bSpawnEveryOtherTargetInCenter && BSConfig->TargetConfig.bUseBatchSpawning;
		});
	CheckBoxOption_AllowSpawnWithoutActivation->AddWarningTooltipData(
		FTooltipData("Invalid_SpawnEveryOtherTargetInCenter_AllowSpawnWithoutActivation2",
			ETooltipImageType::Warning)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.bSpawnEveryOtherTargetInCenter && BSConfig->TargetConfig.
			bAllowSpawnWithoutActivation;
	});
}

void UCustomGameModeSpawningWidget::UpdateDependentOptions_TargetSpawningPolicy(
	const ETargetSpawningPolicy& InTargetSpawningPolicy)
{
	if (InTargetSpawningPolicy == ETargetSpawningPolicy::RuntimeOnly)
	{
		SetMenuOptionEnabledStateAndAddTooltip(CheckBoxOption_BatchSpawning, EMenuOptionEnabledState::Enabled);
		SetMenuOptionEnabledStateAndAddTooltip(CheckBoxOption_SpawnAtOriginWheneverPossible,
			EMenuOptionEnabledState::Enabled);
		SetMenuOptionEnabledStateAndAddTooltip(CheckBoxOption_SpawnEveryOtherTargetInCenter,
			EMenuOptionEnabledState::Enabled);

		SliderTextBoxOption_NumUpfrontTargetsToSpawn->SetVisibility(ESlateVisibility::Collapsed);
		SliderTextBoxOption_NumRuntimeTargetsToSpawn->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		if (BSConfig->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid)
		{
			SetMenuOptionEnabledStateAndAddTooltip(ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode,
				EMenuOptionEnabledState::Enabled);
		}
		else
		{
			BSConfig->TargetConfig.RuntimeTargetSpawningLocationSelectionMode =
				ERuntimeTargetSpawningLocationSelectionMode::Random;
			ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode->ComboBox->SetSelectedOption(
				GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.RuntimeTargetSpawningLocationSelectionMode));
			SetMenuOptionEnabledStateAndAddTooltip(ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode,
				EMenuOptionEnabledState::DependentMissing, "DM_RuntimeTargetSpawningLocationSelectionMode_NonGrid");
		}

		SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_MaxNumTargetsAtOnce,
			EMenuOptionEnabledState::Enabled);
	}
	else if (InTargetSpawningPolicy == ETargetSpawningPolicy::UpfrontOnly)
	{
		BSConfig->TargetConfig.bUseBatchSpawning = false;
		BSConfig->TargetConfig.bSpawnEveryOtherTargetInCenter = false;
		BSConfig->TargetConfig.bSpawnAtOriginWheneverPossible = false;

		UpdateValueIfDifferent(CheckBoxOption_BatchSpawning, false);
		UpdateValueIfDifferent(CheckBoxOption_SpawnAtOriginWheneverPossible, false);
		UpdateValueIfDifferent(CheckBoxOption_SpawnEveryOtherTargetInCenter, false);

		SetMenuOptionEnabledStateAndAddTooltip(CheckBoxOption_BatchSpawning, EMenuOptionEnabledState::DependentMissing,
			"DM_RuntimeTargetSpawningExclusive");
		SetMenuOptionEnabledStateAndAddTooltip(CheckBoxOption_SpawnAtOriginWheneverPossible,
			EMenuOptionEnabledState::DependentMissing, "DM_RuntimeTargetSpawningExclusive");
		SetMenuOptionEnabledStateAndAddTooltip(CheckBoxOption_SpawnEveryOtherTargetInCenter,
			EMenuOptionEnabledState::DependentMissing, "DM_RuntimeTargetSpawningExclusive");

		SliderTextBoxOption_NumUpfrontTargetsToSpawn->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		SliderTextBoxOption_NumRuntimeTargetsToSpawn->SetVisibility(ESlateVisibility::Collapsed);

		SetMenuOptionEnabledStateAndAddTooltip(ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode,
			EMenuOptionEnabledState::DependentMissing, "DM_RuntimeTargetSpawningExclusive");

		if (BSConfig->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid)
		{
			SliderTextBoxOption_NumUpfrontTargetsToSpawn->SetValue(
				BSConfig->GridConfig.NumHorizontalGridTargets * BSConfig->GridConfig.NumVerticalGridTargets);
			SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_NumUpfrontTargetsToSpawn,
				EMenuOptionEnabledState::DependentMissing, "DM_NumUpfrontTargetsToSpawn_Grid");
			SliderTextBoxOption_MaxNumTargetsAtOnce->SetValue(-1.f);
			SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_MaxNumTargetsAtOnce,
				EMenuOptionEnabledState::DependentMissing, "DM_MaxNumTargetsAtOnce_Upfront_Grid");
		}
		else
		{
			SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_NumUpfrontTargetsToSpawn,
				EMenuOptionEnabledState::Enabled);
			SetMenuOptionEnabledStateAndAddTooltip(SliderTextBoxOption_MaxNumTargetsAtOnce,
				EMenuOptionEnabledState::Enabled);
		}
	}
}

void UCustomGameModeSpawningWidget::OnCheckStateChanged_AllowSpawnWithoutActivation(const bool bChecked)
{
	BSConfig->TargetConfig.bAllowSpawnWithoutActivation = bChecked;
	//UpdateAllOptionsValid();
}

void UCustomGameModeSpawningWidget::OnCheckStateChanged_BatchSpawning(const bool bChecked)
{
	BSConfig->TargetConfig.bUseBatchSpawning = bChecked;
	//UpdateAllOptionsValid();
}

void UCustomGameModeSpawningWidget::OnCheckStateChanged_SpawnAtOriginWheneverPossible(const bool bChecked)
{
	BSConfig->TargetConfig.bSpawnAtOriginWheneverPossible = bChecked;
	//UpdateAllOptionsValid();
}

void UCustomGameModeSpawningWidget::OnCheckStateChanged_SpawnEveryOtherTargetInCenter(const bool bChecked)
{
	BSConfig->TargetConfig.bSpawnEveryOtherTargetInCenter = bChecked;
	//UpdateAllOptionsValid();
}

void UCustomGameModeSpawningWidget::OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value)
{
	if (Widget == SliderTextBoxOption_MaxNumTargetsAtOnce)
	{
		BSConfig->TargetConfig.MaxNumTargetsAtOnce = Value;
	}
	else if (Widget == SliderTextBoxOption_NumUpfrontTargetsToSpawn)
	{
		BSConfig->TargetConfig.NumUpfrontTargetsToSpawn = Value;
	}
	else if (Widget == SliderTextBoxOption_NumRuntimeTargetsToSpawn)
	{
		BSConfig->TargetConfig.NumRuntimeTargetsToSpawn = Value;
	}

	//UpdateAllOptionsValid();
}

void UCustomGameModeSpawningWidget::OnSelectionChanged_RuntimeTargetSpawningLocationSelectionMode(
	const TArray<FString>& Selected, const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}

	BSConfig->TargetConfig.RuntimeTargetSpawningLocationSelectionMode = GetEnumFromString_FromTagMap<
		ERuntimeTargetSpawningLocationSelectionMode>(Selected[0]);

	UpdateBrushColors();
	//UpdateAllOptionsValid();
}

void UCustomGameModeSpawningWidget::OnSelectionChanged_TargetSpawningPolicy(const TArray<FString>& Selected,
	const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}

	BSConfig->TargetConfig.TargetSpawningPolicy = GetEnumFromString_FromTagMap<ETargetSpawningPolicy>(Selected[0]);

	UpdateDependentOptions_TargetSpawningPolicy(BSConfig->TargetConfig.TargetSpawningPolicy);
	UpdateBrushColors();
	//UpdateAllOptionsValid();
}

FString UCustomGameModeSpawningWidget::GetComboBoxEntryTooltipStringTableKey_RuntimeTargetSpawningLocationSelectionMode(
	const FString& EnumString)
{
	const ERuntimeTargetSpawningLocationSelectionMode EnumValue = GetEnumFromString_FromTagMap<
		ERuntimeTargetSpawningLocationSelectionMode>(EnumString);
	return GetStringTableKeyNameFromEnum(EnumValue);
}

FString UCustomGameModeSpawningWidget::GetComboBoxEntryTooltipStringTableKey_TargetSpawningPolicy(
	const FString& EnumString)
{
	const ETargetSpawningPolicy EnumValue = GetEnumFromString_FromTagMap<ETargetSpawningPolicy>(EnumString);
	return GetStringTableKeyNameFromEnum(EnumValue);
}
