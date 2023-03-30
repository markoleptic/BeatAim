// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BeatShot.generated.h"

class FBeatShot : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

#define ACTOR_ROLE_FSTRING *(FindObject<UEnum>(nullptr, TEXT("/Script/Engine.ENetRole"), true)->GetNameStringByValue(GetLocalRole()))
#define GET_ACTOR_ROLE_FSTRING(Actor) *(FindObject<UEnum>(nullptr, TEXT("/Script/Engine.ENetRole"), true)->GetNameStringByValue(Actor->GetLocalRole()))

DECLARE_MULTICAST_DELEGATE_OneParam(FOnQTableUpdate, const TArray<float>& UpdatedQTable);

UENUM(BlueprintType)
enum class EMovementType : uint8
{
	Sprinting UMETA(DisplayName="Sprinting"),
	Walking UMETA(DisplayName="Walking"),
	Crouching UMETA(DisplayName="Crouching")};

/** Enum representing the different times of the day */
UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	Day UMETA(DisplayName="Day"),
	Night UMETA(DisplayName="Night"),
	DayToNight UMETA(DisplayName="DayToNight"),
	NightToDay UMETA(DisplayName="NightToDay"),
};

UENUM(BlueprintType)
enum class EPlatformTransitionType : uint8
{
	None UMETA(DisplayName="MoveUpByInteract"),
	MoveUpByInteract UMETA(DisplayName="MoveUpByInteract"),
	MoveDownByInteract UMETA(DisplayName="MoveDownByInteract"),
	MoveDownByStepOff UMETA(DisplayName="MoveDownByStepOff")};

UENUM(BlueprintType)
enum class EDirection : uint8
{
	Left UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right"),
	Up UMETA(DisplayName="Up"),
	Down UMETA(DisplayName="Down"),
};

/** A struct representing two consecutively spawned targets */
USTRUCT()
struct FTargetPair
{
	GENERATED_BODY()

	/** The location of the target spawned before Current */
	FVector Previous;

	/** The location spawned after Previous */
	FVector Current;

	float Reward;

	FTargetPair()
	{
		Previous = FVector::ZeroVector;
		Current = FVector::ZeroVector;
		Reward = 0.f;
	}

	FTargetPair(const FVector CurrentPoint)
	{
		Previous = FVector::ZeroVector;
		Current = CurrentPoint;
		Reward = 0.f;
	}

	FTargetPair(const FVector PreviousPoint, const FVector CurrentPoint)
	{
		Previous = PreviousPoint;
		Current = CurrentPoint;
		Reward = 0.f;
	}

	FORCEINLINE bool operator ==(const FTargetPair& Other) const
	{
		if (Current.Y == Other.Current.Y && Current.Z == Other.Current.Z)
		{
			return true;
		}
		return false;
	}
};

/** A struct representing the space in the grid that a recently spawned target occupies */
USTRUCT()
struct FRecentTarget
{
	GENERATED_BODY()

	/** An array of points that were inside the SpawnBox and inside the target */
	TArray<FVector> OverlappingPoints;

	/** The location of the center of the target */
	FVector CenterVector;

	/** A unique ID for the target, used to find the target when it comes time to free the blocked points of a target */
	FGuid TargetGuid;

	/** The scale of the target relative to the world */
	float TargetScale;

	FRecentTarget()
	{
		CenterVector = FVector::ZeroVector;
		TargetScale = 0.f;
	}

	explicit FRecentTarget(const FGuid Guid)
	{
		CenterVector = FVector::ZeroVector;
		TargetGuid = Guid;
		TargetScale = 0.f;
	}

	FRecentTarget(const FGuid NewTargetGuid, const TArray<FVector> Points, const float NewTargetScale, const FVector NewCenter)
	{
		TargetGuid = NewTargetGuid;
		OverlappingPoints = Points;
		TargetScale = NewTargetScale;
		CenterVector = NewCenter;
	}

	FORCEINLINE bool operator ==(const FRecentTarget& Other) const
	{
		if (TargetGuid == Other.TargetGuid)
		{
			return true;
		}
		return false;
	}
};

