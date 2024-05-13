// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Target/ReinforcementLearningComponent.h"
#if !UE_BUILD_SHIPPING
#include "Target/TargetManager.h"
#endif


UReinforcementLearningComponent::UReinforcementLearningComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CompositeCurveTable_HyperParameters = nullptr;
	ReinforcementLearningMode = EReinforcementLearningMode::None;
	HyperParameterMode = EReinforcementLearningHyperParameterMode::None;
	QTable = nc::NdArray<float>();
	TrainingSamples = nc::NdArray<int32>();
	SpawnAreaToQTableIndexMap = TArray<FSpawnAreaQTableIndexPair>();
	QTableToSpawnAreaIndexMap = TMap<int32, FGenericIndexMapping>();
	ActiveTargetPairs = TArray<FTargetPair>();
	Alpha = 0;
	Gamma = 0;
	Epsilon = 0;
	TotalTrainingSamples = 0;
	M = Constants::DefaultNumberOfQTableRows;
	N = Constants::DefaultNumberOfQTableColumns;;

#if !UE_BUILD_SHIPPING
	bPrintDebug_ActiveTargetPairs = false;
	bPrintDebug_QTableInit = false;
	bPrintDebug_QTableUpdate = false;
	bPrintDebug_GetMaxIndex = false;
	bBroadcastAverageOnQTableUpdate = false;
	bPrintDebug_ChooseActionIndex = false;

	IntegerFormatting.MaximumFractionalDigits = 0;
	IntegerFormatting.MaximumIntegralDigits = 2;
	IntegerFormatting.MinimumIntegralDigits = 2;

	FloatFormatting.MinimumFractionalDigits = 2;
	FloatFormatting.MaximumFractionalDigits = 2;
	FloatFormatting.MaximumIntegralDigits = 1;
	FloatFormatting.MinimumIntegralDigits = 1;
#endif
}

void UReinforcementLearningComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UReinforcementLearningComponent::DestroyComponent(bool bPromoteChildren)
{
	Super::DestroyComponent(bPromoteChildren);
}

void UReinforcementLearningComponent::Init(const FRLAgentParams& AgentParams)
{
	Alpha = AgentParams.AIConfig.Alpha;
	Gamma = AgentParams.AIConfig.Gamma;
	Epsilon = AgentParams.AIConfig.Epsilon;
	ReinforcementLearningMode = AgentParams.AIConfig.ReinforcementLearningMode;
	HyperParameterMode = AgentParams.AIConfig.HyperParameterMode;
	TotalTrainingSamples = AgentParams.ScoreInfo.TotalTrainingSamples;

	QTableToSpawnAreaIndexMap = MapMatrixTo5X5(AgentParams.SpawnAreaSize.Z, AgentParams.SpawnAreaSize.Y);

	// Each row in QTable has size equal to ScaledSize, and so does each column
	QTable = nc::zeros<float>(nc::Shape(M, N));
	TrainingSamples = nc::zeros<int32>(nc::Shape(M, N));

	// Init QTableIndices array
	for (const TPair<int32, FGenericIndexMapping>& Mapping : QTableToSpawnAreaIndexMap)
	{
		for (const int32 SpawnAreaIndex : Mapping.Value.MappedIndices)
		{
			SpawnAreaToQTableIndexMap.Add(FSpawnAreaQTableIndexPair(SpawnAreaIndex, Mapping.Key));
		}
#if !UE_BUILD_SHIPPING
		if (bPrintDebug_QTableInit)
		{
			UE_LOG(LogTargetManager, Display, TEXT("Index %d has %d SpawnAreas associated with it."), Mapping.Key,
				Mapping.Value.MappedIndices.Num());
		}
#endif
	}

	// Use existing QTable if possible
	if (AgentParams.ScoreInfo.NumQTableRows == QTable.numRows() && AgentParams.ScoreInfo.NumQTableColumns == QTable.
		numCols())
	{
		if (QTable.size() == AgentParams.ScoreInfo.QTable.Num())
		{
			QTable = GetNdArrayFromTArray<float>(AgentParams.ScoreInfo.QTable, AgentParams.ScoreInfo.NumQTableRows,
				AgentParams.ScoreInfo.NumQTableColumns);
		}
		if (TrainingSamples.size() == AgentParams.ScoreInfo.TrainingSamples.Num())
		{
			TrainingSamples = GetNdArrayFromTArray<int32>(AgentParams.ScoreInfo.TrainingSamples,
				AgentParams.ScoreInfo.NumQTableRows, AgentParams.ScoreInfo.NumQTableColumns);
		}
	}

	// Check NaNs
	QTable = nc::nan_to_num<float>(QTable, 0.f);

#if !UE_BUILD_SHIPPING
	if (bPrintDebug_QTableInit)
	{
		UE_LOG(LogTargetManager, Display, TEXT("Unique Indices across entire mapping: %d Input Size: %d"),
			QTableToSpawnAreaIndexMap.Num(), AgentParams.SpawnAreaSize.Z * AgentParams.SpawnAreaSize.Y);
		UE_LOG(LogTargetManager, Display, TEXT("In QTable Size: %d  Actual QTable Size: %d"),
			AgentParams.ScoreInfo.QTable.Num(), QTable.size());
		UE_LOG(LogTargetManager, Display, TEXT("SpawnAreasRows: %d SpawnAreasColumns: %d"), AgentParams.SpawnAreaSize.Z,
			AgentParams.SpawnAreaSize.Y);
		UE_LOG(LogTargetManager, Display, TEXT("QTableRows: %d QTableColumns: %d"), M, N);
		UE_LOG(LogTargetManager, Display, TEXT("QTable Training Samples: %lld"), TotalTrainingSamples);
	}
#endif
}

