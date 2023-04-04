// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameCustomGameMode.generated.h"

#pragma region Enums

/** Enum representing the default game mode names */
UENUM(BlueprintType)
enum class EDefaultMode : uint8
{
	Custom UMETA(DisplayName="Custom"),
	SingleBeat UMETA(DisplayName="SingleBeat"),
	MultiBeat UMETA(DisplayName="MultiBeat"),
	BeatGrid UMETA(DisplayName="BeatGrid"),
	BeatTrack UMETA(DisplayName="BeatTrack")};

ENUM_RANGE_BY_FIRST_AND_LAST(EDefaultMode, EDefaultMode::Custom, EDefaultMode::BeatTrack);

/** Enum representing the spread type of the targets */
UENUM(BlueprintType)
enum class ESpreadType : uint8
{
	None UMETA(DisplayName="None"),
	DynamicEdgeOnly UMETA(DisplayName="Dynamic Edge Only"),
	DynamicRandom UMETA(DisplayName="Dynamic Random"),
	StaticNarrow UMETA(DisplayName="Static Narrow"),
	StaticWide UMETA(DisplayName="Static Wide")};

inline bool IsDynamicSpreadType(const ESpreadType SpreadType) { return (SpreadType == ESpreadType::DynamicEdgeOnly || SpreadType == ESpreadType::DynamicRandom); }

ENUM_RANGE_BY_FIRST_AND_LAST(ESpreadType, ESpreadType::None, ESpreadType::StaticWide);

/** Enum representing the default game mode difficulties */
UENUM(BlueprintType)
enum class EGameModeDifficulty : uint8
{
	None UMETA(DisplayName="None"),
	Normal UMETA(DisplayName="Normal"),
	Hard UMETA(DisplayName="Hard"),
	Death UMETA(DisplayName="Death")};

ENUM_RANGE_BY_FIRST_AND_LAST(EGameModeDifficulty, EGameModeDifficulty::None, EGameModeDifficulty::Death);

/** The player chosen audio format for the current game mode */
UENUM()
enum class EAudioFormat : uint8
{
	None UMETA(DisplayName="None"),
	File UMETA(DisplayName="File"),
	Capture UMETA(DisplayName="Capture")};

ENUM_RANGE_BY_FIRST_AND_LAST(EAudioFormat, EAudioFormat::File, EAudioFormat::Capture);

#pragma endregion

/* Struct representing AI parameters */
USTRUCT(BlueprintType)
struct FBS_AIConfig
{
	GENERATED_BODY()
	
	/* Whether or not to enable the reinforcement learning agent to handle target spawning */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | AI")
	bool bEnableRLAgent;
	
	/* The stored QTable for this game mode */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | AI")
	TArray<float> QTable;
	
	/** Learning rate, or how much to update the Q-Table rewards when a reward is received */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | AI")
	float Alpha;

	/** The exploration/exploitation balance factor. A value = 1 will result in only choosing random values (explore),
	 *  while a value of zero will result in only choosing the max Q-value (exploitation) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | AI")
	float Epsilon;
	
	/** Discount factor, or how much to value future rewards vs immediate rewards */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | AI")
	float Gamma;

	FBS_AIConfig()
	{
		bEnableRLAgent = false;
		QTable = TArray<float>();
		Alpha = 0.9f;
		Epsilon = 0.9f;
		Gamma = 0.9f;
	}
};

/* Struct representing a game mode */
USTRUCT(BlueprintType)
struct FBSConfig
{
	GENERATED_BODY()

	/* The song title */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defining Properties")
	FString SongTitle;

	/* The default game mode name, or custom if custom */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defining Properties")
	EDefaultMode DefaultMode;

	/* The base game mode this game mode is based off of */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defining Properties")
	EDefaultMode BaseGameMode;

	/* Custom game mode name if custom, otherwise empty string */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defining Properties")
	FString CustomGameModeName;

	/* Default game mode difficulties, or none if custom */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defining Properties")
	EGameModeDifficulty GameModeDifficulty;

	/* Whether or not to playback streamed audio */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defining Properties")
	bool bPlaybackAudio;

	/* The audio format type used for the AudioAnalyzer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defining Properties")
	EAudioFormat AudioFormat;

	/* The input audio device */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defining Properties")
	FString InAudioDevice;

	/* The output audio device */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defining Properties")
	FString OutAudioDevice;

	/* The path to the song file */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defining Properties")
	FString SongPath;

	/** Changes how targets are spawned relative to the spawn area. If static, it simply sets the spawn area size.
	 * If dynamic, the spawn area will gradually shrink as consecutive targets are hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	ESpreadType SpreadType;

	/* Whether or not to dynamically change the size of targets as consecutive targets are hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	bool UseDynamicSizing;

	/* Length of song */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	float GameModeLength;

