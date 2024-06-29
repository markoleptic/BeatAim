// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Animation/AnimInstance.h"
#include "BSAnimInstance.generated.h"

class UBSAnimationSet;
class UAbilitySystemComponent;

/**	The base game animation instance class used by this project. */
UCLASS(Config = Game)
class UBSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UBSAnimInstance(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** Gameplay tags that can be mapped to blueprint variables. The variables will automatically update as the tags are
	 *  added or removed. These should be used instead of manually querying for the gameplay tags. */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "BeatShot|Animation")
	UBSAnimationSet* AnimationSet;

	UPROPERTY(BlueprintReadOnly, Category = "BeatShot|CharacterStateData")
	float GroundDistance = -1.0f;
};
