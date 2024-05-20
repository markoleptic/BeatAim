// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TargetCommon.h"
#include "UObject/Object.h"
#include "SpawnArea.generated.h"

/** A small piece of the total spawn area containing info about its state in relation to targets,
 *  including points that represents where targets have spawned. */
UCLASS()
class BEATSHOT_API USpawnArea : public UObject
{
	GENERATED_BODY()

	/** The unique index for this SpawnArea. */
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
	static FExtrema TotalSpawnAreaExtrema;

	/** Guid associated with a managed target. */
	FGuid Guid;

	/** The center point of the box. */
	FVector CenterPoint;

	/** The point chosen after a successful spawn or activation. Will be inside the sub area. */
	FVector ChosenPoint;

	/** Bottom left vertex of the box, used for comparison between SpawnAreas. This Vertex corresponds to a location
	 *  in AllSpawnLocations (in TargetManager). */
	FVector Vertex_BottomLeft;
	FVector Vertex_TopRight;
	FVector Vertex_BottomRight;
	FVector Vertex_TopLeft;

	/** whether this SpawnArea has a target active. */
	bool bIsActivated;

	/** whether this SpawnArea can be reactivated while activated. */
	bool bAllowActivationWhileActivated;

	/** whether this SpawnArea still corresponds to a managed target. */
	bool bIsManaged;

	/** whether this SpawnArea recently had a target occupy its space. */
	bool bIsRecent;

	/** The time that this SpawnArea was flagged as recent. */
	double TimeSetRecent;

	/** The type of Grid Index. */
	EGridIndexType GridIndexType;

	/** The scale associated with the target if the SpawnArea is currently representing one. */
	FVector TargetScale;

	/** The total number of target spawns in this SpawnArea. */
	int32 TotalSpawns;

	/** The total number of target hits by player in this SpawnArea. */
	int32 TotalHits;

	/** The total amount of tracking damage that was possible at this SpawnArea. */
	int32 TotalTrackingDamagePossible;

	/** The total amount of tracking damage that was dealt at this SpawnArea. */
	int32 TotalTrackingDamage;

	/** The indices of the SpawnAreas adjacent to this SpawnArea, including their direction from this SpawnArea. */
	TMap<EAdjacentDirection, int32> AdjacentIndexMap;

	/** The indices of the SpawnAreas adjacent to this SpawnArea. */
	TSet<int32> AdjacentIndices;

	/** The vertices that this spawn area occupies in the 2D grid based on the target scale, minimum distance
	 *  between targets, minimum overlap radius, and size of the SpawnArea.
	 *
	 *  OccupiedVertices are set when finding multiple spawn locations (RemoveOverlappingSpawnAreas) or when flagged as
	 *  managed or activated. Only updated if the target scale changes. */
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

	/** Returns the index assigned on initialization. */
	int32 GetIndex() const { return Index; }

	/** Returns the chosen point of the last spawn or activation. */
	FVector GetChosenPoint() const { return ChosenPoint; }

	/** Returns the bottom left vertex of the spawn area 2D representation. */
	FVector GetBottomLeftVertex() const { return Vertex_BottomLeft; }

	/** Returns the middle location between the bottom left and top left. */
	FVector GetCenterPoint() const { return CenterPoint; };

	/** Returns whether the SpawnArea contains an activated target. */
	bool IsActivated() const { return bIsActivated; }

	/** Returns whether the SpawnArea contains a managed target. */
	bool IsManaged() const { return bIsManaged; }

	/** Returns whether the SpawnArea contains a recent target. */
	bool IsRecent() const { return bIsRecent; }

	/** Returns whether this SpawnArea can be reactivated while activated. */
	bool CanActivateWhileActivated() const { return bAllowActivationWhileActivated; }

	/** Returns whether the index borders the SpawnArea. */
	bool IsBorderingIndex(const int32 InIndex) const;

	/** Returns the time that this SpawnArea was flagged as recent. */
	double GetTimeSetRecent() const { return TimeSetRecent; }

	/** Returns a const reference to the map that maps the direction from this SpawnArea to adjacent SpawnArea
	 *  indices. */
	const TMap<EAdjacentDirection, int32>& GetAdjacentIndexMap() const { return AdjacentIndexMap; }

