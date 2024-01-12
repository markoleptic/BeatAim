// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "BSGameModeDataAsset.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "SaveGamePlayerSettings.h"
#include "AbilitySystem/BSAbilitySystemComponent.h"
#include "AbilitySystem/Globals/BSAttributeSetBase.h"
#include "Target.generated.h"

class UProjectileMovementComponent;
class UBSHealthComponent;
class UBSAbilitySystemComponent;
class UCapsuleComponent;
class UTimelineComponent;
class UNiagaraSystem;
class UCurveFloat;
class ATarget;

/** Struct containing info about a target that is broadcast when a target takes damage
 *  or the the ExpirationTimer timer expires */
USTRUCT()
struct FTargetDamageEvent
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* DamageCauser;

	/** Whether or not the target's lifetime expired, causing it to damage itself */
	bool bDamagedSelf;

	/** Whether or not the target had zero health as a result of the damage event */
	bool bOutOfHealth;

	/** Whether or not the target will deactivate itself and handle deactivation responses shortly after the damage
	 *  event is broadcast */
	bool bWillDeactivate;

	/** Whether or not the target will destroy itself shortly after the damage event is broadcast */
	bool bWillDestroy;

	/** The type of damage that was used to cause damage */
	ETargetDamageType DamageType;

	/** Any array of DamageTypes that the target was vulnerable to when the damage event occured */
	TArray<ETargetDamageType> VulnerableToDamageTypes;

	/** A unique ID for the target, used to find the target when it comes time to free the blocked points of a target */
	FGuid Guid;

	/** The health attribute's NewValue */
	float CurrentHealth;

	/** The absolute value between the health attribute's NewValue and OldValue */
	float DamageDelta;
	
	/** The time the target was alive for before the damage event, or -1 if expired/damaged self */
	float TimeAlive;

	/** The total amount of tracking damage possible at the time of the damage event. Updated in SetTargetManagerData */
	double TotalPossibleTrackingDamage;
	
	/** The transform of the target */
	FTransform Transform;

	/** The amount of targets damaged in a row without missing. Updated in SetTargetManagerData */
	int32 Streak;

	FTargetDamageEvent()
	{
		DamageCauser = nullptr;
		bDamagedSelf = false;
		bOutOfHealth = false;
		bWillDeactivate = false;
		bWillDestroy = false;

		DamageType = ETargetDamageType::None;
		VulnerableToDamageTypes = TArray<ETargetDamageType>();

		CurrentHealth = 0.f;
		DamageDelta = 0.f;
		TimeAlive = -1.f;
		TotalPossibleTrackingDamage = 0.f;
		
		Transform = FTransform();
		
		Streak = -1;
	}

	FTargetDamageEvent(const float InTimeAlive, const float OldValue, const float NewValue, const FTransform& InTransform,
		const FGuid& InGuid, const ETargetDamageType InDamageType)
	{
		DamageCauser = nullptr;
		bDamagedSelf = InDamageType == ETargetDamageType::Self;
		bOutOfHealth = NewValue <= 0.f;
		DamageType = InDamageType;
		Guid = InGuid;
		CurrentHealth = NewValue;
		DamageDelta = abs(OldValue - NewValue);
		TimeAlive = InDamageType == ETargetDamageType::Self ? -1.f : InTimeAlive; // Override to -1 if damaged self
		Transform = InTransform;

		// Variables not changed on construction
		bWillDeactivate = false;
		bWillDestroy = false;
		VulnerableToDamageTypes = TArray<ETargetDamageType>();
		TotalPossibleTrackingDamage = 0.f;
		Streak = -1;
	}

	/** Called by the Target to set data that only it will have access to */
	void SetTargetData(const bool bDeactivate, const bool bDestroy, const TArray<ETargetDamageType>& InTypes)
	{
		bWillDeactivate = bDeactivate;
		bWillDestroy = bDestroy;
		VulnerableToDamageTypes = InTypes;
	}

	/** Called by the TargetManager to set data that only it will have access to */
	void SetTargetManagerData(const int32 InStreak, const float InTotalPossibleTrackingDamage)
	{
		TotalPossibleTrackingDamage = InTotalPossibleTrackingDamage;
		Streak = InStreak;
	}

	FORCEINLINE bool operator ==(const FTargetDamageEvent& Other) const
	{
		if (Guid == Other.Guid)
		{
			return true;
		}
		return false;
	}
};

