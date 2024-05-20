// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSConstants.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "SimpleBeamLight.generated.h"

class USpotLightComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UCurveLinearColor;
class UCurveVector;
class USceneComponent;
class UMaterialInstanceDynamic;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBeamLightLifetimeCompleted, const int32 OutIndex);

using namespace Constants;

USTRUCT(BlueprintType, Category = "Simple Beam Light Config")
struct FSimpleBeamLightConfig
{
	GENERATED_BODY()

	/** The length of time this light will be on for. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | General",
		meta=(DisplayPriority=0))
	float LightDuration = 1.f;

	/** The base color to apply to Spotlight, BeamEndLight, and SimpleBeamComponent. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | General",
		meta=(DisplayPriority=100))
	FLinearColor LightColor;

	/** Max intensity of the EmissiveLightBulb. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | EmissiveLightBulb",
		meta=(DisplayName="Max Light Bulb Intensity", DisplayPriority=200))
	float MaxEmissiveLightBulbLightIntensity;

	/** whether this light will need to use the LightPositionTimeline and LineTracing at runtime.
	 *  If true, LightMovementCurve must have a curve. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | Movement",
		meta=(DisplayName="Moving Light", DisplayPriority=300))
	bool bIsMovingLight = false;

	/** Curve that defines the position of the end of the beam over time. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | Movement",
		meta=(DisplayPriority=400, EditCondition="bIsMovingLight", EditConditionHides))
	UCurveVector* LightMovementCurve;

	/** whether to activate the Spotlight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | Spotlight",
		meta=(DisplayPriority=500))
	bool bUseSpotlight = false;

	/** Max intensity of the Spotlight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | Spotlight",
		meta=(DisplayPriority=600, EditCondition="bUseSpotlight", EditConditionHides))
	float MaxSpotlightIntensity = DefaultMaxSpotlightIntensity;

	/** Inner cone angle for the Spotlight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | Spotlight",
		meta=(DisplayName="Spotlight Inner Cone Angle", DisplayPriority=700, EditCondition="bUseSpotlight",
			EditConditionHides))
	float InnerConeAngle = DefaultSimpleBeamLightInnerConeAngle;

	/** Outer cone angle for the Spotlight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | Spotlight",
		meta=(DisplayName="Spotlight Outer Cone Angle", DisplayPriority=800, EditCondition="bUseSpotlight",
			EditConditionHides))
	float OuterConeAngle = DefaultSimpleBeamLightOuterConeAngle;

	/** whether to activate the BeamEndLight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | BeamEndLight",
		meta=(DisplayPriority=900))
	bool bUseBeamEndLight = false;

	/** Max intensity of the MaxBeamEndLight. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | BeamEndLight",
		meta=(DisplayPriority=1000, EditCondition="bUseBeamEndLight", EditConditionHides))
	float MaxBeamEndLightIntensity = DefaultMaxBeamEndLightIntensity;

	/** The Niagara System used for this light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | Niagara Beam",
		meta=(DisplayPriority=1100))
	UNiagaraSystem* NiagaraSystem;

	/** Auto calculates the beam length by doing a line trace on initialization. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | Niagara Beam",
		meta=(DisplayPriority=1200, EditCondition="NiagaraSystem!=nullptr", EditConditionHides))
	bool bAutoCalculateBeamLength = true;

	/** The Niagara System float parameters to used for this light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | Niagara Beam",
		meta=(ForceInlineRow), meta=(DisplayPriority=1300, EditCondition="NiagaraSystem!=nullptr", EditConditionHides))
	TMap<FString, float> NiagaraFloatParameters;

	/** The Niagara System Color parameters to used for this light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simple Beam Light Config | Niagara Beam",
		meta=(ForceInlineRow), meta=(DisplayPriority=1400, EditCondition="NiagaraSystem!=nullptr", EditConditionHides))
	TMap<FString, FLinearColor> NiagaraColorParameters;

	/** The index of this instance inside an array of this type. */
	int32 Index = INDEX_NONE;

