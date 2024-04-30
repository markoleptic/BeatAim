// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveGamePlayerSettings.h"
#include "BSPlayerSettingsInterface.h"
#include "Delegates/DelegateCombinations.h"
#include "SubmenuWidgets/SettingsWidgets/BSSettingCategoryWidget.h"
#include "SettingsMenuWidget_CrossHair.generated.h"

class UCheckBoxOptionWidget;
class UColorSelectOptionWidget;
class USliderTextBoxOptionWidget;
class USavedTextWidget;
class UColorSelectWidget;
class UCrossHairWidget;
class USlider;
class UImage;
class UEditableTextBox;
class UBSButton;

/** Settings category widget holding CrossHair settings */
UCLASS()
class USERINTERFACE_API USettingsMenuWidget_CrossHair : public UBSSettingCategoryWidget,
                                                        public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	/** Returns OnPlayerSettingsChangedDelegate_CrossHair, the delegate that is broadcast when this class saves CrossHair settings */
	FOnPlayerSettingsChanged_CrossHair& GetPublicCrossHairSettingsChangedDelegate()
	{
		return OnPlayerSettingsChangedDelegate_CrossHair;
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UCrossHairWidget* CrossHairWidget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USavedTextWidget* SavedTextWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectOptionWidget* MenuOption_ColorSelect;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectOptionWidget* MenuOption_OutlineColorSelect;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectOptionWidget* MenuOption_CrossHairDotColorSelect;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* MenuOption_InnerOffset;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* MenuOption_LineLength;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* MenuOption_LineWidth;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* MenuOption_OutlineSize;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* MenuOption_ShowCrossHairDot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* MenuOption_CrossHairDotSize;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Reset;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Revert;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Save;

private:
	void OnSliderTextBoxValueChanged(USliderTextBoxOptionWidget* Widget, const float Value);

	UFUNCTION()
	void OnColorChanged_CrossHair(const FLinearColor& NewColor);
	UFUNCTION()
	void OnColorChanged_CrossHairDot(const FLinearColor& NewColor);
	UFUNCTION()
	void OnColorChanged_CrossHairOutline(const FLinearColor& NewColor);
	UFUNCTION()
	void OnCheckStateChanged_MenuOption_ShowCrossHairDot(const bool bIsChecked);

	UFUNCTION()
	void OnButtonClicked_Reset();
	UFUNCTION()
	void OnButtonClicked_Revert();
	UFUNCTION()
	void OnButtonClicked_Save();

	void OnButtonClicked_BSButton(const UBSButton* Button);

	/** Fills out all CrossHair Settings given PlayerSettings */
	void SetCrossHairOptions(const FPlayerSettings_CrossHair& CrossHairSettings);

	/** The PlayerSettings that were initially loaded */
	FPlayerSettings_CrossHair InitialCrossHairSettings;

	/** The PlayerSettings that are changed when interacting with the CrossHairSettingsWidget */
	FPlayerSettings_CrossHair NewCrossHairSettings;
};
