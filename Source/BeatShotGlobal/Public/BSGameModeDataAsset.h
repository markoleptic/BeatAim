﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GlobalConstants.h"
#include "Engine/DataAsset.h"
#include "BSGameModeDataAsset.generated.h"

using namespace Constants;

/* --------------------- */
/* ------- Enums ------- */
/* --------------------- */

/** The player chosen audio format for the current game mode */
UENUM(BlueprintType)
enum class EAudioFormat : uint8
{
	None UMETA(DisplayName="None"),
	File UMETA(DisplayName="File"),
	Capture UMETA(DisplayName="Capture"),
	Loopback UMETA(DisplayName="Loopback")};

ENUM_RANGE_BY_FIRST_AND_LAST(EAudioFormat, EAudioFormat::File, EAudioFormat::Loopback);


/** Enum representing the type of game mode */
UENUM(BlueprintType)
enum class EGameModeType : uint8
{
	None UMETA(DisplayName="None"),
	Preset UMETA(DisplayName="Preset"),
	Custom UMETA(DisplayName="Custom")};

ENUM_RANGE_BY_FIRST_AND_LAST(EGameModeType, EGameModeType::Preset, EGameModeType::Custom);


/** Enum representing the base game modes. Preset game modes aren't necessarily a BaseGameMode */
UENUM(BlueprintType)
enum class EBaseGameMode : uint8
{
	None UMETA(DisplayName="None"),
	SingleBeat UMETA(DisplayName="SingleBeat"),
	MultiBeat UMETA(DisplayName="MultiBeat"),
	BeatGrid UMETA(DisplayName="BeatGrid"),
	BeatTrack UMETA(DisplayName="BeatTrack"),
	ChargedBeatTrack UMETA(DisplayName="ChargedBeatTrack"),
	ClusterBeat UMETA(DisplayName="ClusterBeat"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EBaseGameMode, EBaseGameMode::SingleBeat, EBaseGameMode::ClusterBeat);


/** Enum representing the default game mode difficulties */
UENUM(BlueprintType)
enum class EGameModeDifficulty : uint8
{
	None UMETA(DisplayName="None"),
	Normal UMETA(DisplayName="Normal"),
	Hard UMETA(DisplayName="Hard"),
	Death UMETA(DisplayName="Death")};

ENUM_RANGE_BY_FIRST_AND_LAST(EGameModeDifficulty, EGameModeDifficulty::None, EGameModeDifficulty::Death);


/** How to scale the bounding box bounds (spawn area where targets are spawned), at runtime */
UENUM(BlueprintType)
enum class EBoundsScalingPolicy : uint8
{
	None UMETA(DisplayName="None"),
	/** The bounding box bounds size will not change throughout the game mode */
	Static UMETA(DisplayName="Static"),
	/** The bounding box bounds size will gradually increase from half the Box Bounds up to the full size of BoxBounds, based on consecutive targets hit */
	Dynamic UMETA(DisplayName="Dynamic"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EBoundsScalingPolicy, EBoundsScalingPolicy::Static, EBoundsScalingPolicy::Dynamic);

inline bool IsDynamicBoundsScalingPolicy(const EBoundsScalingPolicy SpreadType)
{
	return SpreadType == EBoundsScalingPolicy::Dynamic;
}


/** Where to spawn/activate targets in the bounding box bounds (spawn area) */
UENUM(BlueprintType)
enum class ETargetDistributionPolicy : uint8
{
	None UMETA(DisplayName="None"),
	/** Only spawns targets at headshot height, so the bounding box has no height */
	HeadshotHeightOnly UMETA(DisplayName="Headshot Height Only"),
	/** Only spawns targets on the edges of the bounding box */
	EdgeOnly UMETA(DisplayName="Edge Only"),
	/** Spawns targets anywhere in the bounding box */
	FullRange UMETA(DisplayName="Full Range"),
	/** Spawns targets in an evenly-spaced grid. Able to fit more targets */
	Grid UMETA(DisplayName="Grid"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetDistributionPolicy, ETargetDistributionPolicy::HeadshotHeightOnly,
	ETargetDistributionPolicy::Grid);


/** Which direction to move a target in */
UENUM(BlueprintType)
enum class EMovingTargetDirectionMode : uint8
{
	None UMETA(DisplayName="None"),
	/** Only move targets left and right */
	HorizontalOnly UMETA(DisplayName="Horizontal Only"),
	/** Only move targets up and down */
	VerticalOnly UMETA(DisplayName="Vertical Only"),
	/** Only move targets up and down */
	AlternateHorizontalVertical UMETA(DisplayName="Alternate Horizontal Vertical"),
	/** Move targets in any direction */
	Any UMETA(DisplayName="Any"),
	/** Only move targets forward */
	ForwardOnly UMETA(DisplayName="Forward Only"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EMovingTargetDirectionMode, EMovingTargetDirectionMode::None,
	EMovingTargetDirectionMode::ForwardOnly);


/** How to handle changing the target scale between consecutively activated targets */
UENUM(BlueprintType)
enum class EConsecutiveTargetScalePolicy : uint8
{
	None UMETA(DisplayName="None"),
	/** The starting scale/size of the target will remain constant throughout the game mode */
	Static UMETA(DisplayName="Static"),
	/** The starting scale/size of the target will be chosen randomly between min and max target scale */
	Random UMETA(DisplayName="Random"),
	/** The starting scale/size of the target will gradually shrink from max to min target scale, based on consecutive targets hit */
	SkillBased UMETA(DisplayName="Skill-Based")};

ENUM_RANGE_BY_FIRST_AND_LAST(EConsecutiveTargetScalePolicy, EConsecutiveTargetScalePolicy::Static,
	EConsecutiveTargetScalePolicy::SkillBased);


/** How the player damages the target and receives score */
UENUM(BlueprintType)
enum class ETargetDamageType : uint8
{
	None UMETA(DisplayName="None"),
	/** Having the CrossHair over the target damages the target and awards score */
	Tracking UMETA(DisplayName="Tracking"),
	/** Firing the gun and hitting the target damages the target and awards score */
	Hit UMETA(DisplayName="Hit"),
	/** Having the CrossHair over the target and hitting the target both damage the target and award score */
	Combined UMETA(DisplayName="Combined"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetDamageType, ETargetDamageType::Tracking, ETargetDamageType::Hit);


/** When to spawn the targets. For now, limited to just all upfront or all at runtime */
UENUM(BlueprintType)
enum class ETargetSpawningPolicy : uint8
{
	None UMETA(DisplayName="None"),
	/** Spawn all the targets before the game mode starts */
	UpfrontOnly UMETA(DisplayName="Upfront Only"),
	/** Spawn targets when the TargetManager receives the signal from AudioAnalyzer */
	RuntimeOnly UMETA(DisplayName="Runtime Only"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetSpawningPolicy, ETargetSpawningPolicy::UpfrontOnly,
	ETargetSpawningPolicy::RuntimeOnly);


/** How to choose the target(s) to activate */
UENUM(BlueprintType)
enum class ETargetActivationSelectionPolicy : uint8
{
	None UMETA(DisplayName="None"),
	/** Only choose spawn points that border the previous point (BeatGrid is an example) */
	Bordering UMETA(DisplayName="Bordering"),
	/** Randomly chooses a target within the available spawn points */
	Random UMETA(DisplayName="Random"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetActivationSelectionPolicy, ETargetActivationSelectionPolicy::Bordering,
	ETargetActivationSelectionPolicy::Random);


/** Specifies the method to remove targets from recent memory, allowing targets to spawn in that location again */
UENUM(BlueprintType)
enum class ERecentTargetMemoryPolicy : uint8
{
	/** Does not remember any recent targets */
	None UMETA(DisplayName="None"),
	/** Uses a specified time to remove recent targets, starting after the target has been destroyed or deactivated */
	CustomTimeBased UMETA(DisplayName="Custom Time Based"),
	/** Removes a recent target after TargetSpawnCD length of time has passed since the target has been destroyed or deactivated */
	UseTargetSpawnCD UMETA(DisplayName="Use TargetSpawnCD"),
	/** Removes recent targets only when the number of recent targets exceeds specified capacity */
	NumTargetsBased UMETA(DisplayName="Num Targets Based"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ERecentTargetMemoryPolicy, ERecentTargetMemoryPolicy::None,
	ERecentTargetMemoryPolicy::NumTargetsBased);


/** Each represents one way that a target can be deactivated */
UENUM(BlueprintType)
enum class ETargetDeactivationCondition : uint8
{
	None UMETA(DisplayName="None"),
	/** Targets are never deactivated, even if their health reaches zero */
	Persistant UMETA(DisplayName="Persistant"),
	/** Target is deactivated when it receives any damage from the player */
	OnAnyExternalDamageTaken UMETA(DisplayName="On Any External Damage Taken"),
	/** Target is deactivated after its damageable window closes */
	OnExpiration UMETA(DisplayName="On Expiration"),
	/** DEPRECATED */
	OnHealthReachedZero UMETA(DisplayName="On Health Reached Zero"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetDeactivationCondition, ETargetDeactivationCondition::Persistant,
	ETargetDeactivationCondition::OnExpiration);


/** Each represents one way that a target can be destroyed */
UENUM(BlueprintType)
enum class ETargetDestructionCondition : uint8
{
	None UMETA(DisplayName="None"),
	/** Targets are never destroyed, even if their health reaches zero. Can still be reactivated/deactivated */
	Persistant UMETA(DisplayName="Persistant"),
	/** Target is deactivated after its damageable window closes */
	OnExpiration UMETA(DisplayName="On Expiration"),
	/** Target is destroyed when it receives any damage from the player */
	OnAnyExternalDamageTaken UMETA(DisplayName="On Any External Damage Taken"),
	/** Target is destroyed when its health reaches zero */
	OnHealthReachedZero UMETA(DisplayName="On Health Reached Zero"),
	/** Target is destroyed when any of its deactivation conditions are met. This essentially makes any deactivation condition a destruction condition */
	OnDeactivation UMETA(DisplayName="On Deactivation")};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetDestructionCondition, ETargetDestructionCondition::Persistant,
	ETargetDestructionCondition::OnDeactivation);


/** What does the target do when its activated: change directions, make damageable, etc */
UENUM(BlueprintType)
enum class ETargetActivationResponse : uint8
{
	None UMETA(DisplayName="None"),
	/** Immunity is removed from the target, damageable window begins if not already damageable */
	RemoveImmunity UMETA(DisplayName="Remove Immunity"),
	/** Immunity is granted to the target */
	AddImmunity UMETA(DisplayName="Add Immunity"),
	/** Immunity is added or removed from the target, depending on its existing state. Same with damageable window */
	ToggleImmunity UMETA(DisplayName="Toggle Immunity"),
	/** If a moving target, the direction is changed */
	ChangeDirection UMETA(DisplayName="Change Direction"),
	/** DEPRECATED */
	ChangeScale UMETA(DisplayName="Change Scale"),
	/** If a moving target, the velocity of the target is changed */
	ChangeVelocity UMETA(DisplayName="Change Velocity"),
	/** The consecutive Target Scale is applied */
	ApplyConsecutiveTargetScale UMETA(DisplayName="Apply Consecutive Target Scale"),
	/** Lifetime Target Scaling is applied throughout the target's lifetime */
	ApplyLifetimeTargetScaling UMETA(DisplayName="Apply Lifetime Target Scaling"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetActivationResponse, ETargetActivationResponse::RemoveImmunity,
	ETargetActivationResponse::ApplyLifetimeTargetScaling);


/** What does the target do when its deactivated */
UENUM(BlueprintType)
enum class ETargetDeactivationResponse : uint8
{
	None UMETA(DisplayName="None"),
	/** Immunity is removed from the target, damageable window begins if not already damageable */
	RemoveImmunity UMETA(DisplayName="Remove Immunity"),
	/** Immunity is granted to the target */
	AddImmunity UMETA(DisplayName="Add Immunity"),
	/** Immunity is added or removed from the target, depending on its existing state. Same with damageable window */
	ToggleImmunity UMETA(DisplayName="Toggle Immunity"),
	/** The scale is set according to ConsecutiveTargetScale. LifetimeTargetScale can still override this */
	ApplyDeactivatedTargetScaleMultiplier UMETA(DisplayName="Apply Deactivated Target Scale Multiplier"),
	/** Reset the scale of the target to the scale it was spawned with */
	ResetScaleToSpawnedScale UMETA(DisplayName="Reset Scale To Spawned Scale"),
	/** Reset the position of the target to the position it was spawned with */
	ResetPositionToSpawnedPosition UMETA(DisplayName="Reset Position To Spawned Position"),
	/** Reset the color of the target to the inactive color */
	ResetColorToInactiveColor UMETA(DisplayName="Reset Color To Inactive Color"),
	/** Reset the scale of the target to the scale it was initialized with */
	ShrinkQuickGrowSlow UMETA(DisplayName="Shrink Quick Grow Slow"),
	/** Play an explosion effect */
	PlayExplosionEffect UMETA(DisplayName="Play Explosion Effect"),
	/** Destroy the target permanently */
	Destroy UMETA(DisplayName="Destroy"),
	/** Hide the target */
	HideTarget UMETA(DisplayName="Hide Target"),
	/** Change the direction of the target */
	ChangeDirection UMETA(DisplayName="Change Direction"),
	/** Change the velocity of the target according to DeactivationVelocity */
	ChangeVelocity UMETA(DisplayName="Change Velocity"),
	/** Reset the scale of the target to the scale it was activated with */
	ResetScaleToActivatedScale UMETA(DisplayName="Reset Scale To Activated Scale"),
	/** Reset the position of the target to the position it was activated with */
	ResetPositionToActivatedPosition UMETA(DisplayName="Reset Position To Activated Position"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETargetDeactivationResponse, ETargetDeactivationResponse::RemoveImmunity,
	ETargetDeactivationResponse::ResetPositionToActivatedPosition);

/** Defines in which directions the box bounds will grow */
UENUM(BlueprintType)
enum class EDynamicBoundsScalingPolicy : uint8
{
	None UMETA(DisplayName="None"),
	/** Allow the bounds to grow horizontally */
	Horizontal UMETA(DisplayName="Horizontal"),
	/** Allow the bounds to grow vertically */
	Vertical UMETA(DisplayName="Vertical"),
	/** Allow the bounds to grow forward  */
	Forward UMETA(DisplayName="Forward")};

ENUM_RANGE_BY_FIRST_AND_LAST(EDynamicBoundsScalingPolicy, EDynamicBoundsScalingPolicy::Horizontal,
	EDynamicBoundsScalingPolicy::Forward);

/** Enum representing the modes in which the Reinforcement Learning Component operates */
UENUM(BlueprintType)
enum class EReinforcementLearningMode : uint8
{
	None UMETA(DisplayName="None"),
	Training UMETA(DisplayName="Training"),
	Exploration UMETA(DisplayName="Exploration"),
	ActiveAgent UMETA(DisplayName="ActiveAgent")};

ENUM_RANGE_BY_FIRST_AND_LAST(EReinforcementLearningMode, EReinforcementLearningMode::None,
	EReinforcementLearningMode::ActiveAgent);

/** Enum representing auto or custom hyper-parameters */
UENUM(BlueprintType)
enum class EReinforcementLearningHyperParameterMode: uint8
{
	None UMETA(DisplayName="None"),
	Auto UMETA(DisplayName="Auto"),
	Custom UMETA(DisplayName="Custom")};

ENUM_RANGE_BY_FIRST_AND_LAST(EReinforcementLearningHyperParameterMode, EReinforcementLearningHyperParameterMode::Auto,
	EReinforcementLearningHyperParameterMode::Custom);


/* --------------------- */
/* ------ Structs ------ */
/* --------------------- */


/** Small Struct containing the information needed to distinguish between unique default game modes and unique custom game modes.
 *  This info persists across different songs, which is why it is separate from FPlayerScore */
USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct FBS_DefiningConfig
{
	GENERATED_BODY()

	/** The type of game mode: either preset or custom */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EGameModeType GameModeType;

	/** The base game mode this game mode is based off of */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EBaseGameMode BaseGameMode;

	/** Custom game mode name if custom, otherwise empty string */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString CustomGameModeName;

	/** Default game mode difficulties, or none if custom */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EGameModeDifficulty Difficulty;

	FBS_DefiningConfig()
	{
		GameModeType = EGameModeType::None;
		BaseGameMode = EBaseGameMode::None;
		CustomGameModeName = "";
		Difficulty = EGameModeDifficulty::None;
	}

	FBS_DefiningConfig(const EGameModeType& InGameModeType, const EBaseGameMode& InBaseGameMode,
		const FString& InCustomGameModeName, const EGameModeDifficulty& InGameModeDifficulty)
	{
		GameModeType = InGameModeType;
		BaseGameMode = InBaseGameMode;
		CustomGameModeName = InCustomGameModeName;
		Difficulty = InGameModeDifficulty;
	}

	FORCEINLINE bool operator==(const FBS_DefiningConfig& Other) const
	{
		/* Preset vs Custom */
		if (GameModeType == Other.GameModeType)
		{
			if (GameModeType == EGameModeType::Custom)
			{
				/* Custom game modes don't depend on difficulty, only CustomGameModeName */
				if (CustomGameModeName.Equals(Other.CustomGameModeName, ESearchCase::IgnoreCase))
				{
					return true;
				}
				return false;
			}
			if (GameModeType == EGameModeType::Preset)
			{
				/* Preset game modes must match the BaseGameMode and difficulty to be considered equal */
				if (BaseGameMode == Other.BaseGameMode && Difficulty == Other.Difficulty)
				{
					return true;
				}
				return false;
			}
		}
		return false;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FBS_DefiningConfig& Config)
	{
		return HashCombine(GetTypeHash(Config.GameModeType), HashCombine(GetTypeHash(Config.BaseGameMode),
			HashCombine(GetTypeHash(Config.CustomGameModeName), GetTypeHash(Config.Difficulty))));
		//return FCrc::MemCrc32(&Config, sizeof(FBS_DefiningConfig));
	}
};

USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct FBS_AIConfig
{
	GENERATED_BODY()

	/** Whether or not to enable the reinforcement learning agent to handle target spawning */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bEnableReinforcementLearning;

	/** Learning rate, or how much to update the Q-Table rewards when a reward is received */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Alpha;

	/** The exploration/exploitation balance factor. A value = 1 will result in only choosing random values (explore),
	 *  while a value of zero will result in only choosing the max Q-value (exploitation) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Epsilon;

	/** Discount factor, or how much to value future rewards vs immediate rewards */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Gamma;

	/** The mode to operate the Reinforcement Learning Component */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EReinforcementLearningMode ReinforcementLearningMode;

	/** Auto adjust Alpha and Epsilon or never change and let user choose */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EReinforcementLearningHyperParameterMode HyperParameterMode;

	FBS_AIConfig()
	{
		bEnableReinforcementLearning = false;
		Alpha = DefaultAlpha;
		Epsilon = DefaultEpsilon;
		Gamma = DefaultGamma;
		ReinforcementLearningMode = EReinforcementLearningMode::None;
		HyperParameterMode = EReinforcementLearningHyperParameterMode::Auto;
	}

	FORCEINLINE bool operator==(const FBS_AIConfig& Other) const
	{
		if (bEnableReinforcementLearning != Other.bEnableReinforcementLearning)
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(Alpha, Other.Alpha))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(Epsilon, Other.Epsilon))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(Gamma, Other.Gamma))
		{
			return false;
		}
		if (ReinforcementLearningMode != Other.ReinforcementLearningMode)
		{
			return false;
		}
		if (HyperParameterMode != Other.HyperParameterMode)
		{
			return false;
		}
		return true;
	}
};

USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct FBS_GridConfig
{
	GENERATED_BODY()

	/** The number of horizontal grid targets*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 NumHorizontalGridTargets;

	/** The number of vertical grid targets*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 NumVerticalGridTargets;

	/** The space between grid targets */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector2D GridSpacing;

	/** Number of grid target visible at any one time */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 NumGridTargetsVisibleAtOnce;

	FBS_GridConfig()
	{
		NumHorizontalGridTargets = NumHorizontalBeatGridTargets_Normal;
		NumVerticalGridTargets = NumVerticalBeatGridTargets_Normal;
		NumGridTargetsVisibleAtOnce = NumTargetsAtOnceBeatGrid_Normal;
		GridSpacing = BeatGridSpacing_Normal;
	}

	FORCEINLINE bool operator==(const FBS_GridConfig& Other) const
	{
		if (NumHorizontalGridTargets != Other.NumHorizontalGridTargets)
		{
			return false;
		}
		if (NumVerticalGridTargets != Other.NumVerticalGridTargets)
		{
			return false;
		}
		if (GridSpacing != Other.GridSpacing)
		{
			return false;
		}
		if (NumHorizontalGridTargets != Other.NumHorizontalGridTargets)
		{
			return false;
		}
		if (NumGridTargetsVisibleAtOnce != Other.NumGridTargetsVisibleAtOnce)
		{
			return false;
		}
		return true;
	}
};

USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct FBS_AudioConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString SongTitle;

