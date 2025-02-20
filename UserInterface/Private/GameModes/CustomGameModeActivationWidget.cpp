﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModeActivationWidget.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "GameModes/CustomGameModeWidget.h"
#include "MenuOptions/CheckBoxWidget.h"
#include "MenuOptions/ComboBoxWidget.h"
#include "MenuOptions/DualRangeInputWidget.h"
#include "MenuOptions/SingleRangeInputWidget.h"
#include "Utilities/ComboBox/BSComboBoxString.h"


UCustomGameModeActivationWidget::UCustomGameModeActivationWidget():
	SliderTextBoxOption_MaxNumActivatedTargetsAtOnce(nullptr), MenuOption_NumTargetsToActivateAtOnce(nullptr),
	CheckBoxOption_AllowActivationWhileActivated(nullptr), ComboBoxOption_TargetActivationSelectionPolicy(nullptr)
{
	GameModeCategory = EGameModeCategory::TargetActivation;
}

void UCustomGameModeActivationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxNumActivatedTargetsAtOnce)),
		SliderTextBoxOption_MaxNumActivatedTargetsAtOnce);
	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxNumTargetsAtOnce)), MenuOption_NumTargetsToActivateAtOnce);
	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bAllowActivationWhileActivated)),
		CheckBoxOption_AllowActivationWhileActivated);
	AssociatePropertyWithMenuOption(
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetActivationSelectionPolicy)),
		ComboBoxOption_TargetActivationSelectionPolicy);

	AddWatchedProperty(UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDistributionPolicy)));

	SliderTextBoxOption_MaxNumActivatedTargetsAtOnce->SetValues(Constants::MinValue_MaxNumActivatedTargetsAtOnce,
		Constants::MaxValue_MaxNumActivatedTargetsAtOnce, Constants::SnapSize_MaxNumActivatedTargetsAtOnce);
	MenuOption_NumTargetsToActivateAtOnce->SetValues(Constants::MinValue_MaxNumActivatedTargetsAtOnce,
		Constants::MaxValue_MaxNumActivatedTargetsAtOnce, Constants::SnapSize_MaxNumActivatedTargetsAtOnce);

	SliderTextBoxOption_MaxNumActivatedTargetsAtOnce->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	MenuOption_NumTargetsToActivateAtOnce->OnMinMaxMenuOptionChanged.AddUObject(this,
		&ThisClass::OnMinMaxMenuOptionChanged);

	CheckBoxOption_AllowActivationWhileActivated->CheckBox->OnCheckStateChanged.AddUniqueDynamic(this,
		&ThisClass::OnCheckStateChanged_AllowActivationWhileActivated);

	ComboBoxOption_TargetActivationSelectionPolicy->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_TargetActivationSelectionPolicy);

	ComboBoxOption_TargetActivationSelectionPolicy->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_TargetActivationSelectionPolicy);

	ComboBoxOption_TargetActivationSelectionPolicy->ComboBox->ClearOptions();

	TArray<FString> Options;
	for (const ETargetActivationSelectionPolicy& Method : TEnumRange<ETargetActivationSelectionPolicy>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_TargetActivationSelectionPolicy->SortAddOptionsAndSetEnumType<
		ETargetActivationSelectionPolicy>(Options);
	Options.Empty();

	UpdateBrushColors();
}

void UCustomGameModeActivationWidget::UpdateOptionsFromConfig()
{
	const bool bConstantNumTargetsToActivateAtOnce = BSConfig->TargetConfig.MinNumTargetsToActivateAtOnce == BSConfig->
		TargetConfig.MaxNumTargetsToActivateAtOnce;
	const bool bConstantTargetSpeed = BSConfig->TargetConfig.MinActivatedTargetSpeed == BSConfig->TargetConfig.
		MaxActivatedTargetSpeed;

	UpdateValueIfDifferent(CheckBoxOption_AllowActivationWhileActivated,
		BSConfig->TargetConfig.bAllowActivationWhileActivated);
	UpdateValueIfDifferent(SliderTextBoxOption_MaxNumActivatedTargetsAtOnce,
		BSConfig->TargetConfig.MaxNumActivatedTargetsAtOnce);
	UpdateValuesIfDifferent(MenuOption_NumTargetsToActivateAtOnce, bConstantNumTargetsToActivateAtOnce,
		BSConfig->TargetConfig.MinNumTargetsToActivateAtOnce, BSConfig->TargetConfig.MaxNumTargetsToActivateAtOnce);

	UpdateValueIfDifferent(ComboBoxOption_TargetActivationSelectionPolicy,
		GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.TargetActivationSelectionPolicy));

	UpdateDependentOptions_TargetActivationResponses(BSConfig->TargetConfig.TargetActivationResponses,
		bConstantTargetSpeed);
	UpdateDependentOptions_TargetDistributionPolicy(BSConfig->TargetConfig.TargetDistributionPolicy);

	UpdateBrushColors();
}

