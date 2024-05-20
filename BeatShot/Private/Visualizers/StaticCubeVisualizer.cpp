// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Visualizers/StaticCubeVisualizer.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

AStaticCubeVisualizer::AStaticCubeVisualizer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	InstancedBaseMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName("InstancedBaseMesh"));
	InstancedVerticalOutlineMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(
		FName("InstancedVerticalOutlineMesh"));
	InstancedTopMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName("InstancedTopMesh"));
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(FName("RootSceneComponent"));
	RootComponent = RootSceneComponent;
	InstancedBaseMesh->SetupAttachment(RootSceneComponent);
	InstancedVerticalOutlineMesh->SetupAttachment(RootSceneComponent);
	InstancedTopMesh->SetupAttachment(RootSceneComponent);
	IndexTransformMap = TMap<int32, FTransform>();
}

void AStaticCubeVisualizer::InitializeVisualizer(const FPlayerSettings_AudioAnalyzer& InAASettings)
{
	Super::InitializeVisualizer(InAASettings);
	CreateCubeInstances();
}

void AStaticCubeVisualizer::CreateCubeInstances()
{
	CubeVisualizerDefinition = GetVisualizerDefinition();

	InstancedBaseMesh->ClearInstances();
	InstancedVerticalOutlineMesh->ClearInstances();
	InstancedTopMesh->ClearInstances();

	TArray<FTransform> SpawnTransforms;

	switch (CubeVisualizerDefinition->VisualizerLightSpawningMethod)
	{
	case EVisualizerLightSpawningMethod::SpawnUsingPositionOffsets:
		{
			// Default Start Location
			FTransform CurrentOffsetTransform(FRotator::ZeroRotator, FVector(0),
				GetFastDef().OffsetScale * GetFastDef().MeshScale);

			// If growing from center, start location will be offset by half the total length, growing in the positive direction
			if (GetFastDef().bGrowFromCenter)
			{
				FVector LocationOffset = -GetFastDef().OffsetLocation * FVector(
					static_cast<float>(GetFastDef().NumVisualizerLightsToSpawn - 1) / 2);
				CurrentOffsetTransform = FTransform(FRotator::ZeroRotator, LocationOffset,
					CurrentOffsetTransform.GetScale3D());
			}

			SpawnTransforms.Add(CurrentOffsetTransform);

			for (int i = 0; i < GetFastDef().NumVisualizerLightsToSpawn - 1; i++)
			{
				CurrentOffsetTransform = FTransform(CurrentOffsetTransform.Rotator() + GetFastDef().OffsetRotation,
					CurrentOffsetTransform.GetLocation() + GetFastDef().OffsetLocation,
					GetFastDef().OffsetScale * GetFastDef().MeshScale);
				SpawnTransforms.Add(CurrentOffsetTransform);
			}
			break;
		}
	case EVisualizerLightSpawningMethod::SpawnUsingSpline:
		{
			const TArray<FVector> SplineLocations = GetSplinePointLocations();
			for (int i = 0; i < SplineLocations.Num(); i++)
			{
				SpawnTransforms.Emplace(GetFastDef().SplineActorRotationOffset, SplineLocations[i],
					GetFastDef().OffsetScale * GetFastDef().MeshScale);
			}
			break;
		}
	case EVisualizerLightSpawningMethod::AddExistingLightsFromLevel:
		return;
	}
	IndexTransformMap.Empty();
	for (int i = 0; i < SpawnTransforms.Num(); i++)
	{
		IndexTransformMap.Add(i, SpawnTransforms[i]);
		AddInstancedCubeMesh(SpawnTransforms[i]);
	}
}