/** Broadcast when a target takes damage or the the ExpirationTimer timer expires */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetDamageEvent, FTargetDamageEvent, TargetDamageEvent);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDeactivationResponse_ChangeDirection, ATarget* InTarget,
	const uint8 InSpawnActivationDeactivation);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeactivationResponse_Reactivate, ATarget* Target);

/** Base target class for this game that is mostly self-managed. TargetManager is responsible for spawning,
 *  but the lifetime is mostly controlled by parameters passed to it */
UCLASS()
class BEATSHOT_API ATarget : public AActor, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

	friend class ATargetManager;

protected:
	UPROPERTY()
	UBSAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	const UBSAttributeSetBase* AttributeSetBase;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Properties")
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Properties")
	UStaticMeshComponent* SphereMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Properties")
	UBSHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, Category = "Target Properties")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Properties")
	UNiagaraSystem* TargetExplosion;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Properties")
	TSubclassOf<UGameplayEffect> GE_TargetImmunity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Properties")
	TSubclassOf<UGameplayEffect> GE_HitImmunity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Properties")
	TSubclassOf<UGameplayEffect> GE_TrackingImmunity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Properties")
	TSubclassOf<UGameplayEffect> GE_ExpirationHealthPenalty;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Properties")
	TSubclassOf<UGameplayEffect> GE_ResetHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Target Properties")
	UCurveFloat* StartToPeakCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Target Properties")
	UCurveFloat* PeakToEndCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Target Properties")
	UCurveFloat* ShrinkQuickAndGrowSlowCurve;

	UPROPERTY()
	UMaterialInstanceDynamic* TargetColorChangeMaterial;

public:
	ATarget();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:
	virtual void Tick(float DeltaSeconds) override;

	/** Called in TargetManager to initialize the target */
	virtual void Init(const FBS_TargetConfig& InTargetConfig);

	UFUNCTION()
	void OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	/** Called by TargetManager if settings were changed that could affect the target */
	void UpdatePlayerSettings(const FPlayerSettings_Game& InPlayerSettings);

	/* ~Begin IAbilitySystemInterface */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	/* ~End IAbilitySystemInterface */

	/** Applies the GE_TargetImmunity gameplay effect to the target */
	void ApplyImmunityEffect();

	/** Applies the GE_TargetImmunity gameplay effect to the target */
	void RemoveImmunityEffect();

	/** Called when a gameplay effect is blocked because of immunity */
	void OnImmunityBlockGameplayEffect(const FGameplayEffectSpec& Spec, const FActiveGameplayEffect* Effect);

	/* ~Begin IGameplayTagAssetInterface */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	/* ~End IGameplayTagAssetInterface */

	/** Removes a gameplay tag from AbilitySystemComponent */
	void RemoveGameplayTag(FGameplayTag TagToRemove) const;

	/** Broadcast when the target needs a new direction because of a deactivation response */
	FOnDeactivationResponse_ChangeDirection OnDeactivationResponse_ChangeDirection;

	/** Broadcast when the target needs to immediately reactivate */
	FOnDeactivationResponse_Reactivate OnDeactivationResponse_Reactivate;

protected:
	/** Called from HealthComponent when a target receives damage. Main Deactivation and Destruction handler */
	virtual void OnIncomingDamageTaken(const FDamageEventData& InData);

	/** Callback function for when ExpirationTimer timer expires */
	UFUNCTION()
	virtual void OnLifeSpanExpired();

	/** Apply damage to self using a GE, for example when the ExpirationTimer timer expires */
	void DamageSelf();

	/** Reset the health of the target using a GE */
	void ResetHealth();

public:
	/** Starts the ExpirationTimer timer and starts playing StartToPeakTimeline if Lifespan > 0 */
	virtual bool ActivateTarget(const float Lifespan);

