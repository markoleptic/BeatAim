// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "AbilitySystem/Tasks/BSAT_TickTrace.h"
#include "AbilitySystemComponent.h"
#include "Character/BSCharacterBase.h"
#include "Character/BSRecoilComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Physics/BSCollisionChannels.h"

UBSAT_TickTrace::UBSAT_TickTrace(): Character(nullptr), bStopWhenAbilityEnds(false)
{
	bTickingTask = true;
}

void UBSAT_TickTrace::Activate()
{
	if (!Ability)
	{
		return;
	}

	CancelledHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UBSAT_TickTrace::OnAbilityCancelled);
}

void UBSAT_TickTrace::ExternalCancel()
{
	OnAbilityCancelled();
	Super::ExternalCancel();
}

void UBSAT_TickTrace::OnDestroy(bool AbilityEnded)
{
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
	}
	Super::OnDestroy(AbilityEnded);
}

void UBSAT_TickTrace::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	PerformSingleWeaponTrace();
}

UBSAT_TickTrace* UBSAT_TickTrace::SingleWeaponTrace(UGameplayAbility* OwningAbility, const FName TaskInstanceName,
	ABSCharacterBase* Character, const float TraceDistance, const bool bStopWhenAbilityEnds)
{
	UBSAT_TickTrace* MyObj = NewAbilityTask<UBSAT_TickTrace>(OwningAbility, TaskInstanceName);
	MyObj->Character = Character;
	MyObj->TraceDistance = TraceDistance;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
	return MyObj;
}

void UBSAT_TickTrace::PerformSingleWeaponTrace()
{
	FHitResult HitResult;
	if (!Character)
	{
		return;
	}

	const USceneComponent* RecoilComponent = Character->GetRecoilComponent();
	const FRotator CurrentRecoilRotation = Character->GetRecoilComponent()->GetCurrentRecoilRotation();

	const FVector RotatedVector1 = UKismetMathLibrary::RotateAngleAxis(RecoilComponent->GetForwardVector(),
		CurrentRecoilRotation.Pitch, RecoilComponent->GetRightVector());
	const FVector RotatedVector2 = UKismetMathLibrary::RotateAngleAxis(RotatedVector1, CurrentRecoilRotation.Yaw,
		RecoilComponent->GetUpVector());
	const FVector EndTrace = RecoilComponent->GetComponentLocation() + RotatedVector2 * TraceDistance;
	const FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, Character);
	GetWorld()->LineTraceSingleByChannel(HitResult, RecoilComponent->GetComponentLocation(), EndTrace,
		BS_TraceChannel_Weapon, TraceParams);

	OnTickTraceHit.Broadcast(HitResult);
}

void UBSAT_TickTrace::OnAbilityCancelled()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
	}
}
