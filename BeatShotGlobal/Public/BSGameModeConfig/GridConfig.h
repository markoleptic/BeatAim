// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSConstants.h"
#include "GridConfig.generated.h"

USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FBS_GridConfig
{
	GENERATED_BODY()

	/** The number of horizontal grid targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 NumHorizontalGridTargets;

	/** The number of vertical grid targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 NumVerticalGridTargets;

	/** The space between grid targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector2D GridSpacing;

	/** Number of grid target visible at any one time. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 NumGridTargetsVisibleAtOnce;

	FBS_GridConfig()
	{
		NumHorizontalGridTargets = Constants::NumHorizontalBeatGridTargets_Normal;
		NumVerticalGridTargets = Constants::NumVerticalBeatGridTargets_Normal;
		NumGridTargetsVisibleAtOnce = Constants::NumTargetsAtOnceBeatGrid_Normal;
		GridSpacing = Constants::BeatGridSpacing_Normal;
	}

	FORCEINLINE bool operator==(const FBS_GridConfig& Other) const
	{
		if (NumHorizontalGridTargets != Other.NumHorizontalGridTargets)
		{
			return false;
		}
		if (NumVerticalGridTargets != Other.NumVerticalGridTargets)
		{
			return false;
		}
		if (GridSpacing != Other.GridSpacing)
		{
			return false;
		}
		if (NumHorizontalGridTargets != Other.NumHorizontalGridTargets)
		{
			return false;
		}
		if (NumGridTargetsVisibleAtOnce != Other.NumGridTargetsVisibleAtOnce)
		{
			return false;
		}
		return true;
	}
};
