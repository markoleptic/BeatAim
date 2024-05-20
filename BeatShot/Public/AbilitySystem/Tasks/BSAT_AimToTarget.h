// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Components/TimelineComponent.h"
#include "BSAT_AimToTarget.generated.h"

class UBSRecoilComponent;
class UBSGameplayAbility;
class UBSAbilitySystemComponent;
class ATarget;
class UCurveFloat;

/** Delegate type used in the AimToTarget ability task. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAimToTargetDelegate);

/** AimToTarget task. */
UCLASS()
class BEATSHOT_API UBSAT_AimToTarget : public UAbilityTask
{
	GENERATED_BODY()

public:
	UBSAT_AimToTarget();

	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual void OnDestroy(bool AbilityEnded) override;
	virtual void TickTask(float DeltaTime) override;

	/** Broadcast when the task has finished aiming. */
	UPROPERTY(BlueprintAssignable)
	FAimToTargetDelegate OnCompleted;

	/** Broadcast if the ability task was cancelled or interrupted. */
	UPROPERTY(BlueprintAssignable)
	FAimToTargetDelegate OnCancelled;

	/**
	 * Linearly interpolate towards the TargetToDestroy starting from Controller's current rotation when passed in,
	 * and wait for it to finish. Need to call ReadyForActivation if calling in C++
	 *
	 * @param OwningAbility The owning ability of this task
	 * @param TaskInstanceName Task instance name
	 * @param RotationCurve The float curve to apply to the linear interpolation
	 * @param TargetToDestroy The target to aim to
	 * @param TimelinePlaybackRate Speed of the interpolation, will use 1 / SpawnBeatDelay if not specified
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UBSAT_AimToTarget* AimToTarget(UBSGameplayAbility* OwningAbility, FName TaskInstanceName,
		UCurveFloat* RotationCurve, ATarget* TargetToDestroy, float TimelinePlaybackRate = -1.f);

private:
	/** Float interpolation function bound to AimBotTimeline, which sets the rotation of the camera. */
	UFUNCTION()
	void OnTimelineTick(const float Alpha) const;

	/** Executes when the AimBotTimeline completes. */
	UFUNCTION()
	void OnTimelineCompleted();

	/** Function called when owning ability is cancelled. */
	void OnAbilityCancelled();

	UPROPERTY()
	TObjectPtr<AController> Controller;

	UPROPERTY()
	class UCameraComponent* Camera;

	UPROPERTY()
	TObjectPtr<UCurveFloat> RotationCurve;

	UPROPERTY()
	TObjectPtr<ATarget> Target;

	/** Timeline for interpolating the rotation to aim at the target to destroy. */
	FTimeline AimBotTimeline;

	/** Delegate bound to Ability's cancel. */
	FDelegateHandle CancelledHandle;

	/** Delegate executed on timeline tick. */
	FOnTimelineFloat OnTimelineFloat;

	/** Delegate for when the timeline is completed. */
	FOnTimelineEvent OnTimelineEvent;
};
