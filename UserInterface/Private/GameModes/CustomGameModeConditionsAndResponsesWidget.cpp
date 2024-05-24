// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModeConditionsAndResponsesWidget.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "GameModes/CustomGameModeWidget.h"
#include "MenuOptions/ComboBoxWidget.h"
#include "Utilities/ComboBox/BSComboBoxString.h"

UCustomGameModeConditionsAndResponsesWidget::UCustomGameModeConditionsAndResponsesWidget():
	ComboBoxOption_TargetSpawnResponses(nullptr), ComboBoxOption_TargetActivationResponses(nullptr),
	ComboBoxOption_TargetDeactivationConditions(nullptr), ComboBoxOption_TargetDeactivationResponses(nullptr),
	ComboBoxOption_TargetDestructionConditions(nullptr)
{
	GameModeCategory = EGameModeCategory::TargetBehavior;
}

void UCustomGameModeConditionsAndResponsesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ComboBoxOption_TargetSpawnResponses->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_TargetSpawnResponses);
	ComboBoxOption_TargetActivationResponses->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_TargetActivationResponses);
	ComboBoxOption_TargetDeactivationConditions->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_TargetDeactivationConditions);
	ComboBoxOption_TargetDeactivationResponses->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_TargetDeactivationResponses);
	ComboBoxOption_TargetDestructionConditions->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_TargetDestructionConditions);

	ComboBoxOption_TargetSpawnResponses->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_TargetSpawnResponses);
	ComboBoxOption_TargetActivationResponses->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_TargetActivationResponses);
	ComboBoxOption_TargetDeactivationConditions->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_TargetDeactivationConditions);
	ComboBoxOption_TargetDeactivationResponses->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_TargetDeactivationResponses);
	ComboBoxOption_TargetDestructionConditions->GetComboBoxEntryTooltipStringTableKey.BindUObject(this,
		&ThisClass::GetComboBoxEntryTooltipStringTableKey_TargetDestructionConditions);

	ComboBoxOption_TargetSpawnResponses->ComboBox->ClearOptions();
	ComboBoxOption_TargetActivationResponses->ComboBox->ClearOptions();
	ComboBoxOption_TargetDeactivationConditions->ComboBox->ClearOptions();
	ComboBoxOption_TargetDeactivationResponses->ComboBox->ClearOptions();
	ComboBoxOption_TargetDestructionConditions->ComboBox->ClearOptions();

	TArray<FString> Options;
	for (const ETargetSpawnResponse& Method : TEnumRange<ETargetSpawnResponse>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_TargetSpawnResponses->SortAddOptionsAndSetEnumType<ETargetSpawnResponse>(Options);
	Options.Empty();

	for (const ETargetActivationResponse& Method : TEnumRange<ETargetActivationResponse>())
	{
		if (Method != ETargetActivationResponse::ChangeScale) // Deprecated
		{
			Options.Add(GetStringFromEnum_FromTagMap(Method));
		}
	}
	ComboBoxOption_TargetActivationResponses->SortAddOptionsAndSetEnumType<ETargetActivationResponse>(Options);
	Options.Empty();

	for (const ETargetDeactivationCondition& Method : TEnumRange<ETargetDeactivationCondition>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_TargetDeactivationConditions->SortAddOptionsAndSetEnumType<ETargetDeactivationCondition>(Options);
	Options.Empty();

	for (const ETargetDeactivationResponse& Method : TEnumRange<ETargetDeactivationResponse>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_TargetDeactivationResponses->SortAddOptionsAndSetEnumType<ETargetDeactivationResponse>(Options);
	Options.Empty();

	for (const ETargetDestructionCondition& Method : TEnumRange<ETargetDestructionCondition>())
	{
		Options.Add(GetStringFromEnum_FromTagMap(Method));
	}
	ComboBoxOption_TargetDestructionConditions->SortAddOptionsAndSetEnumType<ETargetDestructionCondition>(Options);
	Options.Empty();

	SetupWarningTooltipCallbacks();
	UpdateBrushColors();
}

void UCustomGameModeConditionsAndResponsesWidget::UpdateOptionsFromConfig()
{
	UpdateValueIfDifferent(ComboBoxOption_TargetSpawnResponses,
		GetStringArrayFromEnumArray_FromTagMap(BSConfig->TargetConfig.TargetSpawnResponses));
	UpdateValueIfDifferent(ComboBoxOption_TargetActivationResponses,
		GetStringArrayFromEnumArray_FromTagMap(BSConfig->TargetConfig.TargetActivationResponses));
	UpdateValueIfDifferent(ComboBoxOption_TargetDeactivationConditions,
		GetStringArrayFromEnumArray_FromTagMap(BSConfig->TargetConfig.TargetDeactivationConditions));
	UpdateValueIfDifferent(ComboBoxOption_TargetDeactivationResponses,
		GetStringArrayFromEnumArray_FromTagMap(BSConfig->TargetConfig.TargetDeactivationResponses));
	UpdateValueIfDifferent(ComboBoxOption_TargetDestructionConditions,
		GetStringArrayFromEnumArray_FromTagMap(BSConfig->TargetConfig.TargetDestructionConditions));

	UpdateDependentOptions_TargetSpawnResponses(BSConfig->TargetConfig.TargetSpawnResponses);
	UpdateDependentOptions_TargetActivationResponses(BSConfig->TargetConfig.TargetActivationResponses);
	UpdateDependentOptions_TargetDeactivationResponses(BSConfig->TargetConfig.TargetDeactivationResponses);

	UpdateBrushColors();
}

void UCustomGameModeConditionsAndResponsesWidget::SetupWarningTooltipCallbacks()
{
	ComboBoxOption_TargetSpawnResponses->AddWarningTooltipData(FTooltipData("Invalid_Velocity_MTDM_None",
		ETooltipImageType::Caution)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeVelocity) && BSConfig->
			TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None;
	});
	ComboBoxOption_TargetSpawnResponses->AddWarningTooltipData(FTooltipData("Invalid_Direction_MTDM_None",
		ETooltipImageType::Caution)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && BSConfig->
			TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeDirection);
	});
	ComboBoxOption_TargetDeactivationResponses->AddWarningTooltipData(FTooltipData("Invalid_Velocity_MTDM_None",
		ETooltipImageType::Caution)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && BSConfig->
			TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeVelocity);
	});
	ComboBoxOption_TargetDeactivationResponses->AddWarningTooltipData(FTooltipData("Invalid_Direction_MTDM_None",
		ETooltipImageType::Caution)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && BSConfig->
			TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeDirection);
	});
	ComboBoxOption_TargetActivationResponses->AddWarningTooltipData(FTooltipData("Invalid_Velocity_MTDM_None",
		ETooltipImageType::Caution)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && BSConfig->
			TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeVelocity);
	});
	ComboBoxOption_TargetActivationResponses->AddWarningTooltipData(FTooltipData("Invalid_Direction_MTDM_None",
		ETooltipImageType::Caution)).BindLambda([this]()
	{
		return BSConfig->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && BSConfig->
			TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeDirection);
	});
}


