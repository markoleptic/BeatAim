// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSVisualizerDefinition.h"
#include "CubeVisualizerDefinition.generated.h"

/** Configuration unique to a CubeVisualizer. */
UCLASS(Blueprintable, Const, Abstract, EditInlineNew, BlueprintType)
class BEATSHOT_API UCubeVisualizerDefinition : public UBSVisualizerDefinition
{
	GENERATED_BODY()

public:
	/** Scale applied to each instanced static mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cube Visualizer|Scale", meta=(DisplayPriority=0))
	FVector MeshScale;

	/** Base height of the cube. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cube Visualizer|Scale", meta=(DisplayPriority=1))
	float CubeHeight;

	/** Min scaling factor to apply to the height of the cube when receiving no input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cube Visualizer|Scale", meta=(DisplayPriority=2))
	float MinCubeVisualizerHeightScale;

	/** Max scaling factor to apply to the height of the cube when receiving max input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cube Visualizer|Scale", meta=(DisplayPriority=3))
	float MaxCubeVisualizerHeightScale;

	UCubeVisualizerDefinition()
	{
		MeshScale = FVector(0.5f);
		CubeHeight = 100.f;
		MinCubeVisualizerHeightScale = DefaultMinCubeVisualizerHeightScale;
		MaxCubeVisualizerHeightScale = DefaultMaxCubeVisualizerHeightScale;
	}
};
