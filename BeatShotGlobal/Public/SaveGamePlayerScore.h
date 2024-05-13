// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSGameModeDataAsset.h"
#include "GameFramework/SaveGame.h"
#include "SaveGamePlayerScore.generated.h"

/** Struct only used to save accuracy to database */
USTRUCT()
struct FAccuracyRow
{
	GENERATED_BODY()

	friend struct FAccuracyData;

	UPROPERTY()
	int32 Size;

	UPROPERTY()
	TArray<float> Accuracy;

	UPROPERTY()
	TArray<int64> TotalSpawns;

	UPROPERTY()
	TArray<int64> TotalHits;

	FAccuracyRow()
	{
		Size = 0;
		Accuracy = TArray<float>();
		TotalSpawns = TArray<int64>();
		TotalHits = TArray<int64>();
	}

	FAccuracyRow(const int32 InSize)
	{
		Size = InSize;
		Accuracy.Init(-1.f, Size);
		TotalSpawns.Init(-1, Size);
		TotalHits.Init(0, Size);
	}

	/** Updates the accuracy array based on all TotalSpawns and TotalHits */
private:
	void CalculateAccuracy()
	{
		CheckForEmpty();
		for (int i = 0; i < Size; i++)
		{
			if (TotalSpawns[i] == INDEX_NONE)
			{
				continue;
			}
			Accuracy[i] = static_cast<float>(TotalHits[i]) / static_cast<float>(TotalSpawns[i]);
		}
	}

	void operator+=(const FAccuracyRow& Other)
	{
		if (Other.Size != Size)
		{
			return;
		}
		for (int i = 0; i < Size; i++)
		{
			const int64 NewSpawns = Other.TotalSpawns[i];
			const int64 NewHits = Other.TotalHits[i];
			if (NewSpawns != -1)
			{
				if (TotalSpawns[i] == -1)
				{
					TotalSpawns[i] = NewSpawns;
				}
				else
				{
					TotalSpawns[i] += NewSpawns;
				}
			}
			if (NewHits != -1)
			{
				if (TotalHits[i] == -1)
				{
					TotalHits[i] = NewHits;
				}
				else
				{
					TotalHits[i] += NewHits;
				}
			}
		}
	}

	void CheckForEmpty()
	{
		Size = FMath::Max(5, FMath::Max3(Accuracy.Num(), TotalSpawns.Num(), TotalHits.Num()));
		if (Accuracy.Num() != Size)
		{
			Accuracy.Init(-1.f, Size);
		}
		if (TotalSpawns.Num() != Size)
		{
			TotalSpawns.Init(-1, Size);
		}
		if (TotalHits.Num() != Size)
		{
			TotalHits.Init(0, Size);
		}
	}
};