void UReinforcementLearningComponent::Clear()
{
	ReinforcementLearningMode = EReinforcementLearningMode::None;
	HyperParameterMode = EReinforcementLearningHyperParameterMode::None;
	QTable = nc::NdArray<float>();
	TrainingSamples = nc::NdArray<int32>();
	SpawnAreaToQTableIndexMap.Empty();
	QTableToSpawnAreaIndexMap.Empty();
	TargetPairs.Empty();
	ActiveTargetPairs.Empty();
	Alpha = 0;
	Gamma = 0;
	Epsilon = 0;
	TotalTrainingSamples = 0;
	M = Constants::DefaultNumberOfQTableRows;
	N = Constants::DefaultNumberOfQTableColumns;;
}

// Main QTable functions

void UReinforcementLearningComponent::AddToActiveTargetPairs(const int32 SpawnAreaIndex_First,
	const int32 SpawnAreaIndex_Second)
{
	ActiveTargetPairs.Emplace(SpawnAreaIndex_First, SpawnAreaIndex_Second);

#if !UE_BUILD_SHIPPING
	if (bPrintDebug_ActiveTargetPairs)
	{
		UE_LOG(LogTargetManager, Display, TEXT("Added [%d, %d] to Active Target Pairs"), SpawnAreaIndex_First,
			SpawnAreaIndex_Second);
	}
#endif
}

void UReinforcementLearningComponent::SetActiveTargetPairReward(const int32 SpawnAreaIndex, const bool bHit)
{
	if (ActiveTargetPairs.IsEmpty())
	{
		return;
	}

	if (FTargetPair* FoundPair = FindTargetPairByCurrentIndex(SpawnAreaIndex))
	{
		// Update reward
		FTargetPair Copy = FTargetPair(*FoundPair);
		Copy.SetReward(bHit ? -1.f : 1.f);
		ActiveTargetPairs.RemoveSingle(*FoundPair);
		TargetPairs.Enqueue(MoveTemp(Copy));
		ClearCachedTargetPairs();
	}
#if !UE_BUILD_SHIPPING
	else
	{
		UE_LOG(LogTargetManager, Warning, TEXT("SpawnAreaIndex not found in ActiveTargetPairs %d"), SpawnAreaIndex);
	}
#endif
}

void UReinforcementLearningComponent::ClearCachedTargetPairs()
{
	while (!TargetPairs.IsEmpty())
	{
		FTargetPair TargetPair;
		if (TargetPairs.Dequeue(TargetPair))
		{
			FQTableUpdateParams UpdateParams = FQTableUpdateParams(TargetPair);
			UpdateQTable(UpdateParams);
		}
	}
}

