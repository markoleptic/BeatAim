// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.
// ReSharper disable CppMemberFunctionMayBeConst


#include "BSPlayerScoreInterface.h"
#include "BSPlayerSettingsInterface.h"
#include "CommonWidgetCarousel.h"
#include "Blueprint/WidgetTree.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "Components/Border.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "GameModes/CreatorViewWidget.h"
#include "GameModes/CustomGameModePreviewWidget.h"
#include "GameModes/CustomGameModeStartWidget.h"
#include "GameModes/PropertyViewWidget.h"
#include "MenuOptions/DefaultGameModeSelectWidget.h"
#include "Menus/GameModeMenuWidget.h"
#include "Overlays/AudioSelectWidget.h"
#include "Overlays/GameModeSharingWidget.h"
#include "SaveGames/SaveGamePlayerSettings.h"
#include "Utilities/SavedTextWidget.h"
#include "Utilities/Buttons/MenuButton.h"
#include "Windows/WindowsPlatformApplicationMisc.h"


void UGameModeMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitDefaultGameModesWidgets();

	CustomGameModeImportSuccessFormattedText = FTextFormat(GetWidgetTextFromKey("GM_CustomGameModeImportSuccess"));
	CustomGameModeImportFailureFormattedText = FTextFormat(GetWidgetTextFromKey("GM_CustomGameModeImportFailure"));
	ResetAIHistoryContentFormattedText = FTextFormat(GetWidgetTextFromKey("GM_ResetAIHistoryContent"));
	ResetAIHistorySuccessFormattedText = FTextFormat(GetWidgetTextFromKey("GM_ResetAIHistorySuccess"));
	CustomGameModeUpToDateFormattedText = FTextFormat(GetWidgetTextFromKey("GM_CustomGameModeUpToDate"));
	CustomGameModeUpdatedFormattedText = FTextFormat(GetWidgetTextFromKey("GM_CustomGameModeUpdated"));
	CustomGameModeRemovalContentFormattedText = FTextFormat(GetWidgetTextFromKey("GM_CustomGameModeRemovalContent"));
	CustomGameModeRemovalSuccessFormattedText = FTextFormat(GetWidgetTextFromKey("GM_CustomGameModeRemovalSuccess"));
	CustomGameModeRemoveAllSuccessFormattedText = FTextFormat(
		GetWidgetTextFromKey("GM_CustomGameModeRemoveAllSuccess"));
	CustomGameModeSaveSuccessFormattedText = FTextFormat(GetWidgetTextFromKey("GM_CustomGameModeSaveSuccess"));
	CustomGameModeSaveFailureFormattedText = FTextFormat(GetWidgetTextFromKey("GM_CustomGameModeSaveFailure"));
	CustomGameModeImportInvalidStringFormattedText = FTextFormat(
		GetWidgetTextFromKey("GM_CustomGameModeImportInvalidString"));


	GameModeValidator = NewObject<UBSGameModeValidator>();
	for (const FValidationPropertyPtr& Property : GameModeValidator->GetValidationProperties())
	{
		for (const FValidationCheckPtr& ValidationCheck : Property->ValidationChecks)
		{
			if (!ValidationCheck->ValidationCheckData.IsEmpty())
			{
				if (!ValidationCheck->ValidationCheckData.StringTableKey.IsEmpty() && !ValidationCheck->
					ValidationCheckData.DynamicStringTableKey.IsEmpty())
				{
					ValidationCheck->ValidationCheckData.FallbackTooltipText = GetTooltipTextFromKey(
						ValidationCheck->ValidationCheckData.StringTableKey);
					ValidationCheck->ValidationCheckData.TooltipText = GetTooltipTextFromKey(
						ValidationCheck->ValidationCheckData.DynamicStringTableKey);
				}
				else
				{
					ValidationCheck->ValidationCheckData.TooltipText = GetTooltipTextFromKey(
						ValidationCheck->ValidationCheckData.StringTableKey);
				}
			}
		}
	}
	BSConfig = MakeShareable(new FBSConfig());

	ForceRefreshProperties = {
		GameModeValidator->FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, FloorDistance)),
		GameModeValidator->FindBSConfigProperty(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDistributionPolicy))
	};

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
	Button_NormalDifficulty->OnBSButtonPressed.AddUObject(this,
		&UGameModeMenuWidget::OnButtonClicked_SelectedDifficulty);
	Button_HardDifficulty->OnBSButtonPressed.AddUObject(this, &UGameModeMenuWidget::OnButtonClicked_SelectedDifficulty);
	Button_DeathDifficulty->OnBSButtonPressed.
	                        AddUObject(this, &UGameModeMenuWidget::OnButtonClicked_SelectedDifficulty);

	// Custom Game Modes Widgets
	CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create->OnBSButtonPressed.AddUObject(this,
		&ThisClass::OnButtonClicked_CustomGameModeButton);
	CustomGameModesWidget_CreatorView->Widget_Preview->Button_RefreshPreview->OnBSButtonPressed.AddUObject(this,
		&ThisClass::OnButtonClicked_CustomGameModeButton);
	CustomGameModesWidget_CreatorView->Widget_Preview->Button_Start->OnBSButtonPressed.AddUObject(this,
		&ThisClass::OnButtonClicked_CustomGameModeButton);
	CustomGameModesWidget_CreatorView->OnPropertyChanged.BindUObject(this, &ThisClass::HandlePropertyChanged);
	CustomGameModesWidget_PropertyView->OnPropertyChanged.BindUObject(this, &ThisClass::HandlePropertyChanged);
	UCustomGameModeStartWidget::OnStartWidgetPropertyChanged.BindUObject(this,
		&ThisClass::HandleStartWidgetPropertyChanged);

	Carousel_DefaultCustom->OnCurrentPageIndexChanged.AddUniqueDynamic(this,
		&ThisClass::OnCarouselWidgetIndexChanged_DefaultCustom);
	Carousel_DefaultCustom->SetActiveWidgetIndex(0);
	CarouselNavBar_DefaultCustom->SetLinkedCarousel(Carousel_DefaultCustom);

	Carousel_CreatorProperty->OnCurrentPageIndexChanged.AddUniqueDynamic(this,
		&ThisClass::OnCarouselWidgetIndexChanged_CreatorProperty);
	Carousel_CreatorProperty->SetActiveWidgetIndex(0);
	CarouselNavBar_CreatorProperty->SetLinkedCarousel(Carousel_CreatorProperty);

	CurrentCustomGameModesWidget = CustomGameModesWidget_CreatorView;
	NotCurrentCustomGameModesWidget = CustomGameModesWidget_PropertyView;

	FBSConfig DefaultConfig;
	FindPresetGameMode(EBaseGameMode::MultiBeat, EGameModeDifficulty::Normal, GameModeDataAsset.Get(), DefaultConfig);
	SetBSConfig(DefaultConfig);

	RefreshGameModes();

	CurrentCustomGameModesWidget->Init(BSConfig);
	NotCurrentCustomGameModesWidget->Init(BSConfig);

	PopulateGameModeOptions(DefaultConfig);

	Border_DifficultySelect->SetVisibility(ESlateVisibility::Collapsed);

	if (CustomGameModesWidget_CreatorView->Widget_Preview)
	{
		CustomGameModesWidget_CreatorView->Widget_Preview->ToggleGameModePreview(bIsMainMenuChild);
	}
}

void UGameModeMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	InitDefaultGameModesWidgets();
}

void UGameModeMenuWidget::NativeDestruct()
{
	Super::NativeDestruct();
	BSConfig.Reset();
	BSConfig = nullptr;
}

void UGameModeMenuWidget::InitDefaultGameModesWidgets()
{
	TArray<UDefaultGameModeSelectWidget*> Temp;
	Box_DefaultGameModesOptions->ClearChildren();
	DefaultGameModesParams.ValueSort([&](const FDefaultGameModeParams& Params, const FDefaultGameModeParams& Params2)
	{
		return Params < Params2;
	});
	for (TPair<EBaseGameMode, FDefaultGameModeParams>& Pair : DefaultGameModesParams)
	{
		UDefaultGameModeSelectWidget* Widget = CreateWidget<UDefaultGameModeSelectWidget>(this,
			DefaultGameModesWidgetClass);
		Widget->SetBaseGameMode(Pair.Key);
		Widget->SetDescriptionText(Pair.Value.GameModeName);
		Widget->SetAltDescriptionText(Pair.Value.AltDescriptionText);
		Widget->SetShowTooltipIcon(false);
		Temp.Add(Widget);
		Box_DefaultGameModesOptions->AddChildToVerticalBox(Widget);
	}

	for (int i = 0; i < Temp.Num(); i++)
	{
		if (!Temp.IsValidIndex(i))
		{
			continue;
		}
		const UDefaultGameModeSelectWidget* Widget = Temp[i];
		const int NextIndex = i == Temp.Num() - 1 ? 0 : i + 1;
		Widget->Button->SetDefaults(static_cast<uint8>(Widget->GetBaseGameMode()), Temp[NextIndex]->Button);
		Widget->Button->OnBSButtonPressed.AddUObject(this,
			&UGameModeMenuWidget::OnButtonClicked_SelectedDefaultGameMode);
	}
	Box_DefaultGameModesOptions->UpdateBrushColors();
}