void UCustomGameModeConditionsAndResponsesWidget::UpdateDependentOptions_TargetDeactivationResponses(
	const TArray<ETargetDeactivationResponse>& Responses)
{
}

void UCustomGameModeConditionsAndResponsesWidget::UpdateDependentOptions_TargetSpawnResponses(
	const TArray<ETargetSpawnResponse>& Responses)
{
}

void UCustomGameModeConditionsAndResponsesWidget::UpdateDependentOptions_TargetActivationResponses(
	const TArray<ETargetActivationResponse>& Responses)
{
}

void UCustomGameModeConditionsAndResponsesWidget::OnSelectionChanged_TargetActivationResponses(
	const TArray<FString>& Selected, const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}

	if (Selected.IsEmpty())
	{
		BSConfig->TargetConfig.TargetActivationResponses.Empty();
	}
	else
	{
		BSConfig->TargetConfig.TargetActivationResponses = GetEnumArrayFromStringArray_FromTagMap<
			ETargetActivationResponse>(Selected);
	}

	UpdateDependentOptions_TargetActivationResponses(BSConfig->TargetConfig.TargetActivationResponses);
	UpdateBrushColors();
	//UpdateAllOptionsValid();
}

void UCustomGameModeConditionsAndResponsesWidget::OnSelectionChanged_TargetSpawnResponses(
	const TArray<FString>& Selected, const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}

	if (Selected.IsEmpty())
	{
		BSConfig->TargetConfig.TargetSpawnResponses.Empty();
	}
	else
	{
		BSConfig->TargetConfig.TargetSpawnResponses = GetEnumArrayFromStringArray_FromTagMap<
			ETargetSpawnResponse>(Selected);
	}

	UpdateDependentOptions_TargetSpawnResponses(BSConfig->TargetConfig.TargetSpawnResponses);
	UpdateBrushColors();
	//UpdateAllOptionsValid();
}

