// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TargetCommon.generated.h"

enum class ETargetDamageType : uint8;
class ATarget;
struct FTargetDamageEvent;

/** Contains a map that maps target guid's to their current location. */
struct FMovingTargetLocations
{
	TMap<FGuid, FVector> Map;
};

DECLARE_DELEGATE_RetVal_TwoParams(int32, FRequestRLCSpawnArea, const int32, const TArray<int32>&);
DECLARE_DELEGATE_OneParam(FOnBeatTrackDirectionChanged, const FVector& Vector);
DECLARE_DELEGATE_OneParam(FRequestMovingTargetLocations, FMovingTargetLocations&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetActivated, const ETargetDamageType& DamageType);
DECLARE_MULTICAST_DELEGATE_OneParam(FPostTargetDamageEvent, const FTargetDamageEvent& Event);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetActivated_AimBot, ATarget* Target);

/** Enum representing the bordering directions for a SpawnArea. */
UENUM(BlueprintType)
enum class EAdjacentDirection : uint8
{
	None UMETA(DisplayName="None"),
	Left UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right"),
	Up UMETA(DisplayName="Up"),
	Down UMETA(DisplayName="Down"),
	UpLeft UMETA(DisplayName="UpLeft"),
	UpRight UMETA(DisplayName="UpRight"),
	DownLeft UMETA(DisplayName="DownLeft"),
	DownRight UMETA(DisplayName="DownRight"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EAdjacentDirection, EAdjacentDirection::Left, EAdjacentDirection::DownRight);

/** Enum representing the types of Grid Indices. */
UENUM(BlueprintType)
enum class EGridIndexType : uint8
{
	None UMETA(DisplayName="None"),
	TopLeftCorner UMETA(DisplayName="TopLeftCorner"),
	TopRightCorner UMETA(DisplayName="TopRightCorner"),
	BottomRightCorner UMETA(DisplayName="BottomRightCorner"),
	BottomLeftCorner UMETA(DisplayName="BottomLeftCorner"),
	Top UMETA(DisplayName="Top"),
	Right UMETA(DisplayName="Right"),
	Bottom UMETA(DisplayName="Bottom"),
	Left UMETA(DisplayName="Left"),
	Middle UMETA(DisplayName="Middle"),
	SingleRowLeft UMETA(DisplayName="SingleRowLeft"),
	SingleRowRight UMETA(DisplayName="SingleRowRight"),
	SingleRowMiddle UMETA(DisplayName="SingleRowMiddle"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EGridIndexType, EGridIndexType::TopLeftCorner, EGridIndexType::SingleRowMiddle);

namespace DirectionTypes
{
	/** Cardinal direction Index types that are valid to use when searching for GridBlocks. */
	inline const TSet GridBlock = {
		EAdjacentDirection::Left, EAdjacentDirection::Right, EAdjacentDirection::Up, EAdjacentDirection::Down
	};

	/** Up-Down only index types. */
	inline const TSet Vertical = {EAdjacentDirection::Up, EAdjacentDirection::Down};

	/** Left-right only index types. */
	inline const TSet Horizontal = {EAdjacentDirection::Left, EAdjacentDirection::Right};

	/** All index types. */
	inline const TSet All = {
		EAdjacentDirection::UpLeft, EAdjacentDirection::UpRight, EAdjacentDirection::DownLeft,
		EAdjacentDirection::DownRight, EAdjacentDirection::Left, EAdjacentDirection::Right, EAdjacentDirection::Up,
		EAdjacentDirection::Down
	};
}

/** Contains the minimum and maximum of a Box, i.e. the bottom left corner location and top right corner location. */
struct FExtrema
{
	/** The min extrema. */
	FVector Min;

	/** The max extrema. */
	FVector Max;

	FExtrema() = default;
	~FExtrema() = default;

	FExtrema(const FVector& InMin, const FVector& InMax) : Min(InMin), Max(InMax)
	{
	}
};


/** Key used for location-based indexing. */
USTRUCT()
struct FAreaKey
{
	GENERATED_BODY()

	FVector Vertex_BottomLeft;
	FVector Vertex_TopRight;

	FAreaKey() = default;

	FAreaKey(const FVector& InBotLeft, const FVector& InTopRight) : Vertex_BottomLeft(InBotLeft),
	                                                                Vertex_TopRight(InTopRight)
	{
	}

	FAreaKey(const FVector& InBotLeft, const FIntVector3& InInc) : Vertex_BottomLeft(InBotLeft),
	                                                               Vertex_TopRight(InBotLeft + FVector(0.f, InInc.Y,
		                                                               InInc.Z))
	{
	}

	bool operator==(const FAreaKey& Other) const
	{
		return FMath::IsNearlyEqual(Vertex_BottomLeft.Y, Other.Vertex_BottomLeft.Y, 0.01f) &&
			FMath::IsNearlyEqual(Vertex_BottomLeft.Z, Other.Vertex_BottomLeft.Z, 0.01f) &&
			FMath::IsNearlyEqual(Vertex_TopRight.Y, Other.Vertex_TopRight.Y, 0.01f) && FMath::IsNearlyEqual(
				Vertex_TopRight.Z, Other.Vertex_TopRight.Z, 0.01f);
	}

	friend FORCEINLINE uint32 GetTypeHash(const FAreaKey& AreaKey)
	{
		uint32 Hash = GetTypeHash(AreaKey.Vertex_BottomLeft.Y);
		Hash = HashCombine(Hash, GetTypeHash(AreaKey.Vertex_BottomLeft.Z));
		Hash = HashCombine(Hash, GetTypeHash(AreaKey.Vertex_TopRight.Y));
		Hash = HashCombine(Hash, GetTypeHash(AreaKey.Vertex_TopRight.Z));
		return Hash;
	}
};

/** Parameters used to spawn ATarget actors. Needs SpawnAreaIndex so that Target Manager can tell the Spawn Area
 *  Manager the correct Spawn Area to associate with the Target's Guid. */
struct FTargetSpawnParams
{
	FVector Location;
	FVector Scale;
	int32 SpawnAreaIndex;

	FTargetSpawnParams() = default;

	FTargetSpawnParams(const FVector& InLoc, const FVector& InScale, const int32 InIndex) : Location(InLoc),
		Scale(InScale), SpawnAreaIndex(InIndex)
	{
	}

	FTransform Transform() const
	{
		return FTransform(FRotator(0.f), Location, Scale);
	}

	bool operator==(const FTargetSpawnParams& Other) const
	{
		return SpawnAreaIndex == Other.SpawnAreaIndex;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FTargetSpawnParams& Params)
	{
		return GetTypeHash(Params.SpawnAreaIndex);
	}
};

#if !UE_BUILD_SHIPPING
/** The center and extents for a MovingTargetDirectionMode::Any debug box. */
struct FAnyMovingTargetDirectionModeSector
{
	FVector Center, Extents;

	FAnyMovingTargetDirectionModeSector(const FVector& InCenter, const FVector& InExtents) : Center(InCenter),
		Extents(InExtents)
	{
	}
};

/** Info to debug MovingTargetDirectionMode::Any. */
struct FAnyMovingTargetDirectionModeDebug
{
	TArray<FAnyMovingTargetDirectionModeSector> Sectors;
	FVector LineStart, LineEnd;
};

#endif