void UGameModeMenuWidget::OnCarouselWidgetIndexChanged_DefaultCustom(UCommonWidgetCarousel* InCarousel,
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

void UGameModeMenuWidget::OnCarouselWidgetIndexChanged_CreatorProperty(UCommonWidgetCarousel* InCarousel,
	const int32 NewIndex)
{
	SynchronizeStartWidgets();

	if (NewIndex == 0)
	{
		CurrentCustomGameModesWidget = CustomGameModesWidget_CreatorView;
		NotCurrentCustomGameModesWidget = CustomGameModesWidget_PropertyView;
		RefreshGameModePreview();
	}
	else
	{
		CurrentCustomGameModesWidget = CustomGameModesWidget_PropertyView;
		NotCurrentCustomGameModesWidget = CustomGameModesWidget_CreatorView;
		StopGameModePreview();
	}
}

void UGameModeMenuWidget::OnButtonClicked_SelectedDefaultGameMode(const UBSButton* Button)
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

void UGameModeMenuWidget::OnButtonClicked_SelectedDifficulty(const UBSButton* Button)
{
	PresetSelection_Difficulty = static_cast<EGameModeDifficulty>(Button->GetEnumValue());
	Button_StartFromPreset->SetIsEnabled(true);
}

void UGameModeMenuWidget::OnButtonClicked_DefaultGameMode(const UBSButton* Button)
{
	if (Button == Button_CustomizeFromPreset)
	{
		if (FBSConfig DefaultConfig; FindPresetGameMode(PresetSelection_PresetGameMode, PresetSelection_Difficulty,
			GameModeDataAsset.Get(), DefaultConfig))
		{
			PopulateGameModeOptions(DefaultConfig);
		}
		Carousel_DefaultCustom->SetActiveWidgetIndex(1);
	}
	else if (Button == Button_StartFromPreset)
	{
		ShowAudioFormatSelect(true);
	}
}

void UGameModeMenuWidget::OnButtonClicked_CustomGameModeButton(const UBSButton* Button)
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

void UGameModeMenuWidget::OnButtonClicked_ImportCustom()
{
	GameModeSharingWidget = CreateWidget<UGameModeSharingWidget>(this, GameModeSharingClass);
	TArray<UBSButton*> Buttons = GameModeSharingWidget->InitPopup(GetWidgetTextFromKey("GM_ImportCustomGameMode"),
		FText::GetEmpty(), 2);

	Buttons[0]->SetButtonText(GetWidgetTextFromKey("G_Cancel"));
	Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		GameModeSharingWidget->FadeOut();
	});

	Buttons[1]->SetButtonText(GetWidgetTextFromKey("G_Import"));
	GameModeSharingWidget->SetImportButton(Buttons[1]);
	Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		const FString ImportString = GameModeSharingWidget->GetImportString();
		GameModeSharingWidget->FadeOut();

		TSharedPtr<FBSConfig> ImportedConfig = MakeShareable(new FBSConfig());
		FText OutFailureReason;
		switch (ImportCustomGameMode(ImportString, *ImportedConfig.ToSharedRef(), OutFailureReason))
		{
		case ECustomGameModeImportResult::Success:
			{
				const FBSConfig Config = *ImportedConfig.ToSharedRef();
				SaveCustomGameMode(Config);
				RefreshGameModes();
				PopulateGameModeOptions(Config);
				SetAndPlaySavedText(FText::Format(CustomGameModeImportSuccessFormattedText, {
					{TEXT("GameMode"), FText::FromString(Config.DefiningConfig.CustomGameModeName)}
				}));
			}
			break;
		case ECustomGameModeImportResult::InvalidImportString:
			SetAndPlaySavedText(FText::Format(CustomGameModeImportFailureFormattedText, {
				{
					TEXT("FailureReason"),
					FText::Format(CustomGameModeImportInvalidStringFormattedText, {
						{TEXT("DecodeFailureReason"), OutFailureReason}
					})
				}
			}));

			break;
		case ECustomGameModeImportResult::DefaultGameMode:
			SetAndPlaySavedText(FText::Format(CustomGameModeImportFailureFormattedText, {
				{TEXT("FailureReason"), GetWidgetTextFromKey("GM_CustomGameModeImportDefaultGameMode")}
			}));
			break;
		case ECustomGameModeImportResult::EmptyCustomGameModeName:
			SetAndPlaySavedText(FText::Format(CustomGameModeImportFailureFormattedText, {
				{TEXT("FailureReason"), GetWidgetTextFromKey("GM_CustomGameModeImportEmptyCustom")}
			}));
			break;
		case ECustomGameModeImportResult::Existing:
			ShowConfirmOverwriteMessage_Import(ImportedConfig);
			break;
		}
	});
	GameModeSharingWidget->AddToViewport();
	GameModeSharingWidget->FadeIn();
}

void UGameModeMenuWidget::OnButtonClicked_ExportCustom()
{
	const UCustomGameModeStartWidget* CurrentStartWidget = GetCurrentStartWidget();
	FStartWidgetProperties& Properties = CurrentStartWidget->GetProperties();
	Properties.NewCustomGameModeName.Empty();
	CurrentStartWidget->RefreshProperties();

	if (IsCustomGameMode(Properties.GameModeName))
	{
		const FBSConfig SelectedConfig = GetCustomGameModeOptions();
		const FString ExportString = ExportCustomGameMode(SelectedConfig);
		FPlatformApplicationMisc::ClipboardCopy(*ExportString);
		SetAndPlaySavedText(GetWidgetTextFromKey("GM_CustomGameModeExportSuccess"));
	}
	else
	{
		SetAndPlaySavedText(GetWidgetTextFromKey("GM_CustomGameModeExportFailure"));
	}
}

