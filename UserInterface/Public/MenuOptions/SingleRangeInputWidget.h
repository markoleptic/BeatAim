// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MenuOptionWidget.h"
#include "SingleRangeInputWidget.generated.h"

class UCheckBox;
class UTextBlock;
class UEditableTextBox;
class USpacer;
class UHorizontalBox;
class USlider;
class UTooltipIcon;
class UBSHorizontalBox;
class USingleRangeInputWidget;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSliderTextBoxValueChanged, USingleRangeInputWidget* Widget, const float Value);

UCLASS()
class USERINTERFACE_API USingleRangeInputWidget : public UMenuOptionWidget
{
	GENERATED_BODY()

public:
	/** Sets the Min and Max values of the slider and sets the grid snap size. */
	void SetValues(const float Min, const float Max, const float SnapSize);

	/** Sets the value for the slider and EditableTextBox. */
	void SetValue(const float Value) const;

	/** Returns the value for the slider. */
	float GetSliderValue() const;

	/** Returns the value for the EditableTextBox. */
	float GetEditableTextBoxValue() const;

	/** Returns the slider value snapped to the grid snap size. */
	float GetSliderValueSnapped() const;

	/** Locks or unlocks the slider and sets the EditableTextBox to read-only or not. */
	void SetSliderAndTextBoxEnabledStates(const bool bEnabled) const;

	/** Broadcast when the slider value changes or the EditableTextBox has text committed to it. */
	FOnSliderTextBoxValueChanged OnSliderTextBoxValueChanged;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USlider* Slider;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBox* EditableTextBox;

protected:
	virtual void NativeConstruct() override;
	virtual void SetStyling() override;

	UFUNCTION()
	void OnSliderChanged_Slider(const float Value);
	UFUNCTION()
	void OnTextCommitted_EditableTextBox(const FText& Text, ETextCommit::Type CommitType);

	float GridSnapSize = 1.f;
};
