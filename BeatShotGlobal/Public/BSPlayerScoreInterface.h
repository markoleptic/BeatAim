// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BSPlayerScoreInterface.generated.h"

struct FCommonScoreInfo;
struct FBS_DefiningConfig;
struct FPlayerScore;
class USaveGamePlayerScore;

UINTERFACE()
class UBSPlayerScoreInterface : public UInterface
{
	GENERATED_BODY()
};

/** Allows interfacing with Player Scores. */
class BEATSHOTGLOBAL_API IBSPlayerScoreInterface
{
	GENERATED_BODY()

public:
	/** Loads all player scores from slot */
	static TArray<FPlayerScore> LoadPlayerScores();

	/** Loads player scores not saved to database */
	static TArray<FPlayerScore> LoadPlayerScores_UnsavedToDatabase();

	/** Marks all player scores as saved to the database and saves to slot */
	static void SetAllPlayerScoresSavedToDatabase();

	/** Finds any PlayerScores that match the input PlayerScore based on DefaultMode, CustomGameModeName, Difficulty, and SongTitle */
	static TArray<FPlayerScore> GetMatchingPlayerScores(const FPlayerScore& PlayerScore);

	/** Saves an instance of an FPlayerScore to slot */
	static void SavePlayerScoreInstance(const FPlayerScore& PlayerScoreToSave);

	/** Returns the CommonScoreInfo that matches a given DefiningConfig, or creates a new one if none found */
	static FCommonScoreInfo FindOrAddCommonScoreInfo(const FBS_DefiningConfig& DefiningConfig);

	/** Finds or Adds a DefiningConfig CommonScoreInfo pair and saves to slot */
	static void SaveCommonScoreInfo(const FBS_DefiningConfig& DefiningConfig,
		const FCommonScoreInfo& CommonScoreInfoToSave);

	/** Removes a DefiningConfig CommonScoreInfo pair and saves to slot. Returns the number of successfully removed entries */
	static int32 RemoveCommonScoreInfo(const FBS_DefiningConfig& DefiningConfig);

	/** Resets the QTable for a DefiningConfig CommonScoreInfo pair and saves to slot. Returns the number of successful resets */
	static int32 ResetQTable(const FBS_DefiningConfig& DefiningConfig);
};
