// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModeStartWidget.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "GameModes/CustomGameModeWidget.h"
#include "MenuOptions/CheckBoxWidget.h"
#include "MenuOptions/ComboBoxWidget.h"
#include "MenuOptions/TextInputWidget.h"
#include "Utilities/ComboBox/BSComboBoxString.h"

UCustomGameModeStartWidget::UCustomGameModeStartWidget(): CheckBoxOption_UseTemplate(nullptr),
                                                          ComboBoxOption_GameModeTemplates(nullptr),
                                                          ComboBoxOption_GameModeDifficulty(nullptr),
                                                          EditableTextBoxOption_CustomGameModeName(nullptr)
{
	GameModeCategory = EGameModeCategory::Start;
}

void UCustomGameModeStartWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CheckBoxOption_UseTemplate->CheckBox->OnCheckStateChanged.AddUniqueDynamic(this,
		&ThisClass::OnCheckStateChanged_UseTemplate);
	EditableTextBoxOption_CustomGameModeName->EditableTextBox->OnTextChanged.AddUniqueDynamic(this,
		&ThisClass::OnTextChanged_CustomGameModeName);
	ComboBoxOption_GameModeTemplates->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_GameModeTemplates);
	ComboBoxOption_GameModeDifficulty->ComboBox->OnSelectionChanged.AddUniqueDynamic(this,
		&ThisClass::OnSelectionChanged_GameModeDifficulty);

	ComboBoxOption_GameModeTemplates->ComboBox->ClearOptions();
	ComboBoxOption_GameModeDifficulty->ComboBox->ClearOptions();

	for (const EGameModeDifficulty& Difficulty : TEnumRange<EGameModeDifficulty>())
	{
		if (Difficulty != EGameModeDifficulty::None)
		{
			ComboBoxOption_GameModeDifficulty->ComboBox->AddOption(UEnum::GetDisplayValueAsText(Difficulty).ToString());
		}
	}

	UpdateBrushColors();
}

FStartWidgetProperties UCustomGameModeStartWidget::GetStartWidgetProperties() const
{
	return StartWidgetProperties;

	/*FStartWidgetProperties Properties = FStartWidgetProperties();

	const FString GameModeTemplateString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption();
	const FString DifficultyString = ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOption();
	const FString NewCustomGameModeName = EditableTextBoxOption_CustomGameModeName->EditableTextBox->GetText().
		ToString();
	const bool bUseTemplateChecked = CheckBoxOption_UseTemplate->CheckBox->IsChecked();

	Properties.bUseTemplateChecked = bUseTemplateChecked;
	Properties.NewCustomGameModeName = NewCustomGameModeName;

	if (!bUseTemplateChecked)
	{
		return Properties;
	}

	if (IBSGameModeInterface::IsPresetGameMode(GameModeTemplateString))
	{
		Properties.DefiningConfig.BaseGameMode = GetEnumFromString<EBaseGameMode>(GameModeTemplateString);
		Properties.DefiningConfig.Difficulty = GetEnumFromString<EGameModeDifficulty>(DifficultyString);
	}
	else
	{
		Properties.DefiningConfig.BaseGameMode = EBaseGameMode::None;
		if (IBSGameModeInterface::IsCustomGameMode(GameModeTemplateString))
		{
			Properties.DefiningConfig.CustomGameModeName = GameModeTemplateString;
			Properties.DefiningConfig.Difficulty = EGameModeDifficulty::None;
		}
	}

	return Properties;*/
}

