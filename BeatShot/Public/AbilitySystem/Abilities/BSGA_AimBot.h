// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSGameplayAbility.h"
#include "BSGA_AimBot.generated.h"

class ATarget;

UCLASS()
class BEATSHOT_API UBSGA_AimBot : public UBSGameplayAbility
{
	GENERATED_BODY()

public:
	UBSGA_AimBot();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Bound to TargetSpawner's OnTargetActivated delegate. */
	UFUNCTION()
	void OnTargetActivated(ATarget* SpawnedTarget);

	/** Sets the IgnoreStartLocation and the bool variables associated with it. */
	void SetIgnoreStartLocation(const FVector& In);

	/** The Aim Bot will ignore targets with a location greater than a positive value or less than a negative value. */
	UPROPERTY(EditDefaultsOnly, Category="BeatShot|AimBot")
	FVector IgnoreStartLocation = FVector::ZeroVector;

	/** The interpolation curve to use during AimToTarget task. */
	UPROPERTY(EditDefaultsOnly, Category="BeatShot|AimBot")
	UCurveFloat* SmoothingCurve;

	/** The ability to try and activate to destroy the target. */
	UPROPERTY(EditDefaultsOnly, Category="BeatShot")
	TSubclassOf<UBSGameplayAbility> GA_FireGun;

private:
	/** A queue of target locations that have not yet been destroyed. */
	TQueue<ATarget*> ActiveTargets_AimBot;

	/** Checks if there are any targets available in the Character's target queue, and if so creates a task to destroy
	 *  it. */
	void CheckTargetQueue();

	/** Callback for when the AimToTarget task is cancelled. */
	UFUNCTION()
	void OnAimToTargetCancelled();

	/** Callback for when the AimToTarget task is completed. */
	UFUNCTION()
	void OnAimToTargetCompleted();

	/** Checks if the location is within the range of ignorable locations. */
	bool TargetLocationIsInIgnoreRange(const FVector& Loc) const;

	bool bYPositive;
	bool bYNegative;
	bool bYZero;

	bool bZPositive;
	bool bZNegative;
	bool bZZero;
};
