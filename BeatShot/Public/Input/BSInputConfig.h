// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "InputTriggers.h"
#include "Containers/Array.h"
#include "Engine/DataAsset.h"
#include "BSInputConfig.generated.h"

class UPlayerMappableInputConfig;
class UInputAction;
class UObject;
struct FFrame;

/**
 *	Struct used to map a input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct FBSInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "TriggerEvent"))
	ETriggerEvent PressedTriggerEvent = ETriggerEvent::Triggered;
};

/**
 *	Non-mutable data asset that contains input configuration properties.
 */
UCLASS(BlueprintType, Const)
class UBSInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UBSInputConfig();

	UFUNCTION(BlueprintCallable, Category = "BeatShot|Input")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable, Category = "BeatShot|Input")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable, Category = "BeatShot|Input")
	FBSInputAction FindBSInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	/** List of input actions used by the owner.  These input actions are mapped to a gameplay tag and must be manually
	 *  bound. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FBSInputAction> NativeInputActions;

	/** List of input actions used by the owner.  These input actions are mapped to a gameplay tag and are automatically
	 *  bound to abilities with matching input tags. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FBSInputAction> AbilityInputActions;
};
