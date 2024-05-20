// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TargetCommon.h"
#include "BSGameModeConfig/BSConfig.h"
#include "SpawnAreaManagerComponent.generated.h"

class USpawnArea;
struct FAccuracyData;

struct FIndexPair
{
	int32 StartIndex, EndIndex;

	FIndexPair() : StartIndex(-1), EndIndex(-1)
	{
	}

	FIndexPair(const int32 InStart, const int32 InEnd) : StartIndex(InStart), EndIndex(InEnd)
	{
	}

	friend FORCEINLINE uint32 GetTypeHash(const FIndexPair& Other)
	{
		return HashCombine(GetTypeHash(Other.StartIndex), GetTypeHash(Other.EndIndex));
	}

	FORCEINLINE bool operator==(const FIndexPair& Other) const
	{
		return StartIndex == Other.StartIndex && EndIndex == Other.EndIndex;
	}

	FORCEINLINE bool operator<(const FIndexPair& Other) const
	{
		if (EndIndex == Other.EndIndex)
		{
			return StartIndex < Other.StartIndex;
		}
		return EndIndex < Other.EndIndex;
	}
};

struct FRectDims
{
	int32 Width, Height;

	FRectDims() : Width(0), Height(0)
	{
	}

	FRectDims(const int32 InWidth, const int32 InHeight) : Width(InWidth), Height(InHeight)
	{
	}
};

/** A unique pair of factors for a number. */
struct FFactor
{
	int32 Factor1;
	int32 Factor2;
	int32 Distance;

	FFactor(): Factor1(-1), Factor2(-1), Distance(-1)
	{
	}

	FFactor(const int32 F1, const int32 F2) : Factor1(F1), Factor2(F2), Distance(abs(F1 - F2))
	{
	}

	explicit FFactor(const int32 InDistance) : Factor1(-1), Factor2(-1), Distance(InDistance)
	{
	}

	bool IsValid() const
	{
		return Factor1 != -1 && Factor2 != -1;
	}

	FORCEINLINE bool operator==(const FFactor& Other) const
	{
		if (Factor1 == Other.Factor1 && Factor2 == Other.Factor2)
		{
			return true;
		}
		if (Factor1 == Other.Factor2 && Factor2 == Other.Factor1)
		{
			return true;
		}
		return false;
	}

	FORCEINLINE bool operator<(const FFactor& Other) const
	{
		if (Distance == Other.Distance)
		{
			if (Factor1 == Other.Factor1)
			{
				return Factor2 < Other.Factor2;
			}
			return Factor1 < Other.Factor1;
		}
		return Distance < Other.Distance;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FFactor& Factor)
	{
		uint32 Hash = 0;

		const int32 Min = Factor.Factor1 < Factor.Factor2 ? Factor.Factor1 : Factor.Factor2;
		const int32 Max = Factor.Factor1 < Factor.Factor2 ? Factor.Factor2 : Factor.Factor1;

		Hash = HashCombine(Hash, GetTypeHash(Min));
		Hash = HashCombine(Hash, GetTypeHash(Max));

		return Hash;
	}
};

/** A rectangle with a unique start and end index found within FindLargestRectangle. */
struct FSubRectangle
{
	/** Number of columns in the total spawn area. */
	inline static int32 NumCols = 0;

	/** Index of the element in IndexValidity. */
	int32 Index;

	/** Column index. */
	int32 ColIndex;

	/** The start index (bottom left) and end index (top right) of the sub rectangle. */
	FIndexPair BoundingIndices;

	/** Dimensions of the sub rectangle. */
	FRectDims Dimensions;

	/** Area of the sub rectangle. */
	int32 Area;

	/** Start and end row indices. */
	FIndexPair Row;

	/** Start and end column indices. */
	FIndexPair Col;

	/** Indices that are included within the Index of this rectangle. */
	TSet<int32> AdjacentIndices;

	/** Indices that have been validated as being potential start index candidates. */
	TArray<FIndexPair> StartIndexCandidates;

	FSubRectangle() : Index(-1), ColIndex(-1), Area(0)
	{
	}

	FSubRectangle(const int32 InIndex, const int32 InColIndex, const int32 InHeight) : Index(InIndex),
		ColIndex(InColIndex), Dimensions(0, InHeight), Area(0)
	{
	}

	/** Updates the width, StartEndIndex, Dimensions, Area, Row, and Col. */
	void UpdateDimensions(const int32 InNewWidth)
	{
		Dimensions.Width = InNewWidth;
		BoundingIndices.StartIndex = Index - NumCols * (Dimensions.Height - 1);
		BoundingIndices.EndIndex = Index + (Dimensions.Width - 1);
		Area = Dimensions.Width * Dimensions.Height;
		Row = FIndexPair(BoundingIndices.StartIndex / NumCols, BoundingIndices.EndIndex / NumCols);
		Col = FIndexPair(BoundingIndices.StartIndex % NumCols, BoundingIndices.EndIndex % NumCols);
	}

	/** Returns whether the rectangle has area greater than 1 and the EndIndex is greater than StartIndex. */
	bool IsValid() const
	{
		return Area > 0 && BoundingIndices.EndIndex > BoundingIndices.StartIndex;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("Index: [%d, %d] Size: %dx%d"), BoundingIndices.StartIndex,
			BoundingIndices.EndIndex, Dimensions.Width, Dimensions.Height);
	}

	FORCEINLINE bool operator==(const FSubRectangle& Other) const
	{
		return BoundingIndices == Other.BoundingIndices;
	}

	FORCEINLINE bool operator<(const FSubRectangle& Other) const
	{
		return BoundingIndices < Other.BoundingIndices;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FSubRectangle& Other)
	{
		return GetTypeHash(Other.BoundingIndices);
	}

	static void SetNumCols(const int32 InNumCols) { NumCols = InNumCols; }
};

