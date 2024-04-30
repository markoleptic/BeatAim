// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.
// ReSharper disable CppMemberFunctionMayBeConst


#include "SubMenuWidgets/GameModesWidgets/GameModesWidget.h"
#include "CommonWidgetCarousel.h"
#include "SaveGamePlayerSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/VerticalBox.h"
#include "Components/Border.h"
#include "Components/HorizontalBoxSlot.h"
#include "OverlayWidgets/PopupWidgets/PopupMessageWidget.h"
#include "OverlayWidgets/PopupWidgets/AudioSelectWidget.h"
#include "OverlayWidgets/PopupWidgets/GameModeSharingWidget.h"
#include "WidgetComponents/SavedTextWidget.h"
#include "SubMenuWidgets/GameModesWidgets/CGMW_CreatorView.h"
#include "SubMenuWidgets/GameModesWidgets/CGMW_PropertyView.h"
#include "WidgetComponents/Buttons/MenuButton.h"
#include "Windows/WindowsPlatformApplicationMisc.h"
#include "SubMenuWidgets/GameModesWidgets/Components/CGMWC_Preview.h"
#include "WidgetComponents/MenuOptionWidgets/DefaultGameModeOptionWidget.h"


void UGameModesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BSConfig = MakeShareable(new FBSConfig());

	const TArray<FBSConfig> CustomGameModes = LoadCustomGameModesWrapper();

	InitDefaultGameModesWidgets();
	SetupButtons();
	BindAllDelegates();

	Carousel_DefaultCustom->OnCurrentPageIndexChanged.AddUniqueDynamic(this,
		&ThisClass::OnCarouselWidgetIndexChanged_DefaultCustom);
	Carousel_DefaultCustom->SetActiveWidgetIndex(0);
	CarouselNavBar_DefaultCustom->SetLinkedCarousel(Carousel_DefaultCustom);

	Carousel_CreatorProperty->OnCurrentPageIndexChanged.AddUniqueDynamic(this,
		&ThisClass::OnCarouselWidgetIndexChanged_CreatorProperty);
	Carousel_CreatorProperty->SetActiveWidgetIndex(0);
	CarouselNavBar_CreatorProperty->SetLinkedCarousel(Carousel_CreatorProperty);

	CustomGameModesWidget_Current = CustomGameModesWidget_CreatorView;

	// Initialize CustomGameModesWidgets
	CustomGameModesWidget_CreatorView->Init(BSConfig, GameModeDataAsset);
	CustomGameModesWidget_PropertyView->Init(BSConfig, GameModeDataAsset);
	CustomGameModesWidget_CreatorView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
	CustomGameModesWidget_PropertyView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);

	// Setup default custom game mode options to MultiBeat Normal
	InitCustomGameModesWidgetOptions(EBaseGameMode::MultiBeat, EGameModeDifficulty::Normal);

	Border_DifficultySelect->SetVisibility(ESlateVisibility::Collapsed);

	if (CustomGameModesWidget_CreatorView->Widget_Preview)
	{
		CustomGameModesWidget_CreatorView->Widget_Preview->ToggleGameModePreview(bIsMainMenuChild);
	}
}

void UGameModesWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	InitDefaultGameModesWidgets();
}

void UGameModesWidget::NativeDestruct()
{
	Super::NativeDestruct();
	BSConfig.Reset();
	BSConfig = nullptr;
}

void UGameModesWidget::SetupButtons()
{
	// Default Button Enabled States
	Button_StartFromPreset->SetIsEnabled(false);
	Button_CustomizeFromPreset->SetIsEnabled(false);

	Button_SaveCustom->SetIsEnabled(false);
	Button_StartFromCustom->SetIsEnabled(false);
	Button_RemoveSelectedCustom->SetIsEnabled(false);
	Button_ExportCustom->SetIsEnabled(false);
	Button_ImportCustom->SetIsEnabled(true);
	Button_ClearRLHistory->SetIsEnabled(false);
	Button_RemoveAllCustom->SetIsEnabled(!bCustomGameModesEmpty);

	CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create->SetIsEnabled(false);
	CustomGameModesWidget_CreatorView->Widget_Preview->Button_RefreshPreview->SetIsEnabled(true);
	CustomGameModesWidget_CreatorView->Widget_Preview->Button_Start->SetIsEnabled(true);

	// Difficulty buttons
	Button_NormalDifficulty->SetDefaults(static_cast<uint8>(EGameModeDifficulty::Normal), Button_HardDifficulty);
	Button_HardDifficulty->SetDefaults(static_cast<uint8>(EGameModeDifficulty::Hard), Button_DeathDifficulty);
	Button_DeathDifficulty->SetDefaults(static_cast<uint8>(EGameModeDifficulty::Death), Button_NormalDifficulty);
}

void UGameModesWidget::InitCustomGameModesWidgetOptions(const EBaseGameMode& BaseGameMode,
	const EGameModeDifficulty& Difficulty)
{
	FBSConfig DefaultConfig;
	if (FindPresetGameMode(BaseGameMode, Difficulty, GameModeDataAsset.Get(), DefaultConfig))
	{
		const FStartWidgetProperties StartWidgetUpdate = FStartWidgetProperties(DefaultConfig.DefiningConfig, true);
		CustomGameModesWidget_CreatorView->SetStartWidgetProperties(StartWidgetUpdate);
		CustomGameModesWidget_PropertyView->SetStartWidgetProperties(StartWidgetUpdate);
		PopulateGameModeOptions(DefaultConfig);
	}
}

