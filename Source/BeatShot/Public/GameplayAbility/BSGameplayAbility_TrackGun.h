﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSGameplayAbility.h"
#include "BSGameplayAbility_TrackGun.generated.h"

class ABSCharacter;
UCLASS()
class BEATSHOT_API UBSGameplayAbility_TrackGun : public UBSGameplayAbility
{
	GENERATED_BODY()

public:
	UBSGameplayAbility_TrackGun();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
							bool bWasCancelled) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	
	/** The damage to apply on trace hit */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Damage = 1.0f;

	/** How far to trace forward from Character camera */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float TraceDistance = 100000.f;

	/** Performs non-gameplay related tasks like muzzle flash, camera recoil, and decal spawning */
	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

	/** Calls OnTargetDataReady */
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);
	
	UFUNCTION()
	void OnTickTraceHitResultHit(const FHitResult& HitResult);

private:

	TObjectPtr<class UBSAbilityTask_TickTrace> TickTraceTask;
	
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
};