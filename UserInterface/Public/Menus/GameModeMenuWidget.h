// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSGameModeInterface.h"
#include "HttpRequestInterface.h"
#include "Blueprint/UserWidget.h"
#include "GameModes/CustomGameModeStartWidget.h"
#include "Utilities/BSCarouselNavBar.h"
#include "Utilities/BSWidgetInterface.h"
#include "Utilities/GameModeTransitionState.h"
#include "GameModeMenuWidget.generated.h"

struct FStartWidgetProperties;
class UBSGameModeValidator;
class UCommonWidgetCarousel;
class UBSVerticalBox;
class UDefaultGameModeSelectWidget;
class UCustomGameModeWidget;
class UPropertyViewWidget;
class UCreatorViewWidget;
class UGameModeSharingWidget;
class UTooltipImage;
class UAudioSelectWidget;
class UHorizontalBox;
class USavedTextWidget;
class UPopupMessageWidget;
class UVerticalBox;
class UBorder;
class UEditableTextBox;
class UComboBoxString;
class USlider;
class UCheckBox;
class UMenuButton;
class UBSButton;

USTRUCT(BlueprintType)
struct FDefaultGameModeParams
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText GameModeName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AltDescriptionText;

	FDefaultGameModeParams()
	{
		GameModeName = FText();
		AltDescriptionText = FText();
	}

	FORCEINLINE bool operator==(const FDefaultGameModeParams& Other) const
	{
		return GameModeName.EqualTo(Other.GameModeName);
	}

	FORCEINLINE bool operator<(const FDefaultGameModeParams& Other) const
	{
		return GameModeName.ToString() < Other.GameModeName.ToString();
	}
};

DECLARE_MULTICAST_DELEGATE_OneParam(FRequestSimulateTargetManagerStateChange, const bool bSimulate)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameModeBreakingChange, const bool bIsGameModeBreaking);

/** The base widget for selecting or customizing a game mode. The custom portion is split into multiple
 *  SettingsCategoryWidgets. Includes a default game modes section. */
UCLASS()
class USERINTERFACE_API UGameModeMenuWidget : public UUserWidget, public IBSWidgetInterface,
                                              public IHttpRequestInterface, public IBSGameModeInterface
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;
	virtual UTooltipWidget* ConstructTooltipWidget() override { return nullptr; }
	void HandlePropertyChanged(const TSet<const FProperty*>& Properties);
	void HandleStartWidgetPropertyChanged(FStartWidgetProperties& Properties);
	UCustomGameModeStartWidget* GetCurrentStartWidget() const;
	UCustomGameModeStartWidget* GetNotCurrentStartWidget() const;

public:
	/** Returns BSConfig. */
	TSharedPtr<FBSConfig> GetBSConfig() const { return BSConfig; }

	/** Ends the game mode preview. */
	void StopGameModePreview();

	/** Whether this widget is MainMenuWidget child or a PostGameMenu child. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn="true"))
	bool bIsMainMenuChild;

	/** Executes when the user is exiting the GameModesWidget, broadcast to GameInstance to handle transition. */
	TMulticastDelegate<void(const FGameModeTransitionState& TransitionState)> OnGameModeStateChanged;

	/** Broadcast false when any non-defining config option is false. Broadcasts true only if all are true.
	 *  Only Broadcasts if different from the previous. */
	FOnGameModeBreakingChange OnGameModeBreakingChange;

	/** Called to request the start or stop of a game mode preview. */
	FRequestSimulateTargetManagerStateChange RequestSimulateTargetManagerStateChange;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GameModesWidget|CustomGameModes")
	TObjectPtr<UBSGameModeDataAsset> GameModeDataAsset;
	UPROPERTY(EditDefaultsOnly, Category = "GameModesWidget|Classes|AudioSelect")
	TSubclassOf<UPopupMessageWidget> PopupMessageClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameModesWidget|Classes|AudioSelect")
	TSubclassOf<UAudioSelectWidget> AudioSelectClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameModesWidget|Classes|Tooltip")
	TSubclassOf<UTooltipImage> WarningEMarkClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameModesWidget|Classes|Custom Game Modes")
	TSubclassOf<UGameModeSharingWidget> GameModeSharingClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameModesWidget|Classes|Tooltip")
	TSubclassOf<UTooltipWidget> TooltipWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameModesWidget|DefaultGameModes")
	TSubclassOf<UDefaultGameModeSelectWidget> DefaultGameModesWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameModesWidget|DefaultGameModes", meta=(ForceInlineRow))
	TMap<EBaseGameMode, FDefaultGameModeParams> DefaultGameModesParams;

	UPROPERTY()
	TObjectPtr<UTooltipImage> TooltipWarningImage_EnableAI;
	UPROPERTY()
	TObjectPtr<UPopupMessageWidget> PopupMessageWidget;
	UPROPERTY()
	TObjectPtr<UAudioSelectWidget> AudioSelectWidget;
	UPROPERTY()
	TObjectPtr<UGameModeSharingWidget> GameModeSharingWidget;
	UPROPERTY()
	TObjectPtr<UBSGameModeValidator> GameModeValidator;

public:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCreatorViewWidget> CustomGameModesWidget_CreatorView;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UPropertyViewWidget> CustomGameModesWidget_PropertyView;
	UPROPERTY()
	TObjectPtr<UCustomGameModeWidget> CurrentCustomGameModesWidget;
	UPROPERTY()
	TObjectPtr<UCustomGameModeWidget> NotCurrentCustomGameModesWidget;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USavedTextWidget* SavedTextWidget_PropertyView;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_DefaultGameModes;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_CustomGameModes;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UVerticalBox* Box_PropertyView;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSVerticalBox* Box_DefaultGameModesOptions;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_SaveCustom;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_StartFromCustom;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_RemoveSelectedCustom;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_RemoveAllCustom;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_CustomizeFromPreset;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_StartFromPreset;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_ImportCustom;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_ExportCustom;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_ClearRLHistory;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBorder* Border_DifficultySelect;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_NormalDifficulty;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_HardDifficulty;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_DeathDifficulty;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonWidgetCarousel* Carousel_DefaultCustom;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSCarouselNavBar* CarouselNavBar_DefaultCustom;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonWidgetCarousel* Carousel_CreatorProperty;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSCarouselNavBar* CarouselNavBar_CreatorProperty;

