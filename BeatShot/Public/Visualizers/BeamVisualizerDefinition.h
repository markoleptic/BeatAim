// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSVisualizerDefinition.h"
#include "BeamVisualizerDefinition.generated.h"

class UNiagaraSystem;
class UCurveVector;
class ASimpleBeamLight;

/** Describes a visualizer to be spawned in the world. */
UCLASS(Blueprintable, Const, Abstract, EditInlineNew, BlueprintType)
class BEATSHOT_API UBeamVisualizerDefinition : public UBSVisualizerDefinition
{
	GENERATED_BODY()

public:
	/** The class of the Visualizer Light to spawn, if any. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning",
		meta=(DisplayPriority=-1298, EditCondition=
			"VisualizerLightSpawningMethod != EVisualizerLightSpawningMethod::AddExistingLightsFromLevel"))
	TSubclassOf<AActor> VisualizerLightClass;

	/** Array of Visualizer Lights that have already been placed in a level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning",
		meta=(DisplayName="Existing Lights from Level", DisplayPriority=-1297, EditCondition=
			"VisualizerLightSpawningMethod == EVisualizerLightSpawningMethod::AddExistingLightsFromLevel"))
	TArray<TSoftObjectPtr<AActor>> VisualizerLightsFromLevel;

	/** Whether override each child light's light colors with BeamLightColors, vs use the light color from each child
	 *  light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|General", meta=(DisplayPriority=-300))
	bool bOverrideLightColors;

	/** The base colors to apply to Spotlight, BeamEndLight, and SimpleBeamComponent. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|General",
		meta=(DisplayPriority=-200, EditCondition="bOverrideLightColors"))
	TArray<FLinearColor> BeamLightColors;

	/** whether to override any visualizer settings that the lights may use. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|General", meta=(DisplayPriority=-100))
	bool bOverrideChildLightConfig;

	/** The length of time this lights will be on for. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|General", meta=(DisplayPriority=0))
	float LightDuration;

	/** Max intensity of the EmissiveLightBulb. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|EmissiveLightBulb",
		meta=(DisplayName="Max Light Bulb Intensity", DisplayPriority=200))
	float MaxEmissiveLightBulbLightIntensity;

	/** whether this light will need to use the LightPositionTimeline and LineTracing at runtime. If true,
	 *  LightMovementCurve must have a curve. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|Movement",
		meta=(DisplayName="Moving Light", DisplayPriority=300))
	bool bIsMovingLight;

	/** Curve that defines the position of the end of the beam over time. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|Movement",
		meta=(DisplayPriority=400, EditCondition="bIsMovingLight"))
	UCurveVector* LightMovementCurve;

	/** whether to activate the Spotlight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|Spotlight", meta=(DisplayPriority=500))
	bool bUseSpotlight;

	/** Max intensity of the Spotlight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|Spotlight",
		meta=(DisplayPriority=600, EditCondition="bUseSpotlight"))
	float MaxSpotlightIntensity;

	/** Inner cone angle for the Spotlight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|Spotlight",
		meta=(DisplayName="Spotlight Inner Cone Angle", DisplayPriority=700, EditCondition="bUseSpotlight"))
	float InnerConeAngle;

	/** Outer cone angle for the Spotlight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|Spotlight",
		meta=(DisplayName="Spotlight Outer Cone Angle", DisplayPriority=800, EditCondition="bUseSpotlight"))
	float OuterConeAngle;

	/** whether to activate the BeamEndLight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|BeamEndLight", meta=(DisplayPriority=900))
	bool bUseBeamEndLight;

	/** Max intensity of the MaxBeamEndLight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|BeamEndLight",
		meta=(DisplayPriority=1000, EditCondition="bUseBeamEndLight"))
	float MaxBeamEndLightIntensity;

	/** The Niagara System used for this light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|Niagara Beam", meta=(DisplayPriority=1100))
	UNiagaraSystem* NiagaraSystem;

	/** Auto calculates the beam length by doing a line trace on initialization. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|Niagara Beam",
		meta=(DisplayPriority=1200, EditCondition="NiagaraSystem!=nullptr"))
	bool bAutoCalculateBeamLength;

	/** The Niagara System float parameters to used for this light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|Niagara Beam", meta=(ForceInlineRow),
		meta=(DisplayPriority=1300, EditCondition="NiagaraSystem!=nullptr"))
	TMap<FString, float> NiagaraFloatParameters;

	/** The Niagara System Color parameters to used for this light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam Visualizer|Niagara Beam", meta=(ForceInlineRow),
		meta=(DisplayPriority=1400, EditCondition="NiagaraSystem!=nullptr"))
	TMap<FString, FLinearColor> NiagaraColorParameters;

	/** The index of this instance inside an array of this type. */
	int32 Index = INDEX_NONE;

	UBeamVisualizerDefinition()
	{
		LightDuration = 1.f;
		Index = INDEX_NONE;

		VisualizerLightClass = TSubclassOf<AActor>();
		VisualizerLightsFromLevel = TArray<TSoftObjectPtr<AActor>>();

		bOverrideChildLightConfig = true;
		bOverrideLightColors = true;
		BeamLightColors = DefaultBeamLightColors;

		bIsMovingLight = false;
		LightMovementCurve = nullptr;
		bUseSpotlight = false;
		MaxSpotlightIntensity = DefaultMaxSpotlightIntensity;
		InnerConeAngle = DefaultSimpleBeamLightInnerConeAngle;
		OuterConeAngle = DefaultSimpleBeamLightOuterConeAngle;
		bUseBeamEndLight = false;
		MaxBeamEndLightIntensity = DefaultMaxBeamEndLightIntensity;
		MaxEmissiveLightBulbLightIntensity = 1.f;
		NiagaraSystem = nullptr;

		NiagaraFloatParameters = TMap<FString, float>();
		NiagaraFloatParameters.Emplace("User.BeamWidth", DefaultSimpleBeamLightBeamWidth);
		NiagaraFloatParameters.Emplace("User.BeamLength", DefaultSimpleBeamLightBeamLength);
		NiagaraFloatParameters.Emplace("User.Lifetime", LightDuration);

		NiagaraColorParameters = TMap<FString, FLinearColor>();

		bAutoCalculateBeamLength = true;
	}
};
