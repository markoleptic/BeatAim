// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DynamicConfig.generated.h"


/** Defines how to dynamically change the scale of a game mode feature. Requires using an external counter that
 *  increments each time a target is consecutively destroyed and decrements by DecrementAmount each time a target was
 *  not destroyed. */
USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FBS_Dynamic
{
	GENERATED_BODY()

	/** The number of consecutively destroyed targets required to begin changing the scale. */
	UPROPERTY(EditDefaultsOnly)
	int32 StartThreshold;

	/** The number of consecutively destroyed targets required to reach the final scale. */
	UPROPERTY(EditDefaultsOnly)
	int32 EndThreshold;

	/** Whether to use cubic interpolation or linear interpolation from StartThreshold to EndThreshold. */
	UPROPERTY(EditDefaultsOnly)
	bool bIsCubicInterpolation;

	/** The amount to decrement from consecutively destroyed targets after a miss. */
	UPROPERTY(EditDefaultsOnly)
	int32 DecrementAmount;

	FBS_Dynamic()
	{
		StartThreshold = 5;
		EndThreshold = 100;
		bIsCubicInterpolation = false;
		DecrementAmount = 5;
	}

	FORCEINLINE bool operator==(const FBS_Dynamic& Other) const
	{
		if (StartThreshold != Other.StartThreshold)
		{
			return false;
		}
		if (EndThreshold != Other.EndThreshold)
		{
			return false;
		}
		if (bIsCubicInterpolation != Other.bIsCubicInterpolation)
		{
			return false;
		}
		if (DecrementAmount != Other.DecrementAmount)
		{
			return false;
		}
		return true;
	}
};

/** Defines how to dynamically change the scale of the SpawnArea. Requires using an external counter that increments
 *  each time a target is consecutively destroyed and decrements by DecrementAmount each time a target was not
 *  destroyed. */
USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FBS_Dynamic_SpawnArea : public FBS_Dynamic
{
	GENERATED_BODY()

	/** The size of the SpawnArea/BoxBounds when zero consecutively destroyed targets. X is forward, Y is horizontal,
	 *  Z is vertical. */
	UPROPERTY(EditDefaultsOnly)
	FVector StartBounds;

	/** Returns the StartBounds multiplied by 0.5. */
	FVector GetStartExtents() const
	{
		return FVector(StartBounds.X, StartBounds.Y, StartBounds.Z) * 0.5f;
	}

	FBS_Dynamic_SpawnArea()
	{
		StartThreshold = 5;
		EndThreshold = 100;
		bIsCubicInterpolation = false;
		DecrementAmount = 5;
		StartBounds = FVector(0, 200.f, 200.f);
	}

	FORCEINLINE bool operator==(const FBS_Dynamic_SpawnArea& Other) const
	{
		if (StartThreshold != Other.StartThreshold)
		{
			return false;
		}
		if (EndThreshold != Other.EndThreshold)
		{
			return false;
		}
		if (bIsCubicInterpolation != Other.bIsCubicInterpolation)
		{
			return false;
		}
		if (DecrementAmount != Other.DecrementAmount)
		{
			return false;
		}
		if (!StartBounds.Equals(Other.StartBounds, 0.1f))
		{
			return false;
		}
		return true;
	}
};
