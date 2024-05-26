// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

// ReSharper disable CppUE4CodingStandardNamingViolationWarning
#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeCategoryWidget.h"
#include "CustomGameModeTargetSizingWidget.generated.h"

enum class ETargetActivationResponse : uint8;
enum class ETargetDeactivationCondition : uint8;
enum class ETargetDeactivationResponse : uint8;
enum class EConsecutiveTargetScalePolicy : uint8;

UCLASS()
class USERINTERFACE_API UCustomGameModeTargetSizingWidget : public UCustomGameModeCategoryWidget
{
	GENERATED_BODY()

protected:
	UCustomGameModeTargetSizingWidget();

	virtual void NativeConstruct() override;

	virtual void UpdateOptionsFromConfig() override;

	/** Updates options that depend on the value selection of TargetActivationResponses. */
	void UpdateDependentOptions_TargetActivationResponses(const TArray<ETargetActivationResponse>& InResponses);

	/** If Persistent, empties and disables Target Deactivation Responses Combo Box. */
	void UpdateDependentOptions_TargetDeactivationResponses(const TArray<ETargetDeactivationCondition>& Conditions,
		const TArray<ETargetDeactivationResponse>& Responses);

	/** Updates options that depend on the value selection of ConsecutiveTargetScalePolicy. */
	void UpdateDependentOptions_ConsecutiveTargetScalePolicy(
		const EConsecutiveTargetScalePolicy InConsecutiveTargetScalePolicy);

	void OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value);

	void OnMinMaxMenuOptionChanged(UDualRangeInputWidget* Widget, const bool bChecked, const float MinOrConstant,
		const float Max);

	UFUNCTION()
	void OnSelectionChanged_ConsecutiveTargetScalePolicy(const TArray<FString>& Selected,
		const ESelectInfo::Type SelectionType);

	FString GetComboBoxEntryTooltipStringTableKey_ConsecutiveTargetScalePolicy(const FString& EnumString);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_DeactivatedTargetScaleMultiplier;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UDualRangeInputWidget* MenuOption_TargetScale;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_StartThreshold;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_EndThreshold;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_DecrementAmount;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_LifetimeTargetScaleMultiplier;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_ConsecutiveTargetScalePolicy;

	EConsecutiveTargetScalePolicy LastSelectedConsecutiveTargetScalePolicy;
};