/** KeyFuncs for a set of FSubRectangles. Uses FIndexPair as set key. */
struct FFSubRectangleKeyFuncs : BaseKeyFuncs<FSubRectangle, FIndexPair, false>
{
	/** Compares two keys for equality. */
	static FORCEINLINE bool Matches(const FIndexPair& A, const FIndexPair& B)
	{
		return A == B;
	}

	/** Calculates a hash index for a key. */
	static FORCEINLINE uint32 GetKeyHash(const FIndexPair& Key)
	{
		return GetTypeHash(Key);
	}

	/** Extracts the key from an element. */
	static FORCEINLINE const FIndexPair& GetSetKey(const FSubRectangle& Element)
	{
		return Element.BoundingIndices;
	}
};

/** Contains info about a candidate for a GridBlock largest rectangle. */
struct FRectCandidate
{
	/** The factor for this candidate. */
	FFactor Factor;

	/** (ChosenStartRowIndex, ChosenEndRowIndex). */
	FIndexPair ChosenRow;

	/** (ChosenStartColIndex, ChosenEndColIndex). */
	FIndexPair ChosenCol;

	/** Sub rectangles within this rectangle candidate. */
	TSet<FSubRectangle, FFSubRectangleKeyFuncs> SubRectangles;

	/** Chosen sub rectangle */
	FSubRectangle ChosenSubRectangle;

	/** The number of rows in the largest rectangle found. */
	int32 NumRowsAvailable;

	/** The number of cols in the largest rectangle found. */
	int32 NumColsAvailable;

	/** The true size of the block. Set to min(Area, BlockSize). */
	int32 ActualBlockSize;

	/** The size of the block used to set the Chosen Start/End Row/Col Indices. */
	int32 ChosenBlockSize;

	FRectCandidate(): NumRowsAvailable(0), NumColsAvailable(0), ActualBlockSize(0), ChosenBlockSize(0)
	{
	}

	explicit FRectCandidate(const FFactor& InFactor) : Factor(InFactor), NumRowsAvailable(-1), NumColsAvailable(-1),
	                                                   ActualBlockSize(-1), ChosenBlockSize(-1)
	{
	}

	~FRectCandidate() = default;

	/** Updates the candidate with new dimensions and indices. */
	void UpdateSubRectangles(const FSubRectangle& SubRectangle)
	{
		SubRectangles.Add(SubRectangle);
	}

	/** Merges similar sub rectangles. */
	void MergeSubRectangles();

	/** Sets the value of ChosenSubRectangle. Updates NumRowsAvailable, NumColsAvailable, ActualBlockSize, 
	 *  and initializes the ChosenRow, ChosenCol to SubRectangle.Row, SubRectangle.Col. */
	void SetChosenSubRectangle(const FSubRectangle& SubRectangle, const int32 InBlockSize)
	{
		ChosenSubRectangle = SubRectangle;

		NumRowsAvailable = SubRectangle.Row.EndIndex - SubRectangle.Row.StartIndex + 1;
		NumColsAvailable = SubRectangle.Col.EndIndex - SubRectangle.Col.StartIndex + 1;
		ActualBlockSize = FMath::Min(InBlockSize, SubRectangle.Area);

		ChosenRow = SubRectangle.Row;
		ChosenCol = SubRectangle.Col;
	}

	/** Returns true if the first factor is less than number of rows available and the second factor is less than the
     *  number of columns available. */
	bool FirstFactorComboFits() const
	{
		const bool bF1FitsRows = Factor.Factor1 <= NumRowsAvailable;
		const bool bF2FitsCols = Factor.Factor2 <= NumColsAvailable;
		return bF1FitsRows && bF2FitsCols;
	}

	/** Returns true if the first factor is less than number of columns available and the second factor is less than the
	 *  number of rows available. */
	bool SecondFactorComboFits() const
	{
		const bool bF1FitsCols = Factor.Factor1 <= NumColsAvailable;
		const bool bF2FitsRows = Factor.Factor2 <= NumRowsAvailable;
		return bF2FitsRows && bF1FitsCols;
	}

	/** Returns true if both factors fit. */
	bool AllFactorsFit() const
	{
		return FirstFactorComboFits() && SecondFactorComboFits();
	}

	/** Returns true if the ChosenSubRectangle is valid. */
	bool HasChosenSubRectangle() const
	{
		return ChosenSubRectangle.IsValid();
	}

	/** Returns a human-readable string of the rectangle. */
	FString ToString() const
	{
		return FString::Printf(TEXT("Candidate [%d, %d]"), Factor.Factor1, Factor.Factor2);
	}

	FORCEINLINE bool operator<(const FRectCandidate& Other) const
	{
		// Sort by factor
		return Factor < Other.Factor;
	}

	FORCEINLINE bool operator==(const FRectCandidate& Other) const
	{
		return Factor == Other.Factor;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FRectCandidate& Factor)
	{
		return GetTypeHash(Factor.Factor);
	}
};

/** KeyFuncs for a set of FRectCandidates. Uses FFactor as set key. */
struct FLargestRectangleKeyFuncs : BaseKeyFuncs<FRectCandidate, FFactor, false>
{
	/** Compares two keys for equality */
	static FORCEINLINE bool Matches(const FFactor& A, const FFactor& B)
	{
		return A == B;
	}

	/** Calculates a hash index for a key. */
	static FORCEINLINE uint32 GetKeyHash(const FFactor& Key)
	{
		return GetTypeHash(Key);
	}

