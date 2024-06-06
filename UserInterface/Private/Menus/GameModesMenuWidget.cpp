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
#include "Utilities/BSWidgetInterface.h"
#include "Utilities/SavedTextWidget.h"
#include "Utilities/Buttons/MenuButton.h"
#include "Windows/WindowsPlatformApplicationMisc.h"


void UGameModeMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitDefaultGameModesWidgets();

	GameModeValidator = NewObject<UBSGameModeValidator>();
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

	//CustomGameModesWidget_PropertyView->RequestButtonStateUpdate.AddUObject(this,
	//	&ThisClass::UpdateSaveStartButtonStates);
	//CustomGameModesWidget_CreatorView->RequestButtonStateUpdate.AddUObject(this,
	//	&ThisClass::UpdateSaveStartButtonStates);
	//CustomGameModesWidget_PropertyView->OnGameModeBreakingChange.AddUObject(this,
	//	&ThisClass::OnGameModeBreakingOptionPresentStateChanged);
	//CustomGameModesWidget_CreatorView->OnGameModeBreakingChange.AddUObject(this,
	//	&ThisClass::OnGameModeBreakingOptionPresentStateChanged);

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
			RefreshGameModes();
			PopulateGameModeOptions(Config);
			SetAndPlaySavedText(FText::FromString("Successfully imported " + Config.DefiningConfig.CustomGameModeName));
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
		SetAndPlaySavedText(FText::FromString("Export String copied to clipboard"));
	}
	else
	{
		SetAndPlaySavedText(FText::FromString("The selected mode is not a custom game mode"));
	}
}

void UGameModeMenuWidget::OnButtonClicked_ClearRLHistory()
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

void UGameModeMenuWidget::OnButtonClicked_SaveCustom()
{
	FStartWidgetProperties& Properties = GetCurrentStartWidget()->GetProperties();
	if ((BSConfig->DefiningConfig.CustomGameModeName.Equals(Properties.NewCustomGameModeName) || Properties.
		NewCustomGameModeName.IsEmpty()) && IsCurrentConfigIdenticalToSelectedCustom())
	{
		Properties.NewCustomGameModeName.Empty();
		GetCurrentStartWidget()->RefreshProperties();
		GetNotCurrentStartWidget()->RefreshProperties();
		SetAndPlaySavedText(FText::FromString(BSConfig->DefiningConfig.CustomGameModeName + " already up to date"));
		UpdateSaveStartButtonStates();
	}
	// Ask to override
	else if (DoesCustomGameModeExist())
	{
		PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
		TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(
			IBSWidgetInterface::GetWidgetTextFromKey("GM_OverwritePopupTitle"), FText::GetEmpty(), 2);

		Buttons[0]->SetButtonText(IBSWidgetInterface::GetWidgetTextFromKey("GM_OverwriteCancel"));
		Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
		});
		Buttons[1]->SetButtonText(IBSWidgetInterface::GetWidgetTextFromKey("GM_OverwriteConfirm"));
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

void UGameModeMenuWidget::OnButtonClicked_StartFromCustom()
{
	// Handle Starting without saving
	FStartWidgetProperties& StartWidgetProperties = GetCurrentStartWidget()->GetProperties();
	const bool bIsPresetMode = IsPresetGameMode(StartWidgetProperties.GameModeName);
	const bool bIsCustomMode = IsCustomGameMode(StartWidgetProperties.GameModeName);
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
		if (FBSConfig DefaultConfig; FindPresetGameMode(EBaseGameMode::MultiBeat, EGameModeDifficulty::Normal,
			GameModeDataAsset.Get(), DefaultConfig))
		{
			PopulateGameModeOptions(DefaultConfig);
		}
	}
	// Bypass saving if identical to existing
	else if ((StartWidgetProperties.GameModeName.Equals(StartWidgetProperties.NewCustomGameModeName,
			ESearchCase::CaseSensitive) || StartWidgetProperties.NewCustomGameModeName.IsEmpty()) &&
		IsCurrentConfigIdenticalToSelectedCustom())
	{
		StartWidgetProperties.NewCustomGameModeName.Empty();
		GetNotCurrentStartWidget()->RefreshProperties();
		GetCurrentStartWidget()->RefreshProperties();
		SetAndPlaySavedText(FText::FromString(BSConfig->DefiningConfig.CustomGameModeName + " up to date"));
		ShowAudioFormatSelect(false);
	}
	// Ask to override
	else if (DoesCustomGameModeExist())
	{
		PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
		TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(
			IBSWidgetInterface::GetWidgetTextFromKey("GM_OverwritePopupTitle"), FText::GetEmpty(), 2);

		Buttons[0]->SetButtonText(IBSWidgetInterface::GetWidgetTextFromKey("GM_OverwriteCancel"));
		Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
		});

		Buttons[1]->SetButtonText(IBSWidgetInterface::GetWidgetTextFromKey("GM_OverwriteConfirm"));
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

