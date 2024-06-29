// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VisualizerManager.generated.h"

struct FPlayerSettings_AudioAnalyzer;
struct FPlayerSettings_Game;
class AVisualizerBase;
class AStaticCubeVisualizer;
class ABeamVisualizer;
class UAudioAnalyzerManager;

/** The class responsible for managing visualizers in a level. There is only one per level,
 *  and the GameMode spawns it. */
UCLASS()
class BEATSHOT_API AVisualizerManager : public AActor
{
	GENERATED_BODY()

public:
	AVisualizerManager();

	/** Checks for any visualizers already in the level that have been placed in Visualizers and calls
	 *  InitializeVisualizer on each. Spawns all default visualizers in Default Spawn Through Code Visualizers array,
	 *  calls InitializeVisualizer on each, and adds them to Visualizers array. Calls SplitVisualizers at the end. */
	void InitializeVisualizers(const FPlayerSettings_Game& PlayerSettings,
		const FPlayerSettings_AudioAnalyzer& InAASettings);

	/** Main function to update all visualizers, called on tick in GameMode. */
	void UpdateVisualizers(const TArray<float>& SpectrumValues);

	/** Deactivates all visualizers. */
	void DeactivateVisualizers();

	/** Updates a Beam Visualizer's activation state. */
	void SetActivationState_BeamVisualizer(const int32 Index, const bool bActivate);

	/** Updates a Cube Visualizer's activation state. */
	void SetActivationState_CubeVisualizer(const int32 Index, const bool bActivate);

	/** Updates visualizers based on player game settings. */
	void UpdateVisualizerSettings(const FPlayerSettings_Game& PlayerSettings);

	/** Updates visualizers based on player AudioAnalyzer settings. */
	void UpdateAASettings(const FPlayerSettings_AudioAnalyzer& NewAASettings);

	UPROPERTY(VisibleAnywhere, Category = "BeatShot|Update")
	TArray<float> AvgSpectrumValues;

	UPROPERTY(VisibleAnywhere, Category = "BeatShot|Update")
	TArray<float> CurrentSpectrumValues;

	UPROPERTY(VisibleAnywhere, Category = "BeatShot|Update")
	TArray<float> CurrentCubeSpectrumValues;

	UPROPERTY(VisibleAnywhere, Category = "BeatShot|Update")
	TArray<float> MaxSpectrumValues;

protected:
	/* The base StaticCubeVisualizer class to spawn through code. */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Classes")
	TSubclassOf<AStaticCubeVisualizer> BaseClass_CubeVisualizer;

	/** The base BeamVisualizer class to spawn through code. */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Classes")
	TSubclassOf<ABeamVisualizer> BaseClass_BeamVisualizer;

	/** An array of visualizers that will be spawned when this actor is initialized. */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Classes")
	TArray<TSubclassOf<AVisualizerBase>> DefaultSpawnThroughCode_Visualizers;

	/** An array of visualizers that have already been placed in the level editor. */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|References")
	TArray<TSoftObjectPtr<AVisualizerBase>> LevelVisualizers;

	/** All visualizers being managed. */
	UPROPERTY()
	TArray<TObjectPtr<AVisualizerBase>> Visualizers;

	UPROPERTY()
	bool bUpdateBeamVisualizers;

	UPROPERTY()
	bool bUpdateCubeVisualizers;

private:
	TArray<TObjectPtr<ABeamVisualizer>> BeamVisualizers;
	TArray<TObjectPtr<AStaticCubeVisualizer>> CubeVisualizers;

	TArray<TObjectPtr<ABeamVisualizer>>& GetBeamVisualizers() { return BeamVisualizers; }
	TArray<TObjectPtr<AStaticCubeVisualizer>>& GetCubeVisualizers() { return CubeVisualizers; }
	TArray<TObjectPtr<AVisualizerBase>>& GetVisualizers() { return Visualizers; }

	/** Splits Visualizers into smaller subclass groups. */
	void SplitVisualizers();

	/** Updates any Beam Visualizers in BeamVisualizers. */
	void UpdateBeamVisualizers(const int32 Index, const float SpectrumValue);

	/** Updates any Cube Visualizers in CubeVisualizers. */
	void UpdateCubeVisualizers(const int32 Index, const float SpectrumValue);

	/** Returns a normalized spectrum value based on recent spectrum values. */
	float GetNormalizedSpectrumValue(const int32 Index, const bool bIsBeam);

	/** Marks the render state dirty on any visualizers using instanced static meshes. */
	void MarkVisualizerRenderStateDirty();
};
