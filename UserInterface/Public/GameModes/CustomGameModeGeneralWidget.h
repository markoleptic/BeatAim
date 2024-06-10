// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeCategoryWidget.h"
#include "CustomGameModeGeneralWidget.generated.h"

enum class EReinforcementLearningHyperParameterMode : uint8;
enum class ETargetDeactivationCondition : uint8;
enum class ERecentTargetMemoryPolicy : uint8;
class UToggleableSingleRangeInputWidget;
class UCheckBoxWidget;
class USingleRangeInputWidget;
class UComboBoxWidget;

UCLASS()
class USERINTERFACE_API UCustomGameModeGeneralWidget : public UCustomGameModeCategoryWidget
{
public:
	UCustomGameModeGeneralWidget();

private:
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	virtual void UpdateOptionsFromConfig() override;

	virtual void HandleWatchedPropertyChanged(uint32 PropertyHash) override;

	/** Updates options that depend on the value selection of RecentTargetMemoryPolicy. */
	void UpdateDependentOptions_RecentTargetMemoryPolicy(const ERecentTargetMemoryPolicy InRecentTargetMemoryPolicy);

	void UpdateDependentOptions_DeactivationConditions(const TArray<ETargetDeactivationCondition>& Conditions);

	/** Updates options that depend on the value selection of bEnableReinforcementLearning. */
	void UpdateDependentOptions_EnableAI(const bool bInEnableReinforcementLearning,
		const EReinforcementLearningHyperParameterMode HyperParameterMode);

	/** Updates options that depend on the value selection of HyperParameterMode. */
	void UpdateDependentOptions_HyperParameterMode(const bool bInEnableReinforcementLearning,
		const EReinforcementLearningHyperParameterMode HyperParameterMode);

	UFUNCTION()
	void OnCheckStateChanged_EnableAI(const bool bChecked);

	void OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value);

	void OnSliderTextBoxCheckBoxOptionChanged(UToggleableSingleRangeInputWidget* Widget, const bool bChecked,
		const float Value);

	UFUNCTION()
	void OnSelectionChanged_RecentTargetMemoryPolicy(const TArray<FString>& Selected,
		const ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSelectionChanged_DamageType(const TArray<FString>& Selected, const ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSelectionChanged_HyperParameterMode(const TArray<FString>& Selected, const ESelectInfo::Type SelectionType);

	FString GetComboBoxEntryTooltipStringTableKey_TargetActivationSelectionPolicy(const FString& EnumString);

	FString GetComboBoxEntryTooltipStringTableKey_DamageType(const FString& EnumString);

	FString GetComboBoxEntryTooltipStringTableKey_HyperParameterMode(const FString& EnumString);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_SpawnBeatDelay;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_TargetSpawnCD;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_RecentTargetMemoryPolicy;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_MaxNumRecentTargets;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_RecentTargetTimeLength;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* CheckBoxOption_EnableAI;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_HyperParameterMode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_Alpha;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_Epsilon;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_Gamma;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UToggleableSingleRangeInputWidget* MenuOption_TargetLifespan;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UToggleableSingleRangeInputWidget* MenuOption_TargetHealth;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_ExpirationHealthPenalty;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_DeactivationHealthLostThreshold;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_DamageType;
};
