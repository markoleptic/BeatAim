// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputMappingWidget.generated.h"

class UHorizontalBox;
class UMenuOptionStyle;
class UTextBlock;
class UInputKeySelector;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnKeySelected, const FName MappingName, const EPlayerMappableKeySlot& Slot,
	const FInputChord SelectedKey);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnIsSelectingKey, UInputKeySelector* KeySelector);

UCLASS()
class USERINTERFACE_API UInputMappingWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual void SetStyles();

public:
	void SetMappingName(const FName& InMappingName, const FText& DisplayName);

	void SetKeyForSlot(const EPlayerMappableKeySlot& InSlot, const FKey& InKey);

	/** Broadcast when a new Key has been selected from either of the InputKeySelectors. */
	FOnKeySelected OnKeySelected;

	/** Broadcast whenever the key selection mode starts or stops for either InputKeySelectors. */
	FOnIsSelectingKey OnIsSelectingKey;

	FName GetMappingName() const { return MappingName; }

	TSet<EPlayerMappableKeySlot> GetSlotsFromKey(const FKey& InKey) const;
	FInputChord GetKeyFromSlot(const EPlayerMappableKeySlot& InSlot) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category="InputMappingWidget|Style")
	TSubclassOf<UMenuOptionStyle> MenuOptionStyleClass;

	UPROPERTY()
	const UMenuOptionStyle* MenuOptionStyle;

	UFUNCTION()
	void OnIsSelectingKeyChanged_Slot1();

	UFUNCTION()
	void OnIsSelectingKeyChanged_Slot2();

	UFUNCTION()
	void OnKeySelected_Slot1(FInputChord SelectedKey);

	UFUNCTION()
	void OnKeySelected_Slot2(FInputChord SelectedKey);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* Box_Left;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_KeyDescription;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UInputKeySelector* InputKeySelectorSlot1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UInputKeySelector* InputKeySelectorSlot2;

	FName MappingName;
};
