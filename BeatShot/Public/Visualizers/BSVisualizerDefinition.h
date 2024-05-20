// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSConstants.h"
#include "Engine/DataAsset.h"
#include "BSVisualizerDefinition.generated.h"

using namespace Constants;

/** How to assign AudioAnalyzer channels to lights if there is an uneven number between the two. */
UENUM(BlueprintType)
enum class EVisualizerLightSpawningMethod : uint8
{
	/** Spawn the lights at runtime, using position offset values. */
	SpawnUsingPositionOffsets UMETA(DisplayName="Spawn Using Position Offsets"),

	/** Spawn the lights at runtime, using the spline component of the visualizer. */
	SpawnUsingSpline UMETA(DisplayName="Spawn Using Spline"),

	/** Add existing lights already placed inside a level. */
	AddExistingLightsFromLevel UMETA(DisplayName="Add Existing Lights From Level"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EVisualizerLightSpawningMethod, EVisualizerLightSpawningMethod::SpawnUsingPositionOffsets,
	EVisualizerLightSpawningMethod::AddExistingLightsFromLevel);

/** How to assign AudioAnalyzer channels to lights if there is an uneven number between the two. */
UENUM(BlueprintType)
enum class ELightVisualizerAssignmentMethod : uint8
{
	/** Don't assign channels to any lights (not implemented). */
	None UMETA(DisplayName="None"),

	/** Assign a channel to every light, or assign a light to every channel, depending on which has more. */
	Auto UMETA(DisplayName="Auto"),

	/** Assign a channel to every light, but don't assign more than one light to any channel. */
	MultiChannelPerLightOnly UMETA(DisplayName="Multi Channel Per Light Only"),

	/** Assign a light to every channel, but don't assign more than one channel to any light. */
	MultiLightPerChannelOnly UMETA(DisplayName="Multi Light Per Channel Only"),

	/** Only assign one channel to any light, and vice versa. */
	SinglePairsOnly UMETA(DisplayName="Single Pairs Only")};

ENUM_RANGE_BY_FIRST_AND_LAST(ELightVisualizerAssignmentMethod, ELightVisualizerAssignmentMethod::None,
	ELightVisualizerAssignmentMethod::SinglePairsOnly);

/** How to group AudioAnalyzer channels and lights together if there is an uneven number between the two. */
UENUM(BlueprintType)
enum class ELightVisualizerGroupingMethod : uint8
{
	/** Assigns nearby lights to the same channel if lights > channels,
	 *  or assigns consecutive channels to the same light if channels > lights. */
	CombineByProximity UMETA(DisplayName="Combine By Proximity"),

	/** Assign in order. If there is an unequal amount of lights and channels, repeat from the start. */
	Repeat UMETA(DisplayName="Repeat")};

ENUM_RANGE_BY_FIRST_AND_LAST(ELightVisualizerGroupingMethod, ELightVisualizerGroupingMethod::CombineByProximity,
	ELightVisualizerGroupingMethod::Repeat);

/** Each instance of this struct represents an AudioAnalyzer channel index, containing 0-multiple visualizer indices. */
USTRUCT(BlueprintType)
struct BEATSHOT_API FChannelToVisualizerMap
{
	GENERATED_BODY()

	/** An array of visualizer indices that are assigned to this AudioAnalyzer channel index. */
	TArray<int32> VisualizerIndices;

	FChannelToVisualizerMap()
	{
		VisualizerIndices = TArray<int32>();
	}

