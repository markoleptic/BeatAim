// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveGamePlayerSettings.h"
#include "BSPlayerSettingsInterface.h"
#include "SubmenuWidgets/SettingsWidgets/BSSettingCategoryWidget.h"
#include "WidgetComponents/Buttons/VideoSettingButton.h"
#include "SettingsMenuWidget_VideoAndSound.generated.h"

class UVideoSettingOptionWidget;
class UEditableTextBoxOptionWidget;
class UComboBoxOptionWidget;
class USliderTextBoxOptionWidget;
class UCheckBoxOptionWidget;
class UCheckBox;
class UComboBoxString;
class USlider;
class UEditableTextBox;
class UBSHorizontalBox;
class UBSVerticalBox;
class UPopupMessageWidget;
class USavedTextWidget;
class UVerticalBox;
class UBSButton;

/** Settings category widget holding Video and Sound settings */
UCLASS()
class USERINTERFACE_API USettingsMenuWidget_VideoAndSound : public UBSSettingCategoryWidget,
                                                            public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

	friend class USettingsMenuWidget;

	virtual void NativeConstruct() override;

	/** Populates the settings menu with BSGameUserSettings */
	void InitializeVideoAndSoundSettings();

	/** Returns the video and sound settings that are currently populated in the menu */
	FPlayerSettings_VideoAndSound GetVideoAndSoundSettings() const;

	/** Returns OnPlayerSettingsChangedDelegate_VideoAndSound, the delegate that is broadcast when this class saves Video and Sound settings */
	FOnPlayerSettingsChanged_VideoAndSound& GetPublicVideoAndSoundSettingsChangedDelegate()
	{
		return OnPlayerSettingsChangedDelegate_VideoAndSound;
	}

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes | PopUp")
	TSubclassOf<UPopupMessageWidget> PopupMessageClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes | Sound")
	USoundClass* GlobalSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes | Sound")
	USoundClass* MenuSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes | Sound")
	USoundMix* GlobalSoundMix;

	UPROPERTY()
	UPopupMessageWidget* PopupMessageWidget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USavedTextWidget* SavedTextWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Save;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Reset;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSVerticalBox* BSBox_Video;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSVerticalBox* BSBox_Sound;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* CheckBoxOption_HDREnabled;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* SliderTextBoxOption_HDRNits;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* SliderTextBoxOption_Brightness;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* SliderTextBoxOption_GlobalSound;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* SliderTextBoxOption_MenuSound;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* SliderTextBoxOption_MusicSound;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxOptionWidget* ComboBoxOption_WindowMode;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxOptionWidget* ComboBoxOption_Resolution;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxOptionWidget* ComboBoxOption_AntiAliasingMethod;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxOptionWidget* ComboBoxOption_DLSS;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxOptionWidget* ComboBoxOption_DLSS_FrameGeneration;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxOptionWidget* ComboBoxOption_DLSS_SuperResolution;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* SliderTextBoxOption_DLSS_Sharpness;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxOptionWidget* ComboBoxOption_NIS;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxOptionWidget* ComboBoxOption_NIS_Mode;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* SliderTextBoxOption_NIS_Sharpness;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxOptionWidget* ComboBoxOption_Reflex;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* SliderTextBoxOption_ResolutionScale;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* CheckBoxOption_VSync;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingOptionWidget* VideoSettingOptionWidget_AA;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingOptionWidget* VideoSettingOptionWidget_GI;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingOptionWidget* VideoSettingOptionWidget_PP;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingOptionWidget* VideoSettingOptionWidget_RQ;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingOptionWidget* VideoSettingOptionWidget_SGQ;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingOptionWidget* VideoSettingOptionWidget_SWQ;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingOptionWidget* VideoSettingOptionWidget_TQ;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingOptionWidget* VideoSettingOptionWidget_VD;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingOptionWidget* VideoSettingOptionWidget_VEQ;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* CheckBoxOption_FPSCounter;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBoxOptionWidget* EditableTextBoxOption_FPSLimitMenu;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBoxOptionWidget* EditableTextBoxOption_FPSLimitGame;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBoxOptionWidget* EditableTextBoxOption_FPSLimitBackground;

