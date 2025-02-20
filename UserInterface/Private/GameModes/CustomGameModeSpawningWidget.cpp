﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


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

	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxNumTargetsAtOnce)), SliderTextBoxOption_MaxNumTargetsAtOnce);
	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, NumUpfrontTargetsToSpawn)),
		SliderTextBoxOption_NumUpfrontTargetsToSpawn);
	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, NumRuntimeTargetsToSpawn)),
		SliderTextBoxOption_NumRuntimeTargetsToSpawn);
	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetSpawningPolicy)), ComboBoxOption_TargetSpawningPolicy);
	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, RuntimeTargetSpawningLocationSelectionMode)),
		ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode);
	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bAllowSpawnWithoutActivation)),
		CheckBoxOption_AllowSpawnWithoutActivation);
	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bUseBatchSpawning)), CheckBoxOption_BatchSpawning);
	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bSpawnAtOriginWheneverPossible)),
		CheckBoxOption_SpawnAtOriginWheneverPossible);
	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bSpawnEveryOtherTargetInCenter)),
		CheckBoxOption_SpawnEveryOtherTargetInCenter);

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

void UCustomGameModeSpawningWidget::UpdateDependentOptions_TargetSpawningPolicy(
	const ETargetSpawningPolicy& InTargetSpawningPolicy)
{
	TSet<uint32> ModifiedProperties;
	if (InTargetSpawningPolicy == ETargetSpawningPolicy::RuntimeOnly)
	{
		CheckBoxOption_BatchSpawning->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
		CheckBoxOption_SpawnAtOriginWheneverPossible->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
		CheckBoxOption_SpawnEveryOtherTargetInCenter->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);

		SliderTextBoxOption_NumUpfrontTargetsToSpawn->SetVisibility(ESlateVisibility::Collapsed);
		SliderTextBoxOption_NumRuntimeTargetsToSpawn->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		if (BSConfig->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid)
		{
			ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode->SetMenuOptionEnabledState(
				EMenuOptionEnabledState::Enabled);
		}
		else
		{
			BSConfig->TargetConfig.RuntimeTargetSpawningLocationSelectionMode =
				ERuntimeTargetSpawningLocationSelectionMode::Random;
			ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode->ComboBox->SetSelectedOption(
				GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.RuntimeTargetSpawningLocationSelectionMode));
			ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode->SetMenuOptionEnabledState(
				EMenuOptionEnabledState::DependentMissing,
				GetTooltipTextFromKey("DM_RuntimeTargetSpawningLocationSelectionMode_NonGrid"));
			ModifiedProperties.Add(UBSGameModeValidator::FindBSConfigProperty(
				GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, RuntimeTargetSpawningLocationSelectionMode)));
		}

		SliderTextBoxOption_MaxNumTargetsAtOnce->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
	}
	else if (InTargetSpawningPolicy == ETargetSpawningPolicy::UpfrontOnly)
	{
		BSConfig->TargetConfig.bUseBatchSpawning = false;
		BSConfig->TargetConfig.bSpawnEveryOtherTargetInCenter = false;
		BSConfig->TargetConfig.bSpawnAtOriginWheneverPossible = false;

		ModifiedProperties.Add(UBSGameModeValidator::FindBSConfigProperty(
			GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bUseBatchSpawning)));
		ModifiedProperties.Add(UBSGameModeValidator::FindBSConfigProperty(
			GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bSpawnEveryOtherTargetInCenter)));
		ModifiedProperties.Add(UBSGameModeValidator::FindBSConfigProperty(
			GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bSpawnAtOriginWheneverPossible)));

		UpdateValueIfDifferent(CheckBoxOption_BatchSpawning, false);
		UpdateValueIfDifferent(CheckBoxOption_SpawnAtOriginWheneverPossible, false);
		UpdateValueIfDifferent(CheckBoxOption_SpawnEveryOtherTargetInCenter, false);

		CheckBoxOption_BatchSpawning->SetMenuOptionEnabledState(EMenuOptionEnabledState::DependentMissing,
			GetTooltipTextFromKey("DM_RuntimeTargetSpawningExclusive"));
		CheckBoxOption_SpawnAtOriginWheneverPossible->SetMenuOptionEnabledState(
			EMenuOptionEnabledState::DependentMissing, GetTooltipTextFromKey("DM_RuntimeTargetSpawningExclusive"));
		CheckBoxOption_SpawnEveryOtherTargetInCenter->SetMenuOptionEnabledState(
			EMenuOptionEnabledState::DependentMissing, GetTooltipTextFromKey("DM_RuntimeTargetSpawningExclusive"));

		SliderTextBoxOption_NumUpfrontTargetsToSpawn->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		SliderTextBoxOption_NumRuntimeTargetsToSpawn->SetVisibility(ESlateVisibility::Collapsed);

		ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode->SetMenuOptionEnabledState(
			EMenuOptionEnabledState::DependentMissing, GetTooltipTextFromKey("DM_RuntimeTargetSpawningExclusive"));

		if (BSConfig->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid)
		{
			SliderTextBoxOption_NumUpfrontTargetsToSpawn->SetValue(
				BSConfig->GridConfig.NumHorizontalGridTargets * BSConfig->GridConfig.NumVerticalGridTargets);
			SliderTextBoxOption_NumUpfrontTargetsToSpawn->SetMenuOptionEnabledState(
				EMenuOptionEnabledState::DependentMissing, GetTooltipTextFromKey("DM_NumUpfrontTargetsToSpawn_Grid"));
			SliderTextBoxOption_MaxNumTargetsAtOnce->SetValue(-1.f);
			SliderTextBoxOption_MaxNumTargetsAtOnce->SetMenuOptionEnabledState(
				EMenuOptionEnabledState::DependentMissing,
				GetTooltipTextFromKey("DM_MaxNumTargetsAtOnce_Upfront_Grid"));
		}
		else
		{
			SliderTextBoxOption_NumUpfrontTargetsToSpawn->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
			SliderTextBoxOption_MaxNumTargetsAtOnce->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
		}
	}

	if (!ModifiedProperties.IsEmpty())
	{
		OnPropertyChanged.Execute(ModifiedProperties);
	}
}

