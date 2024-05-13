// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveGamePlayerSettings.h"
#include "BSPlayerSettingsInterface.h"
#include "Delegates/DelegateCombinations.h"
#include "Utilities/BSSettingCategoryWidget.h"
#include "CrossHairSettingsWidget.generated.h"

class UCheckBoxWidget;
class UColorSelectOptionWidget;
class USingleRangeInputWidget;
class USavedTextWidget;
class UColorSelectWidget;
class UCrossHairWidget;
class USlider;
class UImage;
class UEditableTextBox;
class UBSButton;

/** Settings category widget holding CrossHair settings. */
UCLASS()
class USERINTERFACE_API UCrossHairSettingsWidget : public UBSSettingCategoryWidget, public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	/** Returns OnPlayerSettingsChangedDelegate_CrossHair, the delegate that is broadcast when this class saves
	 *  CrossHair settings. */
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
	UColorSelectWidget* MenuOption_ColorSelect;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectWidget* MenuOption_OutlineColorSelect;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectWidget* MenuOption_CrossHairDotColorSelect;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* MenuOption_InnerOffset;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* MenuOption_LineLength;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* MenuOption_LineWidth;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* MenuOption_OutlineSize;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* MenuOption_ShowCrossHairDot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* MenuOption_CrossHairDotSize;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Reset;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Revert;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Save;

private:
	void OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value);

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

	/** Fills out all CrossHair Settings given PlayerSettings. */
	void SetCrossHairOptions(const FPlayerSettings_CrossHair& CrossHairSettings);

	/** The PlayerSettings that were initially loaded. */
	FPlayerSettings_CrossHair InitialCrossHairSettings;

	/** The PlayerSettings that are changed when interacting with the CrossHairSettingsWidget. */
	FPlayerSettings_CrossHair NewCrossHairSettings;
};
