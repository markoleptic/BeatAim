// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SingleRangeInputWidget.h"
#include "BandThresholdWidget.generated.h"

class UBandThresholdWidget;
DECLARE_DELEGATE_ThreeParams(FOnThresholdValueCommitted, const UBandThresholdWidget* BandThreshold, const int32 Index,
	const float NewValue);

class UBorder;
class USlider;
class UEditableTextBox;
class UTextBlock;

/** Widget representing a Band Threshold designed to be used with a linked list. Contains pointer to the next band
 *  threshold widget. */
UCLASS()
class USERINTERFACE_API UBandThresholdWidget : public USingleRangeInputWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	/** Pointer to the next band threshold widget inside a collection of them. */
	UPROPERTY()
	UBandThresholdWidget* Next = nullptr;

	/** Executed when a new threshold value has been committed. */
	FOnThresholdValueCommitted OnThresholdValueCommitted;

	/** Sets the threshold value and the channel index. */
	void SetDefaultValue(const float Value, const int32 ChannelIndex);

	/** The index of this widget inside a collection of them. */
	int32 Index;

	void OnSliderTextBoxValueChanged_Threshold(USingleRangeInputWidget* SliderTextBoxOptionWidget, const float Value);
};
