// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DefiningConfig.generated.h"

/** Enum representing the type of game mode. */
UENUM(BlueprintType)
enum class EGameModeType : uint8
{
	None UMETA(DisplayName="None"),
	Preset UMETA(DisplayName="Preset"),
	Custom UMETA(DisplayName="Custom")};

ENUM_RANGE_BY_FIRST_AND_LAST(EGameModeType, EGameModeType::Preset, EGameModeType::Custom);


/** Enum representing the base game modes. Preset game modes aren't necessarily a BaseGameMode. */
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
	MultiBeatPrecision UMETA(DisplayName="MultiBeatPrecision"),
	MultiBeatSpeed UMETA(DisplayName="MultiBeatSpeed"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EBaseGameMode, EBaseGameMode::SingleBeat, EBaseGameMode::MultiBeatSpeed);


/** Enum representing the default game mode difficulties. */
UENUM(BlueprintType)
enum class EGameModeDifficulty : uint8
{
	None UMETA(DisplayName="None"),
	Normal UMETA(DisplayName="Normal"),
	Hard UMETA(DisplayName="Hard"),
	Death UMETA(DisplayName="Death")};

ENUM_RANGE_BY_FIRST_AND_LAST(EGameModeDifficulty, EGameModeDifficulty::None, EGameModeDifficulty::Death);

/** Small Struct containing the information needed to distinguish between unique default game modes and unique custom
*  game modes.This info persists across different songs, which is why it is separate from FPlayerScore. */
USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FBS_DefiningConfig
{
	GENERATED_BODY()

	/** The type of game mode: either preset or custom. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EGameModeType GameModeType;

	/** The base game mode this game mode is based off of. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EBaseGameMode BaseGameMode;

	/** Custom game mode name if custom, otherwise empty string. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString CustomGameModeName;

	/** Default game mode difficulties, or none if custom. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EGameModeDifficulty Difficulty;

	FBS_DefiningConfig()
	{
		GameModeType = EGameModeType::None;
		BaseGameMode = EBaseGameMode::None;
		CustomGameModeName = "";
		Difficulty = EGameModeDifficulty::None;
	}

	FBS_DefiningConfig(const EGameModeType InGameModeType, const EBaseGameMode InBaseGameMode,
		const FString& InCustomGameModeName, const EGameModeDifficulty InGameModeDifficulty)
	{
		GameModeType = InGameModeType;
		BaseGameMode = InBaseGameMode;
		CustomGameModeName = InCustomGameModeName;
		Difficulty = InGameModeDifficulty;
	}

	FORCEINLINE bool operator==(const FBS_DefiningConfig& Other) const
	{
		// Preset vs Custom
		if (GameModeType == Other.GameModeType)
		{
			if (GameModeType == EGameModeType::Custom)
			{
				// Custom game modes don't depend on difficulty, only CustomGameModeName
				if (CustomGameModeName.Equals(Other.CustomGameModeName, ESearchCase::IgnoreCase))
				{
					return true;
				}
				return false;
			}
			if (GameModeType == EGameModeType::Preset)
			{
				// Preset game modes must match the BaseGameMode and difficulty to be considered equal
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
	}
};