/** Struct only used to save accuracy to database */
USTRUCT()
struct FAccuracyData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FAccuracyRow> AccuracyRows;

	/** Y is number of SpawnArea columns, Z is number of SpawnArea rows */
	UPROPERTY()
	FVector SpawnAreaSize;

	FAccuracyData()
	{
		AccuracyRows.Init(FAccuracyRow(5), 5);
		SpawnAreaSize = FVector::ZeroVector;
	}

	FAccuracyData(const int32 InNumRows, const int32 InNumCols)
	{
		AccuracyRows.Init(FAccuracyRow(InNumCols), InNumRows);
	}

	/** Adds the InUpdate AccuracyRows to this struct's Accuracy rows, and recalculates the accuracy */
	void UpdateAccuracyRows(const FAccuracyData& InUpdateData)
	{
		for (int i = 0; i < AccuracyRows.Num(); i++)
		{
			if (InUpdateData.AccuracyRows.IsValidIndex(i))
			{
				AccuracyRows[i] += InUpdateData.AccuracyRows[i];
				AccuracyRows[i].CalculateAccuracy();
			}
		}
		SpawnAreaSize = FVector::ZeroVector;
	}

	/** Calculates the accuracy for each accuracy row */
	void CalculateAccuracy()
	{
		for (int i = 0; i < AccuracyRows.Num(); i++)
		{
			AccuracyRows[i].CalculateAccuracy();
		}
	}

	/** Modifies only the accuracy values to conform to a 5X5 matrix if the SpawnAreaSize.Y or SpawnAreaSize.Z is less
	 *  than 5. Only used just before placing inside the PlayerScore. Resets when CalculateAccuracy is called */
	void ModifyForSmallerInput()
	{
		TArray<float> Temp = MakeIndexArray();

		// (2X2) Use Average2X2Array function
		if (SpawnAreaSize.Y == 2 && SpawnAreaSize.Z == 2)
		{
			Average2X2Array(Temp[0], Temp[1], Temp[2], Temp[3]);
		}
		// (3X3) Use AverageUsingUnderflow function
		else if (SpawnAreaSize.Y == 3 && SpawnAreaSize.Z == 3)
		{
			AverageUsingUnderflow(Temp, 3, 3);
		}
		// (4X4) Use AverageUsingUnderflow function
		else if (SpawnAreaSize.Y == 4 && SpawnAreaSize.Z == 4)
		{
			AverageUsingUnderflow(Temp, 4, 4);
		}
		// (2X3) or (3X2) Convert to 3X3 then use AverageUsingUnderflow
		else if ((SpawnAreaSize.Y == 2 || SpawnAreaSize.Z == 2) && (SpawnAreaSize.Y == 3 || SpawnAreaSize.Z == 3))
		{
			const float TopLeft = AccuracyRows[0].Accuracy[0];
			const float BotLeft = AccuracyRows[4].Accuracy[0];
			const float TopRight = AccuracyRows[0].Accuracy[4];
			const float BotRight = AccuracyRows[4].Accuracy[4];

			// (3X2), change TopMid, Mid, BotMid
			if (SpawnAreaSize.Y == 2)
			{
				const float LeftMid = AccuracyRows[2].Accuracy[0];
				const float RightMid = AccuracyRows[2].Accuracy[4];
				Temp.Insert((TopLeft + TopRight) / 2.f, 1);
				Temp.Insert((LeftMid + RightMid) / 2.f, 4);
				Temp.Insert((BotLeft + BotRight) / 2.f, 7);
			}
			// (2X3) SpawnArea, change LeftMid, Mid, RightMid
			else
			{
				const float TopMid = AccuracyRows[0].Accuracy[2];
				const float BotMid = AccuracyRows[4].Accuracy[2];
				Temp.Insert((TopLeft + BotLeft) / 2.f, 3);
				Temp.Insert((TopMid + BotMid) / 2.f, 4);
				Temp.Insert((TopRight + BotRight) / 2.f, 5);
			}

			AverageUsingUnderflow(Temp, 3, 3);
		}
		// (2X4) or (4X2) Average the last row/column of the one that has 4, copy 2 entire row/columns, then average
		else if ((SpawnAreaSize.Y == 2 || SpawnAreaSize.Z == 2) && (SpawnAreaSize.Y == 4 || SpawnAreaSize.Z == 4))
		{
			// (4X2)
			if (SpawnAreaSize.Y == 2)
			{
				AverageMiddle(2, 0, true);
				AverageMiddle(2, 4, true);
				CopyAccuracyCol(0, 1);
				CopyAccuracyCol(4, 3);
				AverageAccuracyCol(2);
			}
			// (2X4)
			else
			{
				AverageMiddle(0, 2, false);
				AverageMiddle(4, 2, false);
				AccuracyRows[1] = AccuracyRows[0];
				AccuracyRows[3] = AccuracyRows[4];
				AverageAccuracyRow(2);
			}
		}
		// (3X4) or (4X3) Fill out 3 missing values, then average remaining 2 empty columns/rows
		else if ((SpawnAreaSize.Y == 3 || SpawnAreaSize.Z == 3) && (SpawnAreaSize.Y == 4 || SpawnAreaSize.Z == 4))
		{
			// 4X3 SpawnArea, change LeftMid, Mid, RightMid
			if (SpawnAreaSize.Y == 3)
			{
				AverageMiddle(2, 0, true);
				AverageMiddle(2, 2, true);
				AverageMiddle(2, 4, true);
				AverageAccuracyCol(1);
				AverageAccuracyCol(3);
			}
			// 3X4 SpawnArea, change TopMid, Mid, BotMid
			else
			{
				AverageMiddle(0, 2, false);
				AverageMiddle(2, 2, false);
				AverageMiddle(4, 2, false);
				AverageAccuracyRow(1);
				AverageAccuracyRow(3);
			}
		}
		// (2X_) or (_X2) Copy 2 entire row/columns, then average
		else if ((SpawnAreaSize.Y == 2 || SpawnAreaSize.Z == 2) && (SpawnAreaSize.Y >= 5 || SpawnAreaSize.Z >= 5))
		{
			// (_X2)
			if (SpawnAreaSize.Y == 2)
			{
				CopyAccuracyCol(0, 1);
				CopyAccuracyCol(4, 3);
				AverageAccuracyCol(2);
			}
			// (2X_)
			else
			{
				AccuracyRows[1] = AccuracyRows[0];
				AccuracyRows[3] = AccuracyRows[4];
				AverageAccuracyRow(2);
			}
		}
		// (3X_) or (_X3) Average the 2 empty columns/rows
		else if ((SpawnAreaSize.Y == 3 || SpawnAreaSize.Z == 3) && (SpawnAreaSize.Y >= 5 || SpawnAreaSize.Z >= 5))
		{
			// 3X_
			if (SpawnAreaSize.Y == 3)
			{
				AverageAccuracyCol(1);
				AverageAccuracyCol(3);
			}
			// _X3
			else
			{
				AverageAccuracyRow(1);
				AverageAccuracyRow(3);
			}
		}
		// (4x_) or (_X4) Average the 1 empty column/row
		else if ((SpawnAreaSize.Y == 4 || SpawnAreaSize.Z == 4) && (SpawnAreaSize.Y >= 5 || SpawnAreaSize.Z >= 5))
		{
			if (SpawnAreaSize.Y == 4)
			{
				AverageAccuracyCol(2);
			}
			else
			{
				AverageAccuracyRow(2);
			}
		}
	}

