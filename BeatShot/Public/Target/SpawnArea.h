﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SpawnArea.generated.h"

/** Enum representing the bordering directions for a SpawnArea */
UENUM(BlueprintType)
enum class EBorderingDirection : uint8
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

ENUM_RANGE_BY_FIRST_AND_LAST(EBorderingDirection, EBorderingDirection::Left, EBorderingDirection::DownRight);

/** Enum representing the types of Grid Indices */
UENUM(BlueprintType)
enum class EGridIndexType : uint8
{
	None UMETA(DisplayName="None"),
	Corner_TopLeft UMETA(DisplayName="Corner_TopLeft"),
	Corner_TopRight UMETA(DisplayName="Corner_TopRight"),
	Corner_BottomRight UMETA(DisplayName="Corner_BottomRight"),
	Corner_BottomLeft UMETA(DisplayName="Corner_BottomLeft"),
	Border_Top UMETA(DisplayName="Border_Top"),
	Border_Right UMETA(DisplayName="Border_Right"),
	Border_Bottom UMETA(DisplayName="Border_Bottom"),
	Border_Left UMETA(DisplayName="Border_Left"),
	Middle UMETA(DisplayName="Middle"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EGridIndexType, EGridIndexType::Corner_TopLeft, EGridIndexType::Middle);

/** A small piece of the total spawn area containing info about its state in relation to targets,
 *  including points that represents where targets have spawned */
UCLASS()
class BEATSHOT_API USpawnArea : public UObject
{
	GENERATED_BODY()

	/** The unique index for this SpawnArea */
	int32 Index;

	/** The width of the SpawnArea in Unreal units. */
	static float Width;

	/** The height of the SpawnArea in Unreal units. */
	static float Height;

	/** The total number of horizontal SpawnAreas. */
	static int32 TotalNumHorizontalSpawnAreas;

	/** The total number of vertical SpawnAreas. */
	static int32 TotalNumVerticalSpawnAreas;
	
	/** The total number of SpawnAreas that this SpawnArea is part of. */
	static int32 Size;

	/** The minimum distance allowed between the edges of targets. */
	static float MinDistanceBetweenTargets;

	/** The minimum and maximum values of the total spawn area. */
	static struct FExtrema TotalSpawnAreaExtrema;

	/** Guid associated with a managed target */
	FGuid Guid;

	/** The center point of the box */
	FVector CenterPoint;

	/** The point chosen after a successful spawn or activation. Will be inside the sub area */
	FVector ChosenPoint;
	
	/** Bottom left vertex of the box, used for comparison between SpawnAreas. This Vertex corresponds to a location
	 *  in AllSpawnLocations (in TargetManager) */
	FVector Vertex_BottomLeft;
	FVector Vertex_TopRight;
	FVector Vertex_BottomRight;
	FVector Vertex_TopLeft;

	/** Whether or not this SpawnArea has a target active */
	bool bIsActivated;

	/** Whether or not this SpawnArea can be reactivated while activated */
	bool bAllowActivationWhileActivated;

	/** Whether or not this SpawnArea still corresponds to a managed target */
	bool bIsManaged;

	/** Whether or not this SpawnArea recently had a target occupy its space */
	bool bIsRecent;

	/** The time that this SpawnArea was flagged as recent */
	double TimeSetRecent;

	/** The type of Grid Index */
	EGridIndexType GridIndexType;

	/** The scale associated with the target if the SpawnArea is currently representing one */
	FVector TargetScale;

	/** The total number of target spawns in this SpawnArea */
	int32 TotalSpawns;

	/** The total number of target hits by player in this SpawnArea */
	int32 TotalHits;

	/** The total amount of tracking damage that was possible at this SpawnArea */
	int32 TotalTrackingDamagePossible;

	/** The total amount of tracking damage that was dealt at this SpawnArea */
	int32 TotalTrackingDamage;

	/** The indices of the SpawnAreas adjacent to this SpawnArea, including their direction from this SpawnArea */
	TMap<EBorderingDirection, int32> AdjacentIndexMap;