void UCustomGameModeActivationWidget::HandleWatchedPropertyChanged(const uint32 PropertyHash)
{
	if (PropertyHash == UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDistributionPolicy)))
	{
		UpdateDependentOptions_TargetDistributionPolicy(BSConfig->TargetConfig.TargetDistributionPolicy);
	}
}

void UCustomGameModeActivationWidget::UpdateDependentOptions_TargetActivationResponses(
	const TArray<ETargetActivationResponse>& InResponses, const bool bUseConstantTargetSpeed)
{
}

void UCustomGameModeActivationWidget::UpdateDependentOptions_TargetDistributionPolicy(
	const ETargetDistributionPolicy& Policy)
{
	switch (Policy)
	{
	case ETargetDistributionPolicy::Grid:
		ComboBoxOption_TargetActivationSelectionPolicy->SetMenuOptionEnabledState(EMenuOptionEnabledState::Enabled);
		ComboBoxOption_TargetActivationSelectionPolicy->ComboBox->SetIsEnabled(true);
		break;
	case ETargetDistributionPolicy::None:
	case ETargetDistributionPolicy::HeadshotHeightOnly:
	case ETargetDistributionPolicy::EdgeOnly:
	case ETargetDistributionPolicy::FullRange:
		BSConfig->TargetConfig.TargetActivationSelectionPolicy = ETargetActivationSelectionPolicy::Random;
		UpdateValueIfDifferent(ComboBoxOption_TargetActivationSelectionPolicy,
			GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.TargetActivationSelectionPolicy));
		ComboBoxOption_TargetActivationSelectionPolicy->SetMenuOptionEnabledState(
			EMenuOptionEnabledState::DependentMissing, GetTooltipTextFromKey("DM_ActivationSelectionPolicy_NonGrid"));
		break;
	}
}

void UCustomGameModeActivationWidget::OnCheckStateChanged_AllowActivationWhileActivated(const bool bChecked)
{
	BSConfig->TargetConfig.bAllowActivationWhileActivated = bChecked;
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bAllowActivationWhileActivated))
	});
}

void UCustomGameModeActivationWidget::OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value)
{
	if (Widget == SliderTextBoxOption_MaxNumActivatedTargetsAtOnce)
	{
		BSConfig->TargetConfig.MaxNumActivatedTargetsAtOnce = Value;
		OnPropertyChanged.Execute({
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxNumActivatedTargetsAtOnce))
		});
	}
}

void UCustomGameModeActivationWidget::OnMinMaxMenuOptionChanged(UDualRangeInputWidget* Widget, const bool bChecked,
	const float MinOrConstant, const float Max)
{
	if (Widget == MenuOption_NumTargetsToActivateAtOnce)
	{
		BSConfig->TargetConfig.MinNumTargetsToActivateAtOnce = MinOrConstant;
		BSConfig->TargetConfig.MaxNumTargetsToActivateAtOnce = bChecked ? MinOrConstant : Max;
		OnPropertyChanged.Execute({
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinNumTargetsToActivateAtOnce)),
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxNumTargetsToActivateAtOnce))
		});
	}
	UpdateBrushColors();
}

void UCustomGameModeActivationWidget::OnSelectionChanged_TargetActivationSelectionPolicy(
	const TArray<FString>& Selected, const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}

	BSConfig->TargetConfig.TargetActivationSelectionPolicy = GetEnumFromString_FromTagMap<
		ETargetActivationSelectionPolicy>(Selected[0]);
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetActivationSelectionPolicy))
	});
}

FString UCustomGameModeActivationWidget::GetComboBoxEntryTooltipStringTableKey_TargetActivationSelectionPolicy(
	const FString& EnumString)
{
	const ETargetActivationSelectionPolicy EnumValue = GetEnumFromString_FromTagMap<
		ETargetActivationSelectionPolicy>(EnumString);
	return GetStringTableKeyNameFromEnum(EnumValue);
}