void UGameModesWidget::InitDefaultGameModesWidgets()
{
	TArray<UDefaultGameModeOptionWidget*> Temp;
	Box_DefaultGameModesOptions->ClearChildren();
	DefaultGameModesParams.ValueSort([&](const FDefaultGameModeParams& Params, const FDefaultGameModeParams& Params2)
	{
		return Params < Params2;
	});
	for (TPair<EBaseGameMode, FDefaultGameModeParams>& Pair : DefaultGameModesParams)
	{
		UDefaultGameModeOptionWidget* Widget = CreateWidget<UDefaultGameModeOptionWidget>(this,
			DefaultGameModesWidgetClass);
		Widget->SetBaseGameMode(Pair.Key);
		Widget->SetDescriptionText(Pair.Value.GameModeName);
		Widget->SetAltDescriptionText(Pair.Value.AltDescriptionText);
		Widget->SetShowTooltipImage(false);
		Temp.Add(Widget);
		Box_DefaultGameModesOptions->AddChildToVerticalBox(Widget);
	}

	for (int i = 0; i < Temp.Num(); i++)
	{
		if (!Temp.IsValidIndex(i)) continue;
		const UDefaultGameModeOptionWidget* Widget = Temp[i];
		const int NextIndex = i == Temp.Num() - 1 ? 0 : i + 1;
		Widget->Button->SetDefaults(static_cast<uint8>(Widget->GetBaseGameMode()), Temp[NextIndex]->Button);
		Widget->Button->OnBSButtonPressed.AddUObject(this, &UGameModesWidget::OnButtonClicked_SelectedDefaultGameMode);
	}
	Box_DefaultGameModesOptions->UpdateBrushColors();
}

void UGameModesWidget::OnCarouselWidgetIndexChanged_DefaultCustom(UCommonWidgetCarousel* InCarousel,
	const int32 NewIndex)
{
	// Custom Game Modes && Creator View
	if (NewIndex == 1 && Carousel_CreatorProperty->GetActiveWidgetIndex() == 0)
	{
		RefreshGameModePreview();
	}
	else
	{
		StopGameModePreview();
	}
}

void UGameModesWidget::OnCarouselWidgetIndexChanged_CreatorProperty(UCommonWidgetCarousel* InCarousel,
	const int32 NewIndex)
{
	SynchronizeStartWidgets();

	if (NewIndex == 0)
	{
		CustomGameModesWidget_Current = CustomGameModesWidget_CreatorView;
		RefreshGameModePreview();
	}
	else
	{
		CustomGameModesWidget_Current = CustomGameModesWidget_PropertyView;
		StopGameModePreview();
	}
}

void UGameModesWidget::BindAllDelegates()
{
	// Custom Game Modes Buttons
	Button_SaveCustom->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_CustomGameModeButton);
	Button_StartFromCustom->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_CustomGameModeButton);
	Button_RemoveAllCustom->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_CustomGameModeButton);
	Button_RemoveSelectedCustom->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_CustomGameModeButton);
	Button_ImportCustom->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_CustomGameModeButton);
	Button_ExportCustom->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_CustomGameModeButton);
	Button_ClearRLHistory->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_CustomGameModeButton);

	// Default Game Modes Buttons
	Button_CustomizeFromPreset->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_DefaultGameMode);
	Button_StartFromPreset->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_DefaultGameMode);

	// Difficulty Buttons
	Button_NormalDifficulty->OnBSButtonPressed.AddUObject(this, &UGameModesWidget::OnButtonClicked_SelectedDifficulty);
	Button_HardDifficulty->OnBSButtonPressed.AddUObject(this, &UGameModesWidget::OnButtonClicked_SelectedDifficulty);
	Button_DeathDifficulty->OnBSButtonPressed.AddUObject(this, &UGameModesWidget::OnButtonClicked_SelectedDifficulty);

	// Custom Game Modes Widgets
	CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create->OnBSButtonPressed.AddUObject(this,
		&ThisClass::OnButtonClicked_CustomGameModeButton);
	CustomGameModesWidget_CreatorView->Widget_Preview->Button_RefreshPreview->OnBSButtonPressed.AddUObject(this,
		&ThisClass::OnButtonClicked_CustomGameModeButton);
	CustomGameModesWidget_CreatorView->Widget_Preview->Button_Start->OnBSButtonPressed.AddUObject(this,
		&ThisClass::OnButtonClicked_CustomGameModeButton);
	CustomGameModesWidget_CreatorView->RequestGameModeTemplateUpdate.AddUObject(this,
		&ThisClass::OnRequestGameModeTemplateUpdate);
	CustomGameModesWidget_PropertyView->RequestGameModeTemplateUpdate.AddUObject(this,
		&ThisClass::OnRequestGameModeTemplateUpdate);

	CustomGameModesWidget_CreatorView->RequestGameModePreviewUpdate.
	                                   AddUObject(this, &ThisClass::RefreshGameModePreview);

	CustomGameModesWidget_PropertyView->RequestButtonStateUpdate.AddUObject(this,
		&ThisClass::UpdateSaveStartButtonStates);
	CustomGameModesWidget_CreatorView->RequestButtonStateUpdate.AddUObject(this,
		&ThisClass::UpdateSaveStartButtonStates);
	CustomGameModesWidget_PropertyView->OnGameModeBreakingChange.AddUObject(this,
		&ThisClass::OnGameModeBreakingOptionPresentStateChanged);
	CustomGameModesWidget_CreatorView->OnGameModeBreakingChange.AddUObject(this,
		&ThisClass::OnGameModeBreakingOptionPresentStateChanged);
}