void UGameModeMenuWidget::OnButtonClicked_ClearRLHistory()
{
	if (!IsCustomGameMode(BSConfig->DefiningConfig.CustomGameModeName))
	{
		return;
	}

	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
	TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(GetWidgetTextFromKey("GM_ResetAIHistoryTitle"),
		FText::Format(ResetAIHistoryContentFormattedText, {
			{TEXT("GameMode"), FText::FromString(BSConfig->DefiningConfig.CustomGameModeName)}
		}), 2);

	Buttons[0]->SetButtonText(GetWidgetTextFromKey("G_No"));
	Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
	});

	Buttons[1]->SetButtonText(GetWidgetTextFromKey("G_Yes"));
	Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
		if (IBSPlayerScoreInterface::ResetQTable(BSConfig->DefiningConfig))
		{
			SetAndPlaySavedText(FText::Format(ResetAIHistorySuccessFormattedText, {
				{TEXT("GameMode"), FText::FromString(BSConfig->DefiningConfig.CustomGameModeName)}
			}));
		}
		UpdateSaveStartButtonStates();
	});

	PopupMessageWidget->AddToViewport();
	PopupMessageWidget->FadeIn();
}

void UGameModeMenuWidget::OnButtonClicked_SaveCustom()
{
	FStartWidgetProperties& Properties = GetCurrentStartWidget()->GetProperties();

	// CustomGameModeName selected in combo box OR game mode name in NewCustomGameModeName already up-to-date
	if ((Properties.GameModeName.Equals(Properties.NewCustomGameModeName, ESearchCase::CaseSensitive) || Properties.
		NewCustomGameModeName.IsEmpty()) && IsCurrentConfigIdenticalToSelectedCustom())
	{
		Properties.NewCustomGameModeName.Empty();
		GetCurrentStartWidget()->RefreshProperties();
		GetNotCurrentStartWidget()->RefreshProperties();
		SetAndPlaySavedText(FText::Format(CustomGameModeUpToDateFormattedText, {
			{TEXT("GameMode"), FText::FromString(BSConfig->DefiningConfig.CustomGameModeName)}
		}));
		UpdateSaveStartButtonStates();
	}
	// Ask to override if game mode name in NewCustomGameModeName is already a game mode
	else if (!Properties.NewCustomGameModeName.IsEmpty() && IsCustomGameMode(Properties.NewCustomGameModeName))
	{
		PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
		TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(GetWidgetTextFromKey("GM_OverwritePopupTitle"),
			FText::GetEmpty(), 2);

		Buttons[0]->SetButtonText(GetWidgetTextFromKey("G_Cancel"));
		Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
		});
		Buttons[1]->SetButtonText(GetWidgetTextFromKey("G_Confirm"));
		Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
			SaveCustomGameModeOptionsAndReselect();
		});
		PopupMessageWidget->AddToViewport();
		PopupMessageWidget->FadeIn();
	}
	else
	{
		SaveCustomGameModeOptionsAndReselect();
	}
}

