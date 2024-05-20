// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIConfig.h"
#include "AudioConfig.h"
#include "DefiningConfig.h"
#include "DynamicConfig.h"
#include "GridConfig.h"
#include "TargetConfig.h"
#include "BSConfig.generated.h"

/** Struct representing a game mode. */
USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FBSConfig
{
	GENERATED_BODY()

	/** The defining config for a game mode, containing the names, base, difficulty. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBS_DefiningConfig DefiningConfig;

	/** Contains info for the target spawner about how to handle the RLAgent. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBS_AIConfig AIConfig;

	/** Contains info for the AudioAnalyzer and PlayerHUD. */
	UPROPERTY(BlueprintReadOnly, Transient, meta=(SkipSerialization))
	FBS_AudioConfig AudioConfig;

	/** Contains info for the target spawner for BeatGrid specific game modes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBS_GridConfig GridConfig;

	/** Contains info for the target spawner about how to spawn the targets, as well as info to give the targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBS_TargetConfig TargetConfig;

	/** Contains info for dynamic SpawnArea scaling. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBS_Dynamic_SpawnArea DynamicSpawnAreaScaling;

	/** Contains info for dynamic target scaling. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBS_Dynamic DynamicTargetScaling;

	FORCEINLINE bool operator==(const FBSConfig& Other) const
	{
		if (DefiningConfig == Other.DefiningConfig)
		{
			return true;
		}
		return false;
	}

	FBSConfig();

	FBSConfig(EBaseGameMode InBaseGameMode, EGameModeDifficulty InDifficulty);

	/** Returns the defining config for a preset base game mode and difficulty. */
	static FBS_DefiningConfig GetConfigForPreset(EBaseGameMode InBaseGameMode, EGameModeDifficulty InDifficulty);

	/** Used to correct any weird conflicts that might appear between Audio Config and Target Config. */
	void OnCreate();

	/** Overrides settings for a custom game mode, and calls OnCreate. */
	void OnCreate_Custom();

	/** Returns whether the config is compatible with reinforcement learning. */
	bool IsCompatibleWithReinforcementLearning() const;

	/** Sets the target colors from user settings. */
	void InitColors(bool bUseSeparateOutlineColor, const FLinearColor& Inactive, const FLinearColor& Outline,
		const FLinearColor& Start, const FLinearColor& Peak, const FLinearColor& End, const FLinearColor& TrackingDam,
		const FLinearColor& NotTrackingDam);

	/** Returns a Json Object representation of the struct. */
	FString ToString() const;

	/** Returns a serialized Json object representation of the struct. */
	FString EncodeToString() const;

	/** Initializes the OutConfig from a Json object, returns true if successful. */
	static bool FromString(const FString& JsonString, FBSConfig& OutConfig, FText* OutFailReason = nullptr);

	/** Initializes the OutConfig from a serialized Json object, returns true if successful. */
	static bool DecodeFromString(const FString& EncodedString, FBSConfig& OutConfig, FText* OutFailReason = nullptr);
};