void UGameModeMenuWidget::OnButtonClicked_RemoveAllCustom()
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
		: IBSWidgetInterface::GetStringFromEnum(BSConfig->DefiningConfig.BaseGameMode);
	StartWidgetProperties.Difficulty = StartWidgetProperties.bIsCustom
		? ""
		: IBSWidgetInterface::GetStringFromEnum(InConfig.DefiningConfig.Difficulty);
	StartWidgetProperties.NewCustomGameModeName.Empty();

	GetCurrentStartWidget()->RefreshProperties();
	GetNotCurrentStartWidget()->RefreshProperties();

	CurrentCustomGameModesWidget->UpdateOptionsFromConfig();
	NotCurrentCustomGameModesWidget->UpdateOptionsFromConfig();

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

bool UGameModeMenuWidget::SaveCustomGameModeOptionsAndReselect(const FText& SuccessMessage)
{
	const FBSConfig GameModeToSave = GetCustomGameModeOptions();

	// Last chance to fail
	if (IsPresetGameMode(GameModeToSave.DefiningConfig.CustomGameModeName) || GameModeToSave.DefiningConfig.GameModeType
		== EGameModeType::Preset)
	{
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
			IBSWidgetInterface::GetWidgetTextFromKey("GM_GameModeSavedText")
		};
		SetAndPlaySavedText(FText::Join(FText::FromString(" "), SavedText));
	}
	else
	{
		SetAndPlaySavedText(SuccessMessage);
	}

	RefreshGameModes();
	PopulateGameModeOptions(GameModeToSave);
	UpdateSaveStartButtonStates();
	return true;
}

