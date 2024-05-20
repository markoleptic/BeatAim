// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.
// Credit to Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "BSPlayerState.generated.h"

class UBSAttributeSetBase;
class UBSAbilitySystemComponent;

/** Base player state class for this game. */
UCLASS()
class BEATSHOT_API ABSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABSPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "BeatShot|PlayerState")
	UBSAbilitySystemComponent* GetBSAbilitySystemComponent() const { return AbilitySystemComponent; }

	virtual void PostInitializeComponents() override;

	const UBSAttributeSetBase* GetAttributeSetBase() const;

protected:
	UPROPERTY()
	UBSAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	const UBSAttributeSetBase* AttributeSetBase;
};
