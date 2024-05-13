// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/BSGameplayAbility.h"
#include "NativeGameplayTags.h"
#include "BSAbilitySystemComponent.generated.h"

/** Base AbilitySystemComponent in this game */
UCLASS()
class BEATSHOT_API UBSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	bool bCharacterAbilitiesGiven = false;
	bool bStartupEffectsApplied = false;

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	typedef TFunctionRef<bool(const UBSGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)>
	TShouldCancelAbilityFunc;
	/** Cancels abilities based on the ShouldCancelFunc. */
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

	/** Cancels abilities that are activated by input. */
	void CancelInputActivatedAbilities(bool bReplicateCancelAbility);

	/** Should be called by the character when an ability key was pressed. */
	void AbilityInputTagPressed(FGameplayTag InputTag);

	/** Should be called by the character when an ability key was released. */
	void AbilityInputTagReleased(FGameplayTag InputTag);

	/** Tries to activate or deactivate all abilities that had their input key pressed, held, or released this frame. */
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	/** Clear the cached ability handles. */
	void ClearAbilityInput();

	/** Returns whether or not the activation group is blocked from activation. */
	bool IsActivationGroupBlocked(EBSAbilityActivationGroup Group) const;

	/** Adds an ability to activation group. */
	void AddAbilityToActivationGroup(EBSAbilityActivationGroup Group, UBSGameplayAbility* Ability);

	/** Removes an ability to activation group. */
	void RemoveAbilityFromActivationGroup(EBSAbilityActivationGroup Group, UBSGameplayAbility* Ability);

	/** Cancels abilities matching the activation group, optionally ignoring the IgnoreAbility. */
	void CancelActivationGroupAbilities(EBSAbilityActivationGroup Group, UBSGameplayAbility* IgnoreAbility,
		bool bReplicateCancelAbility);

	/** Gets the ability target data associated with the given ability handle and activation info. */
	void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle,
		FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	/** Wrapper around GetActivatableGameplayAbilitySpecsByAllMatchingTags to shorten code. */
	TArray<FGameplayAbilitySpec*> GetAbilitySpecsFromGameplayTag(const FGameplayTag& InputTag) const;

protected:
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	// Number of abilities running in each activation group.
	int32 ActivationGroupCounts[static_cast<uint8>(EBSAbilityActivationGroup::Max)];
};