void UGameModeMenuWidget::UpdateSaveStartButtonStates()
{
	//const bool bAllCustomGameModeOptionsValid = CurrentCustomGameModesWidget->GetAllNonStartChildWidgetOptionsValid();
	const FStartWidgetProperties& Properties = UCustomGameModeStartWidget::GetProperties();
	const bool bAllCustomGameModeOptionsValid = true;
	const bool bIsPresetMode = IsPresetGameMode(Properties.GameModeName);
	const bool bIsCustomMode = IsCustomGameMode(Properties.GameModeName);
	const bool bNewCustomGameModeNameEmpty = Properties.NewCustomGameModeName.IsEmpty();
	const bool bInvalidCustomGameModeName = IsPresetGameMode(Properties.NewCustomGameModeName);

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

bool UGameModeMenuWidget::DoesCustomGameModeExist()
{
	const FStartWidgetProperties StartWidgetProperties = GetCurrentStartWidget()->GetProperties();
	const FString NewCustomGameModeName = StartWidgetProperties.NewCustomGameModeName;

	// If NewCustomGameModeName is blank, ask to override
	if (IsCustomGameMode(StartWidgetProperties.GameModeName) && NewCustomGameModeName.IsEmpty())
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

void UGameModeMenuWidget::ShowConfirmOverwriteMessage_Import(TSharedPtr<FBSConfig>& ImportedConfig)
{
	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
	TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(
		IBSWidgetInterface::GetWidgetTextFromKey("GM_OverwritePopupTitle"), FText::GetEmpty(), 2);

	Buttons[0]->SetButtonText(IBSWidgetInterface::GetWidgetTextFromKey("GM_OverwriteCancel"));
	Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
	});

	Buttons[1]->SetButtonText(IBSWidgetInterface::GetWidgetTextFromKey("GM_OverwriteConfirm"));
	Buttons[1]->OnBSButtonPressed.AddLambda([this, ImportedConfig](const UBSButton* /*Button*/)
	{
		PopupMessageWidget->FadeOut();
		if (ImportedConfig.IsValid())
		{
			const FBSConfig Config = *ImportedConfig.ToSharedRef();
			SaveCustomGameMode(Config);
			RefreshGameModes();
			PopulateGameModeOptions(Config);
			SetAndPlaySavedText(FText::FromString("Successfully imported " + Config.DefiningConfig.CustomGameModeName));
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
	const FString From = bGameModeBreakingOptionPresent ? "True" : "False";
	const FString To = bIsPresent ? "True" : "False";
	UE_LOG(LogTemp, Display, TEXT("OnGameModeBreakingOption GameModesWidget: %s -> %s"), *From, *To);
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

void UGameModeMenuWidget::HandlePropertyChanged(const TSet<FPropertyHash>& Properties)
{
	for (const FPropertyHash& Prop : Properties)
	{
		if (const FValidationPropertyPtr& PropertyPtr = GameModeValidator->FindValidationProperty(Prop))
		{
			UE_LOG(LogTemp, Display, TEXT("Property Changed: %s"), *PropertyPtr->PropertyName);
		}
	}

	if (!Properties.Intersect(ForceRefreshProperties).IsEmpty())
	{
		RefreshGameModePreview();
	}

	const FValidationResult Result = GameModeValidator->Validate(BSConfig, Properties);
	TSet<FValidationCheckResult, FValidationCheckKeyFuncs> Succeeded = Result.GetSucceeded();
	TSet<FValidationCheckResult, FValidationCheckKeyFuncs> Failed = Result.GetFailed();

	auto UpdateTooltipText = [&](TSet<FValidationCheckResult, FValidationCheckKeyFuncs>& CheckResults)
	{
		for (auto& Elem : CheckResults)
		{
			for (auto& [Property, Data] : Elem.PropertyData)
			{
				Data.TooltipText = Data.DynamicStringTableKey.IsEmpty()
					? IBSWidgetInterface::GetTooltipTextFromKey(Data.StringTableKey)
					: IBSWidgetInterface::GetTooltipTextFromKey(Data.DynamicStringTableKey);
			}
		}
	};

	UpdateTooltipText(Succeeded);
	UpdateTooltipText(Failed);

	for (auto Widget : CurrentCustomGameModesWidget->GetCustomGameModeCategoryWidgets())
	{
		Widget->HandlePropertyValidation(Succeeded);
	}
	for (auto Widget : CurrentCustomGameModesWidget->GetCustomGameModeCategoryWidgets())
	{
		Widget->HandlePropertyValidation(Failed);
	}
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
				Properties.Difficulty = IBSWidgetInterface::GetStringFromEnum(EGameModeDifficulty::Normal);
			}
			BSConfig->DefiningConfig.BaseGameMode = IBSWidgetInterface::GetEnumFromString<EBaseGameMode>(
				Properties.GameModeName);
			BSConfig->DefiningConfig.GameModeType = EGameModeType::Preset;
			BSConfig->DefiningConfig.Difficulty = IBSWidgetInterface::GetEnumFromString<EGameModeDifficulty>(
				Properties.Difficulty);
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
			Properties.GameModeName = IBSWidgetInterface::GetStringFromEnum(EBaseGameMode::MultiBeat);
			Properties.Difficulty = IBSWidgetInterface::GetStringFromEnum(EGameModeDifficulty::Normal);

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