void UGameModesWidget::OnButtonClicked_SelectedDefaultGameMode(const UBSButton* Button)
{
	PresetSelection_PresetGameMode = static_cast<EBaseGameMode>(Button->GetEnumValue());

	Button_NormalDifficulty->SetInActive();
	Button_HardDifficulty->SetInActive();
	Button_DeathDifficulty->SetInActive();
	PresetSelection_Difficulty = EGameModeDifficulty::Normal;

	Border_DifficultySelect->SetVisibility(ESlateVisibility::Visible);
	Button_StartFromPreset->SetIsEnabled(false);
	Button_CustomizeFromPreset->SetIsEnabled(true);
}

void UGameModesWidget::OnButtonClicked_SelectedDifficulty(const UBSButton* Button)
{
	PresetSelection_Difficulty = static_cast<EGameModeDifficulty>(Button->GetEnumValue());
	Button_StartFromPreset->SetIsEnabled(true);
}

void UGameModesWidget::OnButtonClicked_DefaultGameMode(const UBSButton* Button)
{
	if (Button == Button_CustomizeFromPreset)
	{
		InitCustomGameModesWidgetOptions(PresetSelection_PresetGameMode, PresetSelection_Difficulty);
		Carousel_DefaultCustom->SetActiveWidgetIndex(1);
	}
	else if (Button == Button_StartFromPreset)
	{
		ShowAudioFormatSelect(true);
	}
}

void UGameModesWidget::OnButtonClicked_CustomGameModeButton(const UBSButton* Button)
{
	if (Button == Button_SaveCustom)
	{
		SynchronizeStartWidgets();
		OnButtonClicked_SaveCustom();
	}
	else if (Button == CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create)
	{
		SynchronizeStartWidgets();
		OnButtonClicked_SaveCustom();
	}
	else if (Button == Button_StartFromCustom || Button == CustomGameModesWidget_CreatorView->Widget_Preview->
		Button_Start)
	{
		SynchronizeStartWidgets();
		OnButtonClicked_StartFromCustom();
	}
	else if (Button == CustomGameModesWidget_CreatorView->Widget_Preview->Button_RefreshPreview)
	{
		RefreshGameModePreview();
	}
	else if (Button == Button_RemoveAllCustom)
	{
		OnButtonClicked_RemoveAllCustom();
	}
	else if (Button == Button_RemoveSelectedCustom)
	{
		OnButtonClicked_RemoveSelectedCustom();
	}
	else if (Button == Button_ImportCustom)
	{
		OnButtonClicked_ImportCustom();
	}
	else if (Button == Button_ExportCustom)
	{
		OnButtonClicked_ExportCustom();
	}
	else if (Button == Button_ClearRLHistory)
	{
		OnButtonClicked_ClearRLHistory();
	}
}

void UGameModesWidget::OnButtonClicked_ImportCustom()
{
	GameModeSharingWidget = CreateWidget<UGameModeSharingWidget>(this, GameModeSharingClass);
	TArray<UBSButton*> Buttons = GameModeSharingWidget->InitPopup(FText::FromString("Import Custom Game Mode"),
		FText::GetEmpty(), 2);

	Buttons[0]->SetButtonText(FText::FromString("Cancel"));
	Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		GameModeSharingWidget->FadeOut();
	});

	Buttons[1]->SetButtonText(FText::FromString("Import"));
	GameModeSharingWidget->SetImportButton(Buttons[1]);
	Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		const FString ImportString = GameModeSharingWidget->GetImportString();
		GameModeSharingWidget->FadeOut();

		TSharedPtr<FBSConfig> ImportedConfig = MakeShareable(new FBSConfig());
		FText OutFailureReason;
		if (!ImportCustomGameMode(ImportString, *ImportedConfig.ToSharedRef(), OutFailureReason))
		{
			if (OutFailureReason.EqualTo(FText::FromString("Existing")))
			{
				ShowConfirmOverwriteMessage_Import(ImportedConfig);
			}
			else
			{
				SetAndPlaySavedText(OutFailureReason);
			}
		}
		else
		{
			const FBSConfig Config = *ImportedConfig.ToSharedRef();
			SaveCustomGameMode(Config);
			const TArray<FBSConfig> CustomGameModes = LoadCustomGameModesWrapper();
			CustomGameModesWidget_CreatorView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
			CustomGameModesWidget_PropertyView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
			PopulateGameModeOptions(Config);
			SetAndPlaySavedText(FText::FromString("Successfully imported " + Config.DefiningConfig.CustomGameModeName));
		}
	});
	GameModeSharingWidget->AddToViewport();
	GameModeSharingWidget->FadeIn();
}

void UGameModesWidget::OnButtonClicked_ExportCustom()
{
	const FStartWidgetProperties StartWidgetProperties = CustomGameModesWidget_Current->GetStartWidgetProperties();
	CustomGameModesWidget_Current->SetNewCustomGameModeName("");

	if (!IsCustomGameMode(StartWidgetProperties.DefiningConfig.CustomGameModeName))
	{
		SetAndPlaySavedText(FText::FromString("The selected mode is not a custom game mode"));
		return;
	}

	const FBSConfig SelectedConfig = GetCustomGameModeOptions();
	const FString ExportString = ExportCustomGameMode(SelectedConfig);
	FPlatformApplicationMisc::ClipboardCopy(*ExportString);

	SetAndPlaySavedText(FText::FromString("Export String copied to clipboard"));
}