	/** Adds a visualizer index to this AudioAnalyzer channel index. */
	void AddIndex(const int32 InIndex)
	{
		VisualizerIndices.Add(InIndex);
	}
};

/** Describes a visualizer to be spawned in the world. */
UCLASS(Blueprintable, Abstract, BlueprintType, EditInlineNew, AutoExpandCategories=("Spawning", "Audio Analyzer"))
class BEATSHOT_API UBSVisualizerDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** whether to spawn visualizer lights, or add already placed lights. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta=(FullyExpand="true", DisplayPriority=-1300))
	EVisualizerLightSpawningMethod VisualizerLightSpawningMethod;

	/** Number of visualizer lights to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning",
		meta=(FullyExpand="true", DisplayPriority=-1299, EditCondition=
			"VisualizerLightSpawningMethod == EVisualizerLightSpawningMethod::SpawnUsingPositionOffsets"))
	int32 NumVisualizerLightsToSpawn;


	/** World location to place this visualizer. Will affect the location of spawned lights if not using
	 *  AddExistingLightsFromLevel spawning method. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning|Position",
		meta=(DisplayPriority=-1100, EditCondition=
			"VisualizerLightSpawningMethod != EVisualizerLightSpawningMethod::AddExistingLightsFromLevel"))
	FVector Location;

	/** World rotation to apply to this visualizer. Will affect the rotation of spawned lights if not using
	 *  AddExistingLightsFromLevel spawning method. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning|Position",
		meta=(ForceUnits="Degrees", DisplayPriority=-1000, EditCondition=
			"VisualizerLightSpawningMethod != EVisualizerLightSpawningMethod::AddExistingLightsFromLevel"))
	FRotator Rotation;

	/** World scale to apply to this actor. Will affect the scale of spawned lights if not using
	 *  AddExistingLightsFromLevel spawning method. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning|Position",
		meta=(DisplayPriority=-900, EditCondition=
			"VisualizerLightSpawningMethod != EVisualizerLightSpawningMethod::AddExistingLightsFromLevel"))
	FVector Scale;


	/** Rotation offset to apply to the lights spawned along a spline path. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning|Offset",
		meta=(ForceUnits="Degrees", DisplayPriority=-850, EditCondition=
			"VisualizerLightSpawningMethod == EVisualizerLightSpawningMethod::SpawnUsingSpline"))
	FRotator SplineActorRotationOffset;

	/** Whether to make the StartTransform the center of the spawned lights when using position offset spawning
	 *  method. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning|Offset",
		meta=(DisplayPriority=-800, EditCondition=
			"VisualizerLightSpawningMethod == EVisualizerLightSpawningMethod::SpawnUsingPositionOffsets"))
	bool bGrowFromCenter;

	/** Relative offset location to apply between lights when using position offset spawning method. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning|Offset",
		meta=(DisplayPriority=-700, EditCondition=
			"VisualizerLightSpawningMethod == EVisualizerLightSpawningMethod::SpawnUsingPositionOffsets"))
	FVector OffsetLocation;

	/** Relative offset rotation to apply between lights when using position offset spawning method. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning|Offset",
		meta=(Units="Degrees", DisplayPriority=-600, EditCondition=
			"VisualizerLightSpawningMethod == EVisualizerLightSpawningMethod::SpawnUsingPositionOffsets"))
	FRotator OffsetRotation;

	/** Relative offset scale to apply between lights when using position offset spawning method. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning|Offset",
		meta=(DisplayPriority=-500, EditCondition=
			"VisualizerLightSpawningMethod == EVisualizerLightSpawningMethod::SpawnUsingPositionOffsets"))
	FVector OffsetScale;

	/** How to assign AudioAnalyzer channels to lights. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Analyzer", meta=(DisplayPriority=-400))
	ELightVisualizerAssignmentMethod AssignmentMethod;

	/** How to group AudioAnalyzer channels and lights together. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Analyzer ", meta=(DisplayPriority=-300))
	ELightVisualizerGroupingMethod GroupingMethod;

	/** Each index in this array represents an AudioAnalyzerChannel, where the element is an array of visualizer
	 *  indices that should sync to this AudioAnalyzerChannel. */
	TArray<FChannelToVisualizerMap> MappedIndices;

	UBSVisualizerDefinition()
	{
		VisualizerLightSpawningMethod = EVisualizerLightSpawningMethod::SpawnUsingSpline;
		bGrowFromCenter = true;
		Location = FVector(0);
		Rotation = FRotator(0);
		Scale = FVector(1.f);
		OffsetLocation = FVector(0);
		OffsetRotation = FRotator(0);
		OffsetScale = FVector(1.f);
		NumVisualizerLightsToSpawn = DefaultNumVisualizerLightsToSpawn;
		AssignmentMethod = ELightVisualizerAssignmentMethod::MultiLightPerChannelOnly;
		GroupingMethod = ELightVisualizerGroupingMethod::CombineByProximity;
		MappedIndices = TArray<FChannelToVisualizerMap>();
	}

	void SetNumberOfVisualizers(const int32 Num)
	{
		NumVisualizerLightsToSpawn = Num;
	}
};