void AStaticCubeVisualizer::UpdateVisualizer(const int32 Index, const float SpectrumAlpha)
{
	if (!bIsActivated)
	{
		return;
	}
	for (const int32 LightIndex : GetLightIndices(Index))
	{
		const float CustomDataValue = (GetScaledHeight(SpectrumAlpha) - GetFastDef().MinCubeVisualizerHeightScale) / (
			GetFastDef().MaxCubeVisualizerHeightScale - GetFastDef().MinCubeVisualizerHeightScale);

		InstancedBaseMesh->UpdateInstanceTransform(LightIndex, GetSideAndBaseTransform(LightIndex, SpectrumAlpha));
		InstancedVerticalOutlineMesh->UpdateInstanceTransform(LightIndex,
			GetSideAndBaseTransform(LightIndex, SpectrumAlpha));
		InstancedTopMesh->UpdateInstanceTransform(LightIndex, GetTopMeshTransform(LightIndex, SpectrumAlpha));

		InstancedBaseMesh->SetCustomDataValue(LightIndex, 0, CustomDataValue, false);
		InstancedVerticalOutlineMesh->SetCustomDataValue(LightIndex, 0, CustomDataValue, false);
		InstancedTopMesh->SetCustomDataValue(LightIndex, 0, CustomDataValue, false);
	}
}

void AStaticCubeVisualizer::MarkRenderStateDirty()
{
	InstancedBaseMesh->MarkRenderStateDirty();
	InstancedVerticalOutlineMesh->MarkRenderStateDirty();
	InstancedTopMesh->MarkRenderStateDirty();
}

void AStaticCubeVisualizer::SetActivationState(const bool bActivate)
{
	Super::SetActivationState(bActivate);
	if (bActivate)
	{
		if (IndexTransformMap.IsEmpty())
		{
			CreateCubeInstances();
		}
	}
	else
	{
		InstancedBaseMesh->ClearInstances();
		InstancedVerticalOutlineMesh->ClearInstances();
		InstancedTopMesh->ClearInstances();
		IndexTransformMap.Empty();
	}
}

float AStaticCubeVisualizer::GetScaledHeight(const float SpectrumValue) const
{
	return UKismetMathLibrary::MapRangeClamped(SpectrumValue, 0, 1, GetFastDef().MinCubeVisualizerHeightScale,
		GetFastDef().MaxCubeVisualizerHeightScale);
}

FVector AStaticCubeVisualizer::GetScale3D(const float ScaledHeight)
{
	return FVector(GetFastDef().MeshScale.X * GetFastDef().OffsetScale.X,
		GetFastDef().MeshScale.Y * GetFastDef().OffsetScale.Y,
		GetFastDef().MeshScale.Z * GetFastDef().OffsetScale.Z * ScaledHeight);
}

FTransform AStaticCubeVisualizer::GetTopMeshTransform(const int32 Index, const float SpectrumValue)
{
	const FTransform* Found = IndexTransformMap.Find(Index);
	return FTransform(Found->GetRotation(),
		Found->GetLocation() + FVector(0, 0, GetFastDef().CubeHeight * (GetScaledHeight(SpectrumValue) - 1)),
		GetScale3D(1.f));
}

FTransform AStaticCubeVisualizer::GetSideAndBaseTransform(const int32 Index, const float SpectrumValue)
{
	const FTransform* Found = IndexTransformMap.Find(Index);
	return FTransform(Found->GetRotation(), Found->GetLocation(), GetScale3D(GetScaledHeight(SpectrumValue)));
}

void AStaticCubeVisualizer::AddInstancedCubeMesh(const FTransform& RelativeTransform)
{
	const int32 BaseMeshIndex = InstancedBaseMesh->AddInstance(RelativeTransform, false);
	InstancedBaseMesh->SetMaterial(BaseMeshIndex, BaseCubeMaterial);
	const int32 OutlineMeshIndex = InstancedVerticalOutlineMesh->AddInstance(RelativeTransform, false);
	InstancedVerticalOutlineMesh->SetMaterial(OutlineMeshIndex, OutlineMaterial);
	const int32 TopMeshIndex = InstancedTopMesh->AddInstance(RelativeTransform, false);
	InstancedTopMesh->SetMaterial(TopMeshIndex, OutlineMaterial);
}