void UGameModeMenuWidget::OnButtonClicked_StartFromCustom()
{
	// Handle Starting without saving
	FStartWidgetProperties& Properties = GetCurrentStartWidget()->GetProperties();
	const bool bIsPresetMode = IsPresetGameMode(Properties.GameModeName);
	const bool bIsCustomMode = IsCustomGameMode(Properties.GameModeName);
	const bool bNewCustomGameModeNameEmpty = Properties.NewCustomGameModeName.IsEmpty();
	const bool bInvalidCustomGameModeName = IsPresetGameMode(Properties.NewCustomGameModeName);

	// Invalid custom game mode name
	if (bInvalidCustomGameModeName)
	{
		PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
		TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(GetWidgetTextFromKey("GM_StartWithoutSavingTitle"),
			GetWidgetTextFromKey("GM_StartWithoutSavingContent"), 2);

		Buttons[0]->SetButtonText(GetWidgetTextFromKey("G_No"));
		Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
		});

		Buttons[1]->SetButtonText(GetWidgetTextFromKey("G_Yes"));
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
		TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(
			GetWidgetTextFromKey("GM_StartInvalidNoCustomGameModeNameTitle"),
			GetWidgetTextFromKey("GM_StartInvalidNoCustomGameModeNameContent"), 2);

		Buttons[0]->SetButtonText(GetWidgetTextFromKey("G_No"));
		Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
		});

		Buttons[1]->SetButtonText(GetWidgetTextFromKey("G_Yes"));
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
		if (FBSConfig DefaultConfig; FindPresetGameMode(EBaseGameMode::MultiBeat, EGameModeDifficulty::Normal,
			GameModeDataAsset.Get(), DefaultConfig))
		{
			PopulateGameModeOptions(DefaultConfig);
		}
	}
	// CustomGameModeName selected in combo box OR game mode name in NewCustomGameModeName already up-to-date
	else if ((Properties.GameModeName.Equals(Properties.NewCustomGameModeName, ESearchCase::CaseSensitive) || Properties
		.NewCustomGameModeName.IsEmpty()) && IsCurrentConfigIdenticalToSelectedCustom())
	{
		Properties.NewCustomGameModeName.Empty();
		GetNotCurrentStartWidget()->RefreshProperties();
		GetCurrentStartWidget()->RefreshProperties();
		SetAndPlaySavedText(FText::Format(CustomGameModeUpToDateFormattedText, {
			{TEXT("GameMode"), FText::FromString(BSConfig->DefiningConfig.CustomGameModeName)}
		}));
		ShowAudioFormatSelect(false);
	}
	// Ask to override if game mode name in NewCustomGameModeName is already a game mode
	else if (!Properties.NewCustomGameModeName.IsEmpty() && IsCustomGameMode(Properties.NewCustomGameModeName))
	{
		PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
		TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(GetWidgetTextFromKey("GM_OverwritePopupTitle"),
			FText::GetEmpty(), 2);

		Buttons[0]->SetButtonText(GetWidgetTextFromKey("G_Cancel"));
		Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
		});

		Buttons[1]->SetButtonText(GetWidgetTextFromKey("G_Confirm"));
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

void UGameModeMenuWidget::OnButtonClicked_RemoveSelectedCustom()
{
	if (!IsCustomGameMode(BSConfig->DefiningConfig.CustomGameModeName))
	{
		return;
	}

	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
	TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(GetWidgetTextFromKey("GM_CustomGameModeRemovalTitle"),
		FText::Format(CustomGameModeRemovalContentFormattedText, {
			{TEXT("GameMode"), FText::FromString(BSConfig->DefiningConfig.CustomGameModeName)}
		}), 3);

	Buttons[0]->SetButtonText(GetWidgetTextFromKey("G_Cancel"));
	Buttons[0]->SetWrapTextAt(350.f);
	Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
	});

	Buttons[1]->SetButtonText(GetWidgetTextFromKey("GM_OnlyRemoveGameMode"));
	Buttons[1]->SetWrapTextAt(350.f);
	Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
		const FString RemovedGameModeName = BSConfig->DefiningConfig.CustomGameModeName;
		if (FBSConfig Found; FindCustomGameMode(RemovedGameModeName, Found))
		{
			if (RemoveCustomGameMode(Found))
			{
				SetAndPlaySavedText(FText::Format(CustomGameModeRemovalSuccessFormattedText, {
					{TEXT("GameMode"), FText::FromString(RemovedGameModeName)}
				}));
				RefreshGameModes();
			}
		}
		if (FBSConfig DefaultConfig; FindPresetGameMode(EBaseGameMode::MultiBeat, EGameModeDifficulty::Normal,
			GameModeDataAsset.Get(), DefaultConfig))
		{
			PopulateGameModeOptions(DefaultConfig);
		}
		UpdateSaveStartButtonStates();
	});

	Buttons[2]->SetButtonText(GetWidgetTextFromKey("GM_RemoveGameModeAndScores"));
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
					if (FBSConfig Found; FindCustomGameMode(GameModeNameToRemove, Found))
					{
						if (RemoveCustomGameMode(Found))
						{
							FFormatNamedArguments Args;
							Args.Add(TEXT("GameMode"), FText::FromString(GameModeNameToRemove));
							Args.Add(TEXT("NumRemoved"), FText::AsNumber(DeleteScoresResponse->NumRemoved));
							SetAndPlaySavedText(FText::Format(CustomGameModeRemovalSuccessFormattedText, Args));
							RefreshGameModes();

							if (FBSConfig DefaultConfig; FindPresetGameMode(EBaseGameMode::MultiBeat,
								EGameModeDifficulty::Normal, GameModeDataAsset.Get(), DefaultConfig))
							{
								PopulateGameModeOptions(DefaultConfig);
							}
							UpdateSaveStartButtonStates();
						}
					}
					else
					{
						SetAndPlaySavedText(GetWidgetTextFromKey("GM_RemoveGameModeDatabaseOnlySuccess"));
					}
				}
				else
				{
					SetAndPlaySavedText(GetWidgetTextFromKey("GM_RemoveGameModeDatabaseFailure"));
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

void UGameModeMenuWidget::OnButtonClicked_RemoveAllCustom()
{
	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
	TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(GetWidgetTextFromKey("GM_CustomGameModeRemovalTitle"),
		GetWidgetTextFromKey("GM_CustomGameModeRemoveAllContent"), 2);

	Buttons[0]->SetButtonText(GetWidgetTextFromKey("G_No"));
	Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
	});

	Buttons[1]->SetButtonText(GetWidgetTextFromKey("G_Yes"));
	Buttons[1]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
		if (const int32 NumRemoved = RemoveAllCustomGameModes(); NumRemoved >= 1)
		{
			FFormatNamedArguments Args;
			Args.Add(TEXT("NumRemoved"), FText::AsNumber(NumRemoved));
			SetAndPlaySavedText(FText::Format(CustomGameModeRemoveAllSuccessFormattedText, Args));
			RefreshGameModes();
		}

		if (FBSConfig DefaultConfig; FindPresetGameMode(EBaseGameMode::MultiBeat, EGameModeDifficulty::Normal,
			GameModeDataAsset.Get(), DefaultConfig))
		{
			PopulateGameModeOptions(DefaultConfig);
		}
		UpdateSaveStartButtonStates();
	});

	PopupMessageWidget->AddToViewport();
	PopupMessageWidget->FadeIn();
}