void UGameModesWidget::OnButtonClicked_ClearRLHistory()
{
	if (!IsCustomGameMode(BSConfig->DefiningConfig.CustomGameModeName))
	{
		return;
	}

	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
	TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(FText::FromString("Reset RL History"),
		FText::FromString(
			"Are you sure you want reset the Reinforcement Learning history of " + BSConfig->DefiningConfig.
			CustomGameModeName + "? This will set the all QTable values to zero, "
			"deleting any learning that has taken place."), 2);

	Buttons[0]->SetButtonText(FText::FromString("No"));
	Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
	});

	Buttons[1]->SetButtonText(FText::FromString("Yes"));
	Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
		const int32 NumReset = IBSPlayerScoreInterface::ResetQTable(BSConfig->DefiningConfig);
		if (NumReset >= 1)
		{
			SetAndPlaySavedText(
				FText::FromString("Cleared RL History for " + BSConfig->DefiningConfig.CustomGameModeName));
		}
		UpdateSaveStartButtonStates();
	});

	PopupMessageWidget->AddToViewport();
	PopupMessageWidget->FadeIn();
}

void UGameModesWidget::OnButtonClicked_SaveCustom()
{
	const FStartWidgetProperties StartWidgetProperties = CustomGameModesWidget_Current->GetStartWidgetProperties();
	const FString NewCustomGameModeName = StartWidgetProperties.NewCustomGameModeName;

	if ((StartWidgetProperties.DefiningConfig.CustomGameModeName.Equals(NewCustomGameModeName) || StartWidgetProperties.
		NewCustomGameModeName.IsEmpty()) && IsCurrentConfigIdenticalToSelectedCustom())
	{
		CustomGameModesWidget_PropertyView->SetNewCustomGameModeName("");
		CustomGameModesWidget_CreatorView->SetNewCustomGameModeName("");
		SetAndPlaySavedText(
			FText::FromString(StartWidgetProperties.DefiningConfig.CustomGameModeName + " already up to date"));
	}
	// Ask to override
	else if (DoesCustomGameModeExist())
	{
		PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
		TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(GetWidgetTextFromKey("GM_OverwritePopupTitle"),
			FText::GetEmpty(), 2);

		Buttons[0]->SetButtonText(GetWidgetTextFromKey("GM_OverwriteCancel"));
		Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
		});
		Buttons[1]->SetButtonText(GetWidgetTextFromKey("GM_OverwriteConfirm"));
		Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
			SaveCustomGameModeOptionsAndReselect();
		});
		PopupMessageWidget->AddToViewport();
		PopupMessageWidget->FadeIn();
	}
	// New custom game mode
	else
	{
		SaveCustomGameModeOptionsAndReselect();
	}
}

void UGameModesWidget::OnButtonClicked_StartFromCustom()
{
	// Handle Starting without saving
	const FStartWidgetProperties StartWidgetProperties = CustomGameModesWidget_Current->GetStartWidgetProperties();
	const bool bIsPresetMode = StartWidgetProperties.DefiningConfig.BaseGameMode != EBaseGameMode::None;
	const bool bIsCustomMode = IsCustomGameMode(StartWidgetProperties.DefiningConfig.CustomGameModeName);
	const bool bNewCustomGameModeNameEmpty = StartWidgetProperties.NewCustomGameModeName.IsEmpty();
	const bool bInvalidCustomGameModeName = IsPresetGameMode(StartWidgetProperties.NewCustomGameModeName);

	// Invalid custom game mode name
	if (bInvalidCustomGameModeName)
	{
		PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
		TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(FText::FromString("Start Without Saving"),
			FText::FromString(
				"The current custom game mode name is not valid. "
				"Do you want to start the game mode without saving it? Your scores will not be saved."), 2);

		Buttons[0]->SetButtonText(FText::FromString("No"));
		Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
		});

		Buttons[1]->SetButtonText(FText::FromString("Yes"));
		Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
			ShowAudioFormatSelect(false);
		});

		PopupMessageWidget->AddToViewport();
		PopupMessageWidget->FadeIn();
	}
	// No name for default mode or invalid name
	else if (bIsPresetMode && bNewCustomGameModeNameEmpty)
	{
		PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
		TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(FText::FromString("Play Preset Game Mode"),
			FText::FromString(
				"A preset game mode is selected with no custom game mode name. Do you want to start a preset game mode?"
				" \n\n If you meant to save this as a custom mode, click no and fill out the New Custom Game Mode field."),
			2);

		Buttons[0]->SetButtonText(FText::FromString("No"));
		Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
		});

		Buttons[1]->SetButtonText(FText::FromString("Yes"));
		Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
			PresetSelection_PresetGameMode = BSConfig->DefiningConfig.BaseGameMode;
			PresetSelection_Difficulty = BSConfig->DefiningConfig.Difficulty;
			ShowAudioFormatSelect(true);
		});

		PopupMessageWidget->AddToViewport();
		PopupMessageWidget->FadeIn();
	}
	// No game mode selected somehow
	else if (!bIsCustomMode && !bIsPresetMode && bNewCustomGameModeNameEmpty)
	{
		CustomGameModesWidget_CreatorView->SetNewCustomGameModeName("");
		CustomGameModesWidget_PropertyView->SetNewCustomGameModeName("");
		InitCustomGameModesWidgetOptions(EBaseGameMode::MultiBeat, EGameModeDifficulty::Normal);
	}
	// Bypass saving if identical to existing
	else if ((StartWidgetProperties.DefiningConfig.CustomGameModeName.
	                                Equals(StartWidgetProperties.NewCustomGameModeName) || StartWidgetProperties.
		NewCustomGameModeName.IsEmpty()) && IsCurrentConfigIdenticalToSelectedCustom())
	{
		CustomGameModesWidget_PropertyView->SetNewCustomGameModeName("");
		CustomGameModesWidget_CreatorView->SetNewCustomGameModeName("");
		SetAndPlaySavedText(FText::FromString(StartWidgetProperties.DefiningConfig.CustomGameModeName + " up to date"));
		ShowAudioFormatSelect(false);
	}
	// Ask to override
	else if (DoesCustomGameModeExist())
	{
		PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
		TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(GetWidgetTextFromKey("GM_OverwritePopupTitle"),
			FText::GetEmpty(), 2);

		Buttons[0]->SetButtonText(GetWidgetTextFromKey("GM_OverwriteCancel"));
		Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
		});

		Buttons[1]->SetButtonText(GetWidgetTextFromKey("GM_OverwriteConfirm"));
		Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
			if (SaveCustomGameModeOptionsAndReselect())
			{
				ShowAudioFormatSelect(false);
			}
		});

		PopupMessageWidget->AddToViewport();
		PopupMessageWidget->FadeIn();
	}
	// New custom game mode
	else
	{
		if (SaveCustomGameModeOptionsAndReselect())
		{
			ShowAudioFormatSelect(false);
		}
	}
}

