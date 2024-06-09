// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModeMovementWidget.h"
#include "BSGameModeConfig/BSConfig.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "MenuOptions/ComboBoxWidget.h"
#include "MenuOptions/DualRangeInputWidget.h"
#include "Utilities/ComboBox/BSComboBoxString.h"

UCustomGameModeMovementWidget::UCustomGameModeMovementWidget(): MenuOption_ActivatedTargetVelocity(nullptr),
                                                                MenuOption_DeactivatedTargetVelocity(nullptr),
                                                                MenuOption_SpawnedTargetVelocity(nullptr),
                                                                ComboBoxOption_MovingTargetDirectionMode(nullptr)
{
	GameModeCategory = EGameModeCategory::TargetMovement;
}

void UCustomGameModeMovementWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinSpawnedTargetSpeed)), MenuOption_SpawnedTargetVelocity);
	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxSpawnedTargetSpeed)), MenuOption_SpawnedTargetVelocity);
	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinActivatedTargetSpeed)), MenuOption_ActivatedTargetVelocity);
	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxActivatedTargetSpeed)), MenuOption_ActivatedTargetVelocity);
	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinDeactivatedTargetSpeed)), MenuOption_DeactivatedTargetVelocity);
	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxDeactivatedTargetSpeed)), MenuOption_DeactivatedTargetVelocity);
	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
			GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MovingTargetDirectionMode)),
		ComboBoxOption_MovingTargetDirectionMode);

	MenuOption_SpawnedTargetVelocity->SetValues(Constants::MinValue_TargetSpeed, Constants::MaxValue_TargetSpeed,
		Constants::SnapSize_TargetSpeed);
	MenuOption_ActivatedTargetVelocity->SetValues(Constants::MinValue_TargetSpeed, Constants::MaxValue_TargetSpeed,
		Constants::SnapSize_TargetSpeed);
	MenuOption_DeactivatedTargetVelocity->SetValues(Constants::MinValue_TargetSpeed, Constants::MaxValue_TargetSpeed,
		Constants::SnapSize_TargetSpeed);

	MenuOption_SpawnedTargetVelocity->OnMinMaxMenuOptionChanged.AddUObject(this, &ThisClass::OnMinMaxMenuOptionChanged);
	MenuOption_ActivatedTargetVelocity->OnMinMaxMenuOptionChanged.AddUObject(this,
		&ThisClass::OnMinMaxMenuOptionChanged);
	MenuOption_DeactivatedTargetVelocity->OnMinMaxMenuOptionChanged.AddUObject(this,
		&ThisClass::OnMinMaxMenuOptionChanged);

	ComboBoxOption_MovingTargetDirectionMode->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_MovingTargetDirectionMode);
	ComboBoxOption_MovingTargetDirectionMode->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_MovingTargetDirectionMode);
	ComboBoxOption_MovingTargetDirectionMode->ComboBox->ClearOptions();

	TArray<FString> Options;
	for (const EMovingTargetDirectionMode& Method : TEnumRange<EMovingTargetDirectionMode>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_MovingTargetDirectionMode->SortAddOptionsAndSetEnumType<EMovingTargetDirectionMode>(Options);
	Options.Empty();

	SetMenuOptionEnabledStateAndAddTooltip(MenuOption_SpawnedTargetVelocity, EMenuOptionEnabledState::Enabled);
	SetMenuOptionEnabledStateAndAddTooltip(MenuOption_ActivatedTargetVelocity, EMenuOptionEnabledState::Enabled);
	SetMenuOptionEnabledStateAndAddTooltip(MenuOption_DeactivatedTargetVelocity, EMenuOptionEnabledState::Enabled);

	UpdateBrushColors();
}

void UCustomGameModeMovementWidget::UpdateOptionsFromConfig()
{
	const bool bConstantSpawnedSpeed = BSConfig->TargetConfig.MinSpawnedTargetSpeed == BSConfig->TargetConfig.
		MaxSpawnedTargetSpeed;
	const bool bConstantActivatedSpeed = BSConfig->TargetConfig.MinActivatedTargetSpeed == BSConfig->TargetConfig.
		MaxActivatedTargetSpeed;
	const bool bConstantDeactivatedSpeed = BSConfig->TargetConfig.MinDeactivatedTargetSpeed == BSConfig->TargetConfig.
		MaxDeactivatedTargetSpeed;

	UpdateValuesIfDifferent(MenuOption_SpawnedTargetVelocity, bConstantSpawnedSpeed,
		BSConfig->TargetConfig.MinSpawnedTargetSpeed, BSConfig->TargetConfig.MaxSpawnedTargetSpeed);
	UpdateValuesIfDifferent(MenuOption_ActivatedTargetVelocity, bConstantActivatedSpeed,
		BSConfig->TargetConfig.MinActivatedTargetSpeed, BSConfig->TargetConfig.MaxActivatedTargetSpeed);
	UpdateValuesIfDifferent(MenuOption_DeactivatedTargetVelocity, bConstantDeactivatedSpeed,
		BSConfig->TargetConfig.MinDeactivatedTargetSpeed, BSConfig->TargetConfig.MaxDeactivatedTargetSpeed);

	UpdateValueIfDifferent(ComboBoxOption_MovingTargetDirectionMode,
		GetStringFromEnum_FromTagMap(BSConfig->TargetConfig.MovingTargetDirectionMode));

	UpdateDependentOptions_SpawnResponses(BSConfig->TargetConfig.TargetSpawnResponses, bConstantSpawnedSpeed);
	UpdateDependentOptions_ActivationResponses(BSConfig->TargetConfig.TargetActivationResponses,
		bConstantActivatedSpeed);
	UpdateDependentOptions_DeactivationResponses(BSConfig->TargetConfig.TargetDeactivationResponses,
		bConstantDeactivatedSpeed);

	UpdateBrushColors();
}

