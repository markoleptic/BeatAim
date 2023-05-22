﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveLoadInterface.h"
#include "ReinforcementLearningComponent.h"
#include "BeatShot/BeatShot.h"
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


/** Reinforcement learning agent using QLearning to assist TargetSpawner in AI-generated spawn locations */
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
	void PrintRewards() const;

	int32 GetWidth() const { return ScaledWidth; }
	int32 GetHeight() const { return ScaledHeight; }

	/** Returns a TArray version of the averaged flipped QTable, used to update widget */
	TArray<float> GetAveragedTArrayFromQTable() const;

	/** Returns a TArray version of the full QTable */
	TArray<float> GetSaveReadyQTable() const;

	/** Returns a copy of the QTable */
	nc::NdArray<float> GetQTable() const;

	/** Delegate that broadcasts when the QTable is updated */
	FOnQTableUpdate OnQTableUpdate;

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
	nc::NdArray<float> GetQTableFromTArray(const TArray<float>& InTArray) const;

	/** Converts an NdArray of floats to a TArray of floats, so that it can be serialized and saved */
	static TArray<float> GetTArrayFromQTable(const nc::NdArray<float>& InQTable);

	/** Converts an NdArray of floats to a TArray of floats, so that it can be passed to widget */
	static TArray<float> GetTArrayFromQTable(const nc::NdArray<double>& InQTable);

	/** Broadcasts OnQTableUpdate delegate */
	void UpdateQTableWidget() const;

	/** A 2D array where the row and column have size equal to the number of possible spawn points. An element in the array represents the expected reward from starting at spawn location RowIndex
	 *  and spawning a target at ColumnIndex. Its a scaled down version of the SpawnCounter where each each point in Q-Table represents multiple points in a square area inside the SpawnCounter */
	nc::NdArray<float> QTable;

	/** An array that accumulates the current episodes rewards. Not used for gameplay events */
	nc::NdArray<float> EpisodeRewards;

	/** Learning rate, or how much to update the Q-Table rewards when a reward is received */
	float Alpha;
	
	/** Discount factor, or how much to value future rewards vs immediate rewards */
	float Gamma;
	
	/** The exploration/exploitation balance factor. A value = 1 will result in only choosing random values (explore),
	 *  while a value of zero will result in only choosing the max Q-value (exploitation) */
	float Epsilon;
	
	/** The number of rows in SpawnCounter array */
	int32 SpawnCounterHeight;

	/** The number of columns in SpawnCounter array */
	int32 SpawnCounterWidth;

	/** The size of the SpawnCounter array */
	int32 SpawnCounterSize;

	/** How many rows the SpawnCounterHeight is divided into */
	int32 ScaledHeight = 5;

	/** The number of the SpawnCounterWidth is divided into */
	int32 ScaledWidth = 5;

	/** The size of both dimensions of the QTable (ScaledHeight * ScaledWidth) */
	int32 ScaledSize;

	/** SpawnCounterHeight divided by ScaledHeight */
	int32 HeightScaleFactor;

	/** SpawnCounterWidth divided by ScaledWidth */
	int32 WidthScaleFactor;

	/** An array of structs where each element represents one QTable index that maps to multiple SpawnCounter indices */
	TArray<FQTableIndex> QTableIndices;
};
