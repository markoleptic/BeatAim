// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Visualizers/VisualizerManager.h"
#include "Visualizers/VisualizerBase.h"
#include "Visualizers/StaticCubeVisualizer.h"
#include "Visualizers/BeamVisualizer.h"
#include "Kismet/KismetMathLibrary.h"

AVisualizerManager::AVisualizerManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AVisualizerManager::InitializeVisualizers(const FPlayerSettings_Game& PlayerSettings, const FPlayerSettings_AudioAnalyzer& InAASettings)
{
	AvgSpectrumValues.Init(0, InAASettings.NumBandChannels);
	CurrentSpectrumValues.Init(0, InAASettings.NumBandChannels);
	MaxSpectrumValues.Init(1.f, InAASettings.NumBandChannels);
	CurrentCubeSpectrumValues.Init(0, InAASettings.NumBandChannels);
	
	/* Initialize visualizers already placed in level that may or not have spawned lights already */
	for (const TSoftObjectPtr<AVisualizerBase>& Visualizer : LevelVisualizers)
	{
		if (Visualizer)
		{
			switch (Visualizer->GetVisualizerDefinition()->VisualizerLightSpawningMethod)
			{
				case EVisualizerLightSpawningMethod::SpawnUsingPositionOffsets:
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid Spawning Method for a Visualizer in LevelVisualizers"));
					continue;
				}
				case EVisualizerLightSpawningMethod::SpawnUsingSpline:
				{
					Visualizer->InitializeVisualizer(InAASettings);
					break;
				}
				case EVisualizerLightSpawningMethod::AddExistingLightsFromLevel:
				{
					Visualizer->InitializeVisualizerFromWorld(InAASettings, INDEX_NONE);
					break;
				}
			}
			Visualizers.AddUnique(Visualizer.Get());
		}
	}

	/* Spawn visualizers that have been configured but have not been spawned in the level */
	for (TSubclassOf<AVisualizerBase>& Visualizer : DefaultSpawnThroughCode_Visualizers)
	{
		if (Visualizer)
		{
			const AVisualizerBase* CDO = Visualizer.GetDefaultObject();
			if (Visualizers.Contains(CDO))
			{
				continue;
			}
			FTransform Transform(CDO->GetVisualizerDefinition()->Rotation, CDO->GetVisualizerDefinition()->Location, CDO->GetVisualizerDefinition()->Scale);
			AVisualizerBase* SpawnedVisualizer = GetWorld()->SpawnActorDeferred<AVisualizerBase>(Visualizer, Transform);
			SpawnedVisualizer->InitializeVisualizer(InAASettings);
			SpawnedVisualizer->FinishSpawning(Transform, true);
			Visualizers.AddUnique(SpawnedVisualizer);
		}
	}

	/* Split visualizer types into their own containers so that UpdateVisualizers can be as efficient as possible */
	SplitVisualizers();
	
	if (PlayerSettings.bShowLightVisualizers)
	{
		bUpdateBeamVisualizers = true;
	}
	bUpdateCubeVisualizers = true;
}

float AVisualizerManager::GetNormalizedSpectrumValue(const int32 Index, const bool bIsBeam)
{
	if (bIsBeam)
	{
		return UKismetMathLibrary::MapRangeClamped(CurrentSpectrumValues[Index], 0, MaxSpectrumValues[Index], 0, 1);
	}
	return UKismetMathLibrary::MapRangeClamped(CurrentCubeSpectrumValues[Index] - AvgSpectrumValues[Index], 0, MaxSpectrumValues[Index], 0, 1);
}