private:
	/** Initializes all Default Game Mode Menu Options found in Box_DefaultGameModesOptions. */
	void InitDefaultGameModesWidgets();

	UFUNCTION()
	void OnCarouselWidgetIndexChanged_DefaultCustom(UCommonWidgetCarousel* InCarousel, const int32 NewIndex);

	UFUNCTION()
	void OnCarouselWidgetIndexChanged_CreatorProperty(UCommonWidgetCarousel* InCarousel, const int32 NewIndex);

	/** Initializes all Custom game mode options based on the BSConfig. */
	void PopulateGameModeOptions(const FBSConfig& InConfig);

	/** Retrieves all Custom game mode options and returns a BSConfig with those options. */
	FBSConfig GetCustomGameModeOptions() const;

	/** Saves a CustomGameMode to save slot using GetCustomGameModeOptions(), displays the saved text,
	 *  refreshes game mode template combo box and selects it. */
	bool SaveCustomGameModeOptionsAndReselect(const FText& SuccessMessage = FText());

	/** Changes the Save and Start Button states depending on what is selected in ComboBox_GameModeName and
	 *  TextBox_CustomGameModeName. */
	void UpdateSaveStartButtonStates();

	/** Checks to see if SelectedGameMode is valid, Binds to ScreenFadeToBlackFinish, and ends the game mode. */
	void ShowAudioFormatSelect(const bool bStartFromDefaultGameMode);

	/** Returns BSConfig is identical to the currently selected template option. */
	bool IsCurrentConfigIdenticalToSelectedCustom();

	/** Checks to see if the GameModeName ComboBox or the CustomGameModeName text box has a matching custom game mode
	 *  that is already saved. */
	bool DoesCustomGameModeExist();

	/** Initializes a PopupMessage asking the player if they want to overwrite an existing custom game mode, calling
	 *  SaveCustomGameModeOptionsAndReselect if they choose to override. */
	void ShowConfirmOverwriteMessage_Import(TSharedPtr<FBSConfig>& ImportedConfig);

	/** Changes the SelectedGameMode depending on input button. */
	UFUNCTION()
	void OnButtonClicked_SelectedDefaultGameMode(const UBSButton* Button);

	/** Changes the Difficulty depending on input button. */
	UFUNCTION()
	void OnButtonClicked_SelectedDifficulty(const UBSButton* Button);

	/** The Button_CustomizeFromPreset and Button_StartFromPreset bind to this function. */
	UFUNCTION()
	void OnButtonClicked_DefaultGameMode(const UBSButton* Button);

	/** Any Custom Game Mode Button binds to this function. */
	UFUNCTION()
	void OnButtonClicked_CustomGameModeButton(const UBSButton* Button);

	/** Saves the custom game mode to slot, repopulates ComboBox_GameModeName, and selects the new custom game mode. */
	void OnButtonClicked_SaveCustom();

	/** Calls DoesCustomGameModeExist, and calls ShowAudioFormatSelect if no existing found. */
	void OnButtonClicked_StartFromCustom();

	/** Creates a confirmation pop up widget and binds to its buttons, removing the selected custom if confirmed. */
	void OnButtonClicked_RemoveSelectedCustom();

	/** Creates a confirmation pop up widget and binds to its buttons, removing all custom if confirmed. */
	void OnButtonClicked_RemoveAllCustom();

	/** Creates an import custom widget and adds to viewport. */
	void OnButtonClicked_ImportCustom();

	/** Copies the game mode to clipboard and updates Text. */
	void OnButtonClicked_ExportCustom();

	/** Creates a confirmation pop up widget and binds to its buttons, clearing the Reinforcement Learning history of a
	 * game mode if confirmed. */
	void OnButtonClicked_ClearRLHistory();

	/** Synchronizes properties like CustomGameModeName between CreatorView and PropertyView. */
	void SynchronizeStartWidgets();

	/** Sets the SavedText and plays FadeInFadeOut for the SavedTextWidget corresponding to the
	 *  CustomGameModesWidget_Current. */
	void SetAndPlaySavedText(const FText& InText);

	/** Called when one of the custom game modes widgets has at least one breaking game mode option, or none.
	 *  Updates the value of bGameModeBreakingOptionPresent and Broadcasts OnGameModeBreakingChange if the value
	 *  is different. */
	void OnGameModeBreakingOptionPresentStateChanged(const bool bIsPresent);

	/** Restarts the game mode preview. */
	void RefreshGameModePreview();

	void RefreshGameModes();

	void SetBSConfig(const FBSConfig& InConfig);

	/** The BaseGameMode for a selected Preset Game Mode. */
	EBaseGameMode PresetSelection_PresetGameMode;

	/** The difficulty for a selected Preset Game Mode. */
	EGameModeDifficulty PresetSelection_Difficulty;

	/** Pointer to the custom game mode config, shared with all CustomGameModeWidgets and their children. */
	TSharedPtr<FBSConfig> BSConfig;

	/** Whether one of the custom game modes widgets has at least one breaking game mode option, or none. */
	bool bGameModeBreakingOptionPresent = false;

	/** Whether the user has any Custom Game Modes saved. */
	bool bCustomGameModesEmpty = true;
};