	FSimpleBeamLightConfig()
	{
		LightDuration = 1.f;
		Index = INDEX_NONE;
		bIsMovingLight = false;
		LightMovementCurve = nullptr;
		LightColor = FLinearColor::White;
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
		NiagaraColorParameters.Emplace("User.BeamColor", LightColor);

		bAutoCalculateBeamLength = true;
	}
};

/** Visualizer light that takes the form of a spotlight and has a Niagara System that represents a light beam. */
UCLASS()
class BEATSHOT_API ASimpleBeamLight : public AActor
{
	GENERATED_BODY()

public:
	ASimpleBeamLight();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SimpleBeamLight | Components")
	UStaticMeshComponent* SpotlightBase;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SimpleBeamLight | Components")
	UStaticMeshComponent* SpotlightLimb;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SimpleBeamLight | Components")
	UStaticMeshComponent* SpotlightHead;

	/** Niagara component that Niagara System SimpleBeam resides in. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SimpleBeamLight | Components")
	UNiagaraComponent* SimpleBeamComponent;

	/** The actual spotlight light that appears at the end of the SimpleBeam. Its position is near the base,
	 *  unlike BeamEndLight. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SimpleBeamLight | Components")
	USpotLightComponent* Spotlight;

	/** Indirect lighting applied to the area at the end of the SimpleBeam. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SimpleBeamLight | Components")
	USpotLightComponent* BeamEndLight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SimpleBeamLight | Components")
	USceneComponent* LightPositionComponent;

	/** The base config for this light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SimpleBeamLight | Config", meta=(DisplayPriority=-10000))
	FSimpleBeamLightConfig SimpleBeamLightConfig;

	/** The light inside the SpotlightHead. */
	UPROPERTY()
	UMaterialInstanceDynamic* EmissiveLightBulb;

public:
	/** Initialize the light components when spawning. */
	void InitSimpleBeamLight(const FSimpleBeamLightConfig& InConfig);

	/** Activate the light components. */
	void ActivateLightComponents();

	/** Deactivate the light components. */
	void DeactivateLightComponents();

	/** Returns the config for this simple beam light. */
	FSimpleBeamLightConfig GetSimpleBeamLightConfig() const { return SimpleBeamLightConfig; }

	/** Broadcast when the niagara component has completed. */
	FOnBeamLightLifetimeCompleted OnBeamLightLifetimeCompleted;

	UFUNCTION(CallInEditor)
	void UpdateBeamEndLightLocation();

private:
	/** Calls DeactivateLightComponents. */
	UFUNCTION()
	void OnNiagaraBeamFinished(UNiagaraComponent* NiagaraComponent);

	/** Traces a line forward from SpotlightHead. */
	UFUNCTION()
	void LineTraceFromSpotlightHead(const FVector& EndLocation, FHitResult& OutHitResult) const;

	/** Sets the position and rotation of LightPositionComponent, which is basically BeamEndLight. */
	void UpdateBeamEndLightTransform(const FHitResult& HitResult) const;

	/** Sets the SpotlightHead and SpotlightLimb rotations to correctly orient themselves with the HitLocation. */
	void UpdateSpotlightHeadAndLimbRotation(const FVector& HitLocation, const FVector& SpotlightHeadLocation) const;

	/** Sets the spotlight intensity and attenuation radius. */
	void UpdateSpotlightIntensityAndAttRadius(const float InPlaybackPosition, const float HitResultDistance) const;

	/** Sets the EmissiveLightBulb intensity. */
	void UpdateEmissiveLightBulbIntensity(const float Intensity) const;

	/** Sets the BeamEndLight intensity. */
	void UpdateBeamEndLightIntensity(const float InPlaybackPosition) const;

	/** Sets the NiagaraSystem's User.BeamLength parameter. */
	void UpdateNiagaraBeamLength(const float HitResultDistance);

	/** Rotates the SpotlightLimb and SpotlightHead, sets position of LightPositionComponent. */
	UFUNCTION()
	void LightMovementCurveCallback(const FVector& Position);

	/** The timeline to link to the LightMovementCurve. */
	FTimeline LightPositionTimeline;

	/** Delegate that binds the LightPositionTimeline to LightMovementCurveCallback(). */
	FOnTimelineVector TimelineVectorDelegate;

	float GimbalRotation;
	float SpotlightHeadRotation;
};