private:
	/** Returns the starting index values for each step inside the AverageUsingUnderflow main for loop,
	 *  based on the number or rows or columns of the smaller matrix to convert from */
	static TArray<int32> GetStartUnderflowArray(const int32 NumRowsOrCols)
	{
		TArray<int32> Start;
		if (NumRowsOrCols == 3)
		{
			Start = {0, 0, 1, 1, 2};
		}
		else if (NumRowsOrCols == 4)
		{
			Start = {0, 0, 1, 2, 3};
		}
		return Start;
	}

	/** Returns the end index values for each step inside the AverageUsingUnderflow main for loop,
	 *  based on the number or rows or columns of the smaller matrix to convert from */
	static TArray<int32> GetEndUnderflowArray(const int32 NumRowsOrCols)
	{
		TArray<int32> End;
		if (NumRowsOrCols == 3)
		{
			End = {0, 1, 1, 2, 2};
		}
		else if (NumRowsOrCols == 4)
		{
			End = {0, 1, 2, 3, 3};
		}
		return End;
	}

	/** Uses predetermined index arrays which specify which rows/columns to average from a smaller input array */
	void AverageUsingUnderflow(const TArray<float>& Temp, const int32 NumRows, const int32 NumCols)
	{
		TArray<int32> StartM = GetStartUnderflowArray(NumRows);
		TArray<int32> StartN = GetStartUnderflowArray(NumCols);
		TArray<int32> EndM = GetStartUnderflowArray(NumRows);
		TArray<int32> EndN = GetEndUnderflowArray(NumCols);

		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				float Sum = 0;
				int32 Count = 0;
				for (int x = StartM[i]; x < EndM[i] + 1; x++)
				{
					for (int y = StartN[j]; y < EndN[j] + 1; y++)
					{
						const float Value = Temp[NumCols * x + y];
						Sum += Value;
						Count++;
					}
				}
				if (Count != 0)
				{
					AccuracyRows[i].Accuracy[j] = Sum / Count;
				}
			}
		}
	}

	/** Gross simplification of a 2X2 to 5X5 array */
	void Average2X2Array(const float TopLeft, const float TopRight, const float BotLeft, const float BotRight)
	{
		// Repeat the TopLeft value to the 3 surrounding
		AccuracyRows[0].Accuracy[1] = TopLeft;
		AccuracyRows[1].Accuracy[1] = TopLeft;
		AccuracyRows[1].Accuracy[0] = TopLeft;

		// Repeat the BotRight value to the 3 surrounding
		AccuracyRows[3].Accuracy[4] = BotRight;
		AccuracyRows[4].Accuracy[3] = BotRight;
		AccuracyRows[3].Accuracy[3] = BotRight;

		// Repeat the BotLeft value to the 3 surrounding
		AccuracyRows[4].Accuracy[1] = BotLeft;
		AccuracyRows[3].Accuracy[0] = BotLeft;
		AccuracyRows[3].Accuracy[1] = BotLeft;

		// Repeat the TopRight value to the 3 surrounding
		AccuracyRows[0].Accuracy[3] = TopRight;
		AccuracyRows[1].Accuracy[4] = TopRight;
		AccuracyRows[1].Accuracy[3] = TopRight;

		// Middle is average the 4 corners
		AccuracyRows[2].Accuracy[2] = (TopLeft + TopRight + BotLeft + BotRight) / 4.f;

		// Average of TopLeft and BotLeft
		AccuracyRows[2].Accuracy[0] = (TopLeft + BotLeft) / 2.f;
		AccuracyRows[2].Accuracy[1] = (TopLeft + BotLeft) / 2.f;

		// Average of TopRight and TopRight
		AccuracyRows[2].Accuracy[3] = (TopRight + BotRight) / 2.f;
		AccuracyRows[2].Accuracy[4] = (TopRight + BotRight) / 2.f;

		// Average of TopLeft and TopRight
		AccuracyRows[0].Accuracy[2] = (TopLeft + TopRight) / 2.f;
		AccuracyRows[1].Accuracy[2] = (TopLeft + TopRight) / 2.f;

		// Average of BotLeft and BotRight
		AccuracyRows[3].Accuracy[2] = (BotLeft + BotRight) / 2.f;
		AccuracyRows[4].Accuracy[2] = (BotLeft + BotRight) / 2.f;
	}

	/** Creates an array of indices that correspond to what the current accuracy matrix looks like before any changes.
	 *  If a 3X3 matrix, the current accuracy matrix will only have 9 non -1 values, and this will return the indices
	 *  that correspond to those 9 non -1 values. */
	TArray<float> MakeIndexArray()
	{
		TArray<float> Temp;

		TArray<int32> YIndices = SpawnAreaSize.Y == 2
			? TArray{0, 4}
			: SpawnAreaSize.Y == 3
			? TArray{0, 2, 4}
			: SpawnAreaSize.Y == 4
			? TArray{0, 1, 3, 4}
			: TArray<int32>();
		TArray<int32> ZIndices = SpawnAreaSize.Z == 2
			? TArray{0, 4}
			: SpawnAreaSize.Z == 3
			? TArray{0, 2, 4}
			: SpawnAreaSize.Z == 4
			? TArray{0, 1, 3, 4}
			: TArray<int32>();

		TArray<FIntPoint> Combined = TArray<FIntPoint>();

		for (int i = 0; i < ZIndices.Num(); i++)
		{
			for (int j = 0; j < YIndices.Num(); j++)
			{
				Combined.Add(FIntPoint(ZIndices[i], YIndices[j]));
			}
		}

		for (int i = 0; i < Combined.Num(); i++)
		{
			UE_LOG(LogTemp, Display, TEXT("ZIndex: %d YIndex: %d Value: %f"), Combined[i].X, Combined[i].Y,
				AccuracyRows[Combined[i].X].Accuracy[Combined[i].Y]);
			const float Value = AccuracyRows[Combined[i].X].Accuracy[Combined[i].Y];
			Temp.Add(Value);
		}
		return Temp;
	}

	/** Averages the values of a particular row, using average of the row above & below */
	void AverageAccuracyRow(const int32 RowIndex)
	{
		for (int i = 0; i < 5; i++)
		{
			AccuracyRows[RowIndex].Accuracy[i] = (AccuracyRows[RowIndex - 1].Accuracy[i] + AccuracyRows[RowIndex + 1].
				Accuracy[i]) / 2.f;
		}
	}

	/** Averages the values of a particular column, using average of the column left & right */
	void AverageAccuracyCol(const int32 ColIndex)
	{
		for (int i = 0; i < 5; i++)
		{
			AccuracyRows[i].Accuracy[ColIndex] = (AccuracyRows[i].Accuracy[ColIndex - 1] + AccuracyRows[i].Accuracy[
				ColIndex + 1]) / 2.f;
		}
	}

	/** Copies the values of a vertical up-down column to another column */
	void CopyAccuracyCol(const int32 SourceIndex, const int32 DestIndex)
	{
		for (int i = 0; i < 5; i++)
		{
			AccuracyRows[i].Accuracy[DestIndex] = AccuracyRows[i].Accuracy[SourceIndex];
		}
	}

	/** Sets the value of a (RowIndex, ColIndex). If bChangeRowValue is true, +-1 the RowIndex is used for the average,
	 *  using the same ColIndex. If false, +-1 the ColIndex is used for the average, using the same RowIndex */
	void AverageMiddle(const int32 RowIndex, const int32 ColIndex, const bool bChangeRowValue)
	{
		if (bChangeRowValue)
		{
			AccuracyRows[RowIndex].Accuracy[ColIndex] = (AccuracyRows[RowIndex + 1].Accuracy[ColIndex] + AccuracyRows[
				RowIndex - 1].Accuracy[ColIndex]) / 2.f;
		}
		else
		{
			AccuracyRows[RowIndex].Accuracy[ColIndex] = (AccuracyRows[RowIndex].Accuracy[ColIndex + 1] + AccuracyRows[
				RowIndex].Accuracy[ColIndex - 1]) / 2.f;
		}
	}
};