	/** The indices of the SpawnAreas adjacent to this SpawnArea */
	TSet<int32> AdjacentIndices;

	/** The vertices that this spawn area occupies in the 2D grid based on the target scale, minimum distance
	 *  between targets, minimum overlap radius, and size of the SpawnArea.
	 *
	 *  OccupiedVertices are set when finding multiple spawn locations (RemoveOverlappingSpawnAreas) or when flagged as
	 *  managed or activated. Only updated if the target scale changes */
	TSet<FVector> OccupiedVertices;

public:
	USpawnArea();

	/** Initializes the SpawnArea object. */
	void Init(const int32 InIndex, const FVector& InBottomLeftVertex);

	/** Sets the size (number of horizontal SpawnAreas * number of vertical SpawnAreas) for all SpawnAreas. */
	static void SetSize(const int32 InSize) { Size = InSize; }

	/** Sets the width for all SpawnAreas. */
	static void SetWidth(const int32 InWidth) { Width = InWidth; }

	/** Sets the width for all SpawnAreas. */
	static void SetHeight(const int32 InHeight) { Height = InHeight; }

	/** Sets the total number of horizontal SpawnAreas for all SpawnAreas. */
	static void SetTotalNumHorizontalSpawnAreas(const int32 InNum) { TotalNumHorizontalSpawnAreas = InNum; }

	/** Sets the total number of vertical SpawnAreas for all SpawnAreas. */
	static void SetTotalNumVerticalSpawnAreas(const int32 InNum) { TotalNumVerticalSpawnAreas = InNum; }

	/** Sets the minimum distance between targets for all SpawnAreas. */
	static void SetMinDistanceBetweenTargets(const float InNum) { MinDistanceBetweenTargets = InNum; }

	/** Sets the total spawn area minimum and maximum values. */
	static void SetTotalSpawnAreaExtrema(const FExtrema& InExtrema);

	/** Returns a random offset between (0, 0, 0) and (0, Width, Height). */
	static FVector GenerateRandomOffset();
	
	/** Returns the width of a Spawn Area. */
	static int32 GetWidth() { return Width; }

	/** Returns the height of a Spawn Area. */
	static int32 GetHeight() { return Height; }

	/** Calculates the radius that should be used to make occupied vertices. */
	static float CalcTraceRadius(const FVector& InScale);
	
	/** Returns the index assigned on initialization */
	int32 GetIndex() const { return Index; }
	
	/** Returns the chosen point of the last spawn or activation */
	FVector GetChosenPoint() const { return ChosenPoint; }
	
	/** Returns the bottom left vertex of the spawn area 2D representation */
	FVector GetBottomLeftVertex() const { return Vertex_BottomLeft; }

	/** Returns the middle location between the bottom left and top left */
	FVector GetCenterPoint() const { return CenterPoint; };
	
	/** Returns whether or not the SpawnArea contains an activated target */
	bool IsActivated() const { return bIsActivated; }
	
	/** Returns whether or not the SpawnArea contains a managed target */
	bool IsManaged() const { return bIsManaged; }
	
	/** Returns whether or not the SpawnArea contains a recent target */
	bool IsRecent() const { return bIsRecent; }
	
	/** Returns whether or not this SpawnArea can be reactivated while activated */
	bool CanActivateWhileActivated() const { return bAllowActivationWhileActivated; }

	/** Returns whether or not the index borders the SpawnArea */
	bool IsBorderingIndex(const int32 InIndex) const;
	
	/** Returns the time that this SpawnArea was flagged as recent */
	double GetTimeSetRecent() const { return TimeSetRecent; }
	
	/** Returns a const reference to the map that maps the direction from this SpawnArea to adjacent SpawnArea indices */
	const TMap<EBorderingDirection, int32>& GetAdjacentIndexMap() const { return AdjacentIndexMap; }

	/** Returns a const reference to the set of SpawnArea indices adjacent to this SpawnArea */
	const TSet<int32>& GetAdjacentIndices() const { return AdjacentIndices; }