int32 UReinforcementLearningComponent::ChooseNextActionIndex(const int32 PreviousSpawnAreaIndex,
	const TArray<int32>& SpawnAreaIndices) const
{
	// Only Exploration and ActiveAgent Reinforcement Learning Modes should choose spawn locations
	if (SpawnAreaIndices.IsEmpty() || ReinforcementLearningMode == EReinforcementLearningMode::None ||
		ReinforcementLearningMode == EReinforcementLearningMode::Training)
	{
		return INDEX_NONE;
	}

	if (FMath::FRandRange(0, 1.f) > Epsilon)
	{
		const int32 BestActionIndex = ChooseBestActionIndex(PreviousSpawnAreaIndex, SpawnAreaIndices);
		if (BestActionIndex == INDEX_NONE)
		{
#if !UE_BUILD_SHIPPING
			if (bPrintDebug_ChooseActionIndex)
			{
				UE_LOG(LogTargetManager, Display,
					TEXT("No acceptable index range found, falling back to choosing random action"));
			}
#endif
			return ChooseRandomActionIndex(SpawnAreaIndices);
		}
		return BestActionIndex;
	}
	return ChooseRandomActionIndex(SpawnAreaIndices);
}

int32 UReinforcementLearningComponent::ChooseRandomActionIndex(const TArray<int32>& SpawnAreaIndices)
{
	return SpawnAreaIndices[FMath::RandRange(0, SpawnAreaIndices.Num() - 1)];
}

int32 UReinforcementLearningComponent::ChooseBestActionIndex(const int32 PreviousSpawnAreaIndex,
	const TArray<int32>& SpawnAreaIndices) const
{
	int32 ReturnIndex = INDEX_NONE;
	int32 NumFiltersRequired = 0;
	int32 NumCurrentIndexChoices = 0;

	const TArray<int32> MaxIndices = (PreviousSpawnAreaIndex == INDEX_NONE)
		? GetIndices_MaximizeFirst()
		: GetIndices_MaximizeSecond(GetIndex_FromSpawnArea_ToQTable(PreviousSpawnAreaIndex));

	for (const int32 Index : MaxIndices)
	{
		/* Get the SpawnArea indices that the chosen index represents */
		TArray<int32> UnfilteredSpawnAreaIndices = GetSpawnAreaIndexRange(Index);

		/* Remove any indices that aren't inside the current SpawnAreaIndices */
		const TArray<int32> FilteredSpawnAreaIndices = UnfilteredSpawnAreaIndices.FilterByPredicate(
			[&SpawnAreaIndices, &NumFiltersRequired](const int32& Value)
			{
				NumFiltersRequired++;
				if (SpawnAreaIndices.Contains(Value))
				{
					return true;
				}
				return false;
			});

		NumCurrentIndexChoices = FilteredSpawnAreaIndices.Num();

		/* Return a random point inside the filtered spawn indices if not empty */
		if (!FilteredSpawnAreaIndices.IsEmpty())
		{
			const int32 RandomIndex = FMath::RandRange(0, FilteredSpawnAreaIndices.Num() - 1);
			ReturnIndex = FilteredSpawnAreaIndices[RandomIndex];
			break;
		}
	}

#if !UE_BUILD_SHIPPING
	if (bPrintDebug_ChooseActionIndex)
	{
		UE_LOG(LogTargetManager, Display, TEXT("ChooseBestActionIdx: Num Filters Required: %d"), NumFiltersRequired);
		UE_LOG(LogTargetManager, Display, TEXT("ChooseBestActionIdx: Num Choices: %d"), NumCurrentIndexChoices);
		UE_LOG(LogTargetManager, Display, TEXT("ChooseBestActionIdx: ChosenIdx: %d"), ReturnIndex);
	}
#endif

	return ReturnIndex;
}