void AVisualizerManager::UpdateVisualizers(const TArray<float>& SpectrumValues)
{
	if (!bUpdateCubeVisualizers && !bUpdateBeamVisualizers)
	{
		return;
	}
	
	for (int i = 0; i < SpectrumValues.Num(); i++)
	{
		if (SpectrumValues[i] > MaxSpectrumValues[i])
		{
			MaxSpectrumValues[i] = SpectrumValues[i];
		}
		if (SpectrumValues[i] > CurrentCubeSpectrumValues[i])
		{
			CurrentCubeSpectrumValues[i] = SpectrumValues[i];
		}

		if (SpectrumValues[i] > 0 && CurrentSpectrumValues[i] < 0)
		{
			CurrentSpectrumValues[i] = SpectrumValues[i];
			if (bUpdateBeamVisualizers)
			{
				UpdateBeamVisualizers(i, GetNormalizedSpectrumValue(i, true));
			}
		}
		if (bUpdateCubeVisualizers)
		{
			UpdateCubeVisualizers(i, GetNormalizedSpectrumValue(i, false));
		}
		if (CurrentSpectrumValues[i] >= 0)
		{
			CurrentSpectrumValues[i] -= AvgSpectrumValues[i] / CurrentSpectrumValueDecrementDivide;
		}
		if (CurrentCubeSpectrumValues[i] >= 0)
		{
			CurrentCubeSpectrumValues[i] -= CurrentCubeSpectrumValueDecrement;
		}
		if (MaxSpectrumValues[i] >= 0)
		{
			MaxSpectrumValues[i] -= AvgSpectrumValues[i] / MaxSpectrumValueDecrementDivide;
		}
	}
	MarkVisualizerRenderStateDirty();
}

void AVisualizerManager::UpdateBeamVisualizers(const int32 Index, const float SpectrumValue)
{
	for (const TObjectPtr<ABeamVisualizer>& BeamVisualizer : GetBeamVisualizers())
	{
		BeamVisualizer->ActivateVisualizer(Index);
	}
}

void AVisualizerManager::UpdateCubeVisualizers(const int32 Index, const float SpectrumValue)
{
	for (const TObjectPtr<AStaticCubeVisualizer>& CubeVisualizer : GetCubeVisualizers())
	{
		CubeVisualizer->UpdateVisualizer(Index, SpectrumValue);
	}
}

void AVisualizerManager::DeactivateVisualizers()
{
	for (const TObjectPtr<AVisualizerBase> Visualizer : GetVisualizers())
	{
		if (Visualizer)
		{
			Visualizer->DeactivateVisualizers();
		}
	}
}

void AVisualizerManager::DeactivateBeamVisualizers()
{
	for (const TObjectPtr<ABeamVisualizer>& BeamVisualizer : GetBeamVisualizers())
	{
		BeamVisualizer->DeactivateVisualizers();
	}
}

void AVisualizerManager::DeactivateCubeVisualizers()
{
	for (const TObjectPtr<AStaticCubeVisualizer>& CubeVisualizer : GetCubeVisualizers())
	{
		CubeVisualizer->DeactivateVisualizers();
	}
}

void AVisualizerManager::UpdateVisualizerSettings(const FPlayerSettings_Game& PlayerSettings)
{
	bUpdateBeamVisualizers = PlayerSettings.bShowLightVisualizers;
	if (!bUpdateBeamVisualizers)
	{
		DeactivateBeamVisualizers();
	}
}

void AVisualizerManager::UpdateAASettings(const FPlayerSettings_AudioAnalyzer& NewAASettings)
{
	AvgSpectrumValues.Init(0, NewAASettings.NumBandChannels);
	CurrentSpectrumValues.Init(0, NewAASettings.NumBandChannels);
	MaxSpectrumValues.Init(1.f, NewAASettings.NumBandChannels);
	CurrentCubeSpectrumValues.Init(0, NewAASettings.NumBandChannels);
	for (const TObjectPtr<AVisualizerBase> Visualizer : GetVisualizers())
	{
		if (Visualizer)
		{
			Visualizer->UpdateAASettings(NewAASettings);
		}
	}
}

void AVisualizerManager::MarkVisualizerRenderStateDirty()
{
	for (const TObjectPtr<AStaticCubeVisualizer>& CubeVisualizer : GetCubeVisualizers())
	{
		CubeVisualizer->MarkRenderStateDirty();
	}
}

void AVisualizerManager::SplitVisualizers()
{
	for (const TObjectPtr<AVisualizerBase>& Visualizer : GetVisualizers())
	{
		if (Cast<ABeamVisualizer>(Visualizer.Get()))
		{
			BeamVisualizers.AddUnique(Cast<ABeamVisualizer>(Visualizer.Get()));
		}
		else if (Cast<AStaticCubeVisualizer>(Visualizer.Get()))
		{
			CubeVisualizers.AddUnique(Cast<AStaticCubeVisualizer>(Visualizer.Get()));
		}
	}
}