/** Struct containing any information to save between game mode sessions that does not define the game mode itself, e.g. accuracy */
USTRUCT(BlueprintType)
struct FCommonScoreInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FAccuracyData AccuracyData;

	UPROPERTY()
	TArray<float> QTable;

	UPROPERTY()
	int32 NumQTableRows;

	UPROPERTY()
	int32 NumQTableColumns;

	UPROPERTY()
	TArray<int32> TrainingSamples;

	UPROPERTY()
	int64 TotalTrainingSamples;

	/** Generic constructor */
	FCommonScoreInfo()
	{
		AccuracyData = FAccuracyData(Constants::DefaultNumberOfAccuracyDataRows,
			Constants::DefaultNumberOfAccuracyDataColumns);
		QTable = TArray<float>();
		QTable.Init(0.f, Constants::DefaultQTableSize);
		TrainingSamples = TArray<int32>();
		TrainingSamples.Init(0.f, Constants::DefaultQTableSize);
		NumQTableRows = Constants::DefaultNumberOfQTableRows;
		NumQTableColumns = Constants::DefaultNumberOfQTableColumns;
		TotalTrainingSamples = 0;
	}

	/** Calls UpdateAccuracyRows on AccuracyData which recalculates the accuracy for each entry */
	void UpdateAccuracy(const FAccuracyData& InAccuracyData)
	{
		AccuracyData.UpdateAccuracyRows(InAccuracyData);
	}

	/** Sets the value of the QTable with InQTable */
	void UpdateQTable(const TArray<float>& InQTable, const int32 InNumQTableRows, const int32 InNumQTableColumns,
		const TArray<int32>& InUpdatedTrainingSamples, const int32 InUpdatedTotalTrainingSamples)
	{
		QTable = InQTable;
		NumQTableRows = InNumQTableRows;
		NumQTableColumns = InNumQTableColumns;
		TrainingSamples = InUpdatedTrainingSamples;
		TotalTrainingSamples = InUpdatedTotalTrainingSamples;
	}

	/** Resets the QTable, TrainingSamples, NumQTableRows & NumQTableColumns, and TotalTrainingSamples to default values */
	void ResetQTable()
	{
		QTable = TArray<float>();
		QTable.Init(0.f, Constants::DefaultQTableSize);
		TrainingSamples = TArray<int32>();
		TrainingSamples.Init(0.f, Constants::DefaultQTableSize);
		NumQTableRows = Constants::DefaultNumberOfQTableRows;
		NumQTableColumns = Constants::DefaultNumberOfQTableColumns;
		TotalTrainingSamples = 0;
	}

	/** Returns the average number of training samples in the TotalTrainingSamples array */
	double GetAverageTrainingSample()
	{
		double Total = 0.f;
		if (TrainingSamples.Num() < 1)
		{
			return Total;
		}
		for (const int32 Sample : TrainingSamples)
		{
			Total += Sample;
		}
		return Total / static_cast<double>(TrainingSamples.Num());
	}
};