void UReinforcementLearningComponent::UpdateQTable(FQTableUpdateParams& UpdateParams)
{
	// Convert SpawnArea indices to QTable indices
	UpdateParams.StateIndex = GetIndex_FromSpawnArea_ToQTable(UpdateParams.TargetPair.First);
	UpdateParams.ActionIndex = GetIndex_FromSpawnArea_ToQTable(UpdateParams.TargetPair.Second);
	UpdateParams.StateIndex_2 = UpdateParams.ActionIndex;

	// Choose a random max value
	const TArray<int32> MaxIndex_2_Candidates = GetIndices_MaximizeSecond(UpdateParams.StateIndex_2);
	UpdateParams.ActionIndex_2 = FMath::RandRange(0, MaxIndex_2_Candidates.Num() - 1);

	// Q value for starting at State 1 and taking Action 1 (State 1, Action 1)
	const float Predict = QTable(UpdateParams.StateIndex, UpdateParams.ActionIndex);

	// Q value for starting at State 2 and taking the Max Action (State 2, Action 2)
	const float Target = UpdateParams.TargetPair.GetReward() + Gamma * QTable(UpdateParams.StateIndex_2,
		UpdateParams.ActionIndex_2);

	// Q Table update function
	const float NewValue = QTable(UpdateParams.StateIndex, UpdateParams.ActionIndex) + Alpha * (Target - Predict);
	const float OldValue = QTable(UpdateParams.StateIndex, UpdateParams.ActionIndex);

	// Assign new value to Q Table entry at (State 1, Action 1)
	QTable(UpdateParams.StateIndex, UpdateParams.ActionIndex) = NewValue;

	// Increment training samples and TotalTrainingSamples
	TrainingSamples(UpdateParams.StateIndex, UpdateParams.ActionIndex) += 1;
	TotalTrainingSamples += 1;

#if !UE_BUILD_SHIPPING
	if (bPrintDebug_QTableUpdate)
	{
		UE_LOG(LogTargetManager, Display,
			TEXT("QTable Value for TargetPair [%d, %d] & QTableIndex [%d, %d]: Old: %f New: %f"),
			UpdateParams.TargetPair.First, UpdateParams.TargetPair.Second, UpdateParams.StateIndex,
			UpdateParams.ActionIndex, OldValue, NewValue);
	}
	UpdateQTableWidget();
#endif
}

// Getters and utility functions

TArray<int32> UReinforcementLearningComponent::GetIndices_MaximizeFirst() const
{
	TArray<int32> MaxIndices;

	// Compute the sum of values for each row
	const nc::NdArray<float> RowSum = nc::sum(GetQTable(), nc::Axis::COL);

	// Get the indices that correspond to sums
	const nc::NdArray<unsigned> SortedIndices = nc::argsort(RowSum);

	// Reverse to get descending order
	const nc::NdArray<unsigned> ReverseSortedIndices = nc::flip(SortedIndices);

	// Compute MaxValue
	const float MaxValue = RowSum[ReverseSortedIndices[0]];

	// Find any values equal to the MaxValue, and add them to MaxIndices
	for (size_t i = 0; i < ReverseSortedIndices.size(); ++i)
	{
		if (RowSum[ReverseSortedIndices[i]] == MaxValue)
		{
			MaxIndices.Add(static_cast<int32>(ReverseSortedIndices[i]));
		}
		else
		{
			break; // Stop when values are no longer the maximum
		}
	}

	return MaxIndices;
}

TArray<int32> UReinforcementLearningComponent::GetIndices_MaximizeSecond(const int32 InPreviousIndex) const
{
	TArray<int32> MaxIndices;

	// Extract the row (all the columns) corresponding to the previous index
	const nc::NdArray<float> PreviousRow = GetQTable()(InPreviousIndex, GetQTable().cSlice(0));

	// Get the indices that correspond to sums
	const nc::NdArray<unsigned> ArgSortedIndices = nc::argsort(PreviousRow);

	// Reverse to get descending order
	nc::NdArray<unsigned> ReverseSortedIndices = nc::flip(ArgSortedIndices);

	// Maximum value within the row
	const float MaxValue = PreviousRow[ReverseSortedIndices[0]];

	// Find any values equal to the MaxValue, and add them to MaxIndices
	for (size_t i = 0; i < ReverseSortedIndices.size(); ++i)
	{
		if (PreviousRow[ReverseSortedIndices[i]] == MaxValue)
		{
			MaxIndices.Add(static_cast<int32>(ReverseSortedIndices[i]));
		}
		else
		{
			break; // Stop when values are no longer the maximum
		}
	}

#if !UE_BUILD_SHIPPING
	if (bPrintDebug_GetMaxIndex)
	{
		PrintGetMaxIndex(InPreviousIndex, MaxValue, PreviousRow, ReverseSortedIndices);
	}
#endif

	return MaxIndices;
}