	/** Extracts the key from an element. */
	static FORCEINLINE const FFactor& GetSetKey(const FRectCandidate& Element)
	{
		return Element.Factor;
	}
};

using FSubRectangleSet = TSet<FSubRectangle, FFSubRectangleKeyFuncs>;
using FRectangleSet = TSet<FRectCandidate, FLargestRectangleKeyFuncs>;

/** Class responsible for creating and managing Spawn Area objects. */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEATSHOT_API USpawnAreaManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class FTargetCollisionTest;
	friend class ABeatShotGameModeFunctionalTest;

public:
	USpawnAreaManagerComponent();

	virtual void DestroyComponent(bool bPromoteChildren) override;

	/** Initializes basic variables in SpawnAreaManagerComponent.
	 * 
	 *  @param InConfig Shared pointer to the game mode config
	 *  @param InOrigin Origin of the total spawn area
	 *  @param InStaticExtents Static extents of the total spawn area
	 *  @param InStaticExtrema Static extrema of the total spawn area
	 *  @return the size of
	 */
	FIntVector3 Init(const TSharedPtr<FBSConfig>& InConfig, const FVector& InOrigin, const FVector& InStaticExtents,
		const FExtrema& InStaticExtrema);

	/** Resets all variables. */
	void Clear();

	/** Get the value of SpawnAreaDimensions.
	 * 	@return (0, Height, Width) of all SpawnAreas
	 */
	FIntVector3 GetSpawnAreaDimensions() const { return SpawnAreaDimensions; }

	/** Get the value of Size.
	 * 	@return (0, NumHorizontal, NumVertical) total spawn areas
	 */
	FIntVector3 GetSpawnAreaSize() const { return TotalSpawnAreaSize; }

	/** Get a reference to the RequestRLCSpawnArea delegate.
	 *  @return FRequestRLCSpawnArea delegate used to request spawn areas from the reinforcement learning component
	 */
	FRequestRLCSpawnArea& GetSpawnAreaRequestDelegate() { return RequestRLCSpawnArea; }

	/** Get a reference to the FRequestMovingTargetLocations delegate.
	 *  @return FRequestMovingTargetLocations delegate used to request moving target locations
	 */
	FRequestMovingTargetLocations& GetRequestMovingTargetLocationsDelegate() { return RequestMovingTargetLocations; }

	/** Finds a SpawnArea with the matching location and increments TotalTrackingDamagePossible.
	 * 	@param InLocation target location to find the SpawnArea by
	 */
	void UpdateTotalTrackingDamagePossible(const FVector& InLocation) const;

	/** Handles dealing with SpawnAreas that correspond to Damage Events.
	 * 	@param DamageEvent the target damage event structure originating from a target actor receiving damage
	 */
	void HandleTargetDamageEvent(const FTargetDamageEvent& DamageEvent);

	/** Called when the BoxBounds of the TargetManager are changed to update CachedExtrema or CachedEdgeOnly sets.
	 * 	@param Extrema the current extrema of the total spawn area
	 */
	void OnExtremaChanged(const FExtrema& Extrema);

	/** Get the number of elements in the CachedActivated set.
	 * 	@return number of activated Spawn Areas
	 */
	int32 GetNumActivated() const { return CachedActivated.Num(); }

	/** Get the number of Spawn Areas that are flagged as managed but not activated.
	 * 	@return number of deactivated Spawn Areas
	 */
	int32 GetNumDeactivated() const { return GetDeactivatedSpawnAreas().Num(); }

	/** Get the number of elements in the CachedManaged set.
	 * 	@return number of managed Spawn Areas
	 */
	int32 GetNumManaged() const { return CachedManaged.Num(); }

	/** Gathers all total hits and total spawns for the game mode session and converts them into a 5X5 matrix using
	 *  GetAveragedAccuracyData. Calls UpdateAccuracy once the values are copied over, and returns the struct */
	FAccuracyData GetLocationAccuracy();

	/** Get the most recent Spawn Area's index.
	 *  @return the index of the most recent Spawn Area, or -1 if there isn't one
	 */
	int32 GetMostRecentSpawnAreaIndex() const;

	/** Get a Spawn Area's index based on TargetGuid.
	 *  @return the index of the found Spawn Area, or -1 if invalid
	 */
	int32 GetSpawnAreaIndex(const FGuid& TargetGuid) const;