/** Used to load and save player scores */
USTRUCT(BlueprintType)
struct FPlayerScore
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defining Properties")
	FBS_DefiningConfig DefiningConfig;

	/** The song title */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defining Properties")
	FString SongTitle;

	/** Length of song */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Properties")
	float SongLength;

	/** The current score at any given time during play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	float Score;

	/** Only represents highest score based on previous entries, and may become outdated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	float HighScore;

	/** Total Targets hit divided by Total shots fired */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	float Accuracy;

	/** Total Targets hit divided by Total targets spawned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	float Completion;

	/** Incremented after receiving calls from FOnShotsFired delegate in DefaultCharacter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	int32 ShotsFired;

	/** Total number of targets destroyed by player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	int32 TargetsHit;

	/** Total number of targets spawned, incremented after receiving calls from FOnTargetSpawnSignature in TargetManager */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	int32 TargetsSpawned;

	/** Total possible damage that could have been done to tracking target, also used to determine if the score object is for Tracking game mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	float TotalPossibleDamage;

	/** Total time offset from Spawn Beat Delay for all destroyed targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	float TotalTimeOffset;

	/** Avg Time offset from Spawn Beat Delay for destroyed targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	float AvgTimeOffset;

	/** time that player completed the session, in Iso8601 UTC format */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	FString Time;

	/** The maximum consecutive targets hit in a row */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	int32 Streak;

	/** The accuracy at each point in the grid */
	UPROPERTY()
	TArray<FAccuracyRow> LocationAccuracy;

	/** Whether or not this instance has been saved to the database yet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Score")
	bool bSavedToDatabase;

	FPlayerScore()
	{
		DefiningConfig = FBS_DefiningConfig();
		SongTitle = "";
		SongLength = 0.f;
		Score = 0;
		HighScore = 0;
		Completion = 0;
		Accuracy = 0;
		ShotsFired = 0;
		TargetsHit = 0;
		TotalTimeOffset = 0;
		AvgTimeOffset = 0;
		TargetsSpawned = 0;
		TotalPossibleDamage = 0.f;
		Streak = 0;
		LocationAccuracy = TArray<FAccuracyRow>();
		bSavedToDatabase = false;
	}

	/** Verifies that the score is greater than zero and that if the mode is custom, that the Custom Game Mode Name
	 *  is not blank. */
	bool IsValidToSave() const
	{
		if (Score <= 0.f)
		{
			return false;
		}
		if (DefiningConfig.GameModeType == EGameModeType::Custom && DefiningConfig.CustomGameModeName.IsEmpty())
		{
			return false;
		}
		return true;
	}

	FORCEINLINE bool operator==(const FPlayerScore& Other) const
	{
		if (DefiningConfig == Other.DefiningConfig && SongTitle.Equals(Other.SongTitle))
		{
			return true;
		}
		return false;
	}
};

