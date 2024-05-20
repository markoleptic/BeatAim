// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "Visualizers/VisualizerBase.h"
#include "Components/SplineComponent.h"
#include "SaveGames/SaveGamePlayerSettings.h"
#include "Visualizers/BSVisualizerDefinition.h"

AVisualizerBase::AVisualizerBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	VisualizerPositioning = CreateDefaultSubobject<USplineComponent>("Visualizer Positioning");
}

void AVisualizerBase::Destroyed()
{
	SetActivationState(false);
	Super::Destroyed();
}

void AVisualizerBase::InitializeVisualizer(const FPlayerSettings_AudioAnalyzer& InAASettings)
{
	SetActorLocation(GetVisualizerDefinition()->Location);
	SetActorRotation(GetVisualizerDefinition()->Rotation);
	SetActorScale3D(GetVisualizerDefinition()->Scale);
	MapAudioAnalyzerChannelsToVisualizerLights(InAASettings.NumBandChannels,
		GetVisualizerDefinition()->NumVisualizerLightsToSpawn);
}

void AVisualizerBase::InitializeVisualizerFromWorld(const FPlayerSettings_AudioAnalyzer& InAASettings,
	const int32 NumSpawnedVisualizers)
{
	MapAudioAnalyzerChannelsToVisualizerLights(InAASettings.NumBandChannels, NumSpawnedVisualizers);
}

void AVisualizerBase::SetActivationState(const bool bActivate)
{
	bIsActivated = bActivate;
}

void AVisualizerBase::UpdateAASettings(const FPlayerSettings_AudioAnalyzer& InAASettings)
{
	InitializeVisualizer(InAASettings);
}

void AVisualizerBase::AddVisualizerPointsToSpline()
{
	FVector Start;
	if (bDistributeFromCenter)
	{
		const float TotalLength = static_cast<float>(NumberOfPointsToAdd - 1) * Offset;
		Start = -PointAddDirection * TotalLength / 2.f;
	}

	VisualizerPositioning->ClearSplinePoints(true);
	TArray<FSplinePoint> Points;
	for (int i = 0; i < NumberOfPointsToAdd; i++)
	{
		Points.Emplace(FSplinePoint(i, Start + PointAddDirection * Offset * i, ESplinePointType::Constant));
	}
	VisualizerPositioning->AddPoints(Points);
	VisualizerDefinition->SetNumberOfVisualizers(VisualizerPositioning->GetNumberOfSplinePoints());
}

void AVisualizerBase::AddTestVisualizer()
{
	if (VisualizerPositioning->GetNumberOfSplinePoints() == 0 || !TestVisualizerToSpawn)
	{
		return;
	}
	if (SpawnedTestVisualizer)
	{
		SpawnedTestVisualizer->Destroy();
	}
	const FVector Position = VisualizerPositioning->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	SpawnedTestVisualizer = GetWorld()->SpawnActor(TestVisualizerToSpawn, &Position, &TestVisualizerRotation);
}

void AVisualizerBase::MapAudioAnalyzerChannelsToVisualizerLights(const int32 NumBandChannels,
	const int32 NumVisualizers)
{
	UBSVisualizerDefinition* BaseConfig = GetVisualizerDefinition();

	BaseConfig->MappedIndices.Init(FChannelToVisualizerMap(), NumBandChannels);

	if (NumVisualizers == NumBandChannels)
	{
		for (int i = 0; i < NumVisualizers; i++)
		{
			BaseConfig->MappedIndices[i].AddIndex(i);
		}
		return;
	}

	if ((NumVisualizers > NumBandChannels) && (BaseConfig->AssignmentMethod !=
		ELightVisualizerAssignmentMethod::MultiLightPerChannelOnly) && (BaseConfig->AssignmentMethod !=
		ELightVisualizerAssignmentMethod::Auto))
	{
		for (int i = 0; i < NumBandChannels; i++)
		{
			BaseConfig->MappedIndices[i].AddIndex(i);
		}
		return;
	}

	if ((NumBandChannels > NumVisualizers) && (BaseConfig->AssignmentMethod !=
		ELightVisualizerAssignmentMethod::MultiChannelPerLightOnly) && (BaseConfig->AssignmentMethod !=
		ELightVisualizerAssignmentMethod::Auto))
	{
		for (int i = 0; i < NumVisualizers; i++)
		{
			BaseConfig->MappedIndices[i].AddIndex(i);
		}
		return;
	}

	switch (BaseConfig->GroupingMethod)
	{
	case ELightVisualizerGroupingMethod::CombineByProximity:
		{
			const int32 GroupingSize = NumVisualizers / NumBandChannels;
			const int32 LeftToDistribute = NumVisualizers % NumBandChannels;
			int32 CurrentVizIndex = 0;
			for (int i = 0; i < NumBandChannels; i++)
			{
				const int StartIndex = CurrentVizIndex;
				for (int j = StartIndex; j < StartIndex + GroupingSize; j++)
				{
					BaseConfig->MappedIndices[i].AddIndex(j);
					CurrentVizIndex++;
				}
				if (i < LeftToDistribute)
				{
					BaseConfig->MappedIndices[i].AddIndex(CurrentVizIndex);
					CurrentVizIndex++;
				}
			}
		}
		break;
	case ELightVisualizerGroupingMethod::Repeat:
		{
			// Assigning multiple lights to channels
			if (NumVisualizers > NumBandChannels)
			{
				for (int i = 0; i < NumVisualizers; i++)
				{
					const int32 ChannelIndex = i % NumBandChannels;
					BaseConfig->MappedIndices[ChannelIndex].AddIndex(i);
				}
			}
			// Assigning multiple channels to lights
			else if (NumBandChannels > NumVisualizers)
			{
				for (int i = 0; i < NumBandChannels; i++)
				{
					const int32 VizIndex = i % NumVisualizers;
					BaseConfig->MappedIndices[i].AddIndex(VizIndex);
				}
			}
		}
		break;
	}

	int Total = 0;
	for (int i = 0; i < BaseConfig->MappedIndices.Num(); i++)
	{
		int Current = 0;
		for (int j = 0; j < BaseConfig->MappedIndices[i].VisualizerIndices.Num(); j++)
		{
			Current++;
			Total++;
		}
	}
	UE_LOG(LogTemp, Display, TEXT("NumVisualizers: %d, NumBandChannels: %d"), NumVisualizers, NumBandChannels);
	UE_LOG(LogTemp, Display, TEXT("TotalVizualizers in all channels: %d"), Total);
}

TArray<int32>& AVisualizerBase::GetLightIndices(const int32 ChannelIndex)
{
	return GetVisualizerDefinition()->MappedIndices[ChannelIndex].VisualizerIndices;
}

TArray<FVector> AVisualizerBase::GetSplinePointLocations() const
{
	TArray<FVector> OutPoints;
	for (int i = 0; i < VisualizerPositioning->GetNumberOfSplinePoints(); i++)
	{
		OutPoints.Add(VisualizerPositioning->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World));
	}
	return OutPoints;
}