protected:
	/** Sets the value of MostRecentSpawnArea.
	 * 	@param SpawnArea the Spawn Area to assign as the most recent spawn area
	 */
	void SetMostRecentSpawnArea(USpawnArea* SpawnArea) { MostRecentSpawnArea = SpawnArea; }

	/** Sets the value of SpawnAreaDimensions based on the Target Distribution Policy
	 *  and PreferredSpawnAreaDimensions. */
	void SetSpawnAreaDimensions();

	/** Initializes all static variables for Spawn Areas, Sets the TotalSpawnAreaSize, creates all Spawn Area objects,
	 *  and adds them to the SpawnAreas set and AreaKeyMap. Initializes CachedExtrema with all Spawn Areas. */
	void InitializeSpawnAreas();

	/** Use the target spawning policy to decide to consider Managed SpawnAreas as invalid choices for activation.
	 * 	@return whether to consider Managed SpawnAreas as invalid choices for activation
	 */
	bool ShouldConsiderManagedAsInvalid() const;

	/* ------------------------------- */
	/* -- SpawnArea finders/getters -- */
	/* ------------------------------- */

	/** Finds a SpawnArea using its index as an FSetElementId.
	 *  @return Spawn Area found by its index
	 */
	USpawnArea* GetSpawnArea(const int32 Index) const;

	/** Finds a SpawnArea with the matching InLocation using the AreaKeyMap for lookup.
	 *  @return Spawn Area found by its location
	 */
	USpawnArea* GetSpawnArea(const FVector& InLocation) const;

	/** Finds a SpawnArea with the matching TargetGuid using the GuidMap for lookup.
	 *  @return Spawn Area found by its TargetGuid
	 */
	USpawnArea* GetSpawnArea(const FGuid& TargetGuid) const;

	/** Get the value of MostRecentSpawnArea.
	 * 	@return the most recent SpawnArea that was set at the end of ATargetManager::ActivateTarget
	 */
	USpawnArea* GetMostRecentSpawnArea() const { return MostRecentSpawnArea; }

	/** Get the value of OriginSpawnArea.
	 * 	@return the SpawnArea containing the origin
	 */
	USpawnArea* GetOriginSpawnArea() const { return OriginSpawnArea; }

	/** Finds the oldest SpawnArea flagged as recent using each Spawn Area's TimeSetRecent.
	 * 	@return the oldest SpawnArea flagged as recent
	 */
	USpawnArea* GetOldestRecentSpawnArea() const;

	/** Finds the oldest SpawnArea flagged as deactivated, managed, and recent.
	 * 	@return the oldest SpawnArea flagged as deactivated and managed
	 */
	USpawnArea* GetOldestDeactivatedSpawnArea() const;

	/** Find out if a SpawnArea is valid based on an index value.
	 *
	 *	@param InIndex the index to check
	 * 	@return whether the SpawnArea is contained in SpawnAreas
	 */
	bool IsSpawnAreaValid(const int32 InIndex) const;

	/** Get the value of CachedManaged.
	 * 	@return a set of SpawnAreas that are flagged as currently managed
	 */
	TSet<USpawnArea*> GetManagedSpawnAreas() const;

	/** Get the value of CachedManaged difference CachedActivated.
	 * 	@return a set of SpawnAreas that are flagged as currently managed and not flagged as activated
	 */
	TSet<USpawnArea*> GetDeactivatedSpawnAreas() const;

	/** Get the value of CachedRecent.
	 * 	@return a set of SpawnAreas that are flagged as recent
	 */
	TSet<USpawnArea*> GetRecentSpawnAreas() const;

	/** Get the value of CachedActivated.
	 * 	@return a set of SpawnAreas that are flagged as activated
	 */
	TSet<USpawnArea*> GetActivatedSpawnAreas() const;

	/** Get the value of CachedActivated union CachedRecent.
	 *  @return a set of SpawnAreas containing only SpawnAreas flagged as activated or recent
	 */
	TSet<USpawnArea*> GetActivatedOrRecentSpawnAreas() const;

	/** Get the value of CachedManaged union CachedActivated union CachedRecent.
	 *  @return a set of SpawnAreas containing SpawnAreas flagged as managed, activated, or recent
	 */
	TSet<USpawnArea*> GetManagedActivatedOrRecentSpawnAreas() const;

	/** Get the value of CachedManaged difference (CachedActivated union CachedRecent)
	 *  @return a set of SpawnAreas containing SpawnAreas flagged as managed, not activated, and not recent
	 */
	TSet<USpawnArea*> GetManagedDeactivatedNotRecentSpawnAreas() const;

	/** Get the value of SpawnAreas difference CachedManaged difference CachedActivated difference CachedRecent.
	 *  @return a set of SpawnAreas containing SpawnAreas not flagged with anything
	 */
	TSet<USpawnArea*> GetUnflaggedSpawnAreas() const;

	/* ------------------------ */
	/* -- SpawnArea flagging -- */
	/* ------------------------ */

public:
	/** Adds to Managed and Deactivated cache, adds to GuidMap, and flags the SpawnArea as being actively managed by
	 *  TargetManager. Calls SetOccupiedVertices.
	 *  
	 *  @param SpawnAreaIndex the index of the Spawn Area to flag as managed
	 *  @param TargetGuid the TargetGuid to set on the Spawn Area
	 */
	void FlagSpawnAreaAsManaged(const int32 SpawnAreaIndex, const FGuid TargetGuid);

	/** Adds to Activated cache, removes from Deactivate cache, and flags the SpawnArea as activated and removes the
	 *  recent flag if present.
	 *  
	 *  @param TargetGuid the TargetGuid to find the Spawn Area by
	 *  @param TargetScale the TargetScale to set on the Spawn Area
	 */
	void FlagSpawnAreaAsActivated(const FGuid TargetGuid, const FVector& TargetScale);

protected:
	/** Adds to Recent cache and flags the SpawnArea as recent. Handles recent flag removal by setting a timer,
	 *  refreshing recent flags, or immediately removing the recent flag.
	 *  @param SpawnArea the Spawn Area to set as recent
	 */
	void FlagSpawnAreaAsRecent(USpawnArea* SpawnArea);

	/** Removes from Managed cache, removes from Deactivated cache, removes from GuidMap, and removes the Managed flag,
	 *  meaning the target that the SpawnArea represents is not longer actively managed by TargetManager.
	 *  @param TargetGuid the TargetGuid to find the Spawn Area by
	 */
	void RemoveManagedFlagFromSpawnArea(const FGuid TargetGuid);

	/** Removes from Activated cache, adds to Deactivated cache, and removes the activated flag.
	 * 	@param SpawnArea the Spawn Area to remove the activated flag from
	 */
	void RemoveActivatedFlagFromSpawnArea(USpawnArea* SpawnArea);

	/** Removes from Recent cache and removes the Recent flag, meaning the SpawnArea is not longer being considered as
	 *  a blocked SpawnArea. SpawnAreas empty their OccupiedVertices when their Recent Flag is removed.
	 *  @param SpawnArea the Spawn Area to remove the recent flag from
	 */
	void RemoveRecentFlagFromSpawnArea(USpawnArea* SpawnArea);

	/** Removes the oldest SpawnArea recent flags if the max number of recent targets has been exceeded */
	void RefreshRecentFlags();

	/* --------------------------------------------------- */
	/* -- Finding Valid SpawnAreas for Spawn/Activation -- */
	/* --------------------------------------------------- */