void UGameModesWidget::OnButtonClicked_RemoveSelectedCustom()
{
	if (!IsCustomGameMode(BSConfig->DefiningConfig.CustomGameModeName))
	{
		return;
	}

	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
	TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(FText::FromString("Removal Confirmation"),
		FText::FromString("Are you sure you want remove " + BSConfig->DefiningConfig.CustomGameModeName + "?"), 3);

	Buttons[0]->SetButtonText(FText::FromString("Cancel"));
	Buttons[0]->SetWrapTextAt(350.f);
	Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
	});

	Buttons[1]->SetButtonText(FText::FromString("Only Remove Game Mode"));
	Buttons[1]->SetWrapTextAt(350.f);
	Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
		const FString RemovedGameModeName = BSConfig->DefiningConfig.CustomGameModeName;
		FBSConfig Found;
		if (FindCustomGameMode(RemovedGameModeName, Found))
		{
			if (RemoveCustomGameMode(Found) >= 1)
			{
				SetAndPlaySavedText(FText::FromString(RemovedGameModeName + " removed"));
				const TArray<FBSConfig> CustomGameModes = LoadCustomGameModesWrapper();
				CustomGameModesWidget_CreatorView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
				CustomGameModesWidget_PropertyView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
			}
		}

		CustomGameModesWidget_CreatorView->SetNewCustomGameModeName("");
		CustomGameModesWidget_PropertyView->SetNewCustomGameModeName("");
		InitCustomGameModesWidgetOptions(EBaseGameMode::MultiBeat, EGameModeDifficulty::Normal);
		UpdateSaveStartButtonStates();
	});

	Buttons[2]->SetButtonText(FText::FromString("Remove Game Mode and Scores"));
	Buttons[2]->SetWrapTextAt(350.f);
	Buttons[2]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
		const FString GameModeNameToRemove = BSConfig->DefiningConfig.CustomGameModeName;

		TSharedPtr<FAccessTokenResponse> AccessTokenResponse = MakeShareable(new FAccessTokenResponse());
		AccessTokenResponse->OnHttpResponseReceived.BindLambda([this, AccessTokenResponse, GameModeNameToRemove]
		{
			if (AccessTokenResponse->AccessToken.IsEmpty())
			{
				return;
			}
			TSharedPtr<FDeleteScoresResponse> DeleteScoresResponse = MakeShareable(new FDeleteScoresResponse());
			DeleteScoresResponse->OnHttpResponseReceived.BindLambda([this, DeleteScoresResponse, GameModeNameToRemove]
			{
				if (DeleteScoresResponse->OK)
				{
					FBSConfig Found;
					if (FindCustomGameMode(GameModeNameToRemove, Found))
					{
						if (RemoveCustomGameMode(Found) >= 1)
						{
							const FString String = GameModeNameToRemove + " removed and " + FString::FromInt(
								DeleteScoresResponse->NumRemoved) + " scores removed";
							SetAndPlaySavedText(FText::FromString(String));
							CustomGameModesWidget_CreatorView->SetNewCustomGameModeName("");
							CustomGameModesWidget_PropertyView->SetNewCustomGameModeName("");
							const TArray<FBSConfig> CustomGameModes = LoadCustomGameModesWrapper();
							CustomGameModesWidget_CreatorView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
							CustomGameModesWidget_PropertyView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
							InitCustomGameModesWidgetOptions(EBaseGameMode::MultiBeat, EGameModeDifficulty::Normal);
							UpdateSaveStartButtonStates();
						}
					}
					else
					{
						SetAndPlaySavedText(FText::FromString("Deleted from database, none found locally"));
					}
				}
				else
				{
					SetAndPlaySavedText(FText::FromString("Error connecting to database, delete aborted"));
				}
			});
			DeleteScores(GameModeNameToRemove, IBSPlayerSettingsInterface::LoadPlayerSettings().User.UserID,
				AccessTokenResponse->AccessToken, DeleteScoresResponse);
		});
		RequestAccessToken(IBSPlayerSettingsInterface::LoadPlayerSettings().User.RefreshCookie, AccessTokenResponse);
	});

	PopupMessageWidget->AddToViewport();
	PopupMessageWidget->FadeIn();
}