void UCustomGameModeStartWidget::SetStartWidgetProperties(const FStartWidgetProperties& InProperties)
{
	StartWidgetProperties = InProperties;

	EditableTextBoxOption_CustomGameModeName->EditableTextBox->SetText(
		FText::FromString(StartWidgetProperties.NewCustomGameModeName));
	TSignalBlocker(CheckBoxOption_UseTemplate->CheckBox->OnCheckStateChanged, this,
		&ThisClass::OnCheckStateChanged_UseTemplate);

	CheckBoxOption_UseTemplate->CheckBox->SetIsChecked(StartWidgetProperties.bUseTemplateChecked);

	if (StartWidgetProperties.Difficulty.IsEmpty())
	{
		ComboBoxOption_GameModeDifficulty->ComboBox->ClearSelection();
	}
	else
	{
		ComboBoxOption_GameModeDifficulty->ComboBox->SetSelectedOption(StartWidgetProperties.Difficulty);
	}

	if (StartWidgetProperties.GameModeName.IsEmpty())
	{
		ComboBoxOption_GameModeTemplates->ComboBox->ClearSelection();
	}
	else
	{
		ComboBoxOption_GameModeTemplates->ComboBox->SetSelectedOption(StartWidgetProperties.GameModeName);
	}

	ComboBoxOption_GameModeDifficulty->SetVisibility(StartWidgetProperties.bIsPreset
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
	ComboBoxOption_GameModeTemplates->ComboBox->SetSelectedOption(StartWidgetProperties.GameModeName);
	const bool bShouldShow = StartWidgetProperties.bIsCustom || StartWidgetProperties.bIsPreset || StartWidgetProperties
		.bUseTemplateChecked;
	ComboBoxOption_GameModeTemplates->SetVisibility(bShouldShow
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
	UpdateBrushColors();
}

/*bool UCustomGameModeStartWidget::UpdateDifficultySelection(const EGameModeDifficulty& Difficulty) const
{
	const int32 SelectedOptionCount = ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOptionCount();
	const FString GameModeTemplateString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption();

	if (IBSGameModeInterface::IsPresetGameMode(GameModeTemplateString))
	{
		if (SelectedOptionCount == 0)
		{
			// Set default difficulty to Normal if switching from Custom (no difficulty) to preset
			ComboBoxOption_GameModeDifficulty->ComboBox->SetSelectedOption(
				GetStringFromEnum(EGameModeDifficulty::Normal));
			return true;
		}

		// No change
		if (GetEnumFromString<EGameModeDifficulty>(ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOption()) ==
			Difficulty)
		{
			return false;
		}

		ComboBoxOption_GameModeDifficulty->ComboBox->SetSelectedOption(GetStringFromEnum(Difficulty));
		return true;
	}

	if (IBSGameModeInterface::IsCustomGameMode(GameModeTemplateString))
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
}*/

/*bool UCustomGameModeStartWidget::UpdateDifficultyVisibility() const
{
	const FString TemplateOptionString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption();
	const ESlateVisibility NewVisibility = IBSGameModeInterface::IsPresetGameMode(TemplateOptionString)
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed;

	if (ComboBoxOption_GameModeDifficulty->GetVisibility() == NewVisibility)
	{
		return false;
	}

	ComboBoxOption_GameModeDifficulty->SetVisibility(NewVisibility);
	return true;
}*/

/*bool UCustomGameModeStartWidget::UpdateGameModeTemplateVisibility() const
{
	const FString TemplateOptionString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption();
	const bool bShouldShow = IBSGameModeInterface::IsPresetGameMode(TemplateOptionString) ||
		IBSGameModeInterface::IsCustomGameMode(TemplateOptionString) || CheckBoxOption_UseTemplate->CheckBox->
		IsChecked();
	const ESlateVisibility NewVisibility = bShouldShow
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed;

	if (ComboBoxOption_GameModeTemplates->GetVisibility() == NewVisibility)
	{
		return false;
	}

	ComboBoxOption_GameModeTemplates->SetVisibility(NewVisibility);
	return true;
}*/

void UCustomGameModeStartWidget::RefreshGameModeTemplateComboBoxOptions(const TArray<FBSConfig>& CustomGameModes) const
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

	for (const FBSConfig& GameMode : CustomGameModes)
	{
		Options.Add(GameMode.DefiningConfig.CustomGameModeName);
	}
	Options.Sort([](const FString& FirstOption, const FString& SecondOption)
	{
		return FirstOption < SecondOption;
	});
	for (const FString& Option : Options)
	{
		ComboBoxOption_GameModeTemplates->ComboBox->AddOption(Option);
	}
}

/*void UCustomGameModeStartWidget::UpdateAllOptionsValid()
{
	const FString GameModeTemplateString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption();
	const FText CustomGameModeNameText = EditableTextBoxOption_CustomGameModeName->EditableTextBox->GetText();
	const FString DifficultyString = ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOption();
	uint8 NumWarnings = 0;

	// Never let a user save a custom game mode with a Preset Game Mode name
	if (IBSGameModeInterface::IsPresetGameMode(CustomGameModeNameText.ToString()))
	{
		NumWarnings++;
	}

	// Can only have empty CustomGameModeName if template is custom
	if (CustomGameModeNameText.IsEmptyOrWhitespace())
	{
		if (!IBSGameModeInterface::IsCustomGameMode(GameModeTemplateString))
		{
			NumWarnings++;
		}
	}

	if (CheckBoxOption_UseTemplate->CheckBox->IsChecked())
	{
		// Need to select a template if use template
		if (ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOptionCount() != 1)
		{
			NumWarnings++;
		}
		if (IBSGameModeInterface::IsPresetGameMode(GameModeTemplateString))
		{
			// Need to select a difficulty if using Preset
			if (ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOptionCount() != 1)
			{
				NumWarnings++;
			}
		}
	}

	CustomGameModeCategoryInfo.Update(0, NumWarnings);
}*/

/*void UCustomGameModeStartWidget::UpdateOptionsFromConfig()
{
	const bool bUpdatedTemplateVisibility = UpdateGameModeTemplateVisibility();
	const bool bUpdatedDifficultyVisibility = UpdateDifficultyVisibility();

	if (IBSGameModeInterface::IsPresetGameMode(ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption()))
	{
		if (ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOptionCount() == 0)
		{
			UpdateDifficultySelection(BSConfig->DefiningConfig.Difficulty);
		}
	}

	// Select a custom mode if conditions permit, mainly for saving and repopulating after saving
	if (CheckBoxOption_UseTemplate->CheckBox->IsChecked() &&
		IBSGameModeInterface::IsCustomGameMode(BSConfig->DefiningConfig.CustomGameModeName) &&
		ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOptionCount() == 0 && ComboBoxOption_GameModeDifficulty->
		ComboBox->GetSelectedOptionCount() == 0 && EditableTextBoxOption_CustomGameModeName->EditableTextBox->GetText().
		IsEmpty())
	{
		ComboBoxOption_GameModeTemplates->ComboBox->SetSelectedOption(BSConfig->DefiningConfig.CustomGameModeName);
	}

	if (bUpdatedTemplateVisibility || bUpdatedDifficultyVisibility)
	{
		UpdateBrushColors();
	}
}*/

void UCustomGameModeStartWidget::OnCheckStateChanged_UseTemplate(const bool bChecked)
{
	// Hide Templates and Difficulty if not checked
	if (!bChecked)
	{
		ComboBoxOption_GameModeTemplates->ComboBox->ClearSelection();
		ComboBoxOption_GameModeDifficulty->ComboBox->ClearSelection();
	}

	StartWidgetProperties.bUseTemplateChecked = bChecked;

	const bool bShouldShow = StartWidgetProperties.bIsCustom || StartWidgetProperties.bIsPreset || StartWidgetProperties
		.bUseTemplateChecked;
	ComboBoxOption_GameModeTemplates->SetVisibility(bShouldShow
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);

	OnStartWidgetPropertyChanged.Execute(StartWidgetProperties);

	//const FString SelectedDifficulty = ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOption();
	//const EGameModeDifficulty Difficulty = SelectedDifficulty.IsEmpty()
	//	? EGameModeDifficulty::None
	//	: GetEnumFromString<EGameModeDifficulty>(SelectedDifficulty);
	//RequestGameModeTemplateUpdate.Broadcast(ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption(),
	//	Difficulty);
}

void UCustomGameModeStartWidget::OnTextChanged_CustomGameModeName(const FText& Text)
{
	//UpdateAllOptionsValid();
	StartWidgetProperties.NewCustomGameModeName = Text.ToString();
	OnStartWidgetPropertyChanged.Execute(StartWidgetProperties);

	OnPropertyChanged.Execute({*PropertyMap.FindKey(EditableTextBoxOption_CustomGameModeName)});
	//OnCustomGameModeNameChanged.Broadcast();
}

void UCustomGameModeStartWidget::OnSelectionChanged_GameModeTemplates(const TArray<FString>& Selected,
	const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}
	StartWidgetProperties.GameModeName = Selected[0];
	OnStartWidgetPropertyChanged.Execute(StartWidgetProperties);

	//const FString SelectedDifficulty = ComboBoxOption_GameModeDifficulty->ComboBox->GetSelectedOption();
	//const EGameModeDifficulty Difficulty = SelectedDifficulty.IsEmpty()
	//	? EGameModeDifficulty::None
	//	: GetEnumFromString<EGameModeDifficulty>(SelectedDifficulty);

	//RequestGameModeTemplateUpdate.Broadcast(Selected[0], Difficulty);
}

void UCustomGameModeStartWidget::OnSelectionChanged_GameModeDifficulty(const TArray<FString>& Selected,
	const ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || Selected.Num() != 1)
	{
		return;
	}

	StartWidgetProperties.Difficulty = Selected[0];
	OnStartWidgetPropertyChanged.Execute(StartWidgetProperties);

	// Only request update for difficulty if Preset GameMode
	/*if (const FString GameModeTemplateString = ComboBoxOption_GameModeTemplates->ComboBox->GetSelectedOption();
		IBSGameModeInterface::IsPresetGameMode(GameModeTemplateString))
	{
		RequestGameModeTemplateUpdate.Broadcast(GameModeTemplateString,
			GetEnumFromString<EGameModeDifficulty>(Selected[0]));
	}*/
}
