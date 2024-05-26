// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeCategoryWidget.h"
#include "CustomGameModeActivationWidget.generated.h"

enum class ETargetDistributionPolicy : uint8;
enum class ETargetActivationResponse : uint8;
class USingleRangeInputWidget;
class UTextInputWidget;
class UCheckBoxWidget;
class UComboBoxWidget;

UCLASS()
class USERINTERFACE_API UCustomGameModeActivationWidget : public UCustomGameModeCategoryWidget
{
	GENERATED_BODY()

protected:
	UCustomGameModeActivationWidget();

	virtual void NativeConstruct() override;

	virtual void UpdateOptionsFromConfig() override;

	/** Updates options that depend on the value selection of TargetActivationResponses. */
	void UpdateDependentOptions_TargetActivationResponses(const TArray<ETargetActivationResponse>& InResponses,
		const bool bUseConstantTargetSpeed);

	/** Updates options that depend on the value selection of TargetDistributionPolicy. */
	void UpdateDependentOptions_TargetDistributionPolicy(const ETargetDistributionPolicy& Policy);

	UFUNCTION()
	void OnCheckStateChanged_AllowActivationWhileActivated(const bool bChecked);

	void OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value);

	void OnMinMaxMenuOptionChanged(UDualRangeInputWidget* Widget, const bool bChecked, const float MinOrConstant,
		const float Max);

	UFUNCTION()
	void OnSelectionChanged_TargetActivationSelectionPolicy(const TArray<FString>& Selected,
		const ESelectInfo::Type SelectionType);

	FString GetComboBoxEntryTooltipStringTableKey_TargetActivationSelectionPolicy(const FString& EnumString);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_MaxNumActivatedTargetsAtOnce;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UDualRangeInputWidget* MenuOption_NumTargetsToActivateAtOnce;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* CheckBoxOption_AllowActivationWhileActivated;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_TargetActivationSelectionPolicy;
};
