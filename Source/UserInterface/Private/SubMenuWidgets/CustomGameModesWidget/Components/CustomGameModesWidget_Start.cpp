﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "SubMenuWidgets/CustomGameModesWidget/Components/CustomGameModesWidget_Start.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "WidgetComponents/BSComboBoxString.h"
#include "WidgetComponents/MenuOptionWidgets/CheckBoxOptionWidget.h"
#include "WidgetComponents/MenuOptionWidgets/ComboBoxOptionWidget.h"
#include "WidgetComponents/MenuOptionWidgets/EditableTextBoxOptionWidget.h"

void UCustomGameModesWidget_Start::NativeConstruct()
{
	Super::NativeConstruct();

	SetupTooltip(CheckBoxOption_UseTemplate->GetTooltipImage(), CheckBoxOption_UseTemplate->GetTooltipImageText());
	SetupTooltip(ComboBoxOption_GameModeTemplates->GetTooltipImage(), ComboBoxOption_GameModeTemplates->GetTooltipImageText());
	SetupTooltip(ComboBoxOption_GameModeDifficulty->GetTooltipImage(), ComboBoxOption_GameModeDifficulty->GetTooltipImageText());
	SetupTooltip(EditableTextBoxOption_CustomGameModeName->GetTooltipImage(), EditableTextBoxOption_CustomGameModeName->GetTooltipImageText());

	CheckBoxOption_UseTemplate->CheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::OnCheckStateChanged_UseTemplate);
	EditableTextBoxOption_CustomGameModeName->EditableTextBox->OnTextChanged.AddUniqueDynamic(this, &ThisClass::OnTextChanged_CustomGameModeName);
	ComboBoxOption_GameModeTemplates->ComboBox->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::OnSelectionChanged_GameModeTemplates);
	ComboBoxOption_GameModeDifficulty->ComboBox->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::OnSelectionChanged_GameModeDifficulty);

	ComboBoxOption_GameModeTemplates->ComboBox->ClearOptions();
	ComboBoxOption_GameModeDifficulty->ComboBox->ClearOptions();

	RefreshGameModeTemplateComboBoxOptions();
	
	for (const EGameModeDifficulty& Difficulty : TEnumRange<EGameModeDifficulty>())
	{
		if (Difficulty != EGameModeDifficulty::None)
		{
			ComboBoxOption_GameModeDifficulty->ComboBox->AddOption(UEnum::GetDisplayValueAsText(Difficulty).ToString());
		}
	}
	
	UpdateBrushColors();
}

void UCustomGameModesWidget_Start::InitComponent(FBSConfig* InConfigPtr, TObjectPtr<UCustomGameModesWidgetComponent> InNext)
{
	Super::InitComponent(InConfigPtr, InNext);
}

FString UCustomGameModesWidget_Start::GetNewCustomGameModeName() const
{
	return EditableTextBoxOption_CustomGameModeName->EditableTextBox->GetText().ToString();
}

FStartWidgetProperties UCustomGameModesWidget_Start::GetStartWidgetProperties() const
{
	FStartWidgetProperties Properties = FStartWidgetProperties();
	
	const FString GameModeTemplateString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption();
	const FString DifficultyString = ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOption();
	const FString NewCustomGameModeName = EditableTextBoxOption_CustomGameModeName->EditableTextBox->GetText().ToString();
	const bool bUseTemplateChecked = CheckBoxOption_UseTemplate->CheckBox->IsChecked();

	Properties.bUseTemplateChecked = bUseTemplateChecked;
	Properties.NewCustomGameModeName = NewCustomGameModeName;

	if (!bUseTemplateChecked)
	{
		return Properties;
	}
	
	if (IsPresetGameMode(GameModeTemplateString))
	{
		Properties.DefiningConfig.BaseGameMode = GetEnumFromString<EBaseGameMode>(GameModeTemplateString, EBaseGameMode::None);
		Properties.DefiningConfig.Difficulty = GetEnumFromString<EGameModeDifficulty>(DifficultyString, EGameModeDifficulty::None);
	}
	else
	{
		Properties.DefiningConfig.BaseGameMode = EBaseGameMode::None;
		if (IsCustomGameMode(GameModeTemplateString))
		{
			Properties.DefiningConfig.CustomGameModeName = GameModeTemplateString;
			Properties.DefiningConfig.Difficulty = EGameModeDifficulty::None;
		}
	}
	
	return Properties;
}

void UCustomGameModesWidget_Start::SetNewCustomGameModeName(const FString& InCustomGameModeName) const
{
	EditableTextBoxOption_CustomGameModeName->EditableTextBox->SetText(FText::FromString(InCustomGameModeName));
}

