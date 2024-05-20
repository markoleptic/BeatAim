// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerScoreInterface.h"
#include "BSPlayerSettingsInterface.h"
#include "Blueprint/UserWidget.h"
#include "BSGameModeConfig/AudioConfig.h"
#include "Delegates/DelegateCombinations.h"
#include "Utilities/BSWidgetInterface.h"
#include "AudioSelectWidget.generated.h"

class UComboBoxString;
class UVerticalBox;
class UHorizontalBox;
class UPopupMessageWidget;
class UBSButton;
class UBorder;
class UTextBlock;
class UTooltipImage;
class UTooltipWidget;
class UCheckBox;
class UEditableTextBox;
class UWidgetAnimation;

/** Widget that prompts user to choose an audio input type and gather info about the audio configuration to be used for
 *  the game mode. */
UCLASS()
class USERINTERFACE_API UAudioSelectWidget : public UUserWidget, public IBSPlayerSettingsInterface,
                                             public IBSWidgetInterface, public IBSPlayerScoreInterface
{
	GENERATED_BODY()

protected:
	virtual UTooltipWidget* ConstructTooltipWidget() override;
	virtual UTooltipWidget* GetTooltipWidget() const override;
	virtual void NativeConstruct() override;

public:
	UFUNCTION()
	void FadeIn();
	UFUNCTION()
	void FadeOut();

	TDelegate<void(const FBS_AudioConfig& AudioConfig)> OnStartButtonClickedDelegate;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Audio Select Widget | Classes")
	TSubclassOf<UPopupMessageWidget> PopupMessageClass;
	UPROPERTY(EditDefaultsOnly, Category = "Audio Select Widget | Classes")
	TSubclassOf<UTooltipWidget> TooltipWidgetClass;

	UPROPERTY()
	UPopupMessageWidget* PopupMessageWidget;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTooltipImage* QMark_PlaybackAudio;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTooltipImage* QMark_Input;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_AudioFromFile;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_CaptureAudio;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_Back;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_Start;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_LoadFile;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UVerticalBox* Box_AudioDevice;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UVerticalBox* Box_SongTitleLength;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UHorizontalBox* Box_SongLength;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UVerticalBox* Box_SongTitle;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UComboBoxString* ComboBox_InAudioDevices;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UComboBoxString* ComboBox_SongTitle;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UCheckBox* Checkbox_PlaybackAudio;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UEditableTextBox* Value_SongTitle;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UEditableTextBox* Value_Minutes;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UEditableTextBox* Value_Seconds;

	UPROPERTY(EditDefaultsOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutAnim;
	UPROPERTY(EditDefaultsOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeInAnim;

private:
	UFUNCTION()
	void OnFadeOutFinish();

	UFUNCTION()
	void OnButtonPressed_BSButton(const UBSButton* Button);

	UFUNCTION()
	void OnButtonClicked_AudioFromFile();
	UFUNCTION()
	void OnButtonClicked_CaptureAudio();
	UFUNCTION()
	void OnButtonClicked_Start();
	UFUNCTION()
	void OnButtonClicked_LoadFile();
	UFUNCTION()
	void OnValueChanged_SongTitle(const FText& NewSongTitle, ETextCommit::Type CommitType);
	UFUNCTION()
	void OnValueChanged_Minutes(const FText& NewMinutes, ETextCommit::Type CommitType);
	UFUNCTION()
	void OnValueChanged_Seconds(const FText& NewSeconds, ETextCommit::Type CommitType);
	UFUNCTION()
	void OnSelectionChanged_InAudioDevice(const FString SelectedInAudioDevice, const ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_SongTitle(const FString NewSongTitle, const ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnCheckStateChanged_PlaybackAudio(const bool bIsChecked);

	/** Displays an error message upon failed AudioAnalyzer initialization. */
	UFUNCTION()
	void ShowSongPathErrorMessage();

	/** Opens file dialog for song selection. */
	bool OpenFileDialog(TArray<FString>& OutFileNames);

	bool FileDialogShared(bool bSave, const void* ParentWindowHandle, const FString& DialogTitle,
		const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, uint32 Flags,
		TArray<FString>& OutFilenames, int32& OutFilterIndex);

	FWidgetAnimationDynamicEvent FadeOutDelegate;

	/** Number formatting options for song length text boxes. */
	FNumberFormattingOptions NumberFormattingOptions;

	/** Contains information relating to the audio format the user has selected. Passed to GameModesWidget
	 *  using OnStartButtonClickedDelegate. */
	FBS_AudioConfig AudioConfig;

	UPROPERTY()
	UTooltipWidget* ActiveTooltipWidget;

	UPROPERTY()
	TMap<FString, float> SongDurationMap;
};