	/** Whether or not to playback streamed audio */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bPlaybackAudio;

	/** The audio format type used for the AudioAnalyzer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAudioFormat AudioFormat;

	/** The input audio device */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString InAudioDevice;

	/** The output audio device */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString OutAudioDevice;

	/** The path to the song file */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString SongPath;

	/** Delay between AudioAnalyzer Tracker and Player. Also the same value as time between target spawn and peak green target color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PlayerDelay;

	/** Length of song */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SongLength;

	FBS_AudioConfig()
	{
		bPlaybackAudio = false;
		AudioFormat = EAudioFormat::None;
		InAudioDevice = "";
		OutAudioDevice = "";
		SongPath = "";
		SongTitle = "";
		SongLength = 0.f;
		PlayerDelay = DefaultPlayerDelay;
	}
};

/** Defines how to dynamically change the scale of a game mode feature.
 *  Requires using an external counter that increments each time a target
 *  is consecutively destroyed and decrements by DecrementAmount each time
 *  a target was not destroyed. */
USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct FBS_Dynamic
{
	GENERATED_BODY()

	/** The number of consecutively destroyed targets required to begin changing the scale */
	UPROPERTY(EditDefaultsOnly)
	int32 StartThreshold;

	/** The number of consecutively destroyed targets required to reach the final scale */
	UPROPERTY(EditDefaultsOnly)
	int32 EndThreshold;

	/** Whether or not to use cubic interpolation or linear interpolation from StartThreshold to EndThreshold */
	UPROPERTY(EditDefaultsOnly)
	bool bIsCubicInterpolation;

	/** The amount to decrement from consecutively destroyed targets after a miss */
	UPROPERTY(EditDefaultsOnly)
	int32 DecrementAmount;

	FBS_Dynamic()
	{
		StartThreshold = 5;
		EndThreshold = 100;
		bIsCubicInterpolation = false;
		DecrementAmount = 5;
	}

	FORCEINLINE bool operator==(const FBS_Dynamic& Other) const
	{
		if (StartThreshold != Other.StartThreshold)
		{
			return false;
		}
		if (EndThreshold != Other.EndThreshold)
		{
			return false;
		}
		if (bIsCubicInterpolation != Other.bIsCubicInterpolation)
		{
			return false;
		}
		if (DecrementAmount != Other.DecrementAmount)
		{
			return false;
		}
		return true;
	}
};

/** Defines how to dynamically change the scale of the SpawnArea.
 *  Requires using an external counter that increments each time a target
 *  is consecutively destroyed and decrements by DecrementAmount each time
 *  a target was not destroyed. */
USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct FBS_Dynamic_SpawnArea : public FBS_Dynamic
{
	GENERATED_BODY()

	/** Which direction(s) to change the SpawnArea/BoxBounds. If a direction is not included,
	 *  it will always stay at StartBounds */
	UPROPERTY(EditDefaultsOnly)
	TArray<EDynamicBoundsScalingPolicy> DynamicBoundsScalingPolicy;

	/** The size of the SpawnArea/BoxBounds when zero consecutively destroyed targets.
	 *  X is forward, Y is horizontal, Z is vertical */
	UPROPERTY(EditDefaultsOnly)
	FVector StartBounds;

	FVector GetStartExtents() const
	{
		return FVector(StartBounds.X, StartBounds.Y, StartBounds.Z) * 0.5f;
	}

	FBS_Dynamic_SpawnArea()
	{
		StartThreshold = 5;
		EndThreshold = 100;
		bIsCubicInterpolation = false;
		DecrementAmount = 5;
		DynamicBoundsScalingPolicy = TArray({
			EDynamicBoundsScalingPolicy::Horizontal,
			EDynamicBoundsScalingPolicy::Vertical
		});
		StartBounds = FVector(0, 200.f, 200.f);
	}

	FORCEINLINE bool operator==(const FBS_Dynamic_SpawnArea& Other) const
	{
		if (StartThreshold != Other.StartThreshold)
		{
			return false;
		}
		if (EndThreshold != Other.EndThreshold)
		{
			return false;
		}
		if (bIsCubicInterpolation != Other.bIsCubicInterpolation)
		{
			return false;
		}
		if (DecrementAmount != Other.DecrementAmount)
		{
			return false;
		}
		if (DynamicBoundsScalingPolicy != Other.DynamicBoundsScalingPolicy)
		{
			return false;
		}
		if (!StartBounds.Equals(Other.StartBounds, 0.1f))
		{
			return false;
		}
		return true;
	}
};

USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct FBS_TargetConfig
{
	GENERATED_BODY()

	/** If true, targets can be spawned without being activated */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bAllowSpawnWithoutActivation;

	/** Should the target be immune to damage when spawned? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bApplyImmunityOnSpawn;

	/** Whether or not the targets should have a velocity when spawned */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bApplyVelocityWhenSpawned;

	/** If true, spawn at the origin if it isn't blocked by a recent target whenever possible */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bSpawnAtOriginWheneverPossible;

	/** If true, alternate every target spawn in the very center */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bSpawnEveryOtherTargetInCenter;

	/** If true, postpones spawning target(s) until the previous target(s) have all been activated and deactivated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bUseBatchSpawning;

	/** If true, use separate outline color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bUseSeparateOutlineColor;

	/** How to scale the bounding box bounds (spawn area where targets are spawned), at runtime */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EBoundsScalingPolicy BoundsScalingPolicy;

	/** Whether or not to dynamically change the size of targets as consecutive targets are hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EConsecutiveTargetScalePolicy ConsecutiveTargetScalePolicy;

	/** Which direction(s) to move targets. Separate from moving a target forward */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EMovingTargetDirectionMode MovingTargetDirectionMode;

	/** Specifies the method to remove targets from recent memory, allowing targets to spawn in that location again */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ERecentTargetMemoryPolicy RecentTargetMemoryPolicy;

	/** How to choose the target(s) to activate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ETargetActivationSelectionPolicy TargetActivationSelectionPolicy;

	/** How the player damages the target and receives score */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ETargetDamageType TargetDamageType;

	/** Where to spawn/activate targets in the bounding box bounds (spawn area) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ETargetDistributionPolicy TargetDistributionPolicy;

	/** When to spawn targets */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ETargetSpawningPolicy TargetSpawningPolicy;

	/** The possible outcomes that a target can do when its activated: change directions, make damageable, etc */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<ETargetActivationResponse> TargetActivationResponses;

	/** Any condition that should deactivate the target (make it immune to damage) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<ETargetDeactivationCondition> TargetDeactivationConditions;

	/** Anything the target should do when it deactivates */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<ETargetDeactivationResponse> TargetDeactivationResponses;

	/** Any condition that should permanently destroy a the target */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<ETargetDestructionCondition> TargetDestructionConditions;

	/** The base damage to set the player's HitDamage Attribute to for Hit-Based damage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BasePlayerHitDamage;

	/** The base damage to set the player's HitDamage Attribute to for Tracking-Based damage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BasePlayerTrackingDamage;

	/** How much to shrink the target each time a charge is consumed, if the target is charged. This is multiplied
	 *  against the last charged target scale. A fully charged target does not receive any multiplier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ConsecutiveChargeScaleMultiplier;

	/** Amount of health to take away from the target if the DamageableWindow timer expires */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ExpirationHealthPenalty;

	/** Distance from bottom of TargetManager BoxBounds to the floor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FloorDistance;

	/** Sets the minimum distance between recent target spawns */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinDistanceBetweenTargets;

	/** Value to set the MaxHealth attribute value to */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth;

	/** The multiplier to apply to the scale of the target if using LifetimeTargetScalePolicy */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float LifetimeTargetScaleMultiplier;

	/** Min target scale to apply to the target when spawned */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinSpawnedTargetScale;

	/** Max target scale to apply to the target when spawned */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxSpawnedTargetScale;

	/** Min velocity to apply to a target when spawned */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinSpawnedTargetSpeed;

	/** Max velocity to apply to a target when spawned */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxSpawnedTargetSpeed;

	/** Min velocity to apply to a target when activated */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinActivatedTargetSpeed;

	/** Max velocity to apply to a target when activated */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxActivatedTargetSpeed;

	/** Min velocity to apply to a target when deactivated */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinDeactivatedTargetSpeed;

	/** Max velocity to apply to a target when deactivated */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxDeactivatedTargetSpeed;

	/** Length of time to keep targets flags as recent, if not using MaxNumRecentTargets */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float RecentTargetTimeLength;

	/** Delay between time between target spawn and peak green target color. Same as PlayerDelay in FBS_AudioConfig */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpawnBeatDelay;

	/** Maximum time in which target will stay on screen */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float TargetMaxLifeSpan;

	/** Sets the minimum time between target spawns */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float TargetSpawnCD;

	/** Gameplay tags applied to the target ASC when spawned */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer OnSpawn_ApplyTags;

	/** The size of the target spawn BoundingBox. Dimensions are half of the the total length/width */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector BoxBounds;

	/** Maximum number of activated targets allowed at one time */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxNumActivatedTargetsAtOnce;

	/** How many recent targets to keep in memory, if not using RecentTargetTimeLength */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxNumRecentTargets;

	/** Maximum number of visible targets allowed at one time, regardless of activation state */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxNumTargetsAtOnce;

	/** Minimum number of targets to activate at one time, if there's more than one target available to activate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MinNumTargetsToActivateAtOnce;

	/** Maximum number of targets to activate at one time, if there's more than one target available to activate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxNumTargetsToActivateAtOnce;

	/** How many targets to spawn at runtime */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 NumRuntimeTargetsToSpawn;

	/** How many targets to spawn before the game mode begins */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 NumUpfrontTargetsToSpawn;

	/** Color to applied to the actor if inactive */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor InactiveTargetColor;

	// TODO: might need to have a bool variable to set spawn color to inactive color
	/** Color applied to target on spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor OnSpawnColor;

	/** Color interpolated from at start of DamageableWindow timer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor StartColor;

	/** Color interpolated to from StartColor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor PeakColor;

	/** Color interpolated to from PeakColor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor EndColor;

	/** Separate outline color if specified */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor OutlineColor;

	FBS_TargetConfig()
	{
		bAllowSpawnWithoutActivation = false;
		bApplyImmunityOnSpawn = false;
		bApplyVelocityWhenSpawned = false;
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

		TargetActivationResponses = TArray<ETargetActivationResponse>();
		TargetDeactivationConditions = TArray<ETargetDeactivationCondition>();
		TargetDeactivationResponses = TArray<ETargetDeactivationResponse>();
		TargetDestructionConditions = TArray<ETargetDestructionCondition>();

		ConsecutiveChargeScaleMultiplier = DefaultChargeScaleMultiplier;
		LifetimeTargetScaleMultiplier = DefaultChargeScaleMultiplier;
		ExpirationHealthPenalty = BaseTargetHealth;
		FloorDistance = DistanceFromFloor;
		MinDistanceBetweenTargets = DefaultMinDistanceBetweenTargets;
		MaxHealth = BaseTargetHealth;
		MinSpawnedTargetScale = DefaultMinTargetScale;
		MaxSpawnedTargetScale = DefaultMaxTargetScale;
		MinSpawnedTargetSpeed = 0.f;
		MaxSpawnedTargetSpeed = 0.f;
		MinActivatedTargetSpeed = 0.f;
		MaxActivatedTargetSpeed = 0.f;
		MinDeactivatedTargetSpeed = 0.f;
		MaxDeactivatedTargetSpeed = 0.f;
		SpawnBeatDelay = DefaultSpawnBeatDelay;
		RecentTargetTimeLength = 0.f;
		TargetMaxLifeSpan = DefaultTargetMaxLifeSpan;
		TargetSpawnCD = DefaultTargetSpawnCD;

		OnSpawn_ApplyTags = FGameplayTagContainer();
		BoxBounds = DefaultSpawnBoxBounds;

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
	}

	FORCEINLINE bool operator==(const FBS_TargetConfig& Other) const
	{
		if (bAllowSpawnWithoutActivation != Other.bAllowSpawnWithoutActivation)
		{
			return false;
		}
		if (bApplyImmunityOnSpawn != Other.bApplyImmunityOnSpawn)
		{
			return false;
		}
		if (bApplyVelocityWhenSpawned != Other.bApplyVelocityWhenSpawned)
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

/** Struct representing a game mode */
USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct FBSConfig
{
	GENERATED_BODY()

	/** The defining config for a game mode, containing the names, base, difficulty */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ShowOnlyInnerProperties))
	FBS_DefiningConfig DefiningConfig;

	/** Contains info for the target spawner about how to handle the RLAgent */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ShowOnlyInnerProperties))
	FBS_AIConfig AIConfig;