/** A struct where each element is an index of a smaller matrix that represents multiple indices of a larger matrix */
USTRUCT()
struct FGenericIndexMapping
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Index;

	UPROPERTY()
	TArray<int32> MappedIndices;

	FGenericIndexMapping()
	{
		Index = INDEX_NONE;
		MappedIndices = TArray<int32>();
	}

	FGenericIndexMapping(const int32 InIndex)
	{
		Index = InIndex;
		MappedIndices = TArray<int32>();
	}

	FORCEINLINE bool operator ==(const FGenericIndexMapping& Other) const
	{
		if (Other.Index == Index)
		{
			return true;
		}
		return false;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FGenericIndexMapping& Struct)
	{
		return GetTypeHash(Struct.Index);
	}
};


UCLASS()
class BEATSHOTGLOBAL_API USaveGamePlayerScore : public USaveGame
{
	GENERATED_BODY()

public:
	USaveGamePlayerScore();

	/** Returns a copy of PlayerScoreArray */
	TArray<FPlayerScore> GetPlayerScores() const;

	/** Returns a copy of player scores not saved to database */
	TArray<FPlayerScore> GetPlayerScores_UnsavedToDatabase() const;

	/** Adds a new entry to PlayerScoreArray */
	void AddPlayerScoreInstance(const FPlayerScore& InPlayerScore);