void UGameModeMenuWidget::PopulateGameModeOptions(const FBSConfig& InConfig)
{
	SetBSConfig(InConfig);
	FStartWidgetProperties& StartWidgetProperties = UCustomGameModeStartWidget::GetProperties();
	StartWidgetProperties.bIsCustom = IsCustomGameMode(BSConfig->DefiningConfig.CustomGameModeName);
	StartWidgetProperties.bIsPreset = !StartWidgetProperties.bIsCustom;
	StartWidgetProperties.bUseTemplateChecked = true;
	StartWidgetProperties.GameModeName = StartWidgetProperties.bIsCustom
		? BSConfig->DefiningConfig.CustomGameModeName
		: GetStringFromEnum(BSConfig->DefiningConfig.BaseGameMode);
	StartWidgetProperties.Difficulty = StartWidgetProperties.bIsCustom
		? ""
		: GetStringFromEnum(InConfig.DefiningConfig.Difficulty);
	StartWidgetProperties.NewCustomGameModeName.Empty();

	GetCurrentStartWidget()->RefreshProperties();
	GetNotCurrentStartWidget()->RefreshProperties();

	CurrentCustomGameModesWidget->UpdateOptionsFromConfig();
	NotCurrentCustomGameModesWidget->UpdateOptionsFromConfig();

	HandleValidation(GameModeValidator->Validate(BSConfig));

	RefreshGameModePreview();
}

FBSConfig UGameModeMenuWidget::GetCustomGameModeOptions() const
{
	FBSConfig ReturnStruct(*BSConfig.Get());
	const FStartWidgetProperties& StartWidgetProperties = GetCurrentStartWidget()->GetProperties();
	if (!StartWidgetProperties.NewCustomGameModeName.IsEmpty())
	{
		ReturnStruct.DefiningConfig.CustomGameModeName = StartWidgetProperties.NewCustomGameModeName;
	}

	ReturnStruct.OnCreate_Custom();

	return ReturnStruct;
}

bool UGameModeMenuWidget::SaveCustomGameModeOptionsAndReselect()
{
	const FBSConfig GameModeToSave = GetCustomGameModeOptions();

	// Last chance to fail
	if (IsPresetGameMode(GameModeToSave.DefiningConfig.CustomGameModeName) || GameModeToSave.DefiningConfig.GameModeType
		== EGameModeType::Preset)
	{
		SetAndPlaySavedText(FText::Format(CustomGameModeSaveFailureFormattedText, {
			{TEXT("GameMode"), FText::FromString(GameModeToSave.DefiningConfig.CustomGameModeName)}
		}));
		return false;
	}

	SaveCustomGameMode(GameModeToSave);

	SetAndPlaySavedText(FText::Format(CustomGameModeSaveSuccessFormattedText, {
		{TEXT("GameMode"), FText::FromString(GameModeToSave.DefiningConfig.CustomGameModeName)}
	}));

	RefreshGameModes();
	PopulateGameModeOptions(GameModeToSave);
	UpdateSaveStartButtonStates();
	return true;
}