TArray<float> UReinforcementLearningComponent::GetTArray_FromNdArray_QTableAvg() const
{
	nc::NdArray<float> QTableMean = nc::mean(GetQTable(), nc::Axis::ROW).astype<float>();
	const nc::NdArray<float> Reshaped = QTableMean.reshape(5, 5);
	const nc::NdArray<float> Flipped = nc::flipud<float>(Reshaped);
	return GetTArrayFromNdArray<float>(Flipped);
}

TArray<float> UReinforcementLearningComponent::GetTArray_FromNdArray_QTableMax() const
{
	nc::NdArray<float> QTableMin = nc::max(GetQTable(), nc::Axis::ROW).astype<float>();
	const nc::NdArray<float> Reshaped = QTableMin.reshape(5, 5);
	const nc::NdArray<float> Flipped = nc::flipud<float>(Reshaped);
	return GetTArrayFromNdArray<float>(Flipped);
}

int32 UReinforcementLearningComponent::GetIndex_FromSpawnArea_ToQTable(const int32 SpawnAreaIndex) const
{
	const FSpawnAreaQTableIndexPair* Found = SpawnAreaToQTableIndexMap.FindByPredicate(
		[&SpawnAreaIndex](const FSpawnAreaQTableIndexPair& IndexPair)
		{
			return IndexPair.SpawnAreaIndex == SpawnAreaIndex;
		});

	return Found ? Found->QTableIndex : INDEX_NONE;
}

TArray<int32> UReinforcementLearningComponent::GetSpawnAreaIndexRange(const int32 QTableIndex) const
{
	TArray<int32> Out;
	const FGenericIndexMapping* Found = QTableToSpawnAreaIndexMap.Find(QTableIndex);

	return Found ? Found->MappedIndices : TArray<int32>();
}

FTargetPair* UReinforcementLearningComponent::FindTargetPairByCurrentIndex(const int32 InCurrentIndex)
{
	return ActiveTargetPairs.FindByPredicate([&InCurrentIndex](const FTargetPair& TargetPair)
	{
		return TargetPair.Second == InCurrentIndex;
	});
}

// Debug functions
#if !UE_BUILD_SHIPPING

void UReinforcementLearningComponent::UpdateQTableWidget() const
{
	if (OnQTableUpdate.IsBound())
	{
		if (bBroadcastAverageOnQTableUpdate)
		{
			OnQTableUpdate.Broadcast(GetTArray_FromNdArray_QTableAvg());
		}
		else
		{
			OnQTableUpdate.Broadcast(GetTArray_FromNdArray_QTableMax());
		}
	}
}

void UReinforcementLearningComponent::PrintRewards() const
{
	FString Row;
	nc::NdArray<float> QTableCopy = GetQTable();
	for (int j = 0; j < static_cast<int>(QTableCopy.numCols()); j++)
	{
		Row.Empty();
		for (int i = 0; i < static_cast<int>(QTableCopy.numRows()); i++)
		{
			const float Value = round(QTableCopy(i, j) * 100.0) / 100.0;
			if (Value >= 0.f)
			{
				Row.Append("+" + FString::SanitizeFloat(Value, 2) + " ");
			}
			else
			{
				Row.Append(FString::SanitizeFloat(Value, 2) + " ");
			}
		}
		UE_LOG(LogTargetManager, Display, TEXT("%s"), *Row);
	}

	int i = 0;
	nc::NdArray<double> FlippedMean = flipud(mean(GetQTable(), nc::Axis::ROW).reshape(5, 5));
	Row.Empty();
	for (const double It : FlippedMean)
	{
		const float Value = roundf(static_cast<float>(It) * 100.0) / 100.0;;
		if (It >= 0.f)
		{
			Row.Append("+" + FString::SanitizeFloat(Value, 2) + " ");
		}
		else
		{
			Row.Append(FString::SanitizeFloat(Value, 2) + " ");
		}
		i++;
		if (i % 5 == 0)
		{
			UE_LOG(LogTargetManager, Display, TEXT("%s"), *Row);
			Row.Empty();
		}
	}
}

