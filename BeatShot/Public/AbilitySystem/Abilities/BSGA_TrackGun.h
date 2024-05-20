// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSGameplayAbility.h"
#include "BSGA_TrackGun.generated.h"

class UBSAT_TickTrace;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerStopTrackingTarget);

/** GameplayAbility that provides the user with constant tracking from their gun barrel, used in BeatTrack game
 *  modes. */
UCLASS()
class BEATSHOT_API UBSGA_TrackGun : public UBSGameplayAbility
{
	GENERATED_BODY()

public:
	UBSGA_TrackGun();

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnPlayerStopTrackingTarget OnPlayerStopTrackingTarget;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** Performs non-gameplay related tasks like muzzle flash, camera recoil, and decal spawning. */
	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

	/** Calls OnTargetDataReady. */
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	UFUNCTION()
	void OnTickTraceHitResultHit(const FHitResult& HitResult);

	/** How far to trace forward from Character camera. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float TraceDistance = 100000.f;

private:
	TObjectPtr<UBSAT_TickTrace> TickTraceTask;

	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
};
