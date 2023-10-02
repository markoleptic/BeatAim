// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveLoadInterface.h"
#include "SubmenuWidgets/SettingsWidgets/BSSettingCategoryWidget.h"
#include "SettingsMenuWidget_AudioAnalyzer.generated.h"

class UBandThresholdWidget;
class UBandChannelWidget;
class USliderTextBoxOptionWidget;
class UComboBoxOptionWidget;
class UBSButton;
class USavedTextWidget;
class UPopupMessageWidget;

DECLARE_DELEGATE(FOnRestartButtonClicked);

/** Settings category widget holding AudioAnalyzer settings */
UCLASS()
class USERINTERFACE_API USettingsMenuWidget_AudioAnalyzer : public UBSSettingCategoryWidget,  public ISaveLoadInterface
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Broadcast when the restart and save button is clicked to apply AudioAnalyzer settings that require a restart */
	FOnRestartButtonClicked OnRestartButtonClicked;

	/** Do specific things if this instance of AASettings belongs to Pause Menu */
	void InitPauseMenuChild();

	/** Returns OnPlayerSettingsChangedDelegate_AudioAnalyzer, the delegate that is broadcast when this class saves Audio Analyzer settings */
	FOnPlayerSettingsChanged_AudioAnalyzer& GetPublicAudioAnalyzerSettingsChangedDelegate() { return OnPlayerSettingsChangedDelegate_AudioAnalyzer; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "AudioAnalyzer | Classes")
	TSubclassOf<UBandChannelWidget> BandChannelWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "AudioAnalyzer | Classes")
	TSubclassOf<UBandThresholdWidget> BandThresholdWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "AudioAnalyzer | Classes")
	TSubclassOf<UPopupMessageWidget> PopupMessageClass;

	UPROPERTY()
	UBandChannelWidget* BandChannelWidget;
	UPROPERTY()
	UBandThresholdWidget* BandThresholdWidget;
	UPROPERTY()
	UPopupMessageWidget* PopupMessageWidget;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	USavedTextWidget* SavedTextWidget;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UComboBoxOptionWidget* ComboBoxOption_NumBandChannels;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* SliderTextBoxOption_TimeWindow;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSVerticalBox* Box_BandChannelBounds;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSVerticalBox* Box_BandThresholdBounds;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_Reset;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_Save;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_SaveAndRestart;
	
	UPROPERTY()
	FPlayerSettings_AudioAnalyzer AASettings;
	UPROPERTY()
	FPlayerSettings_AudioAnalyzer NewAASettings;
	
	UFUNCTION()
	void OnChannelValueCommitted(const UBandChannelWidget* BandChannel, const int32 Index, const float NewValue, const bool bIsMinValue);
	UFUNCTION()
	void OnBandThresholdChanged(const UBandThresholdWidget* BandThreshold, const int32 Index, const float NewValue);

	void OnSliderTextBoxValueChanged(USliderTextBoxOptionWidget* Widget, const float Value);
	
	UFUNCTION()
	void OnSelectionChanged_NumBandChannels(const TArray<FString>& SelectedOptions, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnButtonClicked_BSButton(const UBSButton* Button);
	
	/** Update values in Settings Menu to match AASettings */
	UFUNCTION()
	void PopulateAASettings();

	/** Reset AASettings to default value and repopulate in Settings Menu. Does not automatically save */
	UFUNCTION()
	void ResetAASettings();

	/** Sorts NewAASettings and checks for overlapping Band Channels. Calls ShowBandLimitErrorMessage() if it finds an overlap, otherwise saves */
	void SortAndCheckOverlap();

	/** Adds a popup message to the viewport displaying the error */
	void ShowBandLimitErrorMessage();

	/** Hides the error message */
	UFUNCTION()
	void HideSongPathErrorMessage();
};
