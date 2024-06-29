// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/BSAbilitySystemComponent.h"
#include "AbilitySystem/Globals/BSAttributeSetBase.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "Target.generated.h"

struct FPlayerSettings_Game;
class UProjectileMovementComponent;
class UBSHealthComponent;
class UBSAbilitySystemComponent;
class UCapsuleComponent;
class UTimelineComponent;
class UNiagaraSystem;
class UCurveFloat;
class ATarget;

/** Struct containing info about a target that is broadcast when a target takes damage or the ExpirationTimer timer
 *  expires. */
USTRUCT()
struct FTargetDamageEvent
{
	GENERATED_BODY()

	/** The target associated with this damage event. */
	UPROPERTY()
	ATarget* Target;

	/** The physical actor tied to this damage event. */
	UPROPERTY()
	AActor* DamageCauser;

	/** whether the target's lifetime expired, causing it to damage itself. */
	bool bDamagedSelf;

	/** whether the target had zero health as a result of the damage event. */
	bool bOutOfHealth;

	/** whether the target will deactivate itself and handle deactivation responses shortly after the damage
	 *  event is broadcast. */
	bool bWillDeactivate;

	/** whether the target will destroy itself shortly after the damage event is broadcast. */
	bool bWillDestroy;

	/** The type of damage that was used to cause damage. */
	ETargetDamageType DamageType;

	/** Any array of DamageTypes that the target was vulnerable to when the damage event occured. */
	TArray<ETargetDamageType> VulnerableToDamageTypes;

	/** A unique ID for the target, used to find the target when it comes time to free the blocked points of a
	 *  target. */
	FGuid Guid;

	/** The health attribute's NewValue. */
	float CurrentHealth;

	/** The absolute value between the health attribute's NewValue and OldValue. */
	float DamageDelta;

	/** The time the target was alive for before the damage event, or -1 if expired/damaged self. */
	float TimeAlive;

	/** The total amount of tracking damage possible at the time of the damage event. Updated in
	 *  SetTargetManagerData. */
	double TotalPossibleTrackingDamage;

	/** The amount of health required to deactivate if a Deactivation Condition is Specific Health Amount. */
	float CurrentDeactivationHealthThreshold;

	/** The transform of the target. */
	FTransform Transform;

	/** The amount of targets damaged in a row without missing. Updated in SetTargetManagerData. */
	int32 Streak;

	FTargetDamageEvent()
	{
		Target = nullptr;
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
		CurrentDeactivationHealthThreshold = 0.f;

		Transform = FTransform();

		Streak = -1;
	}

	FTargetDamageEvent(const FDamageEventData& InData, const float InTimeAlive, ATarget* InTarget);

	/** Called by the Target to set data that only it will have access to. */
	void SetTargetData(const float InCurrentDeactivationHealthThreshold, const TArray<ETargetDamageType>& InTypes);

	/** Called by the TargetManager to set data that only it will have access to. */
	void SetTargetManagerData(const bool bDeactivate, const bool bDestroy, const int32 InStreak,
		const float InTotalPossibleTrackingDamage);

	FORCEINLINE bool operator ==(const FTargetDamageEvent& Other) const
	{
		if (Guid == Other.Guid)
		{
			return true;
		}
		return false;
	}
};

/** Broadcast when a target takes damage or the ExpirationTimer timer expires. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetDamageEvent, FTargetDamageEvent& TargetDamageEvent);

/** Base target class for this game that is mostly self-managed. TargetManager is responsible for spawning,
 *  but the lifetime is mostly controlled by parameters passed to it. */
UCLASS()
class BEATSHOT_API ATarget : public AActor, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

	friend class ATargetManager;
	friend class ABeatShotGameModeFunctionalTest;
	friend class FTargetCollisionTest;

protected:
	UPROPERTY()
	UBSAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	const UBSAttributeSetBase* AttributeSetBase;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	UStaticMeshComponent* SphereMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	UBSHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, Category = "BeatShot|Components")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Effects")
	UNiagaraSystem* TargetExplosion;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Classes")
	TSubclassOf<UGameplayEffect> GE_TargetImmunity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Classes")
	TSubclassOf<UGameplayEffect> GE_HitImmunity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Classes")
	TSubclassOf<UGameplayEffect> GE_TrackingImmunity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Classes")
	TSubclassOf<UGameplayEffect> GE_ExpirationHealthPenalty;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Classes")
	TSubclassOf<UGameplayEffect> GE_ResetHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Constants")
	FName MaterialParameterColorName = "BaseColor";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Constants")
	FName TargetExplosionSphereRadiusParameterName = "SphereRadius";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Constants")
	FName TargetExplosionColorParameterName = "SphereColor";

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Curves")
	UCurveFloat* StartToPeakCurve;

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Curves")
	UCurveFloat* PeakToEndCurve;

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Curves")
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

	/** Called in TargetManager to initialize the target. */
	virtual void Init(const FBS_TargetConfig& InTargetConfig);

	UFUNCTION()
	void OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	/** Called by TargetManager if settings were changed that could affect the target. */
	void UpdatePlayerSettings(const FPlayerSettings_Game& InPlayerSettings);

	/* ~Begin IAbilitySystemInterface */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	/* ~End IAbilitySystemInterface */

	/* ~Begin IGameplayTagAssetInterface */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	/* ~End IGameplayTagAssetInterface */

	/** Applies the GE_TargetImmunity gameplay effect to the target. */
	void ApplyImmunityEffect();

	/** Removes the GE_TargetImmunity gameplay effect to the target. */
	void RemoveImmunityEffect();

	/** Called when a gameplay effect is blocked because of immunity. */
	void OnImmunityBlockGameplayEffect(const FGameplayEffectSpec& Spec, const FActiveGameplayEffect* Effect);

