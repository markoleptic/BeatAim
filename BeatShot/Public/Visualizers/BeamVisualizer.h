// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BeamVisualizerDefinition.h"
#include "SimpleBeamLight.h"
#include "VisualizerBase.h"
#include "BeamVisualizer.generated.h"

/** Light Visualizer that displays beams of light. Controls instances of ASimpleBeamLights. */
UCLASS()
class BEATSHOT_API ABeamVisualizer : public AVisualizerBase
{
	GENERATED_BODY()

public:
	ABeamVisualizer();

	/** Calls the parent implementation and spawns a new array of visualizers given the current AASettings. */
	virtual void InitializeVisualizer(const FPlayerSettings_AudioAnalyzer& InAASettings) override;

	/** Initializes a visualizer already in the level. */
	virtual void InitializeVisualizerFromWorld(const FPlayerSettings_AudioAnalyzer& InAASettings,
		const int32 NumSpawnedVisualizers = INDEX_NONE) override;

	/** Activates the matching visualizer from the given index if it isn't already. */
	virtual void UpdateVisualizer(const int32 Index, const float SpectrumAlpha) override;

	/** Deactivates all Beam Light visualizers. */
	void DeactivateVisualizers();

	virtual void SetActivationState(const bool bActivate) override;

	/** Called by child beam lights to remove them from ActiveLightIndices. */
	UFUNCTION()
	void OnBeamLightLifetimeCompleted(const int32 IndexToRemove);

protected:
	UPROPERTY()
	UBeamVisualizerDefinition* BeamVisualizerDefinition;

	/** An array of spawned Simple Beam Lights. */
	TArray<TObjectPtr<ASimpleBeamLight>> BeamLights;

	/** Returns the Beam Visualizer Definition. */
	UBeamVisualizerDefinition& GetFastDef() const { return *BeamVisualizerDefinition; }

	virtual UBeamVisualizerDefinition* GetVisualizerDefinition() const override
	{
		return Cast<UBeamVisualizerDefinition>(VisualizerDefinition);
	}

	/** Returns the array of simple beam lights. */
	TArray<TObjectPtr<ASimpleBeamLight>>& GetSimpleBeamLights() { return BeamLights; }

	/** Array of beam light indices that are currently active (Niagara particles are active). */
	TArray<int32> ActiveLightIndices;
};
