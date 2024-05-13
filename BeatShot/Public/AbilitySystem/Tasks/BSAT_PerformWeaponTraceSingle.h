// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "BSAT_PerformWeaponTraceSingle.generated.h"

class UBSRecoilComponent;
class UBSGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPerformWeaponTraceDelegate, const bool, bSuccess, const FHitResult&,
	HitResult);

UCLASS()
class BEATSHOT_API UBSAT_PerformWeaponTraceSingle : public UAbilityTask
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	UBSAT_PerformWeaponTraceSingle();

	/** Calls LineTraceSingle and broadcasts delegate */
	virtual void Activate() override;

	/** Broadcasts delegate */
	virtual void ExternalCancel() override;

	/** Broadcast when the task has obtained a HitResult, or was cancelled. */
	UPROPERTY(BlueprintAssignable)
	FPerformWeaponTraceDelegate OnCompleted;

	/**
	 * If a BSCharacter is the found on the OwningAbility, a Line Trace is performed starting at the Recoil Component's transform. Traces forward by TraceDistance.
	 * @param OwningAbility The owning ability of this task
	 * @param TaskInstanceName Task instance name
	 * @param TraceDistance How far to trace the line forward
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UBSAT_PerformWeaponTraceSingle* PerformWeaponTraceSingle(UBSGameplayAbility* OwningAbility,
		const FName TaskInstanceName, const float TraceDistance);

	/** Performs actual LineTraceSingleByChannel, returning true on success */
	bool LineTraceSingle(FHitResult& HitResult) const;

private:
	/** How far to trace forward from Character camera */
	float TraceDistance = 100000.f;
};