public:
	/** Returns a set of valid Spawn Area Guids filtered from the SpawnAreas array. Only considers SpawnAreas that
	 *  are linked to a managed target since activatable requires being managed. Also considers the
	 *  Target Activation Selection Policy.
	 *  
	 *  @param NumToActivate the maximum number of activatable Spawn Areas to return
	 *  @return a set of target Guids
	 */
	TSet<FGuid> GetActivatableTargets(const int32 NumToActivate) const;

	/** Returns a set of target spawn parameters filtered from all Spawn Areas. Broadest search since Spawn Areas
	 *  do not have to be linked to a managed target to be considered. Also considers the Target Distribution Policy
	 *  and Bounds Scaling Policy.
	 *  
	 *  @param Scales an array of target scales to use to help find and filter Spawn Areas. Also sets the found
	 *  Spawn Areas target scales
	 *  @param NumToSpawn the maximum number of spawnable Spawn Areas to return
	 *  @return A set of target spawn parameters
	 */
	TSet<FTargetSpawnParams> GetTargetSpawnParams(const TArray<FVector>& Scales, const int32 NumToSpawn) const;

protected:
	/** Uses a priority list to return a SpawnArea to activate. Always verifies that the candidate has a valid Guid,
	 *  meaning that it corresponds to a valid target. Priority is origin (setting permitting), reinforcement learning
	 *  component (setting permitting), and lastly chooses a random index of ValidSpawnAreas.
	 *
	 *  @param PreviousSpawnArea the previously selected Spawn Area to activate
	 *  @param ValidSpawnAreas a set of valid Spawn Areas to choose from
	 *  @param SelectedSpawnAreas a set of Spawn Areas already chosen to activate
	 *  @return the Spawn Area to activate, or nullptr if none found
	 */
	USpawnArea* ChooseActivatableSpawnArea(const USpawnArea* PreviousSpawnArea,
		const TSet<USpawnArea*>& ValidSpawnAreas, const TSet<USpawnArea*>& SelectedSpawnAreas) const;

	/** Uses a priority list to return a SpawnArea to spawn. Priority is origin (setting permitting), reinforcement
	 *  learning component (setting permitting), and lastly chooses a random index of ValidSpawnAreas.
	 *  
	 *  @param PreviousSpawnArea the previously selected Spawn Area to spawn
	 *  @param ValidSpawnAreas a set of valid Spawn Areas to choose from
	 *  @param SelectedSpawnAreas a set of Spawn Areas already chosen to spawn
	 *  @return the Spawn Area to spawn, or nullptr if none found
	 */
	USpawnArea* ChooseSpawnableSpawnArea(const USpawnArea* PreviousSpawnArea, const TSet<USpawnArea*>& ValidSpawnAreas,
		const TSet<USpawnArea*>& SelectedSpawnAreas) const;

	/** Performs a depth-first search of ValidSpawnAreas, returning a set of SpawnAreas that are all bordering at
	 *  least one another.
	 *  
	 *  @param ValidSpawnAreas a set of valid Spawn Areas to choose from and modify
	 *  @param NumToSpawn the maximum number of Spawn Areas to choose
	 */
	void FindAdjacentGridUsingDFS(TSet<USpawnArea*>& ValidSpawnAreas, const int32 NumToSpawn) const;

	/** Finds the largest valid rectangle and populates ValidSpawnAreas based on it. \n\n
	 *  
	 * 	@param ValidSpawnAreas a set of valid Spawn Areas to choose from and modify
	 *  @param IndexValidity an array of valid Spawn Area indices to choose from
	 *  @param BlockSize the size of block to try and create
	 *  @param bBordering whether to try place the block adjacent to a recent SpawnArea
	 */
	void FindGridBlockUsingLargestRectangle(TSet<USpawnArea*>& ValidSpawnAreas, const TArray<int32>& IndexValidity,
		const int32 BlockSize, const bool bBordering) const;

	/** Removes all SpawnAreas that are occupied by activated, recent targets, and possibly managed targets.
	 *  Recalculates occupied vertices for each spawn area if necessary. Only called when finding Spawnable
	 *  Non-Grid SpawnAreas since grid-based will never have to worry about overlapping.
	 *  
	 * 	@param ValidSpawnAreas a set of valid Spawn Areas to modify
	 *  @param InvalidSpawnAreas a set of Spawn Areas that are invalid or have already been chosen
	 *  @param NewScale the scale of the target to be spawned
	 */
	void RemoveOverlappingSpawnAreas(TSet<USpawnArea*>& ValidSpawnAreas, const TSet<USpawnArea*>& InvalidSpawnAreas,
		const FVector& NewScale) const;

	/** Filters out any SpawnAreas that aren't bordering Current.
	 *
	 * 	@param ValidSpawnAreas a set of valid Spawn Areas to modify
	 *  @param Current the Spawn Area to filter non-adjacent Spawn Areas from
	 *  @return the number of non-adjacent indices removed
	 */
	int32 RemoveNonAdjacentIndices(TSet<USpawnArea*>& ValidSpawnAreas, const USpawnArea* Current) const;

	/** Update the value of MostRecentGridBlocks.
	 *
	 * 	@param ValidSpawnAreas a set of valid Spawn Areas to possible insert into MostRecentGridBlocks
	 *  @param NumToSpawn the number of Spawn Areas that that were attempted to be spawned
	 */
	void UpdateMostRecentGridBlocks(const TSet<USpawnArea*>& ValidSpawnAreas, const int32 NumToSpawn) const;

	/* ------------- */
	/* -- Utility -- */
	/* ------------- */

	/** Returns a set of OutType (Only USpawnArea* and int32 specialized) adjacent to the InSpawnAreas
	 *  according to Directions. 
	 *
	 * 	@param InSpawnAreas a set of Spawn Areas to find adjacent Spawn Areas from
	 *  @param Directions the directions allowed to choose from
	 *  @return a set of type OutType adjacent to InSpawnAreas
	 */
	template <typename OutType>
	TSet<OutType> GetAdjacentSpawnAreas(const TSet<USpawnArea*>& InSpawnAreas,
		const TSet<EAdjacentDirection>& Directions) const;

	/** Creates an array with size equal to the number of Spawn Areas, where each index represents whether the
	 *  SpawnArea should be consider valid.
	 *
	 * 	@param ValidSpawnAreas a set of valid Spawn Areas to get indices from
	 *  @param NumSpawnAreas the total number of Spawn Areas
	 *  @return an array where each index represents whether the SpawnArea should be consider valid
	 */
	static TArray<int32> CreateIndexValidityArray(const TSet<USpawnArea*>& ValidSpawnAreas, const int32 NumSpawnAreas);

	/** Finds the maximum rectangle of valid indices in the matrix. Returns a struct containing the area, start index,
	 *  and end index that correspond to SpawnAreas.
	 *
	 *  @param IndexValidity an array of Spawn Area indices to filter the rectangles from
	 *  @param Factors an array of factors to filter the rectangles from
	 *  @param NumRows total number of Spawn Area rows
	 *  @param NumCols total number of Spawn Area columns
	 */
	static FRectangleSet FindLargestValidRectangles(const TArray<int32>& IndexValidity, const TArray<FFactor>& Factors,
		const int32 NumRows, const int32 NumCols);

	/** Called for every row inside FindLargestValidRectangle. Iterates through the number of columns both forward and
	 *  backward, updating Rectangle values if a new largest rectangle matching the factors criteria is found.
	 *
	 *  @param Rectangles a set of rectangle candidates to update
	 *  @param Factors an array of factors to filter the rectangles from
	 *  @param Heights an array of heights
	 *  @param CurrentRow the current row in the iteration
	 */
	static void UpdateLargestRectangles(FRectangleSet& Rectangles, const TArray<FFactor>& Factors,
		TArray<int32>& Heights, const int32 CurrentRow);

	/** Called if the current height is greater than zero inside the other UpdateLargestRectangles function.
	 *
	 *  @param Rectangles a set of rectangle candidates to update
	 *  @param Factors an array of factors to filter the rectangles from
	 *  @param SubRectangle the rectangle info at the top of the stack
	 */
	static void UpdateSubRectangles(FRectangleSet& Rectangles, const TArray<FFactor>& Factors,
		const FSubRectangle& SubRectangle);

	/** Converts the rectangle set into a sorted array. If bordering, it returns the first rectangle where StartIndex
	 *  candidates is not empty. Otherwise, it returns the first value in the sorted array.
	 *
	 * 	@param Rectangles a set of valid Spawn Areas to modify
	 *  @param bBordering whether to find the first rectangle where StartIndex candidates is not empty
	 *  @param BlockSize Number of targets to spawn
	 *  @return the chosen rectangle candidate
	 */
	static FRectCandidate ChooseRectangleCandidate(const FRectangleSet& Rectangles, const bool bBordering,
		const int32 BlockSize);

	/** Chooses the orientation of the rectangle based on the factors.
	 * 
	 *  @param Rect the rectangle to choose the orientation for
	 *  @param Factor the factor to pull the rectangle dimensions from
	 *  @return a pair of start, end indices
	 */
	static FIndexPair ChooseRectangleOrientation(const FRectCandidate& Rect, const FFactor& Factor);

	/** Chooses the position of the rectangle inside the larger rectangle that was chosen.
	 * 
	 *  @param ChosenRectangle the rectangle to choose the position for
	 *  @param Orientation the IndexPair return from ChooseRectangleOrientation
	 *  @param bBordering whether to prefer bordering indices
	 *  @return A pair of bool values where the first indicates if i corresponds to rows and the second indicates
	 *  if incrementing or decrementing
	 */
	static std::pair<bool, bool> ChooseRectanglePosition(FRectCandidate& ChosenRectangle, const FIndexPair& Orientation,
		const bool bBordering);

	/** Returns a set of factors with the minimum distance between Factor1 and Factor2. */
	static TSet<FFactor> GetPreferredRectangleDimensions(const int32 BlockSize, const int32 NumRows,
		const int32 NumCols);

	/** Updates the rectangle candidates' AdjacentIndices and StartIndexCandidates.
	 *
	 *  @param Rectangles the rectangles to update
	 *  @param Adjacent a set of adjacent indices to to update the rectangles with
	 */
	static void UpdateRectangleCandidateAdjacentIndices(FRectangleSet& Rectangles, const TSet<int32>& Adjacent);

	/** Find all unique factors for a number.
	 *
	 *  @param Number the number to find factors for
	 *  @return a set of all unique factors for a number
	 */
	static TSet<FFactor> FindAllFactors(const int32 Number);

	/** Finds the two factors for a number with the smallest difference between factors. If the number has no factors
	 *  greater than 1, an invalid FFactor is returned.
	 *  
	 *  @param Number the number to find factors for
	 *  @return the Factor with the smallest difference between factors
	 */
	static FFactor FindLargestFactors(const int32 Number);

	/** Tries to find pairs of factors for a number that fit within the two constraints. Initially tries
	 *  FindLargestFactors and falls back to FindAllFactors if it returned invalid. It then iterates through all
	 *  factors that meet the constraints, and chooses the ones with the smallest difference between factors.
	 *
	 *  @param Number the number to find factors for
	 *  @param Constraint1 the first constraint
	 *  @param Constraint2 the second constraint
	 *  @return a set of factors that best fit the given constraints
	 */
	static TSet<FFactor> FindBestFittingFactors(const int32 Number, const int32 Constraint1, const int32 Constraint2);

	/** Returns whether a number is prime.
	 *
	 * 	@param Number the number in question
	 *  @return whether the number is prime
	 */
	static constexpr bool IsPrime(const int32 Number);

	/** Calculates the Manhattan distance between to indices given the number of columns.
	 *
	 * 	@param Index1 the first index
	 * 	@param Index2 the second index
	 * 	@param NumCols the number of columns in the total spawn area
	 *  @return the manhattan distance between the two indices
	 */
	static int32 CalcManhattanDist(const int32 Index1, const int32 Index2, const int32 NumCols);

	/** General SpawnAreas filter function that takes in a filter function to apply. */
	static TArray<int32> FilterIndices(TArray<USpawnArea*>& ValidSpawnAreas, bool (USpawnArea::*FilterFunc)() const);

	/** Preferred dimensions for a Spawn Area */
	UPROPERTY(EditAnywhere, Category="SpawnArea")
	TArray<int32> PreferredSpawnAreaDimensions = {50, 45, 40, 30, 25, 20, 15, 10, 5};

	/* ----------- */
	/* -- Debug -- */
	/* ----------- */

	/** Debug box color for all spawn areas. */
	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColor_AllSpawnAreas = FColor::Cyan;

	/** Debug box color for spawn areas removed due to overlap */
	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColor_InvalidOverlap = FColor::Red;

	/** Debug box color for spawn areas not removed due to overlap */
	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColor_ValidOverlap = FColor::Emerald;

	/** Debug box color for spawnable spawn areas. */
	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColor_SpawnableSpawnAreas = FColor::Emerald;

	/** Debug box color for activatable spawn areas. */
	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColor_ActivatableSpawnAreas = FColor::Emerald;

	/** Debug box color for activated spawn areas. */
	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColor_ActivatedSpawnAreas = FColor::Blue;

	/** Debug box color for deactivated managed spawn areas. */
	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColor_DeactivatedSpawnAreas = FColor::Purple;

	/** Debug box color for deactivated managed spawn areas. */
	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColor_RecentSpawnAreas = FColor::Orange;

	/** Debug box color for spawn areas removed due to the BoxBounds. */
	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColor_RemovedFromExtremaChange = FColor::Red;

	/** Debug box color for spawn areas removed due to being non adjacent. */
	UPROPERTY(EditAnywhere, Category="Debug")
	FColor DebugColor_NonAdjacent = FColor::Yellow;

	/** Line thickness for all debug boxes. */
	UPROPERTY(EditAnywhere, Category="Debug")
	float DebugBoxLineThickness = 4.f;

	/** Size of the debug point for occupied vertices and non-occupied vertices. */
	UPROPERTY(EditAnywhere, Category="Debug")
	float DebugVertexSize = 24.f;

	UPROPERTY(EditAnywhere, Category="Debug")
	float DebugBoxXOffset = 2.f;

	UPROPERTY(EditAnywhere, Category="Debug")
	int32 DebugSphereSegments = 12;

