// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CubeVisualizerDefinition.h"
#include "VisualizerBase.h"
#include "GameFramework/Actor.h"
#include "StaticCubeVisualizer.generated.h"

class UCubeVisualizerDefinition;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;

/** Visualizer that uses instanced static meshes and updates them using CustomDataValues in the materials and adjusting
 *  their transforms using UpdateInstanceTransform. */
UCLASS()
class BEATSHOT_API AStaticCubeVisualizer : public AVisualizerBase
{
	GENERATED_BODY()

public:
	AStaticCubeVisualizer();

	virtual void InitializeVisualizer(const FPlayerSettings_AudioAnalyzer& InAASettings) override;

	/** Updates the CubeHeightScale and the RedGreenAlpha for a cube at Index. */
	virtual void UpdateVisualizer(const int32 Index, const float SpectrumAlpha) override;

	/** Marks all instanced static meshes render states as dirty. Should be called by a VisualizerManager to limit the
	 *  frequency of calls. */
	virtual void MarkRenderStateDirty() override;

	virtual void SetActivationState(const bool bActivate) override;

protected:
	/** Clears all StaticMeshInstances and generates new ones. */
	void CreateCubeInstances();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* RootSceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInstancedStaticMeshComponent* InstancedBaseMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInstancedStaticMeshComponent* InstancedVerticalOutlineMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInstancedStaticMeshComponent* InstancedTopMesh;

	/* The base mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualizer Config | Meshes")
	UStaticMesh* BaseMesh;

	/* The mesh used as the vertical outline. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualizer Config | Meshes")
	UStaticMesh* VerticalOutlineMesh;

	/* The mesh used as the outline for the top of the cube. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualizer Config | Meshes")
	UStaticMesh* TopMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Visualizer Config | Materials")
	UMaterialInterface* BaseCubeMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Visualizer Config | Materials")
	UMaterialInterface* OutlineMaterial;

private:
	UPROPERTY()
	UCubeVisualizerDefinition* CubeVisualizerDefinition;

	/** Returns the Beam Visualizer Definition. */
	virtual UCubeVisualizerDefinition* GetVisualizerDefinition() const override
	{
		return Cast<UCubeVisualizerDefinition>(VisualizerDefinition);
	}

	/** Returns the Cube Visualizer Definition. */
	UCubeVisualizerDefinition& GetFastDef() const { return *CubeVisualizerDefinition; }

	/** Returns the SpectrumValue scaled between MinCubeHeightScale and MaxCubeHeightScale. */
	float GetScaledHeight(const float SpectrumValue) const;

	/** Returns the scale to be supplied to an instanced static mesh, combining OffsetTransform and MeshScale. */
	FVector GetScale3D(const float ScaledHeight);

	/** Returns the transform to be supplied to the InstancedTopMesh. */
	FTransform GetTopMeshTransform(const int32 Index, const float SpectrumValue);

	/** Returns the transform to be supplied to the InstancedBaseMesh and InstancedVerticalOutlineMesh. */
	FTransform GetSideAndBaseTransform(const int32 Index, const float SpectrumValue);

	/** Adds an instance for each mesh that is updated, i.e. InstancedBaseMesh, InstancedVerticalOutlineMesh, and
	 *  InstancedTopMesh with the given RelativeTransform. */
	void AddInstancedCubeMesh(const FTransform& RelativeTransform);

	TMap<int32, FTransform> IndexTransformMap;
};
