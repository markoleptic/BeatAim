// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSVisualizerDefinition.h"
#include "GameFramework/Actor.h"
#include "VisualizerBase.generated.h"

struct FPlayerSettings_AudioAnalyzer;
class UBSVisualizerDefinition;
class USplineComponent;
class USceneComponent;
class UAudioAnalyzerManager;

/** Base class for Light Visualizers. BeamVisualizer and StaticCubeVisualizer are examples of classes that subclass this
 *  one. */
UCLASS(Abstract)
class BEATSHOT_API AVisualizerBase : public AActor
{
	GENERATED_BODY()

	friend class AVisualizerManager;

public:
	AVisualizerBase();

protected:
	virtual void Destroyed() override;

	UPROPERTY(EditInstanceOnly, Category = "Spline")
	USplineComponent* VisualizerPositioning;

public:
	/** Copies the new AudioAnalyzer Settings. Overrides should spawn their specific visualizer implementations. */
	virtual void InitializeVisualizer(const FPlayerSettings_AudioAnalyzer& InAASettings);

	/** Initializes a visualizer already in the level. */
	virtual void InitializeVisualizerFromWorld(const FPlayerSettings_AudioAnalyzer& InAASettings,
		const int32 NumSpawnedVisualizers);

	/** Sets whether the visualizer should receive input from the Audio Analyzer. */
	virtual void SetActivationState(const bool bActivate);

	/** Returns whether the visualizer is receiving input from the Audio Analyzer. */
	bool IsActivated() const { return bIsActivated; }

	/** Updates a visualizer state at Index inside Visualizers array. SpectrumAlpha should be a value between
	 *  0 and 1, with 1 being the maximum visualizer state and 0 being the minimum. */
	virtual void UpdateVisualizer(const int32 Index, const float SpectrumAlpha)
	{
	}

	/** Called by the VisualizerManager when AudioAnalyzer settings are changed. Calls InitializeVisualizer. */
	virtual void UpdateAASettings(const FPlayerSettings_AudioAnalyzer& InAASettings);

	/** If the visualizer is using an instanced static mesh, use this function to rerender the meshes. Should be called
	 *  by VisualizerManager. */
	virtual void MarkRenderStateDirty()
	{
	}

	/** Returns the definition for this visualizer. */
	virtual UBSVisualizerDefinition* GetVisualizerDefinition() const { return VisualizerDefinition; }

	/** The number of points to add to the spline (how many visualizers will be spawned). */
	UPROPERTY(EditInstanceOnly, Category = "Spline")
	int32 NumberOfPointsToAdd;

	/** The direction to distribute the splint points towards. */
	UPROPERTY(EditInstanceOnly, Category = "Spline")
	FVector PointAddDirection;

	/** The spacing in between spline points. */
	UPROPERTY(EditInstanceOnly, Category = "Spline")
	float Offset;

	/** whether to distribute the points centered from the spline component location. */
	UPROPERTY(EditInstanceOnly, Category = "Spline")
	bool bDistributeFromCenter;

	/** Clears all exiting spline points, and adds points according to NumberOfPointsToAdd, PointAddDirection, Offset,
	 *  and DistributeFromCenter. */
	UFUNCTION(CallInEditor, Category = "Spline")
	void AddVisualizerPointsToSpline();

	UPROPERTY(EditInstanceOnly, Category = "Spline")
	TSubclassOf<AActor> TestVisualizerToSpawn;

	UPROPERTY(Transient)
	AActor* SpawnedTestVisualizer;

	/** The direction to distribute the splint points towards. */
	UPROPERTY(EditInstanceOnly, Category = "Spline")
	FRotator TestVisualizerRotation;

	/** Spawns a visualizer specified in TestVisualizerToSpawn to determine what location and rotation is needed for
	 *  alignment. */
	UFUNCTION(CallInEditor, Category = "Spline")
	void AddTestVisualizer();

protected:
	/** Maps AudioAnalyzer channels to individual visualizer lights. */
	virtual void MapAudioAnalyzerChannelsToVisualizerLights(const int32 NumBandChannels,
		const int32 NumVisualizers = INDEX_NONE);

	/** Returns the visualizer light indices that correspond to AudioAnalyzer ChannelIndex. */
	virtual TArray<int32>& GetLightIndices(const int32 ChannelIndex);

	/** Returns the locations to spawn visualizer lights based on the spline component's points. */
	TArray<FVector> GetSplinePointLocations() const;

	/** The data asset that specifies the configuration for this visualizer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Visualizer Definition",
		meta=(DisplayPriority=-20000))
	UBSVisualizerDefinition* VisualizerDefinition;

	bool bIsActivated;
};
