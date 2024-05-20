// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModeMovementWidget.h"
#include "MenuOptions/ComboBoxWidget.h"
#include "MenuOptions/DualRangeInputWidget.h"
#include "Utilities/ComboBox/BSComboBoxString.h"

void UCustomGameModeMovementWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

	SetupWarningTooltipCallbacks();
	UpdateBrushColors();
}

void UCustomGameModeMovementWidget::UpdateAllOptionsValid()
{
	Super::UpdateAllOptionsValid();
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

void UCustomGameModeMovementWidget::SetupWarningTooltipCallbacks()
{
	ComboBoxOption_MovingTargetDirectionMode->AddWarningTooltipData(FTooltipData("Invalid_Velocity_MTDM_None_2",
		ETooltipImageType::Caution)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && (BSConfig->
			TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeVelocity) || BSConfig->TargetConfig.
			TargetActivationResponses.Contains(ETargetActivationResponse::ChangeVelocity) || BSConfig->TargetConfig.
			TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeVelocity));
	});
	ComboBoxOption_MovingTargetDirectionMode->AddWarningTooltipData(FTooltipData("Invalid_Direction_MTDM_None_2",
		ETooltipImageType::Caution)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && (BSConfig->
			TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeDirection) || BSConfig->
			TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeDirection) || BSConfig
			->TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeVelocity));
	});
	ComboBoxOption_MovingTargetDirectionMode->AddWarningTooltipData(
		FTooltipData("Caution_ZeroForwardDistance_MTDM_ForwardOnly", ETooltipImageType::Caution)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::ForwardOnly && BSConfig->
			TargetConfig.BoxBounds.X <= 0.f;
	});
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
	UpdateAllOptionsValid();
}

void UCustomGameModeMovementWidget::OnMinMaxMenuOptionChanged(UDualRangeInputWidget* Widget, const bool bChecked,
	const float MinOrConstant, const float Max)
{
	if (Widget == MenuOption_SpawnedTargetVelocity)
	{
		BSConfig->TargetConfig.MinSpawnedTargetSpeed = MinOrConstant;
		BSConfig->TargetConfig.MaxSpawnedTargetSpeed = bChecked ? MinOrConstant : Max;
	}
	else if (Widget == MenuOption_ActivatedTargetVelocity)
	{
		BSConfig->TargetConfig.MinActivatedTargetSpeed = MinOrConstant;
		BSConfig->TargetConfig.MaxActivatedTargetSpeed = bChecked ? MinOrConstant : Max;
	}
	else if (Widget == MenuOption_DeactivatedTargetVelocity)
	{
		BSConfig->TargetConfig.MinDeactivatedTargetSpeed = MinOrConstant;
		BSConfig->TargetConfig.MaxDeactivatedTargetSpeed = bChecked ? MinOrConstant : Max;
	}
	UpdateBrushColors();
	UpdateAllOptionsValid();
}

FString UCustomGameModeMovementWidget::GetComboBoxEntryTooltipStringTableKey_MovingTargetDirectionMode(
	const FString& EnumString)
{
	const EMovingTargetDirectionMode EnumValue = GetEnumFromString_FromTagMap<EMovingTargetDirectionMode>(EnumString);
	return GetStringTableKeyNameFromEnum(EnumValue);
}
