// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveLoadInterface.h"
#include "Delegates/DelegateCombinations.h"
#include "WidgetComponents/BSSettingCategoryWidget.h"
#include "SettingsMenuWidget_CrossHair.generated.h"

class UColorSelectOptionWidget;
class USliderTextBoxWidget;
class USavedTextWidget;
class UColorSelectWidget;
class UCrossHairWidget;
class USlider;
class UImage;
class UEditableTextBox;
class UBSButton;

/** Settings category widget holding CrossHair settings */
UCLASS()
class USERINTERFACE_API USettingsMenuWidget_CrossHair : public UBSSettingCategoryWidget, public ISaveLoadInterface
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	/** Returns OnPlayerSettingsChangedDelegate_CrossHair, the delegate that is broadcast when this class saves CrossHair settings */
	FOnPlayerSettingsChanged_CrossHair& GetPublicCrossHairSettingsChangedDelegate() { return OnPlayerSettingsChangedDelegate_CrossHair; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UCrossHairWidget* CrossHairWidget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USavedTextWidget* SavedTextWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectOptionWidget* MenuOption_ColorSelect;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxWidget* MenuOption_InnerOffset;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxWidget* MenuOption_LineLength;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxWidget* MenuOption_LineWidth;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxWidget* MenuOption_OutlineOpacity;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxWidget* MenuOption_OutlineWidth;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Reset;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Revert;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Save;

private:
	void OnSliderTextBoxValueChanged(USliderTextBoxWidget* Widget, const float Value);
	
	UFUNCTION()
	void OnColorChanged(const FLinearColor& NewColor);

	UFUNCTION()
	void OnButtonClicked_Reset();
	UFUNCTION()
	void OnButtonClicked_Revert();
	UFUNCTION()
	void OnButtonClicked_Save();
	UFUNCTION()
	void OnButtonClicked_BSButton(const UBSButton* Button);

	/** Fills out all CrossHair Settings given PlayerSettings */
	void SetCrossHairOptions(const FPlayerSettings_CrossHair& CrossHairSettings);
	
	/** The PlayerSettings that were initially loaded */
	FPlayerSettings_CrossHair InitialCrossHairSettings;
	
	/** The PlayerSettings that are changed when interacting with the CrossHairSettingsWidget */
	FPlayerSettings_CrossHair NewCrossHairSettings;
};