	/* Sets the minimum time between target spawns */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	float TargetSpawnCD;

	/* Sets the minimum distance between recent target spawns */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	float MinDistanceBetweenTargets;

	/* Min multiplier to target size */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	float MinTargetScale;

	/* Max multiplier to target size */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	float MaxTargetScale;

	/* Whether or not to spawn targets only at headshot height */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	bool HeadshotHeight;

	/* Whether or not to center spawn area in the center of wall, vs as close to the ground as possible */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	bool WallCentered;

	/* Maximum time in which target will stay on screen */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	float TargetMaxLifeSpan;

	/* The size of the target spawn BoundingBox. Dimensions are half of the the total length/width */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	FVector BoxBounds;

	/* The min size of the target spawn BoundingBox. Dimensions are half of BoxBounds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	FVector MinBoxBounds;

	/* Delay between AudioAnalyzer Tracker and Player. Also the same value as time between target spawn and peak green target color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	float PlayerDelay;

	/* Whether or not to move the targets forward towards the player after spawning */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	bool bMoveTargetsForward;

	/* How far to move the target forward over its lifetime */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | General")
	float MoveForwardDistance;

	/* How far to move the target forward over its lifetime */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | AI")
	FBS_AIConfig AIConfig;

	/* The minimum speed multiplier for Tracking Game Mode */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | BeatTrack")
	float MinTrackingSpeed;

	/* The maximum speed multiplier for Tracking Game Mode */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | BeatTrack")
	float MaxTrackingSpeed;

	/* The number of horizontal BeatGrid targets*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | BeatGrid")
	int32 NumHorizontalBeatGridTargets;

	/* The number of vertical BeatGrid targets*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | BeatGrid")
	int32 NumVerticalBeatGridTargets;
	
	/* Whether or not to randomize the activation of BeatGrid targets vs only choosing adjacent targets */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | BeatGrid")
	bool RandomizeBeatGrid;

	/* The space between BeatGrid targets */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | BeatGrid")
	FVector2D BeatGridSpacing;

	/* not implemented yet */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Properties | BeatGrid")
	int32 NumTargetsAtOnceBeatGrid;
	
	FORCEINLINE bool operator==(const FBSConfig& Other) const
	{
		if (DefaultMode == Other.DefaultMode && CustomGameModeName.Equals(Other.CustomGameModeName))
		{
			return true;
		}
		return false;
	}
	
	/* Generic initialization */
	FBSConfig()
	{
		DefaultMode = EDefaultMode::Custom;
		BaseGameMode = EDefaultMode::MultiBeat;
		SpreadType = ESpreadType::None;
		GameModeDifficulty = EGameModeDifficulty::Normal;
		bPlaybackAudio = false;
		AudioFormat = EAudioFormat::None;
		InAudioDevice = "";
		OutAudioDevice = "";
		SongPath = "";
		UseDynamicSizing = false;
		MinDistanceBetweenTargets = 10.f;
		GameModeLength = 0.f;
		TargetSpawnCD = 0.35f;
		TargetMaxLifeSpan = 1.5f;
		MinTargetScale = 0.8f;
		MaxTargetScale = 2.f;
		HeadshotHeight = false;
		WallCentered = false;
		PlayerDelay = 0.3f;
		bMoveTargetsForward = false;
		MoveForwardDistance = 0.f;
		AIConfig = FBS_AIConfig();
		SongTitle = "";
		CustomGameModeName = "";
		MinTrackingSpeed = 500.f;
		MaxTrackingSpeed = 500.f;
		NumHorizontalBeatGridTargets = 0;
		NumVerticalBeatGridTargets = 0;
		RandomizeBeatGrid = false;
		NumTargetsAtOnceBeatGrid = -1;
		BeatGridSpacing = FVector2D::ZeroVector;
		BoxBounds = FVector(0.f, 3200.f, 1000.f);
		MinBoxBounds = FVector(0.f, 3200.f, 1000.f);
	}

