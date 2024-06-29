// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpec.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Containers/Array.h"
#include "Engine/DataAsset.h"
#include "BSAbilitySet.generated.h"

class UBSGameplayAbility;
class UGameplayEffect;
class UAttributeSet;
class UBSAbilitySystemComponent;

/** Contains the class default object for the ability, along with the level and input tag. */
USTRUCT(BlueprintType)
struct FBSAbilitySetGameplayAbility
{
	GENERATED_BODY()

	/** Gameplay ability to grant. Class default object. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UBSGameplayAbility> Ability = nullptr;

	/** Level of ability to grant. */
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	/** Tag used to process input for the ability. */
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/** Contains the gameplay effect class default object and the effect level. */
USTRUCT(BlueprintType)
struct FBSAbilitySetGameplayEffect
{
	GENERATED_BODY()

	/** Gameplay effect to grant. Class default object. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	/** Level of gameplay effect to grant. */
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

/** Contains the AttributeSet class default object */
USTRUCT(BlueprintType)
struct FBSAbilitySetAttributeSet
{
	GENERATED_BODY()

	/** Class default object for the attribute set. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;
};

/** Data used to store handles to what has been granted by the ability set. */
USTRUCT(BlueprintType)
struct FBSGrantedAbilitySet
{
	GENERATED_BODY()

	/** Adds an entry to AbilitySpecHandles. */
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);

	/** Adds an entry to GameplayEffectHandles. */
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);

	/** Adds an entry to GrantedAttributeSets. */
	void AddAttributeSet(UAttributeSet* Set);

	/** Removes all granted Gameplay Abilities, Gameplay Effects, and Attribute Sets. */
	void TakeFromAbilitySystem(UBSAbilitySystemComponent* ASC);

	/** Wrapper around ASCs FindAbilitySpecFromHandle, just returns the first one. */
	FGameplayAbilitySpec* FindAbilitySpecFromHandle(UBSAbilitySystemComponent* ASC);

	/** Returns whether all granted AbilitySpecHandles, GameplayEffectHandles,
	 *  and GrantedAttributeSets are empty. */
	bool IsEmpty() const;

protected:
	/** Handles to the granted abilities. */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	/** Handles to the granted gameplay effects. */
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	/** Pointers to the granted attribute sets */
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};


/** A data asset that contains GameplayAbilities, GameplayEffects, and AttributeSets that should be granted to the
 *  owner. */
UCLASS(BlueprintType, Const)
class UBSAbilitySet : public UDataAsset
{
	GENERATED_BODY()

public:
	UBSAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Grants the ability set to the specified ability system component. The returned handles can be used later
	 *  to take away anything that was granted. */
	void GiveToAbilitySystem(UBSAbilitySystemComponent* ASC, FBSGrantedAbilitySet* OutGrantedHandles,
		UObject* SourceObject = nullptr) const;

protected:
	/** Gameplay abilities to grant when this ability set is granted. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TArray<FBSAbilitySetGameplayAbility> GrantedGameplayAbilities;

	/** Gameplay effects to grant when this ability set is granted. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<FBSAbilitySetGameplayEffect> GrantedGameplayEffects;

	/** Attribute sets to grant when this ability set is granted. */
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets")
	TArray<FBSAbilitySetAttributeSet> GrantedAttributes;
};