protected:
	/** Called from HealthComponent when a target receives damage. Main Deactivation and Destruction handler. */
	void OnIncomingDamageTaken(const FDamageEventData& InData);

	/** Callback function for when ExpirationTimer timer expires. */
	UFUNCTION()
	virtual void OnLifeSpanExpired();

	/** Apply damage to self using a GE, for example when the ExpirationTimer timer expires. */
	void DamageSelf(const bool bTreatAsExternalDamage = false);

	/** Reset the health of the target using a GE. */
	void ResetHealth();

public:
	/** Starts the ExpirationTimer timer and starts playing StartToPeakTimeline if Lifespan > 0. */
	virtual bool ActivateTarget(const float Lifespan);

	/** Calls StopAllTimelines, sets TargetScale_Deactivation. */
	void DeactivateTarget();

	/** Checks to see if ResetHealth should be called based on TargetDestructionConditions, UnlimitedHealth, and
	 *  if the target is out of health. */
	void CheckForHealthReset(const bool bOutOfHealth);

protected:
	/** Play the StartToPeakTimeline, which corresponds to the StartToPeakCurve. */
	UFUNCTION()
	void PlayStartToPeakTimeline();

	/** Play the PeakToEndTimeline, which corresponds to the PeakToEndCurve. */
	UFUNCTION()
	void PlayPeakToEndTimeline();

	/** Quickly shrinks the target, then slowly takes it back to original size. */
	UFUNCTION()
	void PlayShrinkQuickAndGrowSlowTimeline();

	/** Stops playing all timelines if any are playing. */
	void StopAllTimelines();

	/** Interpolates between StartTargetColor and PeakTargetColor. This occurs between initial spawning of target,
	 *  up to SpawnBeatDelay seconds. */
	UFUNCTION()
	void InterpStartToPeak(const float Alpha);

	/** Interpolates between PeakTargetColor and EndTargetColor. This occurs between SpawnBeatDelay seconds,
	 *  up to TargetMaxLifeSpan seconds. */
	UFUNCTION()
	void InterpPeakToEnd(const float Alpha);

	/** Used to shrink the target quickly, and more slowly return it to it's BeatGrid size and color.
	 *  Interpolates both sphere scale and sphere color. */
	UFUNCTION()
	void InterpShrinkQuickAndGrowSlow(const float Alpha);

