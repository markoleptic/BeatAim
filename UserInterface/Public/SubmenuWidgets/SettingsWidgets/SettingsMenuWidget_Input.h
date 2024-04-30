// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerSettingsInterface.h"
#include "SubmenuWidgets/SettingsWidgets/BSSettingCategoryWidget.h"
#include "WidgetComponents/InputMappingWidget.h"
#include "SettingsMenuWidget_Input.generated.h"

class UInputMappingContext;
class UEditableTextBoxOptionWidget;
class USliderTextBoxOptionWidget;
class UScrollBox;
class UInputKeySelector;
class USavedTextWidget;
class UBSButton;
class UTextBlock;
class UInputMappingWidget;

UCLASS()
class USERINTERFACE_API USettingsMenuWidget_Input : public UBSSettingCategoryWidget, public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

	friend class USettingsMenuWidget;

public:
	/** Returns OnPlayerSettingsChangedDelegate_User, the delegate that is broadcast when this class saves User settings */
	FOnPlayerSettingsChanged_User& GetPublicUserSettingsChangedDelegate()
	{
		return OnPlayerSettingsChangedDelegate_User;
	}

	virtual void NativeConstruct() override;

	/** Populates the settings menu */
	void InitializeInputSettings(const TMap<FName, FKeyMappingRow>& InPlayerMappedRows = TMap<FName, FKeyMappingRow>());

protected:
	/** Function bound to all InputMappingWidgets' OnKeySelected delegates */
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
	UEditableTextBoxOptionWidget* MenuOption_CurrentSensitivity;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* MenuOption_NewSensitivity;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* MenuOption_NewSensitivityCsgo;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Saving")
	UBSButton* Button_Save;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Saving")
	UBSButton* Button_Reset;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Saving")
	UBSButton* Button_Revert;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USavedTextWidget* SavedTextWidget;

	void OnSliderTextBoxValueChanged(USliderTextBoxOptionWidget* Widget, const float Value);

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
