// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "AbilitySystem/Tasks/BSAT_AimToTarget.h"
#include "AbilitySystem/BSAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/BSCharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Target/Target.h"

UBSAT_AimToTarget::UBSAT_AimToTarget(): Camera(nullptr)
{
	bTickingTask = true;
}

UBSAT_AimToTarget* UBSAT_AimToTarget::AimToTarget(UBSGameplayAbility* OwningAbility, FName TaskInstanceName,
	UCurveFloat* RotationCurve, ATarget* TargetToDestroy, float TimelinePlaybackRate)
{
	UBSAT_AimToTarget* MyObj = NewAbilityTask<UBSAT_AimToTarget>(OwningAbility, TaskInstanceName);

	MyObj->Controller = OwningAbility->GetControllerFromActorInfo();
	MyObj->Camera = OwningAbility->GetBSCharacterFromActorInfo()->GetCamera();
	MyObj->Target = TargetToDestroy;
	MyObj->RotationCurve = RotationCurve;

	MyObj->OnTimelineFloat.BindUFunction(MyObj, FName("OnTimelineTick"));
	MyObj->OnTimelineEvent.BindUFunction(MyObj, FName("OnTimelineCompleted"));

	MyObj->AimBotTimeline.AddInterpFloat(RotationCurve, MyObj->OnTimelineFloat);
	MyObj->AimBotTimeline.SetTimelineFinishedFunc(MyObj->OnTimelineEvent);
	MyObj->AimBotTimeline.SetPlayRate(TimelinePlaybackRate);

	MyObj->CancelledHandle = OwningAbility->OnGameplayAbilityCancelled.AddUObject(MyObj,
		&UBSAT_AimToTarget::OnAbilityCancelled);

	return MyObj;
}

void UBSAT_AimToTarget::Activate()
{
	AimBotTimeline.PlayFromStart();
	Super::Activate();
}

void UBSAT_AimToTarget::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	if (TaskState == EGameplayTaskState::Active)
	{
		AimBotTimeline.TickTimeline(DeltaTime);
	}
}

void UBSAT_AimToTarget::ExternalCancel()
{
	OnAbilityCancelled();
	Super::ExternalCancel();
}

void UBSAT_AimToTarget::OnDestroy(bool AbilityEnded)
{
	AimBotTimeline.Stop();

	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
	}

	OnCompleted.Clear();
	OnCancelled.Clear();

	Super::OnDestroy(AbilityEnded);
}

void UBSAT_AimToTarget::OnAbilityCancelled()
{
	AimBotTimeline.Stop();

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCancelled.Broadcast();
	}

	OnCompleted.Clear();
	OnCancelled.Clear();
}

void UBSAT_AimToTarget::OnTimelineTick(const float Alpha) const
{
	const FVector Loc = Camera->GetComponentLocation();
	const FRotator Rot = Controller->GetControlRotation();
	Controller->SetControlRotation(UKismetMathLibrary::RLerp(Rot,
		UKismetMathLibrary::FindLookAtRotation(Loc, Target->GetActorLocation()), Alpha, true));
}

void UBSAT_AimToTarget::OnTimelineCompleted()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCompleted.Broadcast();
	}
	EndTask();
}