void UGameModesWidget::OnButtonClicked_RemoveAllCustom()
{
	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
	TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(FText::FromString("Removal Confirmation"),
		FText::FromString("Are you sure you want remove all custom game modes?"), 2);

	Buttons[0]->SetButtonText(FText::FromString("No"));
	Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
	});

	Buttons[1]->SetButtonText(FText::FromString("Yes"));
	Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
		const int32 NumRemoved = RemoveAllCustomGameModes();
		if (NumRemoved >= 1)
		{
			SetAndPlaySavedText(FText::FromString(FString::FromInt(NumRemoved) + " game modes removed"));
			const TArray<FBSConfig> CustomGameModes = LoadCustomGameModesWrapper();
			CustomGameModesWidget_CreatorView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
			CustomGameModesWidget_PropertyView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
		}

		CustomGameModesWidget_PropertyView->SetNewCustomGameModeName("");
		CustomGameModesWidget_CreatorView->SetNewCustomGameModeName("");
		UpdateSaveStartButtonStates();
	});

	PopupMessageWidget->AddToViewport();
	PopupMessageWidget->FadeIn();
}

void UGameModesWidget::PopulateGameModeOptions(const FBSConfig& InConfig)
{
	*BSConfig = InConfig;

	// Override Difficulty for Custom Modes to always be None
	if (IsCustomGameMode(BSConfig->DefiningConfig.CustomGameModeName))
	{
		BSConfig->DefiningConfig.Difficulty = EGameModeDifficulty::None;
		const FStartWidgetProperties Update = FStartWidgetProperties(BSConfig->DefiningConfig, true);
		CustomGameModesWidget_CreatorView->SetStartWidgetProperties(Update);
		CustomGameModesWidget_PropertyView->SetStartWidgetProperties(Update);
	}

	CustomGameModesWidget_CreatorView->UpdateOptionsFromConfig();
	CustomGameModesWidget_PropertyView->UpdateOptionsFromConfig();

	RefreshGameModePreview();
}

FBSConfig UGameModesWidget::GetCustomGameModeOptions() const
{
	FBSConfig ReturnStruct(*BSConfig.Get());

	if (!CustomGameModesWidget_Current->GetNewCustomGameModeName().IsEmpty())
	{
		ReturnStruct.DefiningConfig.CustomGameModeName = CustomGameModesWidget_Current->GetNewCustomGameModeName();
	}

	ReturnStruct.OnCreate_Custom();

	return ReturnStruct;
}

bool UGameModesWidget::SaveCustomGameModeOptionsAndReselect(const FText& SuccessMessage)
{
	const FBSConfig GameModeToSave = GetCustomGameModeOptions();

	// Last chance to fail
	if (IsPresetGameMode(GameModeToSave.DefiningConfig.CustomGameModeName) || GameModeToSave.DefiningConfig.GameModeType
		== EGameModeType::Preset)
	{
		CustomGameModesWidget_PropertyView->SetNewCustomGameModeName("");
		CustomGameModesWidget_CreatorView->SetNewCustomGameModeName("");
		const TArray SavedText = {
			FText::FromString("Error trying to save Custom Game Mode:"),
			FText::FromString(GameModeToSave.DefiningConfig.CustomGameModeName)
		};
		SetAndPlaySavedText(FText::Join(FText::FromString(" "), SavedText));
		return false;
	}

	SaveCustomGameMode(GameModeToSave);

	if (SuccessMessage.IsEmpty())
	{
		const TArray SavedText = {
			FText::FromString(GameModeToSave.DefiningConfig.CustomGameModeName),
			GetWidgetTextFromKey("GM_GameModeSavedText")
		};
		SetAndPlaySavedText(FText::Join(FText::FromString(" "), SavedText));
	}
	else
	{
		SetAndPlaySavedText(SuccessMessage);
	}

	const TArray<FBSConfig> CustomGameModes = LoadCustomGameModesWrapper();
	CustomGameModesWidget_CreatorView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
	CustomGameModesWidget_PropertyView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);

	PopulateGameModeOptions(GameModeToSave);

	return true;
}