void UReinforcementLearningComponent::PrintMaxAverageIndices() const
{
	FString Row = " ";
	FString Row2;
	FString Row3 = " ";
	FString Row4;

	/* Averages instead of maxes */
	auto AvgValues = mean(GetQTable(), nc::Axis::ROW).sort();
	auto AvgIndices = AvgValues.argsort(nc::Axis::COL);

	auto MaxValues = nc::max(GetQTable(), nc::Axis::ROW).sort();
	auto MaxIndices = MaxValues.argsort(nc::Axis::COL);

	for (int j = 0; j < static_cast<int>(MaxIndices.numCols()); j++)
	{
		const float MaxIndex = MaxIndices(0, j);
		const float AverageIndex = AvgIndices(0, j);
		const float MaxValue = MaxValues(0, j);
		const float AverageValue = AvgValues(0, j);

		Row += (FText::AsNumber(MaxIndex, &IntegerFormatting).ToString() + "     ");

		if (MaxValue >= 0.f)
		{
			Row2 += ("  " + FText::AsNumber(MaxValue, &FloatFormatting).ToString() + " ");
		}
		else
		{
			Row2 += (" " + FText::AsNumber(MaxValue, &FloatFormatting).ToString() + " ");
		}

		Row3 += (FText::AsNumber(AverageIndex, &IntegerFormatting).ToString() + "     ");

		if (AverageValue >= 0.f)
		{
			Row4 += ("  " + FText::AsNumber(AverageValue, &FloatFormatting).ToString() + " ");
		}
		else
		{
			Row4 += (" " + FText::AsNumber(AverageValue, &FloatFormatting).ToString() + " ");
		}
	}
	UE_LOG(LogTargetManager, Display, TEXT("Max Indices & Max Values:"));
	UE_LOG(LogTargetManager, Display, TEXT("%s"), *Row);
	UE_LOG(LogTargetManager, Display, TEXT("%s"), *Row2);
	UE_LOG(LogTargetManager, Display, TEXT("Average Indices & Average Values"));
	UE_LOG(LogTargetManager, Display, TEXT("%s"), *Row3);
	UE_LOG(LogTargetManager, Display, TEXT("%s"), *Row4);
}

void UReinforcementLearningComponent::PrintGetMaxIndex(const int32 PreviousIndex, const float MaxValue,
	const nc::NdArray<float>& PreviousRow, const nc::NdArray<unsigned>& ReverseSortedIndices) const
{
	FString String;
	for (const float Value : PreviousRow)
	{
		String += ("  " + FText::AsNumber(Value, &FloatFormatting).ToString() + " ");
	}
	UE_LOG(LogTargetManager, Display, TEXT("RowIdx %d: %s"), PreviousIndex, *String);
	String.Empty();

	for (const unsigned Value : ReverseSortedIndices)
	{
		String += ("  " + FText::AsNumber(Value, &IntegerFormatting).ToString() + " ");
	}
	UE_LOG(LogTargetManager, Display, TEXT("RowIdx %d ReversedArgSort: %s"), PreviousIndex, *String);
	String.Empty();

	// Find any values equal to the MaxValue, and add them to MaxIndices
	for (size_t i = 0; i < ReverseSortedIndices.size(); ++i)
	{
		if (PreviousRow[ReverseSortedIndices[i]] == MaxValue)
		{
			String += ("  " + FText::AsNumber(ReverseSortedIndices[i], &IntegerFormatting).ToString() + " ");
		}
		else
		{
			break; // Stop when values are no longer the maximum
		}
	}
	UE_LOG(LogTargetManager, Display, TEXT("Max Value for Row Index %d: %s, Max Indices: %s"), PreviousIndex,
		*FString("  " + FText::AsNumber(MaxValue, &FloatFormatting).ToString() + " "), *String);
}

#endif