void UCustomGameModesWidget_Start::SetStartWidgetProperties(const FStartWidgetProperties& InProperties)
{
	// Always update NewCustomGameModeName and Checkbox
	UpdateValueIfDifferent(EditableTextBoxOption_CustomGameModeName, FText::FromString(InProperties.NewCustomGameModeName));
	const bool bUpdatedUseTemplate = UpdateValueIfDifferent(CheckBoxOption_UseTemplate, InProperties.bUseTemplateChecked);
	
	// If transition from using template to not using template, clear collapse return
	if (bUpdatedUseTemplate)
	{
		if (!InProperties.bUseTemplateChecked)
		{
			ComboBoxOption_GameModeTemplates->ComboBox->ClearSelection();
			ComboBoxOption_GameModeDifficulty->ComboBox->ClearSelection();
			ComboBoxOption_GameModeTemplates->SetVisibility(ESlateVisibility::Collapsed);
			ComboBoxOption_GameModeDifficulty->SetVisibility(ESlateVisibility::Collapsed);
			UpdateBrushColors();
			SetAllOptionsValid(UpdateAllOptionsValid());
			return;
		}
		// Otherwise show game mode templates
		ComboBoxOption_GameModeTemplates->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	const FString BaseGameModeString = GetStringFromEnum(InProperties.DefiningConfig.BaseGameMode);
	const FString CustomGameModeString = InProperties.DefiningConfig.CustomGameModeName;
	const FString DifficultyString = GetStringFromEnum(InProperties.DefiningConfig.Difficulty);

	const bool bIsCustom = IsCustomGameMode(InProperties.DefiningConfig.CustomGameModeName);
	const bool bIsPreset = IsPresetGameMode(BaseGameModeString);
	
	const FString NewTemplateOptionString = bIsCustom ? CustomGameModeString : bIsPreset ? BaseGameModeString : "";
	
	// Updating values
	UpdateValueIfDifferent(ComboBoxOption_GameModeTemplates, NewTemplateOptionString);
	UpdateDifficultySelection(InProperties.DefiningConfig.Difficulty);
	
	// Updating visibility
	const bool bUpdatedTemplateVisibility = UpdateGameModeTemplateVisibility();
	const bool bUpdatedDifficultyVisibility = UpdateDifficultyVisibility();
	
	if (bUpdatedUseTemplate || bUpdatedTemplateVisibility || bUpdatedDifficultyVisibility)
	{
		UpdateBrushColors();
	}

	SetAllOptionsValid(UpdateAllOptionsValid());
}

bool UCustomGameModesWidget_Start::UpdateDifficultySelection(const EGameModeDifficulty& Difficulty) const
{
	const int32 SelectedOptionCount = ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOptionCount();
	const FString GameModeTemplateString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption();
	
	if (IsPresetGameMode(GameModeTemplateString))
	{
		if (SelectedOptionCount == 0)
		{
			// Set default difficulty to Normal if switching from Custom (no difficulty) to preset
			ComboBoxOption_GameModeDifficulty->ComboBox->SetSelectedOption(GetStringFromEnum(EGameModeDifficulty::Normal));
			return true;
		}

		// No change
		if (GetEnumFromString<EGameModeDifficulty>(ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOption()) == Difficulty)
		{
			return false;
		}

		ComboBoxOption_GameModeDifficulty->ComboBox->SetSelectedOption(GetStringFromEnum(Difficulty));
		return true;
	}
	
	if (IsCustomGameMode(GameModeTemplateString))
	{
		// No change
		if (SelectedOptionCount == 0)
		{
			return false;
		}
		ComboBoxOption_GameModeDifficulty->ComboBox->ClearSelection();
		return true;
	}

	return false;
}

bool UCustomGameModesWidget_Start::UpdateDifficultyVisibility() const
{
	const FString TemplateOptionString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption();
	const ESlateVisibility NewVisibility = IsPresetGameMode(TemplateOptionString) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	
	if (ComboBoxOption_GameModeDifficulty->GetVisibility() == NewVisibility)
	{
		return false;
	}
	
	ComboBoxOption_GameModeDifficulty->SetVisibility(NewVisibility);
	return true;
}

bool UCustomGameModesWidget_Start::UpdateGameModeTemplateVisibility() const
{
	const FString TemplateOptionString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption();
	const bool bShouldShow = IsPresetGameMode(TemplateOptionString) || IsCustomGameMode(TemplateOptionString) || CheckBoxOption_UseTemplate->CheckBox->IsChecked();
	const ESlateVisibility NewVisibility = bShouldShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;

	if (ComboBoxOption_GameModeTemplates->GetVisibility() == NewVisibility)
	{
		return false;
	}
	
	ComboBoxOption_GameModeTemplates->SetVisibility(NewVisibility);
	return true;
}

void UCustomGameModesWidget_Start::RefreshGameModeTemplateComboBoxOptions() const
{
	ComboBoxOption_GameModeTemplates->ComboBox->ClearOptions();
	TArray<FString> Options;
	
	for (const EBaseGameMode& GameMode : TEnumRange<EBaseGameMode>())
	{
		if (GameMode != EBaseGameMode::None)
		{
			Options.Add(GetStringFromEnum(GameMode));
		}
	}
	for (const FString& Option : Options)
	{
		ComboBoxOption_GameModeTemplates->ComboBox->AddOption(Option);
	}
	Options.Empty();

	for (const FBSConfig& GameMode : LoadCustomGameModes())
	{
		Options.Add(GameMode.DefiningConfig.CustomGameModeName);
	}
	Options.Sort([] (const FString& FirstOption, const FString& SecondOption)
	{
		return FirstOption < SecondOption;
	});
	for (const FString& Option : Options)
	{
		ComboBoxOption_GameModeTemplates->ComboBox->AddOption(Option);
	}
	Options.Empty();
}

bool UCustomGameModesWidget_Start::UpdateAllOptionsValid()
{
	const FString GameModeTemplateString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption();
	const FText CustomGameModeNameText = EditableTextBoxOption_CustomGameModeName->EditableTextBox->GetText();
	const FString DifficultyString = ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOption();

	// Never let a user save a custom game mode with a Preset Game Mode name
	if (IsPresetGameMode(CustomGameModeNameText.ToString()))
	{
		return false;
	}

	// Can only have empty CustomGameModeName if template is custom
	if (CustomGameModeNameText.IsEmptyOrWhitespace())
	{
		if (!IsCustomGameMode(GameModeTemplateString))
		{
			return false;
		}
	}
	
	if (CheckBoxOption_UseTemplate->CheckBox->IsChecked())
	{
		// Need to select a template if use template
		if (ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOptionCount() != 1)
		{
			return false;
		}
		if (IsPresetGameMode(GameModeTemplateString))
		{
			// Need to select a difficulty if using Preset
			if (ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOptionCount() != 1)
			{
				return false;
			}
		}
	}
	return true;
}

void UCustomGameModesWidget_Start::UpdateOptionsFromConfig()
{
	const bool bUpdatedTemplateVisibility = UpdateGameModeTemplateVisibility();
	const bool bUpdatedDifficultyVisibility = UpdateDifficultyVisibility();

	if (IsPresetGameMode(ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption()))
	{
		if (ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOptionCount() == 0)
		{
			UpdateDifficultySelection(BSConfig->DefiningConfig.Difficulty);
		}
	}

	// Select a custom mode if conditions permit, mainly for saving and repopulating after saving
	if (CheckBoxOption_UseTemplate->CheckBox->IsChecked() &&
		IsCustomGameMode(BSConfig->DefiningConfig.CustomGameModeName) &&
		ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOptionCount() == 0 &&
		ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOptionCount() == 0 &&
		EditableTextBoxOption_CustomGameModeName->EditableTextBox->GetText().IsEmpty())
	{
		ComboBoxOption_GameModeTemplates->ComboBox->SetSelectedOption(BSConfig->DefiningConfig.CustomGameModeName);
	}
	
	if (bUpdatedTemplateVisibility || bUpdatedDifficultyVisibility)
	{
		UpdateBrushColors();
	}

	SetAllOptionsValid(UpdateAllOptionsValid());
}

void UCustomGameModesWidget_Start::OnCheckStateChanged_UseTemplate(const bool bChecked)
{
	// Hide Templates and Difficulty if not checked
	if (!bChecked)
	{
		ComboBoxOption_GameModeTemplates->ComboBox->ClearSelection();
		ComboBoxOption_GameModeDifficulty->ComboBox->ClearSelection();
	}
	
	const bool bUpdatedTemplateVisibility = UpdateGameModeTemplateVisibility();
	const bool bUpdatedDifficultyVisibility = UpdateDifficultyVisibility();
	
	if (bUpdatedTemplateVisibility || bUpdatedDifficultyVisibility)
	{
		UpdateBrushColors();
	}

	SetAllOptionsValid(UpdateAllOptionsValid());

	const FString SelectedDifficulty = ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOption();
	const EGameModeDifficulty Difficulty = SelectedDifficulty.IsEmpty() ? EGameModeDifficulty::None : GetEnumFromString<EGameModeDifficulty>(SelectedDifficulty);
	RequestGameModeTemplateUpdate.Broadcast(ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption(), Difficulty);
}

void UCustomGameModesWidget_Start::OnTextChanged_CustomGameModeName(const FText& Text)
{
	SetAllOptionsValid(UpdateAllOptionsValid());
}

void UCustomGameModesWidget_Start::OnSelectionChanged_GameModeTemplates(const TArray<FString>& Selected, const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}
	const FString SelectedDifficulty = ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOption();
	const EGameModeDifficulty Difficulty = SelectedDifficulty.IsEmpty() ? EGameModeDifficulty::None : GetEnumFromString<EGameModeDifficulty>(SelectedDifficulty);
	RequestGameModeTemplateUpdate.Broadcast(Selected[0], Difficulty);
}

void UCustomGameModesWidget_Start::OnSelectionChanged_GameModeDifficulty(const TArray<FString>& Selected, const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}
	
	// Only request update for difficulty if Preset GameMode
	if (const FString GameModeTemplateString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption(); IsPresetGameMode(GameModeTemplateString))
	{
		RequestGameModeTemplateUpdate.Broadcast(GameModeTemplateString,  GetEnumFromString<EGameModeDifficulty>(Selected[0]));
	}
}
