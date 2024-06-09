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

	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetSpawnResponses)), ComboBoxOption_TargetSpawnResponses);
	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
			GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetActivationResponses)),
		ComboBoxOption_TargetActivationResponses);
	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
			GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDeactivationConditions)),
		ComboBoxOption_TargetDeactivationConditions);
	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
			GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDeactivationResponses)),
		ComboBoxOption_TargetDeactivationResponses);
	AssociatePropertyWithMenuOption(UBSGameModeValidator::FindBSConfigProperty(
			GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDestructionConditions)),
		ComboBoxOption_TargetDestructionConditions);

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
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetActivationResponses))
	});
	UpdateBrushColors();
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
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetSpawnResponses))
	});
	UpdateBrushColors();
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
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDeactivationConditions))
	});
	UpdateBrushColors();
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
	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDeactivationResponses))
	});
	UpdateBrushColors();
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

	OnPropertyChanged.Execute({
		UBSGameModeValidator::FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDestructionConditions))
	});
	UpdateBrushColors();
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
