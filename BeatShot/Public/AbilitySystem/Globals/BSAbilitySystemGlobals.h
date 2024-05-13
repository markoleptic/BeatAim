// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "BSAbilitySystemGlobals.generated.h"

UCLASS()
class BEATSHOT_API UBSAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	virtual void PushCurrentAppliedGE(const FGameplayEffectSpec* Spec,
		UAbilitySystemComponent* AbilitySystemComponent) override;
	virtual void SetCurrentAppliedGE(const FGameplayEffectSpec* Spec) override;
	virtual void PopCurrentAppliedGE() override;
	const FGameplayEffectSpec* GetCurrentAppliedGE();

	static UBSAbilitySystemGlobals& GetTestGlobals();

private:
	TArray<FGameplayEffectSpec> CurrentGameplayEffectSpecStack;
};
