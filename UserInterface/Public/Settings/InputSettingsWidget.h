// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerSettingsInterface.h"
#include "Utilities/BSSettingCategoryWidget.h"
#include "Utilities/InputMappingWidget.h"
#include "InputSettingsWidget.generated.h"

class UInputMappingContext;
class UTextInputWidget;
class USingleRangeInputWidget;
class UScrollBox;
class UInputKeySelector;
class USavedTextWidget;
class UBSButton;
class UTextBlock;
class UInputMappingWidget;

UCLASS()
class USERINTERFACE_API UInputSettingsWidget : public UBSSettingCategoryWidget, public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

	friend class USettingsMenuWidget;

public:
	/** Returns OnPlayerSettingsChangedDelegate_User, the delegate that is broadcast when this class saves User
	 *  settings. */
	FOnPlayerSettingsChanged_User& GetPublicUserSettingsChangedDelegate()
	{
		return OnPlayerSettingsChangedDelegate_User;
	}

	virtual void NativeConstruct() override;

	/** Populates the settings menu. */
	void InitializeInputSettings(const TMap<FName, FKeyMappingRow>& InPlayerMappedRows = TMap<FName, FKeyMappingRow>());

protected:
	/** Function bound to all InputMappingWidgets' OnKeySelected delegates. */
	void OnKeySelected(const FName MappingName, const EPlayerMappableKeySlot& InSlot, const FInputChord SelectedKey);

	void OnIsSelectingKey(UInputKeySelector* KeySelector);

	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInputMappingWidget> InputMappingWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	UInputMappingContext* BaseMappingContext;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSVerticalBox* BSBox_Combat;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSVerticalBox* BSBox_General;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSVerticalBox* BSBox_Movement;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSVerticalBox* BSBox_Sensitivity;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* ScrollBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextInputWidget* MenuOption_CurrentSensitivity;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* MenuOption_NewSensitivity;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* MenuOption_NewSensitivityCsgo;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Saving")
	UBSButton* Button_Save;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Saving")
	UBSButton* Button_Reset;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Saving")
	UBSButton* Button_Revert;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USavedTextWidget* SavedTextWidget;

	void OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value);

	void OnButtonClicked_BSButton(const UBSButton* Button);

	void OnButtonClicked_Save();

	void OnButtonClicked_Reset();

	void OnButtonClicked_Revert();

	float Sensitivity;

	TMap<FName, FKeyMappingRow> InitialPlayerMappingRows;
	UPROPERTY()
	TMap<FName, UInputMappingWidget*> InputMappingWidgetMap;

	bool bIsSelectingKey = false;

	UPROPERTY()
	UInputKeySelector* CurrentKeySelector;
};