	/** Returns a set of SpawnArea indices adjacent to this SpawnArea that match the provided directions */
	TSet<int32> GetAdjacentIndices(const TSet<EBorderingDirection>& Directions) const;
	
	/** Returns the vertices that this SpawnArea occupies in space based on target scale and other factors */
	const TSet<FVector>& GetOccupiedVertices() const { return OccupiedVertices; }
	
	/** Returns the vertices that this SpawnArea did not occupy in space after tracing a sphere
	 *  based on target scale and other factors. Only used for debug purposes */
	TSet<FVector> MakeUnoccupiedVertices(const FVector& InScale) const;
	
	/** Returns the type of grid index */
	EGridIndexType GetIndexType() const { return GridIndexType; }
	
	/** Returns the total targets spawned within this SpawnArea */
	int32 GetTotalSpawns() const { return TotalSpawns; }
	
	/** Returns the total targets hit by the player within this SpawnArea */
	int32 GetTotalHits() const { return TotalHits; }
	
	/** Returns the total tracking damage applied to a target by the player within this SpawnArea */
	int32 GetTotalTrackingDamage() const { return TotalTrackingDamage; }
	
	/** Returns the total tracking damage that could possibly be applied to a target by the player
	 *  within this SpawnArea */
	int32 GetTotalTrackingDamagePossible() const { return TotalTrackingDamagePossible; }
	
	/** Returns the scale of the last target spawned in this SpawnArea */
	FVector GetTargetScale() const { return TargetScale; }
	
	/** Returns the Guid of the last target spawned in this SpawnArea */
	FGuid GetGuid() const { return Guid; }
	
	/** Sets the Guid of this SpawnArea */
	void SetGuid(const FGuid InGuid) { Guid = InGuid; }
	
	/** Resets the Guid of this SpawnArea */
	void ResetGuid() { Guid.Invalidate(); }

	/** Sets the TargetScale */
	void SetTargetScale(const FVector& InScale);

	/** Sets the value of ChosenPoint, where the target should actually be spawned. */
	void SetChosenPoint(const FVector& InLocation);
	
	/** Returns an array of indices that border the index when looking at the array like a 2D grid */
	void SetAdjacentIndices(const EGridIndexType InGridIndexType, const int32 InIndex, const int32 InWidth);

	/** Returns the corresponding index type depending on the InIndex, InSize, and InWidth */
	static EGridIndexType FindIndexType(const int32 InIndex, const int32 InSize, const int32 InWidth);

	/** Flags this SpawnArea as corresponding to a target being managed by TargetManager. If false,
	 *  clears Occupied Vertices */
	void SetIsManaged(const bool bManaged);

	/** Sets the activated state and the persistently activated state for this SpawnArea */
	void SetIsActivated(const bool bActivated, const bool bAllow = false);

	/** Flags this SpawnArea as recent, and records the time it was set as recent. */
	void SetIsRecent(const bool bSetIsRecent);

	/** Sets the value of OccupiedVertices */
	void SetOccupiedVertices(const TSet<FVector>& InVertices);

	/** Finds and returns the vertices that overlap with SpawnArea by tracing a circle around the SpawnArea based on
	 *  the target scale, minimum distance between targets, minimum overlap radius, and size of the SpawnArea */
	TSet<FVector> MakeOccupiedVertices(const FVector& InScale) const;

	/** Increments the total amount of spawns in this SpawnArea, including handling special case where it has not
	 *  spawned there yet */
	void IncrementTotalSpawns();

	/** Increments the total amount of hits in this SpawnArea */
	void IncrementTotalHits();

	/** Increments TotalTrackingDamagePossible */
	void IncrementTotalTrackingDamagePossible();

	/** Increments TotalTrackingDamage */
	void IncrementTotalTrackingDamage();

	#if !UE_BUILD_SHIPPING
	/** The scale last used to generate occupied vertices. */
	FVector LastOccupiedVerticesTargetScale;