#if !UE_BUILD_SHIPPING

public:
	/** Draws debug boxes and/or lines based on debug bool variables. */
	void DrawDebug() const;

protected:
	/** Draws debug boxes using SpawnAreas. */
	void DrawDebug_Boxes(const TSet<USpawnArea*>& InSpawnAreas, const FColor& Color, const int32 Thickness,
		bool bPersistent) const;

	/** Draws debug points for the spawn areas' occupied vertices and non-occupied vertices as well as a debug sphere. */
	void DrawDebug_Vertices(const TSet<USpawnArea*>& InSpawnAreas, const bool bGenerateNew,
		const bool bDrawSphere) const;

	/** Prints the number of activated, recent, and managed targets. */
	void PrintDebug_SpawnAreaStateInfo() const;

	/** Prints debug info about a SpawnArea. */
	static void PrintDebug_SpawnArea(const USpawnArea* SpawnArea);

	/** Prints debug info about SpawnArea distance. */
	void PrintDebug_SpawnAreaDist(const USpawnArea* SpawnArea) const;

	/** Prints debug info about rectangles found. */
	static void PrintDebug_GridLargestRect(const FRectangleSet& Rectangles, const FRectCandidate& Chosen,
		const int32 NumCols, const FIndexPair& Orientation);

	/** Prints a formatted matrix (upside down from how indexes appear in SpawnAreas so that it matches in game). */
	static void PrintDebug_Matrix(const TArray<int32>& Matrix, const int32 NumRows, const int32 NumCols);