	/** Contains info for the AudioAnalyzer and PlayerHUD */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ShowOnlyInnerProperties))
	FBS_AudioConfig AudioConfig;

	/** Contains info for the target spawner for BeatGrid specific game modes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ShowOnlyInnerProperties))
	FBS_GridConfig GridConfig;

	/** Contains info for the target spawner about how to spawn the targets, as well as info to give the targets */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ShowOnlyInnerProperties))
	FBS_TargetConfig TargetConfig;

	/** Contains info for dynamic SpawnArea scaling */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ShowOnlyInnerProperties))
	FBS_Dynamic_SpawnArea DynamicSpawnAreaScaling;

	/** Contains info for dynamic target scaling */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ShowOnlyInnerProperties))
	FBS_Dynamic DynamicTargetScaling;

	FORCEINLINE bool operator==(const FBSConfig& Other) const
	{
		if (DefiningConfig == Other.DefiningConfig)
		{
			return true;
		}
		return false;
	}

	/** Generic constructor */
	FBSConfig()
	{
		DefiningConfig = FBS_DefiningConfig();
		AIConfig = FBS_AIConfig();
		AudioConfig = FBS_AudioConfig();
		GridConfig = FBS_GridConfig();
		TargetConfig = FBS_TargetConfig();
		DynamicSpawnAreaScaling = FBS_Dynamic_SpawnArea();
		DynamicTargetScaling = FBS_Dynamic();
	}

	FBSConfig(const EBaseGameMode& InBaseGameMode, const EGameModeDifficulty& InDifficulty)
	{
		DefiningConfig = GetConfigForPreset(InBaseGameMode, InDifficulty);
		AIConfig = FBS_AIConfig();
		AudioConfig = FBS_AudioConfig();
		GridConfig = FBS_GridConfig();
		TargetConfig = FBS_TargetConfig();
	}

	/** Returns the defining config for a preset base game mode and difficulty */
	static FBS_DefiningConfig GetConfigForPreset(const EBaseGameMode& InBaseGameMode,
		const EGameModeDifficulty& InDifficulty)
	{
		FBS_DefiningConfig Config;
		Config.BaseGameMode = InBaseGameMode;
		Config.Difficulty = InDifficulty;
		Config.GameModeType = EGameModeType::Preset;
		Config.CustomGameModeName = "";
		return Config;
	}

	/** Used to correct any weird conflicts that might appear between Audio Config and Target Config */
	void OnCreate()
	{
		// Override the player delay to zero if using Capture
		if (AudioConfig.AudioFormat == EAudioFormat::Capture || AudioConfig.AudioFormat == EAudioFormat::Loopback)
		{
			AudioConfig.PlayerDelay = 0.f;
			TargetConfig.SpawnBeatDelay = 0.f;
		}
		AudioConfig.PlayerDelay = TargetConfig.SpawnBeatDelay;

		// Set the Reinforcement Learning Mode
		if (IsCompatibleWithReinforcementLearning())
		{
			AIConfig.ReinforcementLearningMode = AIConfig.bEnableReinforcementLearning
				? EReinforcementLearningMode::ActiveAgent
				: EReinforcementLearningMode::Training;
		}
		else
		{
			AIConfig.ReinforcementLearningMode = EReinforcementLearningMode::None;
		}
	}

	/** Overrides settings for a custom game mode, and calls OnCreate */
	void OnCreate_Custom()
	{
		OnCreate();
		// Override GameModeType to always be Custom
		DefiningConfig.GameModeType = EGameModeType::Custom;
		// Override Difficulty to always be None
		DefiningConfig.Difficulty = EGameModeDifficulty::None;
	}

	/** Returns whether or not the config is able to be used for reinforcement learning */
	bool IsCompatibleWithReinforcementLearning() const
	{
		if (TargetConfig.TargetDamageType == ETargetDamageType::Tracking)
		{
			return false;
		}
		return true;
	}

	/** Sets the target colors from user settings */
	void InitColors(const bool bUseSeparateOutlineColor, const FLinearColor Inactive, const FLinearColor Outline,
		const FLinearColor Start, const FLinearColor Peak, const FLinearColor End)
	{
		TargetConfig.bUseSeparateOutlineColor = bUseSeparateOutlineColor;
		TargetConfig.OutlineColor = Outline;
		if (TargetConfig.TargetSpawningPolicy == ETargetSpawningPolicy::UpfrontOnly)
		{
			TargetConfig.OnSpawnColor = Inactive;
		}
		else
		{
			TargetConfig.OnSpawnColor = Start;
		}
		TargetConfig.InactiveTargetColor = Inactive;
		TargetConfig.StartColor = Start;
		TargetConfig.PeakColor = Peak;
		TargetConfig.EndColor = End;
	}
};