public:
	/** Sets the color of the Base Target. */
	UFUNCTION(BlueprintCallable)
	virtual void SetTargetColor(const FLinearColor& Color);
	/** Sets the color of the Target Outline. */
	UFUNCTION(BlueprintCallable)
	void SetTargetOutlineColor(const FLinearColor& Color);

	/** Toggles between using the BaseColor or a separate OutlineColor in the Sphere Material. */
	void SetUseSeparateOutlineColor(const bool bUseSeparateOutlineColor);

	/** Set the color to inactive target color. */
	UFUNCTION()
	void SetTargetColorToInactiveColor();

	/** Sets the velocity of the ProjectileMovementComponent by multiplying the InitialSpeed and the new direction. */
	void SetTargetDirection(const FVector& NewDirection) const;

	/** Finds the direction by dividing the velocity by the initial speed. Sets initial speed equal to
	 *  NewMovingTargetSpeed and calls SetTargetDirection. */
	void SetTargetSpeed(const float NewMovingTargetSpeed) const;

	/** Changes the current scale of the target. */
	virtual void SetTargetScale(const FVector& NewScale) const;

	void SetTargetDamageType(const ETargetDamageType& InType);

	/** Sets whether the last direction change was horizontally. */
	void SetLastDirectionChangeHorizontal(const bool bLastHorizontal)
	{
		bLastDirectionChangeHorizontal = bLastHorizontal;
	}

	/** Play the explosion effect at the location of target, scaled to size with the color of the target when
	 *  it was destroyed. */
	void PlayExplosionEffect(const FVector& ExplosionLocation, const float SphereRadius,
		const FLinearColor& InColorWhenDestroyed) const;

	/** Returns the color the target be after SpawnBeatDelay seconds have passed. */
	UFUNCTION(BlueprintPure)
	FLinearColor GetPeakTargetColor() const;

	/** Returns the color that the target should change to at the end of its life. */
	UFUNCTION(BlueprintPure)
	FLinearColor GetEndTargetColor() const;

	/** Returns the color inactive target color. */
	UFUNCTION(BlueprintPure)
	FLinearColor GetInActiveTargetColor() const;

	/** Returns the color for when the target is actively taking tracking damage. */
	UFUNCTION(BlueprintPure)
	FLinearColor GetTakingTrackingDamageColor() const;

	/** Returns the color for when the target is not actively taking tracking damage. */
	UFUNCTION(BlueprintPure)
	FLinearColor GetNotTakingTrackingDamageColor() const;

	/** Returns the generated Guid for this target. */
	FGuid GetGuid() const { return Guid; }

	/** Returns the radius of the target. */
	float GetRadius() const { return GetActorScale().X * Constants::SphereTargetRadius; }

	/** Returns whether the target has been activated before. */
	bool HasBeenActivatedBefore() const;

	/** Returns whether the target is currently activated. */
	bool IsActivated() const;

	/** whether the target is immune to all damage. */
	bool IsImmuneToDamage() const;

	/** whether the target is immune to hit damage. */
	bool IsImmuneToHitDamage() const;

	/** whether the target is immune to tracking damage. */
	UFUNCTION(BlueprintPure)
	bool IsImmuneToTrackingDamage() const;

	ETargetDamageType GetTargetDamageType() const;

	/** Returns the velocity / speed of the ProjectileMovementComponent (unit direction vector). */
	FVector GetTargetDirection() const;

	/** Returns the scale of the target when it was last activated, or the spawn scale if it has not been activated. */
	FVector GetTargetScale_Activation() const;

	/** Returns the scale of the target when it was last deactivated, falling back to activation or spawn scale if not
	 *  deactivated yet. */
	FVector GetTargetScale_Deactivation() const;

	/** Returns the scale of the target when it was spawned. */
	FVector GetTargetScale_Spawn() const;

	/** Returns the location of the target when it was activated, falling back to spawn location if not activated
	 *  yet. */
	FVector GetTargetLocation_Activation() const;

	/** Returns the location of the target when it was spawned. */
	FVector GetTargetLocation_Spawn() const;

	/** Returns the InitialSpeed of the ProjectileMovementComponent. */
	float GetTargetSpeed() const;

	/** Returns the velocity of the ProjectileMovementComponent. */
	FVector GetTargetVelocity() const;

	/** Returns whether the last direction change was horizontally. */
	bool GetLastDirectionChangeHorizontal() const { return bLastDirectionChangeHorizontal; }

	/** Mainly used so BSAT_AimBot can access SpawnBeatDelay easily. */
	float GetSpawnBeatDelay() const;

	/** Broadcast when a target takes damage or the ExpirationTimer timer expires. */
	FOnTargetDamageEvent OnTargetDamageEvent;

protected:
	/** Guid to keep track of a target's properties after it has been destroyed. */
	UPROPERTY()
	FGuid Guid;

	/** Locally stored BSConfig to access GameMode properties without storing ref to game instance. */
	UPROPERTY()
	FBS_TargetConfig Config;

	/** Timer to track the length of time the target has been damageable for. */
	UPROPERTY()
	FTimerHandle ExpirationTimer;

	FTimeline StartToPeakTimeline;
	FTimeline PeakToEndTimeline;
	FTimeline ShrinkQuickAndGrowSlowTimeline;

	FOnTimelineFloat OnStartToPeak;
	FOnTimelineFloat OnPeakToFade;
	FOnTimelineFloat OnShrinkQuickAndGrowSlow;

	FOnTimelineEvent OnStartToPeakFinished;

	/** The world scale of the target when spawned. */
	FVector TargetScale_Spawn;

	/** The world scale of the target when activated. */
	FVector TargetScale_Activation;

	/** The world scale of the target when deactivated. */
	FVector TargetScale_Deactivation;

	/** The location of the target when spawned. */
	FVector TargetLocation_Spawn;

	/** The location of the target when activated. */
	FVector TargetLocation_Activation;

	/** The color of the target when it was destroyed. */
	FLinearColor ColorWhenDamageTaken;

	/** The type of damage this target is vulnerable to. */
	ETargetDamageType TargetDamageType;

	/** The amount of health required to deactivate if a Deactivation Condition is Specific Health Amount. */
	float CurrentDeactivationHealthThreshold;

	/** Playback rate for StartToPeak timeline. */
	float StartToPeakTimelinePlayRate;

	/** Playback rate for PeakToEnd timeline. */
	float PeakToEndTimelinePlayRate;

	/** whether the last direction change was horizontally. */
	bool bLastDirectionChangeHorizontal;

	/** whether to apply the LifetimeTargetScaling Method. */
	bool bApplyLifetimeTargetScaling;

	/** whether the target has ever been activated. */
	bool bHasBeenActivated;

	/** whether the target is currently activated. */
	bool bIsCurrentlyActivated;

	FActiveGameplayEffectHandle ActiveGE_TargetImmunity;
	FActiveGameplayEffectHandle ActiveGE_HitImmunity;
	FActiveGameplayEffectHandle ActiveGE_TrackingImmunity;
};