public:
	/** Toggles showing debug boxes for all SpawnAreas. */
	bool bShowDebug_AllSpawnAreas;

	/** Toggles showing debug boxes for valid spawnable spawn areas. */
	bool bShowDebug_SpawnableSpawnAreas;

	/** Toggles showing debug boxes for valid activatable spawn areas. */
	bool bShowDebug_ActivatableSpawnAreas;

	/** Toggles showing debug boxes for activated spawn areas. */
	bool bShowDebug_ActivatedSpawnAreas;

	/** Toggles showing debug boxes for deactivated managed spawn areas. */
	bool bShowDebug_DeactivatedSpawnAreas;

	/** Toggles showing debug boxes for spawn areas removed due to overlap. */
	bool bShowDebug_ValidInvalidSpawnAreas;

	/** Toggles showing debug boxes for recent spawn areas. */
	bool bShowDebug_RecentSpawnAreas;

	/** Toggles showing debug boxes for spawn areas removed due to the BoxBounds. */
	bool bShowDebug_RemovedFromExtremaChange;

	/** Toggles showing debug boxes for spawn areas removed due to being non-adjacent. */
	bool bShowDebug_NonAdjacent;

	/** Toggles showing debug points and a debug sphere for overlapping vertices of activated spawn areas. */
	int32 ShowDebug_Vertices;

	/** Toggles printing the number managed, activated, and recent Spawn Areas. */
	bool bPrintDebug_SpawnAreaStateInfo;

	/** Toggles printing various grid-distribution related info. */
	bool bPrintDebug_Grid;

	// ReSharper disable twice CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	mutable TSet<USpawnArea*> DebugCached_SpawnableValidSpawnAreas;
	mutable TSet<USpawnArea*> DebugCached_NonAdjacentSpawnAreas;