UCLASS(Blueprintable, BlueprintType)
class BEATSHOTGLOBAL_API UBSGameModeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UBSGameModeDataAsset()
	{
		if (!DefaultGameModes.IsEmpty())
		{
			return;
		}
		for (const EBaseGameMode& GameMode : TEnumRange<EBaseGameMode>())
		{
			for (const EGameModeDifficulty& Difficulty : TEnumRange<EGameModeDifficulty>())
			{
				if (Difficulty == EGameModeDifficulty::None)
				{
					continue;
				}
				DefaultGameModes.Add(FBSConfig::GetConfigForPreset(GameMode, Difficulty),
					FBSConfig(GameMode, Difficulty));
			}
		}
	}

	/** Returns all FBSConfig structs representing the default/preset game modes */
	TArray<FBSConfig> GetDefaultGameModes() const
	{
		TArray<FBSConfig> ReturnArray;
		for (const TTuple<FBS_DefiningConfig, FBSConfig>& KeyValue : DefaultGameModes)
		{
			ReturnArray.Add(KeyValue.Value);
		}
		return ReturnArray;
	}

	const TMap<FBS_DefiningConfig, FBSConfig>& GetDefaultGameModesMap() const
	{
		return DefaultGameModes;
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ShowOnlyInnerProperties))
	TMap<FBS_DefiningConfig, FBSConfig> DefaultGameModes;
};