/** A struct representing a point in a 2D grid with information about that point */
USTRUCT()
struct FVectorCounter
{
	GENERATED_BODY()

	/** Unscaled, world spawn location point. Bottom left of the square sub-area */
	FVector Point;

	/** The center of the square sub-area */
	FVector Center;

	/** The chosen point for this vector counter, it might be different than Point, but will be within the sub-area bounded by incrementY and incrementZ */
	FVector ActualChosenPoint;

	/** The total number of target spawns at this point */
	int32 TotalSpawns;

	/** The total number of target hits by player at this point */
	int32 TotalHits;

	/** The index inside SpawnCounter for this VectorCounter */
	int32 Index;

	float IncrementY;
	
	float IncrementZ;

	FVectorCounter()
	{
		Point = FVector();
		ActualChosenPoint = FVector();
		TotalSpawns = -1;
		TotalHits = 0;
		IncrementY = 0.f;
		IncrementZ = 0.f;
		Index = -1;
	}

	FVectorCounter(const FVector NewPoint)
	{
		Point = NewPoint;
		ActualChosenPoint = FVector();
		TotalSpawns = -1;
		TotalHits = 0;
		IncrementY = 0.f;
		IncrementZ = 0.f;
		Index = -1;
	}

	FVectorCounter(const int32 NewIndex, const FVector NewPoint, const float IncY, const float IncZ)
	{
		Index = NewIndex;
		Point = NewPoint;
		ActualChosenPoint = FVector();
		TotalSpawns = -1;
		TotalHits = 0;
		IncrementY = IncY;
		IncrementZ = IncZ;
		Center = FVector(Point.X, roundf(Point.Y + IncrementY / 2.f), roundf(Point.Z + IncrementZ / 2.f));
	}

	FORCEINLINE bool operator ==(const FVectorCounter& Other) const
	{
		if (Other.Point.Y >= Point.Y && Other.Point.Y < Point.Y + IncrementY
			&& (Other.Point.Z >= Point.Z && Other.Point.Z < Point.Z + IncrementZ))
		{
			return true;
		}
		return false;
	}

	FORCEINLINE bool operator <(const FVectorCounter& Other) const
	{
		if (Point.Z < Other.Point.Z)
		{
			return true;
		}
		if (Point.Z == Other.Point.Z && Point.Y < Other.Point.Y)
		{
			return true;
		}
		return false;
	}

	FVector GetRandomSubPoint(const TArray<EDirection> BlockedDirections) const
	{
		float MinY = Point.Y;
		float MaxY = Point.Y + IncrementY;
		float MinZ = Point.Z;
		float MaxZ = Point.Z + IncrementZ;
		if (BlockedDirections.Contains(EDirection::Left))
		{
			MinY = Center.Y;
		}
		if (BlockedDirections.Contains(EDirection::Right))
		{
			MaxY = Center.Y;
		}
		if (BlockedDirections.Contains(EDirection::Down))
		{
			MinZ = Center.Z;
		}
		if (BlockedDirections.Contains(EDirection::Up))
		{
			MaxZ = Center.Z;
		}
		
		const float Y = roundf(FMath::FRandRange(MinY, MaxY - 1.f));
		const float Z = roundf(FMath::FRandRange(MinZ, MaxZ - 1.f));
		return FVector(Point.X, Y, Z);
	}
};

/** Used to store movement properties for different movement types */
USTRUCT(BlueprintType)
struct FMovementTypeVariables
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Movement Variables")
	float MaxAcceleration;

	UPROPERTY(EditDefaultsOnly, Category = "Movement Variables")
	float BreakingDecelerationWalking;

	UPROPERTY(EditDefaultsOnly, Category = "Movement Variables")
	float GroundFriction;

	UPROPERTY(EditDefaultsOnly, Category = "Movement Variables")
	float MaxWalkSpeed;

	FMovementTypeVariables()
	{
		MaxAcceleration = 0.f;
		BreakingDecelerationWalking = 0.f;
		GroundFriction = 0.f;
		MaxWalkSpeed = 0.f;
	}
};