	FBSConfig(EDefaultMode InDefaultMode, EGameModeDifficulty NewGameModeDifficulty = EGameModeDifficulty::Normal, ESpreadType NewSpreadType = ESpreadType::None)
	{
		// Parameters
		DefaultMode = InDefaultMode;
		BaseGameMode = EDefaultMode::MultiBeat;
		GameModeDifficulty = NewGameModeDifficulty;
		SpreadType = NewSpreadType;

		// Constant for all Game Modes and Difficulties
		bPlaybackAudio = false;
		AudioFormat = EAudioFormat::None;
		InAudioDevice = "";
		OutAudioDevice = "";
		SongPath = "";
		GameModeLength = 0.f;
		HeadshotHeight = false;
		SongTitle = "";
		CustomGameModeName = "";
		WallCentered = false;
		UseDynamicSizing = false;
		MinDistanceBetweenTargets = 10.f;
		PlayerDelay = 0.3f;
		bMoveTargetsForward = false;
		MoveForwardDistance = 0.f;
		TargetSpawnCD = 0.35f;
		TargetMaxLifeSpan = 1.5f;
		MinTargetScale = 0.8f;
		MaxTargetScale = 2.f;
		MinTrackingSpeed = 500.f;
		MaxTrackingSpeed = 500.f;
		NumTargetsAtOnceBeatGrid = -1;
		RandomizeBeatGrid = false;
		BeatGridSpacing = FVector2D::ZeroVector;
		NumHorizontalBeatGridTargets = 0;
		NumVerticalBeatGridTargets = 0;
		BoxBounds = FVector(0.f, 3200.f, 1000.f);
		MinBoxBounds = FVector(0.f, 3200.f, 1000.f);

		// BeatGrid
		if (DefaultMode == EDefaultMode::BeatGrid)
		{
			SpreadType = ESpreadType::None;
			BaseGameMode = EDefaultMode::BeatGrid;
			BoxBounds = FVector(0.f, 3200.f, 1000.f);

			// BeatGrid Difficulties
			if (GameModeDifficulty == EGameModeDifficulty::Normal)
			{
				PlayerDelay = 0.35f;
				TargetSpawnCD = 0.35f;
				TargetMaxLifeSpan = 1.2f;
				MinTargetScale = 0.85f;
				MaxTargetScale = 0.85f;
				NumHorizontalBeatGridTargets = 5;
				NumVerticalBeatGridTargets = 5;
				BeatGridSpacing = FVector2D(75, 50);
			}
			else if (GameModeDifficulty == EGameModeDifficulty::Hard)
			{
				PlayerDelay = 0.3f;
				TargetSpawnCD = 0.30f;
				TargetMaxLifeSpan = 1.f;
				MinTargetScale = 0.7f;
				MaxTargetScale = 0.7f;
				NumHorizontalBeatGridTargets = 10;
				NumVerticalBeatGridTargets = 5;
				BeatGridSpacing = FVector2D(75, 50);
			}
			else if (GameModeDifficulty == EGameModeDifficulty::Death)
			{
				PlayerDelay = 0.25f;
				TargetSpawnCD = 0.25f;
				TargetMaxLifeSpan = 0.75f;
				MinTargetScale = 0.5f;
				MaxTargetScale = 0.5f;
				NumHorizontalBeatGridTargets = 15;
				NumVerticalBeatGridTargets = 10;
				BeatGridSpacing = FVector2D(75, 50);
			}
		}
		// BeatTrack
		else if (DefaultMode == EDefaultMode::BeatTrack)
		{
			SpreadType = ESpreadType::None;
			BaseGameMode = EDefaultMode::BeatTrack;
			WallCentered = true;
			PlayerDelay = 0.f;
			TargetMaxLifeSpan = 0.f;
			MinTrackingSpeed = 500.f;
			MaxTrackingSpeed = 500.f;

			// BeatTrack Difficulties
			if (GameModeDifficulty == EGameModeDifficulty::Normal)
			{
				MinTrackingSpeed = 400.f;
				MaxTrackingSpeed = 500.f;
				TargetSpawnCD = 0.75f;
				MinTargetScale = 1.3f;
				MaxTargetScale = 1.3f;
			}
			else if (GameModeDifficulty == EGameModeDifficulty::Hard)
			{
				MinTrackingSpeed = 500.f;
				MaxTrackingSpeed = 600.f;
				TargetSpawnCD = 0.6f;
				MinTargetScale = 1.f;
				MaxTargetScale = 1.f;
			}
			else if (GameModeDifficulty == EGameModeDifficulty::Death)
			{
				MinTrackingSpeed = 500.f;
				MaxTrackingSpeed = 700.f;
				TargetSpawnCD = 0.45f;
				MinTargetScale = 0.75f;
				MaxTargetScale = 0.75;
			}
		}
		// MultiBeat
		else if (DefaultMode == EDefaultMode::MultiBeat)
		{
			UseDynamicSizing = true;
			BaseGameMode = EDefaultMode::MultiBeat;
			// SpreadType = ESpreadType::DynamicRandom;
			// MultiBeat Difficulties
			if (GameModeDifficulty == EGameModeDifficulty::Normal)
			{
				PlayerDelay = 0.35f;
				TargetSpawnCD = 0.35f;
				TargetMaxLifeSpan = 1.f;
				MinTargetScale = 0.75f;
				MaxTargetScale = 2.f;
			}
			else if (GameModeDifficulty == EGameModeDifficulty::Hard)
			{
				PlayerDelay = 0.3f;
				TargetSpawnCD = 0.3f;
				TargetMaxLifeSpan = 0.75f;
				MinTargetScale = 0.6f;
				MaxTargetScale = 1.5f;
			}
			else if (GameModeDifficulty == EGameModeDifficulty::Death)
			{
				PlayerDelay = 0.25f;
				TargetSpawnCD = 0.20f;
				TargetMaxLifeSpan = 0.5f;
				MinTargetScale = 0.4f;
				MaxTargetScale = 1.25f;
			}

			// MultiBeat Spread Types, defaults to DynamicRandom
			if (SpreadType == ESpreadType::StaticNarrow)
			{
				UseDynamicSizing = false;
				BoxBounds = FVector(0.f, 1600.f, 500.f);
			}
			else if (SpreadType == ESpreadType::StaticWide)
			{
				UseDynamicSizing = false;
				BoxBounds = FVector(0.f, 3200.f, 1000.f);
			}
			else
			{
				SpreadType = ESpreadType::DynamicRandom;
				UseDynamicSizing = true;
				BoxBounds = FVector(0.f, 2000.f, 800.f);
				MinBoxBounds = 0.5f * BoxBounds;
			}
		}
		// SingleBeat
		else if (DefaultMode == EDefaultMode::SingleBeat)
		{
			BaseGameMode = EDefaultMode::SingleBeat;
			UseDynamicSizing = true;
			// SingleBeat Difficulties
			if (GameModeDifficulty == EGameModeDifficulty::Normal)
			{
				PlayerDelay = 0.3f;
				TargetSpawnCD = 0.3f;
				TargetMaxLifeSpan = 0.8f;
				MinTargetScale = 0.75f;
				MaxTargetScale = 2.f;
			}
			else if (GameModeDifficulty == EGameModeDifficulty::Hard)
			{
				PlayerDelay = 0.25f;
				TargetSpawnCD = 0.25f;
				TargetMaxLifeSpan = 0.65f;
				MinTargetScale = 0.6f;
				MaxTargetScale = 1.5f;
			}
			else if (GameModeDifficulty == EGameModeDifficulty::Death)
			{
				PlayerDelay = 0.2f;
				TargetSpawnCD = 0.2f;
				TargetMaxLifeSpan = 0.45f;
				MinTargetScale = 0.4f;
				MaxTargetScale = 1.5f;
			}

			// SingleBeat Spread Types, defaults to DynamicEdgeOnly
			if (SpreadType == ESpreadType::StaticNarrow)
			{
				UseDynamicSizing = false;
				BoxBounds = FVector(0.f, 1600.f, 500.f);
			}
			else if (SpreadType == ESpreadType::StaticWide)
			{
				UseDynamicSizing = false;
				BoxBounds = FVector(0.f, 3200.f, 1000.f);
			}
			else
			{
				SpreadType = ESpreadType::DynamicEdgeOnly;
				UseDynamicSizing = true;
				BoxBounds = FVector(0.f, 2000.f, 800.f);
				MinBoxBounds = 0.5f * BoxBounds;
			}
		}
	}

