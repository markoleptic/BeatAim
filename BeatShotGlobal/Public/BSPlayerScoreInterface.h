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
	/** @return all player scores loaded from slot. */
	static TArray<FPlayerScore> LoadPlayerScores();

	/** @return all player scores loaded from slot not saved to database. */
	static TArray<FPlayerScore> LoadPlayerScores_UnsavedToDatabase();

	/** Marks all player scores as saved to the database and saves to slot */
	static void SetAllPlayerScoresSavedToDatabase();

	/** Finds any PlayerScores that match the input PlayerScore.
	 *  @param PlayerScore object to use to find matching scores
	 *  @return matching player scores based on DefaultMode, CustomGameModeName, Difficulty, and SongTitle
	 */
	static TArray<FPlayerScore> GetMatchingPlayerScores(const FPlayerScore& PlayerScore);

	/** Saves an instance of an FPlayerScore to slot.
	 *  @param PlayerScoreToSave scores to save
	 */
	static void SavePlayerScoreInstance(const FPlayerScore& PlayerScoreToSave);

	/**
	 *  @param DefiningConfig defining config to use to search for CommonScoreInfo
	 *  @return CommonScoreInfo that matches a given DefiningConfig, or creates a new one if none found
	 */
	static FCommonScoreInfo FindOrAddCommonScoreInfo(const FBS_DefiningConfig& DefiningConfig);

	/** Finds or Adds a DefiningConfig CommonScoreInfo pair and saves to slot.
	 *  @param DefiningConfig defining config to use to search for CommonScoreInfo
	 *  @param CommonScoreInfoToSave CommonScoreInfo to save
	 */
	static void SaveCommonScoreInfo(const FBS_DefiningConfig& DefiningConfig,
		const FCommonScoreInfo& CommonScoreInfoToSave);

	/** Removes a DefiningConfig CommonScoreInfo pair and saves to slot.
	 *  @param DefiningConfig defining config to use to search for CommonScoreInfo
	 *  @return the number of successfully removed entries
	 */
	static int32 RemoveCommonScoreInfo(const FBS_DefiningConfig& DefiningConfig);

	/** Resets the QTable for a DefiningConfig CommonScoreInfo pair and saves to slot.
	 *  @param DefiningConfig defining config to use to search for CommonScoreInfo
	 *  @return the number of successful resets
	 */
	static int32 ResetQTable(const FBS_DefiningConfig& DefiningConfig);
};
