// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeCategoryWidget.h"
#include "CustomGameModeSpawnAreaWidget.generated.h"

enum class EMenuOptionEnabledState : uint8;
class USingleRangeInputWidget;
class UTextInputWidget;
class UCheckBoxWidget;
class UComboBoxWidget;
class UDualRangeInputWidget;

struct FSpreadWidgetState
{
	EMenuOptionEnabledState MenuOptionEnabledState;
	EMenuOptionEnabledState SubWidgetEnabledState;
	FString MenuOptionTooltipKey;
	FString SubWidgetTooltipKey;
	FSpreadWidgetState() = default;

	FSpreadWidgetState(const EMenuOptionEnabledState MenuState, const EMenuOptionEnabledState SubWidgetState,
		const FString& MenuKey, const FString& SubWidgetKey) : MenuOptionEnabledState(MenuState),
		                                                       SubWidgetEnabledState(SubWidgetState),
		                                                       MenuOptionTooltipKey(MenuKey),
		                                                       SubWidgetTooltipKey(SubWidgetKey)
	{
	}

	explicit FSpreadWidgetState(const EMenuOptionEnabledState AllState) : MenuOptionEnabledState(AllState),
	                                                                      SubWidgetEnabledState(AllState),
	                                                                      MenuOptionTooltipKey(""),
	                                                                      SubWidgetTooltipKey("")
	{
	}
};

UCLASS()
class USERINTERFACE_API UCustomGameModeSpawnAreaWidget : public UCustomGameModeCategoryWidget
{
	GENERATED_BODY()

public:
	UCustomGameModeSpawnAreaWidget();

protected:
	virtual void NativeConstruct() override;
	virtual void UpdateOptionsFromConfig() override;
	void UpdateDependentOptions_TargetDistributionPolicy();
	void UpdateSpread();
	void UpdateSpreadWidgetState(UDualRangeInputWidget* Widget, const bool bTracking, const bool bHeadShotHeightOnly,
		const bool bGrid);
	void UpdateDependentOption_BoundsScalingPolicy();
	void OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value);
	void OnMinMaxValueChanged(UDualRangeInputWidget* Widget, const bool bChecked, const float Min, const float Max);

	UFUNCTION()
	void OnSelectionChanged_TargetDistributionPolicy(const TArray<FString>& Selected,
		const ESelectInfo::Type SelectionType);

	FString GetComboBoxEntryTooltipStringTableKey_TargetDistributionPolicy(const FString& EnumString);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_TargetDistributionPolicy;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_StartThreshold;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_EndThreshold;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_DecrementAmount;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_NumHorizontalGridTargets;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_NumVerticalGridTargets;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_HorizontalSpacing;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_VerticalSpacing;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UDualRangeInputWidget* MenuOption_HorizontalSpread;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UDualRangeInputWidget* MenuOption_VerticalSpread;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UDualRangeInputWidget* MenuOption_ForwardSpread;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* SliderTextBoxOption_FloorDistance;
};
