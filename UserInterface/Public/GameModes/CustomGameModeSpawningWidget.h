// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeCategoryWidget.h"
#include "CustomGameModeSpawningWidget.generated.h"

enum class ETargetSpawningPolicy : uint8;
class USingleRangeInputWidget;
class UCheckBoxWidget;
class UComboBoxWidget;

UCLASS()
class USERINTERFACE_API UCustomGameModeSpawningWidget : public UCustomGameModeCategoryWidget
{
	GENERATED_BODY()

protected:
	UCustomGameModeSpawningWidget();

	virtual void NativeConstruct() override;

	virtual void UpdateOptionsFromConfig() override;

	void UpdateDependentOptions_TargetSpawningPolicy(const ETargetSpawningPolicy& InTargetSpawningPolicy);

	UFUNCTION()
	void OnCheckStateChanged_AllowSpawnWithoutActivation(const bool bChecked);

	UFUNCTION()
	void OnCheckStateChanged_BatchSpawning(const bool bChecked);

	UFUNCTION()
	void OnCheckStateChanged_SpawnAtOriginWheneverPossible(const bool bChecked);

	UFUNCTION()
	void OnCheckStateChanged_SpawnEveryOtherTargetInCenter(const bool bChecked);

	void OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value);

	UFUNCTION()
	void OnSelectionChanged_RuntimeTargetSpawningLocationSelectionMode(const TArray<FString>& Selected,
		const ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSelectionChanged_TargetSpawningPolicy(const TArray<FString>& Selected,
		const ESelectInfo::Type SelectionType);

	FString GetComboBoxEntryTooltipStringTableKey_TargetSpawningPolicy(const FString& EnumString);

	FString GetComboBoxEntryTooltipStringTableKey_RuntimeTargetSpawningLocationSelectionMode(const FString& EnumString);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_MaxNumTargetsAtOnce;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_TargetSpawningPolicy;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_RuntimeTargetSpawningLocationSelectionMode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* CheckBoxOption_AllowSpawnWithoutActivation;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* CheckBoxOption_BatchSpawning;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* CheckBoxOption_SpawnAtOriginWheneverPossible;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* CheckBoxOption_SpawnEveryOtherTargetInCenter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_NumUpfrontTargetsToSpawn;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_NumRuntimeTargetsToSpawn;
};