	/** Returns an array of all default game modes */
	static TArray<FBSConfig> GetDefaultGameModes()
	{
		TArray<FBSConfig> DefaultModes;
		DefaultModes.Add(FBSConfig(EDefaultMode::BeatGrid, EGameModeDifficulty::Normal));
		DefaultModes.Add(FBSConfig(EDefaultMode::BeatTrack, EGameModeDifficulty::Normal));
		DefaultModes.Add(FBSConfig(EDefaultMode::SingleBeat, EGameModeDifficulty::Normal, ESpreadType::DynamicEdgeOnly));
		DefaultModes.Add(FBSConfig(EDefaultMode::MultiBeat, EGameModeDifficulty::Normal, ESpreadType::DynamicRandom));
		return DefaultModes;
	}
	
	void ResetStruct()
	{
		DefaultMode = EDefaultMode::Custom;
		SpreadType = ESpreadType::None;
		MinDistanceBetweenTargets = 10.f;
		GameModeLength = 0.f;
		TargetSpawnCD = 0.35f;
		TargetMaxLifeSpan = 1.5f;
		MinTargetScale = 0.8f;
		MaxTargetScale = 1.5f;
		HeadshotHeight = false;
		WallCentered = false;
		RandomizeBeatGrid = false;
		UseDynamicSizing = false;
		PlayerDelay = 0.3f;
		SongTitle = "";
		CustomGameModeName = "";
		MinTrackingSpeed = 500.f;
		MaxTrackingSpeed = 500.f;
		NumTargetsAtOnceBeatGrid = -1;
		BeatGridSpacing = FVector2D::ZeroVector;
		BoxBounds.X = 0.f;
		BoxBounds.Y = 1600.f;
		BoxBounds.Z = 500.f;
	}
};

UCLASS()
class GLOBAL_API USaveGameCustomGameMode : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FBSConfig> CustomGameModes;
};