void UGameModeMenuWidget::UpdateSaveStartButtonStates()
{
	const FStartWidgetProperties& Properties = UCustomGameModeStartWidget::GetProperties();
	const bool bAllCustomGameModeOptionsValid = !bGameModeBreakingOptionPresent;
	const bool bIsPresetMode = IsPresetGameMode(Properties.GameModeName);
	const bool bIsCustomMode = IsCustomGameMode(Properties.GameModeName);
	const bool bNewCustomGameModeNameEmpty = Properties.NewCustomGameModeName.IsEmpty();
	const bool bInvalidCustomGameModeName = IsPresetGameMode(Properties.NewCustomGameModeName);

	Button_RemoveAllCustom->SetIsEnabled(!bCustomGameModesEmpty);
	Button_RemoveSelectedCustom->SetIsEnabled(bIsCustomMode);

	if (bIsCustomMode && bNewCustomGameModeNameEmpty)
	{
		CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create->SetButtonText(GetWidgetTextFromKey("G_Save"));
	}
	else
	{
		CustomGameModesWidget_CreatorView->Widget_Preview->Button_Create->SetButtonText(
			GetWidgetTextFromKey("G_Create"));
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

	// At this point, all custom game mode options are valid. They can play the game mode but not be able to save it
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

void UGameModeMenuWidget::ShowAudioFormatSelect(const bool bStartFromDefaultGameMode)
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

bool UGameModeMenuWidget::IsCurrentConfigIdenticalToSelectedCustom()
{
	// Bypass saving if identical to existing
	if (const FStartWidgetProperties& Properties = GetCurrentStartWidget()->GetProperties();
		DoesCustomGameModeMatchConfig(Properties.GameModeName, *BSConfig))
	{
		return true;
	}
	return false;
}

void UGameModeMenuWidget::ShowConfirmOverwriteMessage_Import(TSharedPtr<FBSConfig>& ImportedConfig)
{
	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
	TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(GetWidgetTextFromKey("GM_OverwritePopupTitle"),
		FText::GetEmpty(), 2);

	Buttons[0]->SetButtonText(GetWidgetTextFromKey("G_Cancel"));
	Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
	});

	Buttons[1]->SetButtonText(GetWidgetTextFromKey("G_Confirm"));
	Buttons[1]->OnBSButtonPressed.AddLambda([this, ImportedConfig](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
		if (ImportedConfig.IsValid())
		{
			const FBSConfig Config = *ImportedConfig.ToSharedRef();
			SaveCustomGameMode(Config);
			RefreshGameModes();
			PopulateGameModeOptions(Config);
			SetAndPlaySavedText(FText::Format(CustomGameModeImportSuccessFormattedText, {
				{TEXT("GameMode"), FText::FromString(Config.DefiningConfig.CustomGameModeName)}
			}));
		}
	});

	PopupMessageWidget->AddToViewport();
	PopupMessageWidget->FadeIn();
}

void UGameModeMenuWidget::SynchronizeStartWidgets()
{
	GetCurrentStartWidget()->RefreshProperties();
	GetNotCurrentStartWidget()->RefreshProperties();
}