void UCustomGameModeSpawningWidget::OnCheckStateChanged_AllowSpawnWithoutActivation(const bool bChecked)
{
	BSConfig->TargetConfig.bAllowSpawnWithoutActivation = bChecked;
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bAllowSpawnWithoutActivation))
	});
}

void UCustomGameModeSpawningWidget::OnCheckStateChanged_BatchSpawning(const bool bChecked)
{
	BSConfig->TargetConfig.bUseBatchSpawning = bChecked;
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bUseBatchSpawning))
	});
}

void UCustomGameModeSpawningWidget::OnCheckStateChanged_SpawnAtOriginWheneverPossible(const bool bChecked)
{
	BSConfig->TargetConfig.bSpawnAtOriginWheneverPossible = bChecked;
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bSpawnAtOriginWheneverPossible))
	});
}

void UCustomGameModeSpawningWidget::OnCheckStateChanged_SpawnEveryOtherTargetInCenter(const bool bChecked)
{
	BSConfig->TargetConfig.bSpawnEveryOtherTargetInCenter = bChecked;
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bSpawnEveryOtherTargetInCenter))
	});
}

void UCustomGameModeSpawningWidget::OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value)
{
	if (Widget == SliderTextBoxOption_MaxNumTargetsAtOnce)
	{
		BSConfig->TargetConfig.MaxNumTargetsAtOnce = Value;
		OnPropertyChanged.Execute({
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxNumTargetsAtOnce))
		});
	}
	else if (Widget == SliderTextBoxOption_NumUpfrontTargetsToSpawn)
	{
		BSConfig->TargetConfig.NumUpfrontTargetsToSpawn = Value;
		OnPropertyChanged.Execute({
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, NumUpfrontTargetsToSpawn))
		});
	}
	else if (Widget == SliderTextBoxOption_NumRuntimeTargetsToSpawn)
	{
		BSConfig->TargetConfig.NumRuntimeTargetsToSpawn = Value;
		OnPropertyChanged.Execute({
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, NumRuntimeTargetsToSpawn))
		});
	}
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
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, RuntimeTargetSpawningLocationSelectionMode))
	});
	UpdateBrushColors();
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

	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetSpawningPolicy))
	});

	UpdateBrushColors();
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
