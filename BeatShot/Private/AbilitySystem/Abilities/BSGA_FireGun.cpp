// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "AbilitySystem/Abilities/BSGA_FireGun.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tasks/BSAT_PerformWeaponTraceSingle.h"
#include "Character/BSCharacter.h"

UBSGA_FireGun::UBSGA_FireGun()
{
	FireHipMontage = nullptr;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UBSGA_FireGun::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* Component = CurrentActorInfo->AbilitySystemComponent.Get();
	OnTargetDataReadyCallbackDelegateHandle = Component->AbilityTargetDataSetDelegate(CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UBSGA_FireGun::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo,
				ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}

		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyAbilityComponent);

		// When ability ends, consume target data and remove delegate
		MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle,
			CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle,
			CurrentActivationInfo.GetActivationPredictionKey());

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UBSGA_FireGun::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData,
	FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	if (MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow ScopedPrediction(MyAbilityComponent);

		// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
		const FGameplayAbilityTargetDataHandle LocalTargetDataHandle(
			MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
		if (bShouldNotifyServer)
		{
			MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle,
				CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag,
				MyAbilityComponent->ScopedPredictionKey);
		}

		if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			// Let the blueprint do stuff like apply effects to the targets
			OnTargetDataReady(LocalTargetDataHandle);
		}
		else
		{
			K2_EndAbility();
		}
	}

	// We've processed the data
	MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey());
}

void UBSGA_FireGun::StartTargeting()
{
	const auto Trace = UBSAT_PerformWeaponTraceSingle::PerformWeaponTraceSingle(this, FName(), TraceDistance);
	Trace->OnCompleted.AddDynamic(this, &ThisClass::OnSingleWeaponTraceCompleted);

	UAbilitySystemComponent* Component = CurrentActorInfo->AbilitySystemComponent.Get();
	FScopedPredictionWindow ScopedPrediction(Component, CurrentActivationInfo.GetActivationPredictionKey());

	Trace->ReadyForActivation();
}

void UBSGA_FireGun::OnSingleWeaponTraceCompleted(const bool bSuccess, const FHitResult& HitResult)
{
	if (!bSuccess)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
	}

	FGameplayAbilityTargetData_SingleTargetHit* SingleTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
	SingleTargetData->HitResult = HitResult;
	const FGameplayAbilityTargetDataHandle TargetData(SingleTargetData);

	OnTargetDataReadyCallback(TargetData, FGameplayTag());
}