void UCustomGameModeMovementWidget::UpdateDependentOptions_SpawnResponses(const TArray<ETargetSpawnResponse>& Responses,
	const bool bConstant)
{
	if (Responses.Contains(ETargetSpawnResponse::ChangeVelocity))
	{
		SetMenuOptionEnabledStateAndAddTooltip(MenuOption_SpawnedTargetVelocity, EMenuOptionEnabledState::Enabled);
	}
	else
	{
		SetMenuOptionEnabledStateAndAddTooltip(MenuOption_SpawnedTargetVelocity,
			EMenuOptionEnabledState::DependentMissing, "DM_SpawnedTargetVelocity");
	}
}

void UCustomGameModeMovementWidget::UpdateDependentOptions_ActivationResponses(
	const TArray<ETargetActivationResponse>& Responses, const bool bConstant)
{
	if (Responses.Contains(ETargetActivationResponse::ChangeVelocity))
	{
		SetMenuOptionEnabledStateAndAddTooltip(MenuOption_ActivatedTargetVelocity, EMenuOptionEnabledState::Enabled);
	}
	else
	{
		SetMenuOptionEnabledStateAndAddTooltip(MenuOption_ActivatedTargetVelocity,
			EMenuOptionEnabledState::DependentMissing, "DM_ActivatedTargetVelocity");
	}
}

void UCustomGameModeMovementWidget::UpdateDependentOptions_DeactivationResponses(
	const TArray<ETargetDeactivationResponse>& Responses, const bool bConstant)
{
	if (Responses.Contains(ETargetDeactivationResponse::ChangeVelocity))
	{
		SetMenuOptionEnabledStateAndAddTooltip(MenuOption_DeactivatedTargetVelocity, EMenuOptionEnabledState::Enabled);
	}
	else
	{
		SetMenuOptionEnabledStateAndAddTooltip(MenuOption_DeactivatedTargetVelocity,
			EMenuOptionEnabledState::DependentMissing, "DM_DeactivatedTargetVelocity");
	}
}

void UCustomGameModeMovementWidget::OnSelectionChanged_MovingTargetDirectionMode(const TArray<FString>& Selected,
	const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}

	BSConfig->TargetConfig.MovingTargetDirectionMode = GetEnumFromString_FromTagMap<
		EMovingTargetDirectionMode>(Selected[0]);
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MovingTargetDirectionMode))
	});
}

void UCustomGameModeMovementWidget::OnMinMaxMenuOptionChanged(UDualRangeInputWidget* Widget, const bool bChecked,
	const float MinOrConstant, const float Max)
{
	if (Widget == MenuOption_SpawnedTargetVelocity)
	{
		BSConfig->TargetConfig.MinSpawnedTargetSpeed = MinOrConstant;
		BSConfig->TargetConfig.MaxSpawnedTargetSpeed = bChecked ? MinOrConstant : Max;
		OnPropertyChanged.Execute({
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinSpawnedTargetSpeed)),
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxSpawnedTargetSpeed))
		});
	}
	else if (Widget == MenuOption_ActivatedTargetVelocity)
	{
		BSConfig->TargetConfig.MinActivatedTargetSpeed = MinOrConstant;
		BSConfig->TargetConfig.MaxActivatedTargetSpeed = bChecked ? MinOrConstant : Max;
		OnPropertyChanged.Execute({
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinActivatedTargetSpeed)),
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxActivatedTargetSpeed))
		});
	}
	else if (Widget == MenuOption_DeactivatedTargetVelocity)
	{
		BSConfig->TargetConfig.MinDeactivatedTargetSpeed = MinOrConstant;
		BSConfig->TargetConfig.MaxDeactivatedTargetSpeed = bChecked ? MinOrConstant : Max;
		OnPropertyChanged.Execute({
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinDeactivatedTargetSpeed)),
			UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxDeactivatedTargetSpeed))
		});
	}
	UpdateBrushColors();
}

FString UCustomGameModeMovementWidget::GetComboBoxEntryTooltipStringTableKey_MovingTargetDirectionMode(
	const FString& EnumString)
{
	const EMovingTargetDirectionMode EnumValue = GetEnumFromString_FromTagMap<EMovingTargetDirectionMode>(EnumString);
	return GetStringTableKeyNameFromEnum(EnumValue);
}