private:
	/** Adds the ConfirmVideoSettingsMessage to viewport, and starts the RevertVideoSettingsTimer */
	UFUNCTION()
	void ShowConfirmVideoSettingsMessage();

	/** Stops the RevertVideoSettingsTimer and applies the video settings and closes the ConfirmVideoSettingsMessage */
	UFUNCTION()
	void OnButtonPressed_ConfirmVideoSettings();

	/** Reverts the video settings and closes the ConfirmVideoSettingsMessage */
	UFUNCTION()
	void OnButtonPressed_CancelVideoSettings();

	/** Saves the Video and Sound Settings */
	UFUNCTION()
	void OnButtonPressed_Save();

	/** Reset Video and Sound Settings to defaults and repopulate in Settings Menu. Does not automatically save */
	UFUNCTION()
	void OnButtonPressed_Reset();

	UFUNCTION()
	void OnBSButtonPressed_SaveReset(const UBSButton* Button);

	UFUNCTION()
	void OnVideoSettingOptionWidget_ButtonPressed(const EVideoSettingType VideoSettingType, const uint8 Quality);

	void OnSliderTextBoxValueChanged(USliderTextBoxOptionWidget* Widget, const float Value);

	UFUNCTION()
	void OnSelectionChanged_WindowMode(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_Resolution(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_DLSS_EnabledMode(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_FrameGeneration(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_SuperResolution(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_NIS_EnabledMode(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_NIS_Mode(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_Reflex(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_AntiAliasingMethod(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnCheckStateChanged_VSyncEnabled(const bool bIsChecked);
	UFUNCTION()
	void OnCheckStateChanged_HDREnabled(const bool bIsChecked);
	UFUNCTION()
	void OnCheckStateChanged_FPSCounter(const bool bIsChecked);
	UFUNCTION()
	void OnTextCommitted_FPSLimitMenu(const FText& Text, ETextCommit::Type CommitType);
	UFUNCTION()
	void OnTextCommitted_FPSLimitGame(const FText& Text, ETextCommit::Type CommitType);
	UFUNCTION()
	void OnTextCommitted_FPSLimitBackground(const FText& Text, ETextCommit::Type CommitType);

	/** Function bound to RevertVideoSettingsTimer_UpdateSecond */
	UFUNCTION()
	void RevertVideoSettingsTimerCallback();

	/** Clears and repopulates the ComboBox_Resolution based on the resolutions from GetSupportedFullscreenResolutions or GetConvenientWindowedResolutions */
	void PopulateResolutionComboBox();

	// TODO: Fix
	/*/** Sets enabled/disabled states for any NVIDIA DLSS related settings #1#
	void HandleDLSSEnabledChanged(const EDLSSEnabledMode DLSSEnabledMode);

	/** Forces certain DLSS/NIS settings depending on if DLSS is enabled, and changes those selected options. Only called when a user changes the DLSS Enabled Mode selection #1#
	void HandleDLSSDependencies(const EDLSSEnabledMode DLSSEnabledMode);

	/** Returns the selected DLSS enabled mode #1#
	EDLSSEnabledMode GetSelectedDLSSEnabledMode() const;

	/** Returns the selected Frame Generation mode #1#
	UStreamlineDLSSGMode GetSelectedFrameGenerationMode() const;

	/** Returns the selected DLSS mode (Super Resolution) #1#
	UDLSSMode GetSelectedDLSSMode() const;

	/** Returns the selected NIS enabled mode #1#
	ENISEnabledMode GetSelectedNISEnabledMode() const;

	/** Returns the selected NIS mode #1#
	UNISMode GetSelectedNISMode() const;

	/** Returns the selected reflex mode #1#
	UStreamlineReflexMode GetSelectedReflexMode() const;

	FString GetComboBoxEntryTooltipStringTableKey_DLSS_FrameGeneration(const FString& EnumString);
	FString GetComboBoxEntryTooltipStringTableKey_DLSS_SuperResolution(const FString& EnumString);
	FString GetComboBoxEntryTooltipStringTableKey_Reflex(const FString& EnumString);*/

	/** Holds the last confirmed resolution */
	FIntPoint LastConfirmedResolution;

	/** Holds the last confirmed WindowMode */
	EWindowMode::Type LastConfirmedWindowMode;

	/** Timer that starts when window mode or resolution is changed. If it expires, it reverts those changes. Not bound to any function, but checked every second in RevertVideoSettingsTimerCallback */
	FTimerHandle RevertVideoSettingsTimer;

	/** Timer that starts when window mode or resolution is changed, and calls RevertVideoSettingsTimerCallback every second */
	FTimerHandle RevertVideoSettingsTimer_UpdateSecond;

	TMap<FString, uint8> WindowModeMap;
	TMap<FString, uint8> AntiAliasingMethodMap;
	TMap<FString, uint8> DLSSModeMap;
	TMap<FString, uint8> DLSSEnabledModeMap;
	TMap<FString, uint8> NISModeMap;
	TMap<FString, uint8> NISEnabledModeMap;
	TMap<FString, uint8> FrameGenerationEnabledModeMap;
	TMap<FString, uint8> StreamlineReflexModeMap;
};