void UGameModesWidget::UpdateSaveStartButtonStates()
{
	const FStartWidgetProperties StartWidgetProperties = CustomGameModesWidget_Current->GetStartWidgetProperties();
	const bool bAllCustomGameModeOptionsValid = CustomGameModesWidget_Current->GetAllNonStartChildWidgetOptionsValid();
	const bool bIsPresetMode = StartWidgetProperties.DefiningConfig.BaseGameMode != EBaseGameMode::None;
	const bool bIsCustomMode = IsCustomGameMode(StartWidgetProperties.DefiningConfig.CustomGameModeName);
	const bool bNewCustomGameModeNameEmpty = StartWidgetProperties.NewCustomGameModeName.IsEmpty();
	const bool bInvalidCustomGameModeName = IsPresetGameMode(StartWidgetProperties.NewCustomGameModeName);

	Button_RemoveAllCustom->SetIsEnabled(!bCustomGameModesEmpty);
	Button_RemoveSelectedCustom->SetIsEnabled(bIsCustomMode);

	if (bIsCustomMode && bNewCustomGameModeNameEmpty)
	{
		CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create->SetButtonText(FText::FromString("Save"));
	}
	else
	{
		CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create->SetButtonText(FText::FromString("Create"));
	}

	// Invalid options, any remaining buttons disabled
	if (!bAllCustomGameModeOptionsValid)
	{
		Button_SaveCustom->SetIsEnabled(false);
		CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create->SetIsEnabled(false);
		Button_StartFromCustom->SetIsEnabled(false);
		CustomGameModesWidget_CreatorView->Widget_Preview->Button_Start->SetIsEnabled(false);
		Button_ClearRLHistory->SetIsEnabled(false);
		Button_ExportCustom->SetIsEnabled(false);
		return;
	}

	/* At this point, all custom game mode options are valid. They can play the game mode but not be able to save it */
	Button_StartFromCustom->SetIsEnabled(true);
	CustomGameModesWidget_CreatorView->Widget_Preview->Button_Start->SetIsEnabled(true);
	Button_ClearRLHistory->SetIsEnabled(bIsCustomMode);
	Button_ExportCustom->SetIsEnabled(bIsCustomMode);

	// No name for default mode or invalid name
	if ((bIsPresetMode && bNewCustomGameModeNameEmpty) || bInvalidCustomGameModeName)
	{
		Button_SaveCustom->SetIsEnabled(false);
		CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create->SetIsEnabled(false);
	}
	// No game mode selected for whatever reason
	else if (!bIsCustomMode && !bIsPresetMode && bNewCustomGameModeNameEmpty)
	{
		Button_SaveCustom->SetIsEnabled(false);
		CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create->SetIsEnabled(false);
	}
	// Can save the custom game mode
	else
	{
		Button_SaveCustom->SetIsEnabled(true);
		CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create->SetIsEnabled(true);
	}
}

void UGameModesWidget::ShowAudioFormatSelect(const bool bStartFromDefaultGameMode)
{
	AudioSelectWidget = CreateWidget<UAudioSelectWidget>(this, AudioSelectClass);

	AudioSelectWidget->OnStartButtonClickedDelegate.BindLambda(
		[this, bStartFromDefaultGameMode](const FBS_AudioConfig& AudioConfig)
		{
			FGameModeTransitionState GameModeTransitionState;

			GameModeTransitionState.bSaveCurrentScores = false;
			GameModeTransitionState.TransitionState = bIsMainMenuChild
				? ETransitionState::StartFromMainMenu
				: ETransitionState::StartFromPostGameMenu;

			if (bStartFromDefaultGameMode)
			{
				FindPresetGameMode(PresetSelection_PresetGameMode, PresetSelection_Difficulty, GameModeDataAsset.Get(),
					GameModeTransitionState.BSConfig);
			}
			else
			{
				GameModeTransitionState.BSConfig = GetCustomGameModeOptions();
			}

			GameModeTransitionState.BSConfig.AudioConfig.SongTitle = AudioConfig.SongTitle;
			GameModeTransitionState.BSConfig.AudioConfig.SongLength = AudioConfig.SongLength;
			GameModeTransitionState.BSConfig.AudioConfig.InAudioDevice = AudioConfig.InAudioDevice;
			GameModeTransitionState.BSConfig.AudioConfig.OutAudioDevice = AudioConfig.OutAudioDevice;
			GameModeTransitionState.BSConfig.AudioConfig.SongPath = AudioConfig.SongPath;
			GameModeTransitionState.BSConfig.AudioConfig.bPlaybackAudio = AudioConfig.bPlaybackAudio;
			GameModeTransitionState.BSConfig.AudioConfig.AudioFormat = AudioConfig.AudioFormat;

			GameModeTransitionState.BSConfig.OnCreate();
			if (!bStartFromDefaultGameMode)
			{
				GameModeTransitionState.BSConfig.OnCreate_Custom();
			}

			OnGameModeStateChanged.Broadcast(GameModeTransitionState);
			AudioSelectWidget->FadeOut();
		});

	AudioSelectWidget->AddToViewport();
	AudioSelectWidget->FadeIn();
}

bool UGameModesWidget::IsCurrentConfigIdenticalToSelectedCustom()
{
	const FStartWidgetProperties StartWidgetProperties = CustomGameModesWidget_Current->GetStartWidgetProperties();

	// Bypass saving if identical to existing
	if (IsCustomGameMode(StartWidgetProperties.DefiningConfig.CustomGameModeName))
	{
		if (DoesCustomGameModeMatchConfig(StartWidgetProperties.DefiningConfig.CustomGameModeName, *BSConfig))
		{
			return true;
		}
	}

	return false;
}

bool UGameModesWidget::DoesCustomGameModeExist()
{
	const FStartWidgetProperties StartWidgetProperties = CustomGameModesWidget_Current->GetStartWidgetProperties();
	const FString NewCustomGameModeName = StartWidgetProperties.NewCustomGameModeName;

	// If NewCustomGameModeName is blank, ask to override
	if (IsCustomGameMode(StartWidgetProperties.DefiningConfig.CustomGameModeName) && NewCustomGameModeName.IsEmpty())
	{
		return true;
	}

	// If NewCustomGameModeName already exists as a saved custom game mode, ask to override
	if (IsCustomGameMode(NewCustomGameModeName))
	{
		return true;
	}

	return false;
}

