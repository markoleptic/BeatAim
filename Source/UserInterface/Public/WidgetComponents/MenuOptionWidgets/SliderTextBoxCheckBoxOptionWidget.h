﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SliderTextBoxOptionWidget.h"
#include "SliderTextBoxCheckBoxOptionWidget.generated.h"

class UCommonTextBlock;
class USliderTextBoxCheckBoxOptionWidget;
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnSliderTextBoxCheckBoxOptionChanged, USliderTextBoxCheckBoxOptionWidget* Widget,
	const bool bChecked, const float MinOrConstantValue);

UCLASS()
class USERINTERFACE_API USliderTextBoxCheckBoxOptionWidget : public USliderTextBoxOptionWidget
{
	GENERATED_BODY()

public:
	/** Returns true if the CheckBox is checked */
	bool GetIsChecked() const;
	
	/** Sets the checked state for the Checkbox and calls UpdateMinMaxDependencies to update visibility stuff */
	void SetIsChecked(const bool bIsChecked) const;
	
	FOnSliderTextBoxCheckBoxOptionChanged OnSliderTextBoxCheckBoxOptionChanged;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual void SetStyling() override;

	void OnSliderTextBoxOptionChanged(USliderTextBoxOptionWidget* Widget, const float Value);

	/** If true, collapses Slider and TextBox. Otherwise sets them visible */
	void UpdateCheckBoxDependencies(const bool bConstant) const;
	
	UFUNCTION()
	void OnCheckStateChanged_CheckBox(const bool bChecked);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBox* CheckBox;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* TextBlock_CheckBox;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* Box_CheckBox;

	UPROPERTY(EditInstanceOnly, Category = "SliderTextBoxCheckBoxOptionWidget")
	FText CheckBoxText = FText::FromString("Infinite?");
};
