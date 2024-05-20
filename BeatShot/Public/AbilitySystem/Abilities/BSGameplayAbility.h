// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BSGameplayAbility.generated.h"

class ABSCharacterBase;
class ABSPlayerController;

/** Defines how an ability is activated. */
UENUM(BlueprintType)
enum class EBSAbilityActivationPolicy : uint8
{
	/** Try to activate the ability when the input is triggered. */
	OnInputTriggered UMETA(DisplayName="OnInputTriggered"),

	/** Continually try to activate the ability while the input is active. */
	WhileInputActive UMETA(DisplayName="WhileInputActive"),

	/** Try to activate the ability when an avatar is assigned. */
	OnSpawn UMETA(DisplayName="OnSpawn"),

	/** Always reactivate the ability when the input is triggered. \n
	 *  IMPORTANT: requires bRetriggerInstancedAbility set to true. */
	SpammableTriggered UMETA(DisplayName="SpammableTriggered"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EBSAbilityActivationPolicy, EBSAbilityActivationPolicy::OnInputTriggered,
	EBSAbilityActivationPolicy::SpammableTriggered);

/** Defines how an ability activates in relation to other abilities. */
UENUM(BlueprintType)
enum class EBSAbilityActivationGroup : uint8
{
	/** Ability runs independently of all other abilities. */
	Independent UMETA(DisplayName="Independent"),

	/** Ability is canceled and replaced by other exclusive abilities. */
	Exclusive_Replaceable UMETA(DisplayName="Exclusive_Replaceable"),

	/** Ability blocks all other exclusive abilities from activating. */
	Exclusive_Blocking UMETA(DisplayName="Exclusive_Blocking"),
	Max UMETA(Hidden)};

ENUM_RANGE_BY_FIRST_AND_LAST(EBSAbilityActivationGroup, EBSAbilityActivationGroup::Independent,
	EBSAbilityActivationGroup::Max);

/** Base GameplayAbility used for this game. */
UCLASS()
class BEATSHOT_API UBSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	friend class UBSAbilitySystemComponent;

public:
	UBSGameplayAbility();

	UFUNCTION(BlueprintPure, Category = "BeatShot|Ability")
	UBSAbilitySystemComponent* GetBSAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "BeatShot|Ability")
	ABSPlayerController* GetBSPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "BeatShot|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "BeatShot|Ability")
	ABSCharacterBase* GetBSCharacterFromActorInfo() const;

	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Ability Activation")
	EBSAbilityActivationPolicy ActivationPolicy;

	// Defines the relationship between this ability activating and other abilities activating.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Ability Activation")
	EBSAbilityActivationGroup ActivationGroup;

	// If an ability is marked as 'ActivateAbilityOnGranted', activate them immediately when given here
	// Epic's comment: Projects may want to initiate passives or do other "BeginPlay" type of logic here.
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	EBSAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	EBSAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