void UGameModesWidget::ShowConfirmOverwriteMessage_Import(TSharedPtr<FBSConfig>& ImportedConfig)
{
	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
	TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(GetWidgetTextFromKey("GM_OverwritePopupTitle"),
		FText::GetEmpty(), 2);

	Buttons[0]->SetButtonText(GetWidgetTextFromKey("GM_OverwriteCancel"));
	Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
	});

	Buttons[1]->SetButtonText(GetWidgetTextFromKey("GM_OverwriteConfirm"));
	Buttons[1]->OnBSButtonPressed.AddLambda([this, ImportedConfig](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
		if (ImportedConfig.IsValid())
		{
			const FBSConfig Config = *ImportedConfig.ToSharedRef();
			SaveCustomGameMode(Config);
			const TArray<FBSConfig> CustomGameModes = LoadCustomGameModesWrapper();
			CustomGameModesWidget_CreatorView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
			CustomGameModesWidget_PropertyView->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
			PopulateGameModeOptions(Config);
			SetAndPlaySavedText(FText::FromString("Successfully imported " + Config.DefiningConfig.CustomGameModeName));
		}
	});

	PopupMessageWidget->AddToViewport();
	PopupMessageWidget->FadeIn();
}

void UGameModesWidget::OnRequestGameModeTemplateUpdate(const FString& InGameMode, const EGameModeDifficulty& Difficulty)
{
	FBSConfig Found;
	if (IsPresetGameMode(InGameMode))
	{
		if (Difficulty == EGameModeDifficulty::None)
		{
			FindPresetGameMode(InGameMode, EGameModeDifficulty::Normal, GameModeDataAsset.Get(), Found);
		}
		else
		{
			FindPresetGameMode(InGameMode, Difficulty, GameModeDataAsset.Get(), Found);
		}
	}
	else if (IsCustomGameMode(InGameMode))
	{
		FindCustomGameMode(InGameMode, Found);
	}
	PopulateGameModeOptions(Found);
	UpdateSaveStartButtonStates();
}

void UGameModesWidget::SynchronizeStartWidgets()
{
	UCGMW_Base* From;
	UCGMW_Base* To;
	if (CustomGameModesWidget_Current == CustomGameModesWidget_CreatorView)
	{
		From = Cast<UCGMW_Base>(CustomGameModesWidget_CreatorView.Get());
		To = Cast<UCGMW_Base>(CustomGameModesWidget_PropertyView.Get());
	}
	else
	{
		From = Cast<UCGMW_Base>(CustomGameModesWidget_PropertyView.Get());
		To = Cast<UCGMW_Base>(CustomGameModesWidget_CreatorView.Get());
	}

	const FStartWidgetProperties FromProperties = From->GetStartWidgetProperties();
	const FStartWidgetProperties ToProperties = To->GetStartWidgetProperties();

	if (FromProperties == ToProperties)
	{
		To->UpdateOptionsFromConfig();
		return;
	}

	To->SetStartWidgetProperties(FromProperties);
	To->UpdateOptionsFromConfig();
}

void UGameModesWidget::SetAndPlaySavedText(const FText& InText)
{
	if (CustomGameModesWidget_Current == CustomGameModesWidget_PropertyView)
	{
		SavedTextWidget_PropertyView->SetSavedText(InText);
		SavedTextWidget_PropertyView->PlayFadeInFadeOut();
	}
	else
	{
		CustomGameModesWidget_CreatorView->SavedTextWidget_CreatorView->SetSavedText(InText);
		CustomGameModesWidget_CreatorView->SavedTextWidget_CreatorView->PlayFadeInFadeOut();
	}
}

void UGameModesWidget::OnGameModeBreakingOptionPresentStateChanged(const bool bIsPresent)
{
	if (bIsPresent == bGameModeBreakingOptionPresent)
	{
		return;
	}

	CustomGameModesWidget_CreatorView->Widget_Preview->Button_RefreshPreview->SetIsEnabled(!bIsPresent);
	const FString From = bGameModeBreakingOptionPresent ? "True" : "False";
	const FString To = bIsPresent ? "True" : "False";
	UE_LOG(LogTemp, Display, TEXT("OnGameModeBreakingOption GameModesWidget: %s -> %s"), *From, *To);
	bGameModeBreakingOptionPresent = bIsPresent;
	OnGameModeBreakingChange.Broadcast(bGameModeBreakingOptionPresent);
}

void UGameModesWidget::RefreshGameModePreview()
{
	if (CustomGameModesWidget_Current == CustomGameModesWidget_CreatorView && RequestSimulateTargetManagerStateChange.
		IsBound())
	{
		RequestSimulateTargetManagerStateChange.Broadcast(true);
	}
}

TArray<FBSConfig> UGameModesWidget::LoadCustomGameModesWrapper()
{
	TArray<FBSConfig> CustomGameModes = LoadCustomGameModes();
	bCustomGameModesEmpty = CustomGameModes.IsEmpty();
	return MoveTemp(CustomGameModes);
}

void UGameModesWidget::StopGameModePreview()
{
	if (RequestSimulateTargetManagerStateChange.IsBound())
	{
		RequestSimulateTargetManagerStateChange.Broadcast(false);
	}
}
