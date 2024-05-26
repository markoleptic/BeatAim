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

FStartWidgetProperties UCustomGameModeStartWidget::StartWidgetProperties;
TDelegate<void(FStartWidgetProperties&)> UCustomGameModeStartWidget::OnStartWidgetPropertyChanged;

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

FStartWidgetProperties& UCustomGameModeStartWidget::GetProperties()
{
	return StartWidgetProperties;
}

void UCustomGameModeStartWidget::RefreshProperties() const
{
	EditableTextBoxOption_CustomGameModeName->EditableTextBox->SetText(
		FText::FromString(StartWidgetProperties.NewCustomGameModeName));

	CheckBoxOption_UseTemplate->CheckBox->SetIsChecked(StartWidgetProperties.bUseTemplateChecked);

	StartWidgetProperties.GameModeName.IsEmpty()
		? ComboBoxOption_GameModeTemplates->ComboBox->ClearSelection()
		: ComboBoxOption_GameModeTemplates->ComboBox->SetSelectedOption(StartWidgetProperties.GameModeName);

	StartWidgetProperties.Difficulty.IsEmpty()
		? ComboBoxOption_GameModeDifficulty->ComboBox->ClearSelection()
		: ComboBoxOption_GameModeDifficulty->ComboBox->SetSelectedOption(StartWidgetProperties.Difficulty);

	ComboBoxOption_GameModeTemplates->SetVisibility(
		StartWidgetProperties.bUseTemplateChecked && (StartWidgetProperties.bIsCustom || StartWidgetProperties.
			bIsPreset)
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);

	ComboBoxOption_GameModeDifficulty->SetVisibility(
		StartWidgetProperties.bUseTemplateChecked && StartWidgetProperties.bIsPreset
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);

	UpdateBrushColors();
}

void UCustomGameModeStartWidget::RefreshGameModes(const TArray<FBSConfig>& CustomGameModes) const
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

void UCustomGameModeStartWidget::OnCheckStateChanged_UseTemplate(const bool bChecked)
{
	StartWidgetProperties.bUseTemplateChecked = bChecked;
	OnStartWidgetPropertyChanged.Execute(StartWidgetProperties);
}

void UCustomGameModeStartWidget::OnTextChanged_CustomGameModeName(const FText& Text)
{
	StartWidgetProperties.NewCustomGameModeName = Text.ToString();
	OnStartWidgetPropertyChanged.Execute(StartWidgetProperties);
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
}