void UCustomGameModeConditionsAndResponsesWidget::OnSelectionChanged_TargetDeactivationConditions(
	const TArray<FString>& Selected, const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}

	if (Selected.IsEmpty())
	{
		BSConfig->TargetConfig.TargetDeactivationConditions.Empty();
	}
	else
	{
		BSConfig->TargetConfig.TargetDeactivationConditions = GetEnumArrayFromStringArray_FromTagMap<
			ETargetDeactivationCondition>(Selected);
	}

	UpdateBrushColors();
	//UpdateAllOptionsValid();
}

void UCustomGameModeConditionsAndResponsesWidget::OnSelectionChanged_TargetDeactivationResponses(
	const TArray<FString>& Selected, const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}

	if (Selected.IsEmpty())
	{
		BSConfig->TargetConfig.TargetDeactivationResponses.Empty();
	}
	else
	{
		BSConfig->TargetConfig.TargetDeactivationResponses = GetEnumArrayFromStringArray_FromTagMap<
			ETargetDeactivationResponse>(Selected);
	}

	UpdateDependentOptions_TargetDeactivationResponses(BSConfig->TargetConfig.TargetDeactivationResponses);
	UpdateBrushColors();
	//UpdateAllOptionsValid();
}

void UCustomGameModeConditionsAndResponsesWidget::OnSelectionChanged_TargetDestructionConditions(
	const TArray<FString>& Selected, const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}

	if (Selected.IsEmpty())
	{
		BSConfig->TargetConfig.TargetDestructionConditions.Empty();
	}
	else
	{
		BSConfig->TargetConfig.TargetDestructionConditions = GetEnumArrayFromStringArray_FromTagMap<
			ETargetDestructionCondition>(Selected);
	}

	UpdateBrushColors();
	//UpdateAllOptionsValid();
}

FString UCustomGameModeConditionsAndResponsesWidget::GetComboBoxEntryTooltipStringTableKey_TargetSpawnResponses(
	const FString& EnumString)
{
	return GetStringTableKeyNameFromEnum(GetEnumFromString_FromTagMap<ETargetSpawnResponse>(EnumString));
}

FString UCustomGameModeConditionsAndResponsesWidget::GetComboBoxEntryTooltipStringTableKey_TargetActivationResponses(
	const FString& EnumString)
{
	return GetStringTableKeyNameFromEnum(GetEnumFromString_FromTagMap<ETargetActivationResponse>(EnumString));
}

FString UCustomGameModeConditionsAndResponsesWidget::GetComboBoxEntryTooltipStringTableKey_TargetDeactivationConditions(
	const FString& EnumString)
{
	return GetStringTableKeyNameFromEnum(GetEnumFromString_FromTagMap<ETargetDeactivationCondition>(EnumString));
}

FString UCustomGameModeConditionsAndResponsesWidget::GetComboBoxEntryTooltipStringTableKey_TargetDeactivationResponses(
	const FString& EnumString)
{
	return GetStringTableKeyNameFromEnum(GetEnumFromString_FromTagMap<ETargetDeactivationResponse>(EnumString));
}

FString UCustomGameModeConditionsAndResponsesWidget::GetComboBoxEntryTooltipStringTableKey_TargetDestructionConditions(
	const FString& EnumString)
{
	return GetStringTableKeyNameFromEnum(GetEnumFromString_FromTagMap<ETargetDestructionCondition>(EnumString));
}