protected:
	/** Calls StopAllTimelines, sets TargetScale_Deactivation, and calls HandleDeactivationResponses */
	virtual void HandleDeactivation(const bool bExpired, const bool bOutOfHealth, const bool bWillDestroy);

	/** Returns true if the target should be deactivated based on TargetDeactivationConditions */
	virtual bool ShouldDeactivate(const bool bExpired, const float CurrentHealth) const;

	/** Performs any responses to the target being deactivated */
	virtual void HandleDeactivationResponses(const bool bExpired);

	/** Returns true if the target should be destroyed based on TargetDestructionConditions */
	virtual bool ShouldDestroy(const bool bExpired, const bool bOutOfHealth) const;

	/** Checks to see if ResetHealth should be called based on TargetDestructionConditions, UnlimitedHealth, and
	 *  if the target is out of health */
	void CheckForHealthReset(const bool bOutOfHealth);

	/** Play the StartToPeakTimeline, which corresponds to the StartToPeakCurve */
	UFUNCTION()
	void PlayStartToPeakTimeline();

	/** Play the PeakToEndTimeline, which corresponds to the PeakToEndCurve */
	UFUNCTION()
	void PlayPeakToEndTimeline();

	/** Quickly shrinks the target, then slowly takes it back to original size */
	UFUNCTION()
	void PlayShrinkQuickAndGrowSlowTimeline();

	/** Stops playing all timelines if any are playing */
	void StopAllTimelines();

	/** Interpolates between StartTargetColor and PeakTargetColor. This occurs between initial spawning of target,
	 *  up to SpawnBeatDelay seconds */
	UFUNCTION()
	void InterpStartToPeak(const float Alpha);

	/** Interpolates between PeakTargetColor and EndTargetColor. This occurs between SpawnBeatDelay seconds,
	 *  up to TargetMaxLifeSpan seconds */
	UFUNCTION()
	void InterpPeakToEnd(const float Alpha);

	/** Used to shrink the target quickly, and more slowly return it to it's BeatGrid size and color.
	 *  Interpolates both sphere scale and sphere color */
	UFUNCTION()
	void InterpShrinkQuickAndGrowSlow(const float Alpha);

public:
	/** Sets the color of the Base Target */
	UFUNCTION(BlueprintCallable)
	virtual void SetTargetColor(const FLinearColor& Color);

	/** Sets the color of the Target Outline */
	UFUNCTION(BlueprintCallable)
	virtual void SetTargetOutlineColor(const FLinearColor& Color);

	/** Toggles between using the BaseColor or a separate OutlineColor in the Sphere Material */
	virtual void SetUseSeparateOutlineColor(const bool bUseSeparateOutlineColor);

	/** Set the color to inactive target color */
	UFUNCTION()
	virtual void SetTargetColorToInactiveColor();

	/** Sets the velocity of the ProjectileMovementComponent by multiplying the InitialSpeed and the new direction */
	void SetTargetDirection(const FVector& NewDirection) const;

	/** Finds the direction by dividing the velocity by the initial speed. Sets initial speed equal to
	 *  NewMovingTargetSpeed and calls SetTargetDirection */
	void SetTargetSpeed(const float NewMovingTargetSpeed) const;

	/** Changes the current scale of the target */
	virtual void SetTargetScale(const FVector& NewScale) const;

	void SetTargetDamageType(const ETargetDamageType& InType);

	/** Sets whether or not the last direction change was horizontally */
	void SetLastDirectionChangeHorizontal(const bool bLastHorizontal)
	{
		bLastDirectionChangeHorizontal = bLastHorizontal;
	}

	/** Play the explosion effect at the location of target, scaled to size with the color of the target when
	 *  it was destroyed. */
	void PlayExplosionEffect(const FVector& ExplosionLocation, const float SphereRadius,
		const FLinearColor& InColorWhenDestroyed) const;

	/** Returns the color the target be after SpawnBeatDelay seconds have passed */
	UFUNCTION(BlueprintPure)
	FLinearColor GetPeakTargetColor() const;

	/** Returns the color that the target should change to at the end of it's life */
	UFUNCTION(BlueprintPure)
	FLinearColor GetEndTargetColor() const;

	/** Returns the color inactive target color */
	UFUNCTION(BlueprintPure)
	FLinearColor GetInActiveTargetColor() const;
	
	/** Returns the color for when the target is actively taking tracking damage */
	UFUNCTION(BlueprintPure)
	FLinearColor GetTakingTrackingDamageColor() const;

	/** Returns the color for when the target is not actively taking tracking damage */
	UFUNCTION(BlueprintPure)
	FLinearColor GetNotTakingTrackingDamageColor() const;

	/** Returns the generated Guid for this target */
	FGuid GetGuid() const { return Guid; }

	/** Returns whether or not the target has been activated before */
	bool HasBeenActivatedBefore() const;

	/** Returns whether or not the target is currently activated */
	bool IsActivated() const;

	/** Returns whether or not the target can be reactivated */
	bool CanBeReactivated() const;

	/** Whether or not the target is immune to all damage */
	bool IsImmuneToDamage() const;

	/** Whether or not the target is immune to hit damage */
	bool IsImmuneToHitDamage() const;

	/** Whether or not the target is immune to tracking damage */
	UFUNCTION(BlueprintPure)
	bool IsImmuneToTrackingDamage() const;
	
	ETargetDamageType GetTargetDamageType() const;

	/** Returns the velocity / speed of the ProjectileMovementComponent (unit direction vector) */
	FVector GetTargetDirection() const;

	/** Returns the scale of the target when it was last activated, or the spawn scale if it has not been activated */
	FVector GetTargetScale_Activation() const;

	/** Returns the current scale of the target */
	FVector GetTargetScale_Current() const;

	/** Returns the scale of the target when it was last deactivated, falling back to activation or spawn scale if not deactivated yet */
	FVector GetTargetScale_Deactivation() const;

	/** Returns the scale of the target when it was spawned */
	FVector GetTargetScale_Spawn() const;

	/** Returns the location of the target when it was activated, falling back to spawn location if not activated yet */
	FVector GetTargetLocation_Activation() const;

	/** Returns the location of the target when it was spawned */
	FVector GetTargetLocation_Spawn() const;

	/** Returns the InitialSpeed of the ProjectileMovementComponent */
	float GetTargetSpeed() const;

	/** Returns the velocity of the ProjectileMovementComponent */
	FVector GetTargetVelocity() const;

	/** Returns whether or not the last direction change was horizontally */
	bool GetLastDirectionChangeHorizontal() const { return bLastDirectionChangeHorizontal; }

	/** Mainly used so BSAT_AimBot can access SpawnBeatDelay easily */
	float GetSpawnBeatDelay() const;

	/** Broadcast when a target takes damage or the the ExpirationTimer timer expires */
	FOnTargetDamageEvent OnTargetDamageEvent;

