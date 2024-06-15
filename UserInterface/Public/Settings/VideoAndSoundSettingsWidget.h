// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerSettingsInterface.h"
#include "Utilities/BSSettingCategoryWidget.h"
#include "VideoAndSoundSettingsWidget.generated.h"

class UScalabilitySettingWidget;
enum class EVideoSettingType : uint8;
class UVideoSettingOptionWidget;
class UTextInputWidget;
class UComboBoxWidget;
class USingleRangeInputWidget;
class UCheckBoxWidget;
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

/** Settings category widget holding Video and Sound settings. */
UCLASS()
class USERINTERFACE_API UVideoAndSoundSettingsWidget : public UBSSettingCategoryWidget,
                                                       public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

	friend class USettingsMenuWidget;

	virtual void NativeConstruct() override;

	/** Populates the settings menu with BSGameUserSettings. */
	void InitializeVideoAndSoundSettings();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes | PopUp")
	TSubclassOf<UPopupMessageWidget> PopupMessageClass;

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
	UCheckBoxWidget* CheckBoxOption_HDREnabled;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_HDRNits;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_Brightness;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_DisplayGamma;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_GlobalSound;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_MenuSound;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_MusicSound;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_SoundFX;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_OutputAudioDevice;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_WindowMode;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_Resolution;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_AntiAliasingMethod;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_DLSS;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_DLSS_FrameGeneration;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_DLSS_RayReconstitution;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_DLSS_SuperResolution;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_DLSS_Sharpness;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_NIS;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_NIS_Mode;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_NIS_Sharpness;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_Reflex;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_ResolutionScale;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* CheckBoxOption_VSync;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingWidget* VideoSettingOptionWidget_AA;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingWidget* VideoSettingOptionWidget_GI;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingWidget* VideoSettingOptionWidget_PP;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingWidget* VideoSettingOptionWidget_RQ;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingWidget* VideoSettingOptionWidget_SGQ;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingWidget* VideoSettingOptionWidget_SWQ;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingWidget* VideoSettingOptionWidget_TQ;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingWidget* VideoSettingOptionWidget_VD;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingWidget* VideoSettingOptionWidget_VEQ;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* CheckBoxOption_FPSCounter;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextInputWidget* EditableTextBoxOption_FPSLimitMenu;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextInputWidget* EditableTextBoxOption_FPSLimitGame;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextInputWidget* EditableTextBoxOption_FPSLimitBackground;

private:
	/** Adds the ConfirmVideoSettingsMessage to viewport, and starts the RevertVideoSettingsTimer. */
	UFUNCTION()
	void ShowConfirmVideoSettingsMessage();

	/** Stops the RevertVideoSettingsTimer and applies the video settings and closes the ConfirmVideoSettingsMessage. */
	void OnButtonPressed_ConfirmVideoSettings(const UBSButton* Button);

	/** Reverts the video settings and closes the ConfirmVideoSettingsMessage. */
	UFUNCTION()
	void OnButtonPressed_CancelVideoSettings(const UBSButton* Button);

	/** Saves the Video and Sound Settings. */
	UFUNCTION()
	void OnButtonPressed_Save();

	/** Reset Video and Sound Settings to defaults and repopulate in Settings Menu. Does not automatically save. */
	UFUNCTION()
	void OnButtonPressed_Reset();

	UFUNCTION()
	void OnBSButtonPressed_SaveReset(const UBSButton* Button);

	UFUNCTION()
	void OnVideoSettingOptionWidget_ButtonPressed(const EVideoSettingType VideoSettingType, const uint8 Quality);

	void OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value);

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
	void OnSelectionChanged_RayReconstitution(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_NIS_EnabledMode(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_NIS_Mode(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_Reflex(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_AntiAliasingMethod(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_OutputAudioDevice(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);

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

	/** Function bound to RevertVideoSettingsTimer_UpdateSecond. */
	UFUNCTION()
	void RevertVideoSettingsTimerCallback();

	/** Clears and repopulates the ComboBox_Resolution based on the resolutions from GetSupportedFullscreenResolutions
	 *  or GetConvenientWindowedResolutions. */
	void PopulateResolutionComboBox();

	/** Updates the widgets for any Nvidia related settings. */
	void UpdateNvidiaSettings();

	/** Sets enabled/disabled states for any Nvidia related settings. */
	void HandleDLSSEnabledChanged(bool bDLSSEnabled, bool bNISEnabled);

	FString GetComboBoxEntryTooltipStringTableKey_DLSS_FrameGeneration(const FString& EnumString);
	FString GetComboBoxEntryTooltipStringTableKey_DLSS_SuperResolution(const FString& EnumString);
	FString GetComboBoxEntryTooltipStringTableKey_Reflex(const FString& EnumString);

	/** Timer that starts when window mode or resolution is changed. If it expires, it reverts those changes.
	 *  Not bound to any function, but checked every second in RevertVideoSettingsTimerCallback. */
	FTimerHandle RevertVideoSettingsTimer;

	/** Timer that starts when window mode or resolution is changed, and calls RevertVideoSettingsTimerCallback every
	 *  second. */
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