	/** Returns a const reference to the set of SpawnArea indices adjacent to this SpawnArea. */
	const TSet<int32>& GetAdjacentIndices() const { return AdjacentIndices; }

	/** Returns a set of SpawnArea indices adjacent to this SpawnArea that match the provided directions. */
	TSet<int32> GetAdjacentIndices(const TSet<EAdjacentDirection>& Directions) const;

	/** Returns the vertices that this SpawnArea occupies in space based on target scale and other factors. */
	const TSet<FVector>& GetOccupiedVertices() const { return OccupiedVertices; }

	/** Adds vectors that are inside the sphere if bOccupied is true, otherwise adds vectors outside the sphere. */
	TSet<FVector> MakeVerticesBase(const FVector& InScale, const bool bOccupied) const;

	/** Finds and returns the vertices that overlap with SpawnArea by tracing a circle around the SpawnArea based on
	 *  the target scale, minimum distance between targets, minimum overlap radius, and size of the SpawnArea. */
	TSet<FVector> MakeOccupiedVertices(const FVector& InScale) const;

	/** Returns the vertices that this SpawnArea did not occupy in space after tracing a sphere
	 *  based on target scale and other factors. Only used for debug purposes. */
	TSet<FVector> MakeUnoccupiedVertices(const FVector& InScale) const;

	/** Returns the type of grid index. */
	EGridIndexType GetIndexType() const { return GridIndexType; }

	/** Returns the total targets spawned within this SpawnArea. */
	int32 GetTotalSpawns() const { return TotalSpawns; }

	/** Returns the total targets hit by the player within this SpawnArea. */
	int32 GetTotalHits() const { return TotalHits; }

	/** Returns the total tracking damage applied to a target by the player within this SpawnArea. */
	int32 GetTotalTrackingDamage() const { return TotalTrackingDamage; }

	/** Returns the total tracking damage that could possibly be applied to a target by the player
	 *  within this SpawnArea. */
	int32 GetTotalTrackingDamagePossible() const { return TotalTrackingDamagePossible; }

	/** Returns the scale of the last target spawned in this SpawnArea. */
	FVector GetTargetScale() const { return TargetScale; }

	/** Returns the Guid of the last target spawned in this SpawnArea. */
	FGuid GetGuid() const { return Guid; }

	/** Sets the Guid of this SpawnArea. */
	void SetGuid(const FGuid InGuid) { Guid = InGuid; }

	/** Resets the Guid of this SpawnArea. */
	void ResetGuid() { Guid.Invalidate(); }

	/** Sets the TargetScale. */
	void SetTargetScale(const FVector& InScale);

	/** Sets the value of ChosenPoint, where the target should actually be spawned. */
	void SetChosenPoint(const FVector& InLocation);

	/** Returns an array of indices that border the index when looking at the array like a 2D grid. */
	static TMap<EAdjacentDirection, int32> CreateAdjacentIndices(const EGridIndexType InGridIndexType,
		const int32 InIndex, const int32 InWidth);

	/** Returns the corresponding index type depending on the InIndex, InSize, and InWidth. */
	static EGridIndexType FindIndexType(const int32 InIndex, const int32 InSize, const int32 InWidth);

	/** Flags this SpawnArea as corresponding to a target being managed by TargetManager. If false,
	 *  clears Occupied Vertices. */
	void SetIsManaged(const bool bManaged);

	/** Sets the activated state and the persistently activated state for this SpawnArea. */
	void SetIsActivated(const bool bActivated, const bool bAllow = false);

	/** Flags this SpawnArea as recent, and records the time it was set as recent. */
	void SetIsRecent(const bool bSetIsRecent);

	/** Sets the value of OccupiedVertices. */
	void SetOccupiedVertices(const TSet<FVector>& InVertices);

	/** Increments the total amount of spawns in this SpawnArea, including handling special case where it has not
	 *  spawned there yet. */
	void IncrementTotalSpawns();

	/** Increments the total amount of hits in this SpawnArea. */
	void IncrementTotalHits();

	/** Increments TotalTrackingDamagePossible. */
	void IncrementTotalTrackingDamagePossible();

	/** Increments TotalTrackingDamage. */
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
