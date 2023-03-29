﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CoreUObject.h"
#include "SaveLoadInterface.h"
THIRD_PARTY_INCLUDES_START
#pragma push_macro("check")
#undef check
#pragma warning (push)
#pragma warning (disable : 4191)
#pragma warning (disable : 4686)
#include "NumCpp/Public/NumCpp.hpp"
#pragma warning (pop)
#pragma pop_macro("check")
THIRD_PARTY_INCLUDES_END
#include "RLBase.generated.h"

/** A struct representing the inputs for a Reinforcement Learning Algorithm */
USTRUCT()
struct FAlgoInput
{
	GENERATED_BODY()

	int32 StateIndex;
	int32 ActionIndex;
	int32 StateIndex_2;
	int32 ActionIndex_2;
	float Reward;

	FAlgoInput()
	{
		StateIndex = -1;
		ActionIndex = -1;
		StateIndex_2 = -1;
		ActionIndex_2 = -1;
		Reward = 1;
	}

	FAlgoInput(const int32 InStateIndex, const int32 InActionIndex, const int32 InStateIndex_2, const int32 InActionIndex_2, float InReward)
	{
		StateIndex = InStateIndex;
		ActionIndex = InActionIndex;
		StateIndex_2 = InStateIndex_2;
		ActionIndex_2 = InActionIndex_2;
		Reward = InReward;
	}
};

/** A struct to pass the Agent upon Initialization */
USTRUCT()
struct FRLAgentParams
{
	GENERATED_BODY()

	EGameModeActorName GameModeActorName;
	FString CustomGameModeName;
	int32 Size;
	float InAlpha;
	float InGamma;
	float InEpsilon;
	int32 Rows;
	int32 Columns;

	FRLAgentParams()
	{
		GameModeActorName = EGameModeActorName::Custom;
		CustomGameModeName = "";
		Size = 0;
		InAlpha = 0;
		InGamma = 0;
		InEpsilon = 0;
		Rows = 0;
		Columns = 0;
	}
};

/** A struct each each element represents one QTable index mapping to multiple SpawnCounter indices */
USTRUCT()
struct FQTableIndex
{
	GENERATED_BODY()

	int32 QTableIndex;
	TArray<int32> SpawnCounterIndices;

	FQTableIndex()
	{
		QTableIndex = INDEX_NONE;
		SpawnCounterIndices = TArray<int32>();
	}
	FQTableIndex(const int32 InQTableIndex)
	{
		QTableIndex = InQTableIndex;
		SpawnCounterIndices = TArray<int32>();
	}

	FORCEINLINE bool operator ==(const FQTableIndex& Other) const
	{
		if (Other.QTableIndex == QTableIndex)
		{
			return true;
		}
		return false;
	}
};

UCLASS()
class BEATSHOT_API URLBase : public UObject, public ISaveLoadInterface
{
	GENERATED_BODY()

public:
	URLBase();
	
	/** Initializes the QTable, called by TargetSpawner */
	void Init(const FRLAgentParams& AgentParams);

	/** Updates a Q-Table element entry after a target has been spawned and either timed out or destroyed by player */
	virtual void UpdateQTable(const FAlgoInput& In);

	/** Updates EpisodeRewards by appending to the array */
	virtual void UpdateEpisodeRewards(const float RewardReceived);
	
	/** Returns the QTable index corresponding to the maximum reward from starting at QTableIndex, using a greedy approach. Used to update Q-Table, but not to get actual spawn locations */
	int32 GetMaxActionIndex(const int32 SpawnCounterIndex) const;

	/** Returns the SpawnCounter index of the next target to spawn, based on the Epsilon value */
	int32 ChooseNextActionIndex(const TArray<int32>& SpawnCounterIndices) const;

	/** Prints the Q-Table to Unreal console */
	void PrintRewards();

	/** Saves the Q-Table to slot */
	void SaveQTable();

private:
	/** Returns a random SpawnCounter index from the provided SpawnCounterIndices */
	int32 ChooseRandomActionIndex(const TArray<int32>& SpawnCounterIndices) const;

	/** First computes a reverse-sorted array containing QTable indices where the rewards is highest. Then checks to see if there is a valid SpawnCounter index that corresponds to the QTable index.
	 *  If there isn't, it goes to the next QTable index until there are no options left, in which case it returns INDEX_NONE. Returns a SpawnCounter index */
	int32 ChooseBestActionIndex(const TArray<int32>& SpawnCounterIndices) const;

	/** Converts a SpawnCounterIndex to a QTableIndex */
	int32 GetQTableIndexFromSpawnCounterIndex(const int32 SpawnCounterIndex) const;

	/** Returns all SpawnCounter indices corresponding to the QTableIndex */
	TArray<int32> GetSpawnCounterIndexRange(const int32 QTableIndex) const;
	
	/** Converts a TArray of floats to an NdArray */
	nc::NdArray<float> GetQTableFromTArray(const FQTableWrapper& InWrapper) const;

	/** Converts an NdArray of floats to a TArray of floats, so that it can be serialized and saved */
	static TArray<float> GetTArrayFromQTable(nc::NdArray<float> InQTable);

	/** A 2D array where the row and column have size equal to the number of possible spawn points. An element in the array represents the expected reward from starting at spawn location RowIndex
	 *  and spawning a target at ColumnIndex. Its a scaled down version of the SpawnCounter where each each point in Q-Table represents multiple points in a square area inside the SpawnCounter */
	nc::NdArray<float> QTable;

	/** An array that accumulates the current episodes rewards. Not used for gameplay events */
	nc::NdArray<float> EpisodeRewards;

	/** An wrapper around the QTable that also includes which game mode it corresponds to */
	UPROPERTY()
	FQTableWrapper QTableWrapper;

	/** Learning rate, or how much to update the Q-Table rewards when a reward is received */
	float Alpha;
	
	/** Discount factor, or how much to value future rewards vs immediate rewards */
	float Gamma;
	
	/** The exploration/exploitation balance factor. A value = 1 will result in only choosing random values (explore),
	 *  while a value of zero will result in only choosing the max Q-value (exploitation) */
	float Epsilon;
	
	/** The number of rows in SpawnCounter array */
	int32 NumSpawnCounterRows;

	/** The number of columns in SpawnCounter array */
	int32 NumSpawnCounterColumns;

	/** How many rows the SpawnCounter total rows are divided into */
	int32 RowScale = 5;

	/** How many columns the SpawnCounter total columns are divided into */
	int32 ColScale = 5;

	/** The size of both dimensions of the QTable (RowScale * ColScale) */
	int32 Size;

	/** NumSpawnCounterRows divided by RowScale */
	int32 NumRowsPerScaledRow;

	/** NumSpawnCounterColumns divided by ColScale */
	int32 NumColsPerScaledCol;

	/** An array of structs where each element represents one QTable index that maps to multiple SpawnCounter indices */
	TArray<FQTableIndex> QTableIndices;
};