protected:
	/** Guid to keep track of a target's properties after it has been destroyed */
	UPROPERTY()
	FGuid Guid;

	/** Locally stored BSConfig to access GameMode properties without storing ref to game instance */
	UPROPERTY()
	FBS_TargetConfig Config;

	/** Timer to track the length of time the target has been damageable for */
	UPROPERTY()
	FTimerHandle ExpirationTimer;

	FTimeline StartToPeakTimeline;
	FTimeline PeakToEndTimeline;
	FTimeline ShrinkQuickAndGrowSlowTimeline;

	FOnTimelineFloat OnStartToPeak;
	FOnTimelineFloat OnPeakToFade;
	FOnTimelineFloat OnShrinkQuickAndGrowSlow;

	FOnTimelineEvent OnStartToPeakFinished;

	/** The world scale of the target when spawned */
	FVector TargetScale_Spawn;

	/** The world scale of the target when activated */
	FVector TargetScale_Activation;

	/** The world scale of the target when deactivated */
	FVector TargetScale_Deactivation;

	/** The location of the target when spawned */
	FVector TargetLocation_Spawn;

	/** The location of the target when activated */
	FVector TargetLocation_Activation;

	/** The color of the target when it was destroyed */
	FLinearColor ColorWhenDamageTaken;
	
	/** The type of damage this target is vulnerable to */
	ETargetDamageType TargetDamageType;

	/** The amount of health required to deactivate if a Deactivation Condition is Specific Health Amount */
	float CurrentDeactivationHealthThreshold;

	/** Playback rate for StartToPeak timeline */
	float StartToPeakTimelinePlayRate;

	/** Playback rate for PeakToEnd timeline */
	float PeakToEndTimelinePlayRate;

	/** Whether or not the last direction change was horizontally */
	bool bLastDirectionChangeHorizontal;
	
	/** Whether or not to apply the LifetimeTargetScaling Method */
	bool bApplyLifetimeTargetScaling;

	/** False if the target is currently activated */
	bool bCanBeReactivated;

	/** Whether or not the target has ever been activated */
	bool bHasBeenActivated;

	/** Whether or not the target is currently activated */
	bool bIsCurrentlyActivated;

	FActiveGameplayEffectHandle ActiveGE_TargetImmunity;
	FActiveGameplayEffectHandle ActiveGE_HitImmunity;
	FActiveGameplayEffectHandle ActiveGE_TrackingImmunity;
};
