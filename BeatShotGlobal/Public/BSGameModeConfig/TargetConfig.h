// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSConstants.h"
#include "TargetConfig.generated.h"


/** How to scale the bounding box bounds (spawn area where targets are spawned), at runtime. */
UENUM(BlueprintType)
enum class EBoundsScalingPolicy : uint8
{
	None UMETA(DisplayName="None"),
	/** The bounding box bounds size will not change throughout the game mode. */
	Static UMETA(DisplayName="Static"),
	/** The bounding box bounds size will gradually increase from half the Box Bounds up to the full size of BoxBounds,
	 *  based on consecutive targets hit. */
	Dynamic UMETA(DisplayName="Dynamic"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EBoundsScalingPolicy, EBoundsScalingPolicy::Static, EBoundsScalingPolicy::Dynamic);


/** The type of SpawnArea distribution to use (spawn area). */
UENUM(BlueprintType)
enum class ETargetDistributionPolicy : uint8
{
	None UMETA(DisplayName="None"),
	/** Only spawns targets at headshot height, so the bounding box has no height. */
	HeadshotHeightOnly UMETA(DisplayName="Headshot Height Only"),
	/** Only spawns targets on the edges of the bounding box. */
	EdgeOnly UMETA(DisplayName="Edge Only"),
	/** Spawns targets anywhere in the bounding box. */
	FullRange UMETA(DisplayName="Full Range"),
	/** Spawns targets in an evenly-spaced grid. Able to fit more targets. */
	Grid UMETA(DisplayName="Grid"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetDistributionPolicy, ETargetDistributionPolicy::HeadshotHeightOnly,
	ETargetDistributionPolicy::Grid);


/** Which direction to move a target in. */
UENUM(BlueprintType)
enum class EMovingTargetDirectionMode : uint8
{
	None UMETA(DisplayName="None"),
	/** Only move targets left and right. */
	HorizontalOnly UMETA(DisplayName="Horizontal Only"),
	/** Only move targets up and down. */
	VerticalOnly UMETA(DisplayName="Vertical Only"),
	/** Only move targets up and down. */
	AlternateHorizontalVertical UMETA(DisplayName="Alternate Horizontal Vertical"),
	/** Move targets in any direction. */
	Any UMETA(DisplayName="Any"),
	/** Only move targets forward. */
	ForwardOnly UMETA(DisplayName="Forward Only"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EMovingTargetDirectionMode, EMovingTargetDirectionMode::None,
	EMovingTargetDirectionMode::ForwardOnly);


/** How to handle changing the target scale between consecutively activated targets. */
UENUM(BlueprintType)
enum class EConsecutiveTargetScalePolicy : uint8
{
	None UMETA(DisplayName="None"),
	/** The starting scale/size of the target will remain constant throughout the game mode. */
	Static UMETA(DisplayName="Static"),
	/** The starting scale/size of the target will be chosen randomly between min and max target scale. */
	Random UMETA(DisplayName="Random"),
	/** The starting scale/size of the target will gradually shrink from max to min target scale, based on consecutive
	 *  targets hit. */
	SkillBased UMETA(DisplayName="Skill-Based")};

ENUM_RANGE_BY_FIRST_AND_LAST(EConsecutiveTargetScalePolicy, EConsecutiveTargetScalePolicy::Static,
	EConsecutiveTargetScalePolicy::SkillBased);


/** How the player damages the target and receives score. */
UENUM(BlueprintType)
enum class ETargetDamageType : uint8
{
	None UMETA(DisplayName="None"),
	/** Having the CrossHair over the target damages the target and awards score. */
	Tracking UMETA(DisplayName="Tracking"),
	/** Firing the gun and hitting the target damages the target and awards score. */
	Hit UMETA(DisplayName="Hit"),
	/** Having the CrossHair over the target and hitting the target both damage the target and award score. */
	Combined UMETA(DisplayName="Combined"),
	/** The target damages itself. */
	Self UMETA(DisplayName="Self"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetDamageType, ETargetDamageType::Tracking, ETargetDamageType::Hit);


/** When to spawn the targets. For now, limited to just all upfront or all at runtime. */
UENUM(BlueprintType)
enum class ETargetSpawningPolicy : uint8
{
	None UMETA(DisplayName="None"),
	/** Spawn all the targets before the game mode starts. */
	UpfrontOnly UMETA(DisplayName="Upfront Only"),
	/** Spawn targets when the TargetManager receives the signal from AudioAnalyzer. */
	RuntimeOnly UMETA(DisplayName="Runtime Only"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetSpawningPolicy, ETargetSpawningPolicy::UpfrontOnly,
	ETargetSpawningPolicy::RuntimeOnly);


/** Where to spawn targets if Runtime Spawning. */
UENUM(BlueprintType)
enum class ERuntimeTargetSpawningLocationSelectionMode : uint8
{
	None UMETA(DisplayName="None"),
	/** Randomly choose an available SpawnArea. */
	Random UMETA(DisplayName="Random"),
	/** Chooses a random bordering SpawnArea based on the last SPAWNED target. */
	Bordering UMETA(DisplayName="Bordering"),
	/** Chooses a random block of targets. */
	RandomGridBlock UMETA(DisplayName="RandomGridBlock"),
	/** Chooses a nearby block of targets based on the last SPAWNED target. */
	NearbyGridBlock UMETA(DisplayName="NearbyGridBlock"),
	/** Randomly choose the horizontal location, but evenly distribute vertically. */
	RandomVertical UMETA(DisplayName="RandomVertical"),
	/** Randomly choose the vertical location, but evenly distribute horizontally. */
	RandomHorizontal UMETA(DisplayName="RandomHorizontal"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ERuntimeTargetSpawningLocationSelectionMode,
	ERuntimeTargetSpawningLocationSelectionMode::Random, ERuntimeTargetSpawningLocationSelectionMode::NearbyGridBlock);


/** How to choose the target(s) to activate. */
UENUM(BlueprintType)
enum class ETargetActivationSelectionPolicy : uint8
{
	None UMETA(DisplayName="None"),
	/** Only choose spawn points that border the previous point (BeatGrid is an example). */
	Bordering UMETA(DisplayName="Bordering"),
	/** Randomly chooses a target within the available spawn points. */
	Random UMETA(DisplayName="Random"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetActivationSelectionPolicy, ETargetActivationSelectionPolicy::Bordering,
	ETargetActivationSelectionPolicy::Random);


/** Specifies the method to remove targets from recent memory, allowing targets to spawn in that location again. */
UENUM(BlueprintType)
enum class ERecentTargetMemoryPolicy : uint8
{
	/** Does not remember any recent targets */
	None UMETA(DisplayName="None"),
	/** Uses a specified time to remove recent targets, starting after the target has been destroyed or deactivated. */
	CustomTimeBased UMETA(DisplayName="Custom Time Based"),
	/** Removes a recent target after TargetSpawnCD length of time has passed since the target has been destroyed or
	 *  deactivated. */
	UseTargetSpawnCD UMETA(DisplayName="Use TargetSpawnCD"),
	/** Removes recent targets only when the number of recent targets exceeds specified capacity. */
	NumTargetsBased UMETA(DisplayName="Num Targets Based"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ERecentTargetMemoryPolicy, ERecentTargetMemoryPolicy::None,
	ERecentTargetMemoryPolicy::NumTargetsBased);


/** What the target does when it is spawned. */
UENUM(BlueprintType)
enum class ETargetSpawnResponse : uint8
{
	None UMETA(DisplayName="None"),
	/** Change the direction of the target. */
	ChangeDirection UMETA(DisplayName="Change Direction"),
	/** Change the velocity of the target according to DeactivationVelocity. */
	ChangeVelocity UMETA(DisplayName="Change Velocity"),
	/** Immunity is granted to the target. */
	AddImmunity UMETA(DisplayName="Add Immunity"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetSpawnResponse, ETargetSpawnResponse::ChangeDirection,
	ETargetSpawnResponse::AddImmunity);


/** What the target does when it is activated. */
UENUM(BlueprintType)
enum class ETargetActivationResponse : uint8
{
	None UMETA(DisplayName="None"),
	/** Immunity is removed from the target, damageable window begins if not already damageable. */
	RemoveImmunity UMETA(DisplayName="Remove Immunity"),
	/** Immunity is granted to the target. */
	AddImmunity UMETA(DisplayName="Add Immunity"),
	/** Immunity is added or removed from the target, depending on its existing state. Same with damageable window. */
	ToggleImmunity UMETA(DisplayName="Toggle Immunity"),
	/** If a moving target, the direction is changed. */
	ChangeDirection UMETA(DisplayName="Change Direction"),
	/** DEPRECATED */
	ChangeScale UMETA(DisplayName="Change Scale"),
	/** If a moving target, the velocity of the target is changed. */
	ChangeVelocity UMETA(DisplayName="Change Velocity"),
	/** The consecutive Target Scale is applied. */
	ApplyConsecutiveTargetScale UMETA(DisplayName="Apply Consecutive Target Scale"),
	/** Lifetime Target Scaling is applied throughout the target's lifetime. */
	ApplyLifetimeTargetScaling UMETA(DisplayName="Apply Lifetime Target Scaling"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetActivationResponse, ETargetActivationResponse::RemoveImmunity,
	ETargetActivationResponse::ApplyLifetimeTargetScaling);


/** Each represents one way that a target can be deactivated. */
UENUM(BlueprintType)
enum class ETargetDeactivationCondition : uint8
{
	None UMETA(DisplayName="None"),
	/** DEPRECATED */
	Persistent_DEPRECATED UMETA(DisplayName="Persistent"),
	/** Target is deactivated when it receives any damage from the player. */
	OnAnyExternalDamageTaken UMETA(DisplayName="On Any External Damage Taken"),
	/** Target is deactivated after its damageable window closes. */
	OnExpiration UMETA(DisplayName="On Expiration"),
	/** Target is deactivated when its health reaches zero. */
	OnHealthReachedZero UMETA(DisplayName="On Health Reached Zero"),
	/** Target is deactivated after it has lost a specific amount of health. */
	OnSpecificHealthLost UMETA(DisplayName="On Specific Health Lost"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetDeactivationCondition, ETargetDeactivationCondition::OnAnyExternalDamageTaken,
	ETargetDeactivationCondition::OnSpecificHealthLost);


/** What the target does when it is deactivated. */
UENUM(BlueprintType)
enum class ETargetDeactivationResponse : uint8
{
	None UMETA(DisplayName="None"),
	/** Immunity is removed from the target, damageable window begins if not already damageable. */
	RemoveImmunity UMETA(DisplayName="Remove Immunity"),
	/** Immunity is granted to the target. */
	AddImmunity UMETA(DisplayName="Add Immunity"),
	/** Immunity is added or removed from the target, depending on its existing state. Same with damageable window. */
	ToggleImmunity UMETA(DisplayName="Toggle Immunity"),
	/** The scale is set according to ConsecutiveTargetScale. LifetimeTargetScale can still override this. */
	ApplyDeactivatedTargetScaleMultiplier UMETA(DisplayName="Apply Deactivated Target Scale Multiplier"),
	/** Reset the scale of the target to the scale it was spawned with. */
	ResetScaleToSpawnedScale UMETA(DisplayName="Reset Scale To Spawned Scale"),
	/** Reset the position of the target to the position it was spawned with. */
	ResetPositionToSpawnedPosition UMETA(DisplayName="Reset Position To Spawned Position"),
	/** Reset the color of the target to the inactive color. */
	ResetColorToInactiveColor UMETA(DisplayName="Reset Color To Inactive Color"),
	/** Reset the scale of the target to the scale it was initialized with. */
	ShrinkQuickGrowSlow UMETA(DisplayName="Shrink Quick Grow Slow"),
	/** Play an explosion effect. */
	PlayExplosionEffect UMETA(DisplayName="Play Explosion Effect"),
	/** Destroy the target permanently. */
	Destroy UMETA(DisplayName="Destroy"),
	/** Hide the target. */
	HideTarget UMETA(DisplayName="Hide Target"),
	/** Change the direction of the target. */
	ChangeDirection UMETA(DisplayName="Change Direction"),
	/** Change the velocity of the target according to DeactivationVelocity. */
	ChangeVelocity UMETA(DisplayName="Change Velocity"),
	/** Reset the scale of the target to the scale it was activated with. */
	ResetScaleToActivatedScale UMETA(DisplayName="Reset Scale To Activated Scale"),
	/** Reset the position of the target to the position it was activated with. */
	ResetPositionToActivatedPosition UMETA(DisplayName="Reset Position To Activated Position"),
	/** Immediately reactivate the target. */
	Reactivate UMETA(DisplayName="Reactivate"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetDeactivationResponse, ETargetDeactivationResponse::RemoveImmunity,
	ETargetDeactivationResponse::Reactivate);


/** Each represents one way that a target can be destroyed. */
UENUM(BlueprintType)
enum class ETargetDestructionCondition : uint8
{
	None UMETA(DisplayName="None"),
	/** DEPRECATED */
	Persistent_DEPRECATED UMETA(DisplayName="Persistent"),
	/** Target is deactivated after its damageable window closes. */
	OnExpiration UMETA(DisplayName="On Expiration"),
	/** Target is destroyed when it receives any damage from the player. */
	OnAnyExternalDamageTaken UMETA(DisplayName="On Any External Damage Taken"),
	/** Target is destroyed when its health reaches zero. */
	OnHealthReachedZero UMETA(DisplayName="On Health Reached Zero"),
	/** Target is destroyed when any of its deactivation conditions are met. This essentially makes any deactivation
	 *  condition a destruction condition. */
	OnDeactivation UMETA(DisplayName="On Deactivation")};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetDestructionCondition, ETargetDestructionCondition::OnExpiration,
	ETargetDestructionCondition::OnDeactivation);


USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FBS_TargetConfig
{
	GENERATED_BODY()

	/** Whether targets can receive Activation Responses if they're already activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	bool bAllowActivationWhileActivated;

	/** If true, targets can be spawned without being activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	bool bAllowSpawnWithoutActivation;

	/** If true, spawn at the origin if it isn't blocked by a recent target whenever possible. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	bool bSpawnAtOriginWheneverPossible;

	/** If true, alternate every target spawn in the very center. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	bool bSpawnEveryOtherTargetInCenter;

	/** If true, postpones spawning target(s) until the previous target(s) have all been activated and deactivated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	bool bUseBatchSpawning;

	/** If true, use separate outline color. */
	UPROPERTY(Transient)
	bool bUseSeparateOutlineColor;

	/** How to scale the bounding box bounds (spawn area where targets are spawned), at runtime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpawnArea")
	EBoundsScalingPolicy BoundsScalingPolicy;

	/** Whether to dynamically change the size of targets as consecutive targets are hit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	EConsecutiveTargetScalePolicy ConsecutiveTargetScalePolicy;

	/** Which direction(s) to move targets. Separate from moving a target forward. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	EMovingTargetDirectionMode MovingTargetDirectionMode;

	/** Specifies the method to remove targets from recent memory, allowing targets to spawn in that location again. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	ERecentTargetMemoryPolicy RecentTargetMemoryPolicy;

	/** How to choose the target(s) to activate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	ETargetActivationSelectionPolicy TargetActivationSelectionPolicy;

	/** How the player damages the target and receives score. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	ETargetDamageType TargetDamageType;

	/** The type of SpawnArea distribution to use (spawn area). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpawnArea")
	ETargetDistributionPolicy TargetDistributionPolicy;

	/** When to spawn targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	ETargetSpawningPolicy TargetSpawningPolicy;

	/** Where to spawn targets if Runtime Spawning. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	ERuntimeTargetSpawningLocationSelectionMode RuntimeTargetSpawningLocationSelectionMode;

	/** Things a target can do when it is activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TArray<ETargetSpawnResponse> TargetSpawnResponses;

	/** The possible outcomes that a target can do when its activated: change directions, make damageable, etc. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	TArray<ETargetActivationResponse> TargetActivationResponses;

	/** Any condition that should deactivate the target (make it immune to damage). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deactivation")
	TArray<ETargetDeactivationCondition> TargetDeactivationConditions;

	/** Anything the target should do when it deactivates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deactivation")
	TArray<ETargetDeactivationResponse> TargetDeactivationResponses;

	/** Any condition that should permanently destroy a target. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Destruction")
	TArray<ETargetDestructionCondition> TargetDestructionConditions;

	/** The base damage to set the player's HitDamage Attribute to for Hit-Based damage. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float BasePlayerHitDamage;

	/** The base damage to set the player's HitDamage Attribute to for Tracking-Based damage. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float BasePlayerTrackingDamage;

	/** How much to shrink the target each time a charge is consumed, if the target is charged. This is multiplied
	 *  against the last charged target scale. A fully charged target does not receive any multiplier. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deactivation")
	float ConsecutiveChargeScaleMultiplier;

	/** The amount of health loss required for a target to deactivate if using OnSpecificHealthLost Target Deactivation
	 *  Condition. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deactivation")
	float DeactivationHealthLostThreshold;

	/** Amount of health to take away from the target if the ExpirationTimer timer expires. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deactivation")
	float ExpirationHealthPenalty;

	/** Distance from bottom of TargetManager BoxBounds to the floor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpawnArea")
	float FloorDistance;

	/** Sets the minimum distance between recent target spawns. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpawnArea")
	float MinDistanceBetweenTargets;

	/** Value to set the MaxHealth attribute value to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float MaxHealth;

	/** The multiplier to apply to the scale of the target if using LifetimeTargetScalePolicy. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	float LifetimeTargetScaleMultiplier;

	/** Min target scale to apply to the target when spawned. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float MinSpawnedTargetScale;

	/** Max target scale to apply to the target when spawned. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float MaxSpawnedTargetScale;

	/** Min velocity to apply to a target when spawned. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float MinSpawnedTargetSpeed;

	/** Max velocity to apply to a target when spawned. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float MaxSpawnedTargetSpeed;

	/** Min velocity to apply to a target when activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	float MinActivatedTargetSpeed;

	/** Max velocity to apply to a target when activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	float MaxActivatedTargetSpeed;

	/** Min velocity to apply to a target when deactivated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deactivation")
	float MinDeactivatedTargetSpeed;

	/** Max velocity to apply to a target when deactivated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deactivation")
	float MaxDeactivatedTargetSpeed;

	/** Length of time to keep targets flags as recent, if not using MaxNumRecentTargets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float RecentTargetTimeLength;

	/** Delay between time between target spawn and peak green target color. Same as PlayerDelay in FBS_AudioConfig. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float SpawnBeatDelay;

	/** Maximum time in which target will stay on screen. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float TargetMaxLifeSpan;

	/** Sets the minimum time between target spawns. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float TargetSpawnCD;

	/** The size of the target spawn BoundingBox. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpawnArea")
	FVector BoxBounds;

	/** Maximum number of activated targets allowed at one time. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	int32 MaxNumActivatedTargetsAtOnce;

	/** How many recent targets to keep in memory, if not using RecentTargetTimeLength. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	int32 MaxNumRecentTargets;

	/** Maximum number of visible targets allowed at one time, regardless of activation state. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	int32 MaxNumTargetsAtOnce;

	/** Minimum number of targets to activate at one time, if there's more than one target available to activate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	int32 MinNumTargetsToActivateAtOnce;

	/** Maximum number of targets to activate at one time, if there's more than one target available to activate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	int32 MaxNumTargetsToActivateAtOnce;

	/** How many targets to spawn at runtime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	int32 NumRuntimeTargetsToSpawn;

	/** How many targets to spawn before the game mode begins. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	int32 NumUpfrontTargetsToSpawn;

	/** Color to applied to the actor if inactive. */
	UPROPERTY(Transient, meta=(SkipSerialization))
	FLinearColor InactiveTargetColor;

	/** Color applied to target on spawn. */
	UPROPERTY(Transient, meta=(SkipSerialization))
	FLinearColor OnSpawnColor;

	/** Color interpolated from at start of ExpirationTimer timer. */
	UPROPERTY(Transient, meta=(SkipSerialization))
	FLinearColor StartColor;

	/** Color interpolated to from StartColor. */
	UPROPERTY(Transient, meta=(SkipSerialization))
	FLinearColor PeakColor;

	/** Color interpolated to from PeakColor. */
	UPROPERTY(Transient, meta=(SkipSerialization))
	FLinearColor EndColor;

	/** Separate outline color if specified. */
	UPROPERTY(Transient, meta=(SkipSerialization))
	FLinearColor OutlineColor;

	/** Color applied to targets that are vulnerable to tracking damage when taking tracking damage. */
	UPROPERTY(Transient, meta=(SkipSerialization))
	FLinearColor TakingTrackingDamageColor;

	/** Color applied to targets that are vulnerable to tracking damage when not taking tracking damage, OR
	 *  if the target lifespan is infinite. */
	UPROPERTY(Transient, meta=(SkipSerialization))
	FLinearColor NotTakingTrackingDamageColor;

	FBS_TargetConfig()
	{
		bAllowSpawnWithoutActivation = false;
		bAllowActivationWhileActivated = false;
		bSpawnAtOriginWheneverPossible = false;
		bSpawnEveryOtherTargetInCenter = false;
		bUseBatchSpawning = false;

		BoundsScalingPolicy = EBoundsScalingPolicy::None;
		ConsecutiveTargetScalePolicy = EConsecutiveTargetScalePolicy::None;
		MovingTargetDirectionMode = EMovingTargetDirectionMode::None;
		RecentTargetMemoryPolicy = ERecentTargetMemoryPolicy::None;
		TargetActivationSelectionPolicy = ETargetActivationSelectionPolicy::None;
		TargetDamageType = ETargetDamageType::None;
		TargetDistributionPolicy = ETargetDistributionPolicy::None;
		TargetSpawningPolicy = ETargetSpawningPolicy::None;
		RuntimeTargetSpawningLocationSelectionMode = ERuntimeTargetSpawningLocationSelectionMode::None;

		TargetSpawnResponses = TArray<ETargetSpawnResponse>();
		TargetActivationResponses = TArray<ETargetActivationResponse>();
		TargetDeactivationConditions = TArray<ETargetDeactivationCondition>();
		TargetDeactivationResponses = TArray<ETargetDeactivationResponse>();
		TargetDestructionConditions = TArray<ETargetDestructionCondition>();

		ConsecutiveChargeScaleMultiplier = Constants::DefaultChargeScaleMultiplier;
		LifetimeTargetScaleMultiplier = Constants::DefaultChargeScaleMultiplier;
		DeactivationHealthLostThreshold = 100.f;
		ExpirationHealthPenalty = Constants::BaseTargetHealth;
		FloorDistance = Constants::DistanceFromFloor;
		MinDistanceBetweenTargets = Constants::DefaultMinDistanceBetweenTargets;
		MaxHealth = Constants::BaseTargetHealth;
		MinSpawnedTargetScale = Constants::DefaultMinTargetScale;
		MaxSpawnedTargetScale = Constants::DefaultMaxTargetScale;
		MinSpawnedTargetSpeed = 0.f;
		MaxSpawnedTargetSpeed = 0.f;
		MinActivatedTargetSpeed = 0.f;
		MaxActivatedTargetSpeed = 0.f;
		MinDeactivatedTargetSpeed = 0.f;
		MaxDeactivatedTargetSpeed = 0.f;
		SpawnBeatDelay = Constants::DefaultSpawnBeatDelay;
		RecentTargetTimeLength = 0.f;
		TargetMaxLifeSpan = Constants::DefaultTargetMaxLifeSpan;
		TargetSpawnCD = Constants::DefaultTargetSpawnCD;

		BoxBounds = Constants::DefaultSpawnBoxBounds;

		BasePlayerHitDamage = 100.f;
		BasePlayerTrackingDamage = 1.f;
		MaxNumActivatedTargetsAtOnce = -1;
		MaxNumRecentTargets = -1;
		MaxNumTargetsAtOnce = -1;
		MinNumTargetsToActivateAtOnce = 1;
		MaxNumTargetsToActivateAtOnce = 1;
		NumRuntimeTargetsToSpawn = 0;
		NumUpfrontTargetsToSpawn = 0;

		bUseSeparateOutlineColor = false;
		InactiveTargetColor = FLinearColor();
		OnSpawnColor = FLinearColor();
		StartColor = FLinearColor();
		PeakColor = FLinearColor();
		EndColor = FLinearColor();
		OutlineColor = FLinearColor();
		TakingTrackingDamageColor = FLinearColor();
		NotTakingTrackingDamageColor = FLinearColor();
	}

	FORCEINLINE bool operator==(const FBS_TargetConfig& Other) const
	{
		if (bAllowActivationWhileActivated != Other.bAllowActivationWhileActivated)
		{
			return false;
		}
		if (bAllowSpawnWithoutActivation != Other.bAllowSpawnWithoutActivation)
		{
			return false;
		}
		if (bSpawnAtOriginWheneverPossible != Other.bSpawnAtOriginWheneverPossible)
		{
			return false;
		}
		if (bSpawnEveryOtherTargetInCenter != Other.bSpawnEveryOtherTargetInCenter)
		{
			return false;
		}
		if (bUseBatchSpawning != Other.bUseBatchSpawning)
		{
			return false;
		}
		if (BoundsScalingPolicy != Other.BoundsScalingPolicy)
		{
			return false;
		}
		if (RuntimeTargetSpawningLocationSelectionMode != Other.RuntimeTargetSpawningLocationSelectionMode)
		{
			return false;
		}
		if (ConsecutiveTargetScalePolicy != Other.ConsecutiveTargetScalePolicy)
		{
			return false;
		}
		if (MovingTargetDirectionMode != Other.MovingTargetDirectionMode)
		{
			return false;
		}
		if (RecentTargetMemoryPolicy != Other.RecentTargetMemoryPolicy)
		{
			return false;
		}
		if (TargetActivationSelectionPolicy != Other.TargetActivationSelectionPolicy)
		{
			return false;
		}
		if (TargetDamageType != Other.TargetDamageType)
		{
			return false;
		}
		if (TargetDistributionPolicy != Other.TargetDistributionPolicy)
		{
			return false;
		}
		if (TargetSpawningPolicy != Other.TargetSpawningPolicy)
		{
			return false;
		}
		if (TargetSpawnResponses != Other.TargetSpawnResponses)
		{
			return false;
		}
		if (TargetActivationResponses != Other.TargetActivationResponses)
		{
			return false;
		}
		if (TargetDeactivationConditions != Other.TargetDeactivationConditions)
		{
			return false;
		}
		if (TargetDeactivationResponses != Other.TargetDeactivationResponses)
		{
			return false;
		}
		if (TargetDestructionConditions != Other.TargetDestructionConditions)
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(ConsecutiveChargeScaleMultiplier, Other.ConsecutiveChargeScaleMultiplier))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(DeactivationHealthLostThreshold, Other.DeactivationHealthLostThreshold))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(LifetimeTargetScaleMultiplier, Other.LifetimeTargetScaleMultiplier))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(ExpirationHealthPenalty, Other.ExpirationHealthPenalty))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(FloorDistance, Other.FloorDistance))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(MinDistanceBetweenTargets, Other.MinDistanceBetweenTargets))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(ConsecutiveChargeScaleMultiplier, Other.ConsecutiveChargeScaleMultiplier))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(MaxHealth, Other.MaxHealth))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(MinSpawnedTargetScale, Other.MinSpawnedTargetScale))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(MaxSpawnedTargetScale, Other.MaxSpawnedTargetScale))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(MaxSpawnedTargetScale, Other.MaxSpawnedTargetScale))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(MinSpawnedTargetSpeed, Other.MinSpawnedTargetSpeed))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(MaxSpawnedTargetSpeed, Other.MaxSpawnedTargetSpeed))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(MinActivatedTargetSpeed, Other.MinActivatedTargetSpeed))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(MaxActivatedTargetSpeed, Other.MaxActivatedTargetSpeed))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(MinDeactivatedTargetSpeed, Other.MinDeactivatedTargetSpeed))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(MaxDeactivatedTargetSpeed, Other.MaxDeactivatedTargetSpeed))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(SpawnBeatDelay, Other.SpawnBeatDelay))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(RecentTargetTimeLength, Other.RecentTargetTimeLength))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(TargetMaxLifeSpan, Other.TargetMaxLifeSpan))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(TargetSpawnCD, Other.TargetSpawnCD))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(BasePlayerHitDamage, Other.BasePlayerHitDamage))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(BasePlayerTrackingDamage, Other.BasePlayerTrackingDamage))
		{
			return false;
		}
		if (BoxBounds != Other.BoxBounds)
		{
			return false;
		}
		if (MaxNumActivatedTargetsAtOnce != Other.MaxNumActivatedTargetsAtOnce)
		{
			return false;
		}
		if (MaxNumRecentTargets != Other.MaxNumRecentTargets)
		{
			return false;
		}
		if (MaxNumTargetsAtOnce != Other.MaxNumTargetsAtOnce)
		{
			return false;
		}
		if (MinNumTargetsToActivateAtOnce != Other.MinNumTargetsToActivateAtOnce)
		{
			return false;
		}
		if (MaxNumTargetsToActivateAtOnce != Other.MaxNumTargetsToActivateAtOnce)
		{
			return false;
		}
		if (NumRuntimeTargetsToSpawn != Other.NumRuntimeTargetsToSpawn)
		{
			return false;
		}
		if (NumUpfrontTargetsToSpawn != Other.NumUpfrontTargetsToSpawn)
		{
			return false;
		}
		return true;
	}
};