void UGameModeMenuWidget::SetAndPlaySavedText(const FText& InText)
{
	if (CurrentCustomGameModesWidget == CustomGameModesWidget_PropertyView)
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

void UGameModeMenuWidget::OnGameModeBreakingOptionPresentStateChanged(const bool bIsPresent)
{
	if (bIsPresent == bGameModeBreakingOptionPresent)
	{
		return;
	}

	CustomGameModesWidget_CreatorView->Widget_Preview->Button_RefreshPreview->SetIsEnabled(!bIsPresent);

#if !UE_BUILD_SHIPPING
	const FString From = bGameModeBreakingOptionPresent ? "True" : "False";
	const FString To = bIsPresent ? "True" : "False";
	UE_LOG(LogTemp, Display, TEXT("OnGameModeBreakingOption GameModesWidget: %s -> %s"), *From, *To);
#endif

	bGameModeBreakingOptionPresent = bIsPresent;
	OnGameModeBreakingChange.Broadcast(bGameModeBreakingOptionPresent);
}

void UGameModeMenuWidget::RefreshGameModePreview()
{
	if (CurrentCustomGameModesWidget == CustomGameModesWidget_CreatorView && RequestSimulateTargetManagerStateChange.
		IsBound())
	{
		RequestSimulateTargetManagerStateChange.Broadcast(true);
	}
}

void UGameModeMenuWidget::RefreshGameModes()
{
	const TArray<FBSConfig> CustomGameModes = LoadCustomGameModes();
	bCustomGameModesEmpty = CustomGameModes.IsEmpty();
	GetCurrentStartWidget()->RefreshGameModes(CustomGameModes);
	GetNotCurrentStartWidget()->RefreshGameModes(CustomGameModes);
}

void UGameModeMenuWidget::SetBSConfig(const FBSConfig& InConfig)
{
	*BSConfig = InConfig;
}

void UGameModeMenuWidget::HandlePropertyChanged(const TSet<uint32>& Properties)
{
	HandleValidation(GameModeValidator->Validate(BSConfig, Properties));

	if (!bGameModeBreakingOptionPresent && !Properties.Intersect(ForceRefreshProperties).IsEmpty())
	{
		RefreshGameModePreview();
	}
}

void UGameModeMenuWidget::HandleValidation(const FValidationResult& Result)
{
	FValidationCheckResultSet Succeeded = Result.GetSucceeded();
	FValidationCheckResultSet Failed = Result.GetFailed();
	TMap<EGameModeCategory, TPair<int32, int32>> NotificationMap;
	bool ContainsWarnings = false;
	for (const auto& Widget : CurrentCustomGameModesWidget->GetCustomGameModeCategoryWidgets())
	{
		Widget->HandlePropertyValidation(Succeeded);
		Widget->HandlePropertyValidation(Failed);
		const int32 NumCautions = Widget->GetNumberOfDynamicTooltipIcons(ETooltipIconType::Caution);
		const int32 NumWarnings = Widget->GetNumberOfDynamicTooltipIcons(ETooltipIconType::Warning);
		ContainsWarnings = ContainsWarnings || NumWarnings > 0;
		NotificationMap.Add(Widget->GetGameModeCategory(), {NumCautions, NumWarnings});
	}

	OnGameModeBreakingOptionPresentStateChanged(ContainsWarnings);
	UpdateSaveStartButtonStates();
	CustomGameModesWidget_CreatorView->UpdateNotificationIcons(NotificationMap);
}

void UGameModeMenuWidget::HandleStartWidgetPropertyChanged(FStartWidgetProperties& Properties)
{
	const FStartWidgetProperties Copy = Properties;
	const FBS_DefiningConfig DefiningConfigCopy = BSConfig->DefiningConfig;

	if (Properties.bUseTemplateChecked)
	{
		if (IsPresetGameMode(Properties.GameModeName))
		{
			Properties.bIsPreset = true;
			Properties.bIsCustom = false;
			if (Properties.Difficulty.IsEmpty())
			{
				Properties.Difficulty = GetStringFromEnum(EGameModeDifficulty::Normal);
			}
			BSConfig->DefiningConfig.BaseGameMode = GetEnumFromString<EBaseGameMode>(Properties.GameModeName);
			BSConfig->DefiningConfig.GameModeType = EGameModeType::Preset;
			BSConfig->DefiningConfig.Difficulty = GetEnumFromString<EGameModeDifficulty>(Properties.Difficulty);
			BSConfig->DefiningConfig.CustomGameModeName.Empty();
		}
		else if (IsCustomGameMode(Properties.GameModeName))
		{
			Properties.bIsPreset = false;
			Properties.bIsCustom = true;
			Properties.Difficulty.Empty();

			//BSConfig->DefiningConfig.BaseGameMode = EBaseGameMode::None;
			BSConfig->DefiningConfig.GameModeType = EGameModeType::Custom;
			BSConfig->DefiningConfig.Difficulty = EGameModeDifficulty::None;
			BSConfig->DefiningConfig.CustomGameModeName = Properties.GameModeName;
		}
		else // Default to a Preset
		{
			Properties.bIsPreset = true;
			Properties.bIsCustom = false;
			Properties.GameModeName = GetStringFromEnum(EBaseGameMode::MultiBeat);
			Properties.Difficulty = GetStringFromEnum(EGameModeDifficulty::Normal);

			BSConfig->DefiningConfig.BaseGameMode = EBaseGameMode::MultiBeat;
			BSConfig->DefiningConfig.GameModeType = EGameModeType::Preset;
			BSConfig->DefiningConfig.Difficulty = EGameModeDifficulty::Normal;
			BSConfig->DefiningConfig.CustomGameModeName.Empty();
		}
	}
	else
	{
		//BSConfig->DefiningConfig.BaseGameMode = EBaseGameMode::None;
		BSConfig->DefiningConfig.GameModeType = EGameModeType::Custom;
		BSConfig->DefiningConfig.Difficulty = EGameModeDifficulty::None;
		Properties.GameModeName.Empty();
		Properties.Difficulty.Empty();
	}

	if (Copy != Properties)
	{
		GetCurrentStartWidget()->RefreshProperties();
	}

	if (Properties.bGameModeNameChanged || Properties.bDifficultyChanged || DefiningConfigCopy != BSConfig->
		DefiningConfig)
	{
		if (FBSConfig Found; (Properties.bIsPreset && FindPresetGameMode(BSConfig->DefiningConfig.BaseGameMode,
			BSConfig->DefiningConfig.Difficulty, GameModeDataAsset.Get(),
			Found)) || Properties.bIsCustom && FindCustomGameMode(Properties.GameModeName, Found))
		{
			PopulateGameModeOptions(Found);
		}
		Properties.bGameModeNameChanged = false;
		Properties.bDifficultyChanged = false;
	}

	UpdateSaveStartButtonStates();
}

UCustomGameModeStartWidget* UGameModeMenuWidget::GetCurrentStartWidget() const
{
	return CurrentCustomGameModesWidget->GetStartWidget();
}

UCustomGameModeStartWidget* UGameModeMenuWidget::GetNotCurrentStartWidget() const
{
	return NotCurrentCustomGameModesWidget->GetStartWidget();
}

void UGameModeMenuWidget::StopGameModePreview()
{
	if (RequestSimulateTargetManagerStateChange.IsBound())
	{
		RequestSimulateTargetManagerStateChange.Broadcast(false);
	}
}
