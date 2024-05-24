// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeCategoryWidget.h"
#include "CustomGameModeMovementWidget.generated.h"

enum class ETargetDeactivationResponse : uint8;
enum class ETargetActivationResponse : uint8;
enum class ETargetSpawnResponse : uint8;

UCLASS()
class USERINTERFACE_API UCustomGameModeMovementWidget : public UCustomGameModeCategoryWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	//virtual void UpdateAllOptionsValid() override;
	virtual void UpdateOptionsFromConfig() override;
	void SetupWarningTooltipCallbacks();

	/** Updates options that depend on TargetSpawnResponses (and ConstantSpawnedTargetVelocity). */
	void UpdateDependentOptions_SpawnResponses(const TArray<ETargetSpawnResponse>& Responses, const bool bConstant);

	/** Updates options that depend on TargetActivationResponses (and ConstantActivationTargetVelocity). */
	void UpdateDependentOptions_ActivationResponses(const TArray<ETargetActivationResponse>& Responses,
		const bool bConstant);

	/** Updates options that depend on TargetDeactivationResponses (and ConstantDeactivatedTargetVelocity). */
	void UpdateDependentOptions_DeactivationResponses(const TArray<ETargetDeactivationResponse>& Responses,
		const bool bConstant);

	UFUNCTION()
	void OnSelectionChanged_MovingTargetDirectionMode(const TArray<FString>& Selected,
		const ESelectInfo::Type SelectionType);

	void OnMinMaxMenuOptionChanged(UDualRangeInputWidget* Widget, const bool bChecked, const float MinOrConstant,
		const float Max);

	FString GetComboBoxEntryTooltipStringTableKey_MovingTargetDirectionMode(const FString& EnumString);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UDualRangeInputWidget* MenuOption_ActivatedTargetVelocity;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UDualRangeInputWidget* MenuOption_DeactivatedTargetVelocity;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UDualRangeInputWidget* MenuOption_SpawnedTargetVelocity;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_MovingTargetDirectionMode;
};