	/** The Occupied vertices set if not shipping build. */
	TSet<FVector> DebugOccupiedVertices;

	/** Sets the value of DebugOccupiedVertices, LastOccupiedVerticesTargetScale,
	 *  and returns the DebugOccupiedVertices. */
	TSet<FVector> SetMakeDebugOccupiedVertices(const FVector& InScale);
	#endif
	
	FORCEINLINE bool operator ==(const USpawnArea& Other) const
	{
		if (Index != INDEX_NONE && Index == Other.Index)
		{
			return true;
		}
		if ((Other.Vertex_BottomLeft.Y >= Vertex_BottomLeft.Y) && (Other.Vertex_BottomLeft.Z >= Vertex_BottomLeft.Z) &&
			(Other.Vertex_BottomLeft.Y < Vertex_TopRight.Y - 0.01) && (Other.Vertex_BottomLeft.Z < Vertex_TopRight.Z -
				0.01))
		{
			return true;
		}
		return false;
	}

	FORCEINLINE bool operator <(const USpawnArea& Other) const
	{
		if (Vertex_BottomLeft.Z < Other.Vertex_BottomLeft.Z)
		{
			return true;
		}
		if (Vertex_BottomLeft.Z == Other.Vertex_BottomLeft.Z && Vertex_BottomLeft.Y < Other.Vertex_BottomLeft.Y)
		{
			return true;
		}
		return false;
	}

	FORCEINLINE bool operator ==(const USpawnArea* Other) const
	{
		if (Index != INDEX_NONE && Index == Other->Index)
		{
			return true;
		}
		if ((Other->Vertex_BottomLeft.Y >= Vertex_BottomLeft.Y) && (Other->Vertex_BottomLeft.Z >= Vertex_BottomLeft.Z)
			&& (Other->Vertex_BottomLeft.Y < Vertex_TopRight.Y - 0.01) && (Other->Vertex_BottomLeft.Z < Vertex_TopRight.
				Z - 0.01))
		{
			return true;
		}
		return false;
	}

	FORCEINLINE bool operator <(const USpawnArea* Other) const
	{
		if (Vertex_BottomLeft.Z < Other->Vertex_BottomLeft.Z)
		{
			return true;
		}
		if (Vertex_BottomLeft.Z == Other->Vertex_BottomLeft.Z && Vertex_BottomLeft.Y < Other->Vertex_BottomLeft.Y)
		{
			return true;
		}
		return false;
	}

	friend FORCEINLINE uint32 GetTypeHash(const USpawnArea& SpawnArea)
	{
		return GetTypeHash(SpawnArea.GetIndex());
	}
};


	
/** Key used for location-based indexing */
USTRUCT()
struct FAreaKey
{
	GENERATED_BODY()

	FVector Vertex_BottomLeft;
	FVector Vertex_TopRight;

	FAreaKey() = default;

	FAreaKey(const FVector& InBotLeft, const FVector& InTopRight)
	{
		Vertex_BottomLeft = InBotLeft;
		Vertex_TopRight = InTopRight;
	}

	FAreaKey(const FVector& InBotLeft, const FIntVector3& InInc)
	{
		Vertex_BottomLeft = InBotLeft;
		Vertex_TopRight = InBotLeft + FVector(0.f, InInc.Y, InInc.Z);
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

struct FSpawnAreaIndexKeyFuncs : BaseKeyFuncs<USpawnArea, int32, false>
{
	// Extracts the key from an element
	static FORCEINLINE const KeyInitType& GetSetKey(const USpawnArea& Element)
	{
		// Return the key based on the subset of the element type
		return Element.GetIndex();
	}

	// Computes the hash value for a key
	static FORCEINLINE uint32 KeyHash(const KeyInitType& Key)
	{
		return GetTypeHash(Key);
	}

	// Compares two keys for equality
	static FORCEINLINE bool Matches(const KeyInitType& A, const KeyInitType& B)
	{
		return A == B;
	}
};