	/** Modifies score instances in PlayerScoreArray */
	void SetAllScoresSavedToDatabase();

	/** Returns a copy of CommonScoreInfo */
	TMap<FBS_DefiningConfig, FCommonScoreInfo> GetCommonScoreInfo() const;

	/** Finds or Adds an entry to CommonScoreInfo map for the given Defining Config. */
	void FindOrAddCommonScoreInfo(const FBS_DefiningConfig& InDefiningConfig, FCommonScoreInfo& OutCommonScoreInfo);

	/** Replaces the CommonScoreInfo entry with InCommonScoreInfo if one is found using InDefiningConfig. Otherwise, it
	 *  adds a new entry. */
	void SaveCommonScoreInfo(const FBS_DefiningConfig& InDefiningConfig, const FCommonScoreInfo& InCommonScoreInfo);

	/** Clears the QTable for an FCommonScoreInfo instance that matches the Defining Config. Returns 1 if successfully found and cleared */
	int32 ResetQTable(const FBS_DefiningConfig& InDefiningConfig);

	/** Removes an FCommonScoreInfo instance that matches the Defining Config. Returns the number of removed instances (should only be 1) */
	int32 RemoveCommonScoreInfo(const FBS_DefiningConfig& InDefiningConfig);

	/** Removes all FCommonScoreInfo instances for custom game modes. Returns the number of removed instances */
	int32 RemoveAllCustomGameModeCommonScoreInfo();

	/** Prints full QTable array to log/console */
	static void PrintQTable(const FBS_DefiningConfig& InDefiningConfig, const FCommonScoreInfo& InCommonScoreInfo,
		const FNumberFormattingOptions& Options);

	/** Prints 5x5 accuracy to log/console */
	static void PrintAccuracy(const FBS_DefiningConfig& InDefiningConfig, const FCommonScoreInfo& InCommonScoreInfo,
		const FNumberFormattingOptions& Options);

	/** Prints full TrainingSamples array to log/console */
	static void PrintTrainingSamples(const FBS_DefiningConfig& InDefiningConfig,
		const FCommonScoreInfo& InCommonScoreInfo, const FNumberFormattingOptions& Options);

private:
	/** Returns whether or not there exists a score with the same time */
	bool ContainsExistingTime(const FPlayerScore& InPlayerScore);

	/** Array containing all saved score instances */
	UPROPERTY()
	TArray<FPlayerScore> PlayerScoreArray;

	/** Map containing common score info for each unique defining config */
	UPROPERTY()
	TMap<FBS_DefiningConfig, FCommonScoreInfo> CommonScoreInfo;

	FNumberFormattingOptions PercentFormat;
	FNumberFormattingOptions QTableFormat;
	FNumberFormattingOptions TrainingSamplesFormat;
};