#endif

private:
	/** Pointer to TargetManager's BSConfig. */
	TSharedPtr<FBSConfig> BSConfig;

	/** BSConfig->TargetConfig */
	FBS_TargetConfig& TargetConfig() const { return BSConfig->TargetConfig; };

	/** The total amount of (-, horizontal, vertical) Spawn Areas. */
	FIntVector3 TotalSpawnAreaSize;

	/** (-, width, height) of individual Spawn Areas. */
	FIntVector3 SpawnAreaDimensions;

	/** BoxBounds origin. */
	FVector Origin;

	/** The largest the BoxExtents will be. */
	FVector StaticExtents;

	/** The largest min and max extrema for the SpawnBox. */
	FExtrema StaticExtrema;

	/** All SpawnArea objects inside the larger total spawn area. Set is filled bottom-up, left-to-right. Does not
	 *  change throughout game mode. */
	UPROPERTY()
	TSet<USpawnArea*> SpawnAreas;

	/** Maps each location in the SpawnBox to a unique SpawnArea. Does not change throughout game mode. */
	UPROPERTY()
	TMap<FAreaKey, USpawnArea*> AreaKeyMap;

	/** Maps each Target Guid to a unique SpawnArea. Added when the SpawnArea is flagged as managed, and removed
	 *  when the managed flag is removed. */
	UPROPERTY()
	TMap<FGuid, USpawnArea*> GuidMap;

	/** A set of SpawnAreas that fall within the current BoxBounds. All are added initially, updated when the SpawnBox
	 *  extents changes through the OnExtremaChanged function. */
	UPROPERTY()
	TSet<USpawnArea*> CachedExtrema;

	/** A set of the currently managed SpawnAreas. Added when the SpawnArea is flagged as managed, and removed
	 *  when the managed flag is removed. */
	UPROPERTY()
	TSet<USpawnArea*> CachedManaged;

	/** A set of the currently activated SpawnAreas. Added when the SpawnArea is flagged as activated, and removed
	 *  when the activated flag is removed. */
	UPROPERTY()
	TSet<USpawnArea*> CachedActivated;

	/** A set of the currently recent SpawnAreas. Added when flagged as recent, and removed when
	 *  the recent flag is removed. */
	UPROPERTY()
	TSet<USpawnArea*> CachedRecent;

	/** An array of the most recently spawned grid block sets. */
	mutable TArray<TSet<USpawnArea*>> RecentGridBlocks;

	/** The most recently activated SpawnArea. */
	UPROPERTY()
	USpawnArea* MostRecentSpawnArea;

	/** The SpawnArea that contains the origin */
	UPROPERTY()
	USpawnArea* OriginSpawnArea;

	/** Delegate used to bind a timer handle to RemoveRecentFlagFromSpawnArea(). */
	FTimerDelegate RemoveFromRecentDelegate;

	/** Delegate used to request a SpawnArea selection from the RLC. */
	FRequestRLCSpawnArea RequestRLCSpawnArea;

	/** Delegate used to request active target locations. */
	FRequestMovingTargetLocations RequestMovingTargetLocations;
};

/** Returns a set of Spawn Area indices adjacent to the InSpawnAreas according to Directions.
 *
 * 	@param InSpawnAreas a set of Spawn Areas to find adjacent Spawn Areas from
 *  @param Directions the directions allowed to choose from
 *  @return a set of adjacent Spawn Area indices
 */
template <>
TSet<int32> USpawnAreaManagerComponent::GetAdjacentSpawnAreas<int32>(const TSet<USpawnArea*>& InSpawnAreas,
	const TSet<EAdjacentDirection>& Directions) const;

/** Returns a set of Spawn Areas adjacent to the InSpawnAreas according to Directions.
 *
 * 	@param InSpawnAreas a set of Spawn Areas to find adjacent Spawn Areas from
 *  @param Directions the directions allowed to choose from
 *  @return a set of adjacent SpawnAreas
 */
template <>
TSet<USpawnArea*> USpawnAreaManagerComponent::GetAdjacentSpawnAreas<USpawnArea*>(const TSet<USpawnArea*>& InSpawnAreas,
	const TSet<EAdjacentDirection>& Directions) const;
