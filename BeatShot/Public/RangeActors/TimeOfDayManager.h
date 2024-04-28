// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerSettingsInterface.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "TimeOfDayManager.generated.h"

class UBSGameUserSettings;
class ARectLight;
class ASpotLight;
class AVisualizerBase;
class UDirectionalLightComponent;
class ASkyLight;
class AStaticMeshActor;
class AVolumetricCloud;
class ADirectionalLight;
class UCurveFloat;
class AMoon;
class UMaterialInstanceDynamic;
class ASpawnAreaSpotLight;

/** Enum representing the different times of the day */
UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	Day UMETA(DisplayName="Day"),
	Night UMETA(DisplayName="Night"),
	DayToNight UMETA(DisplayName="DayToNight"),
	NightToDay UMETA(DisplayName="NightToDay"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETimeOfDay, ETimeOfDay::Day, ETimeOfDay::NightToDay);

DECLARE_DELEGATE_OneParam(FOnTimeOfDayTransitionCompleted, const ETimeOfDay NewTimeOfDay);

UCLASS()
class BEATSHOT_API ATimeOfDayManager : public AActor, public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

public:
	ATimeOfDayManager();

	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaTime) override;

	virtual void PostLoad() override;

	/** Changes TimeOfDay and plays TransitionTimeline */
	void BeginTransitionToNight();

	/** Changes TimeOfDay and plays TransitionTimeline */
	void BeginTransitionToDay();

	/** Changes TimeOfDay based on the value of TimeOfDay_Editor */
	UFUNCTION(CallInEditor)
	void UpdateTimeOfDay_Editor();

	UFUNCTION(CallInEditor)
	void BeginTransitionToNight_Editor();

	UFUNCTION(CallInEditor)
	void BeginTransitionToDay_Editor();

	/** Instantly changes the time of day */
	void SetTimeOfDay(const ETimeOfDay InTimeOfDay);

	/** Returns the current time of day */
	ETimeOfDay GetTimeOfDay() const { return TimeOfDay; }

	void SetSpotLightFrontEnabledState(const bool bEnable);

	FOnTimeOfDayTransitionCompleted OnTimeOfDayTransitionCompleted;

protected:
	/** Callback function to respond to NightMode change from WallMenu */
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings) override;

	/** Callback function to respond to NightMode change from WallMenu */
	void HandleGameUserSettingsChanged(const UBSGameUserSettings* InGameUserSettings);

	/** Calls RefreshMaterial function in SkySphere */
	UFUNCTION(BlueprintImplementableEvent)
	void RefreshSkySphereMaterial();

	/** Soft Reference to SkySphere in Range level */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lighting|References")
	TSoftObjectPtr<AActor> SkySphere;

	/** Soft Reference to Moon in Range level */
	UPROPERTY(EditInstanceOnly, Category = "Lighting|References")
	TSoftObjectPtr<AMoon> Moon;

	/** Soft Reference to Daylight in Range level */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lighting|References")
	TSoftObjectPtr<ADirectionalLight> DayDirectionalLight;

	/** Soft Reference to Skylight in Range level */
	UPROPERTY(EditInstanceOnly, Category = "Lighting|References")
	TSoftObjectPtr<ASkyLight> SkyLight;

	/** Reference to left roof mesh to move */
	UPROPERTY(EditInstanceOnly, Category = "Lighting|References")
	TSoftObjectPtr<AStaticMeshActor> LeftWindowCover;

	/** Reference to right roof mesh to move */
	UPROPERTY(EditInstanceOnly, Category = "Lighting|References")
	TSoftObjectPtr<AStaticMeshActor> RightWindowCover;

	UPROPERTY(EditInstanceOnly, Category = "Lighting|References")
	TSoftObjectPtr<ASpawnAreaSpotLight> SpawnAreaSpotLight;

	UPROPERTY(EditInstanceOnly, Category = "Lighting|References")
	TSoftObjectPtr<ARectLight> RectLight;

	UPROPERTY(EditDefaultsOnly, Category = "Lighting")
	UCurveFloat* TransitionCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Lighting")
	UCurveFloat* SkyMaterialCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Lighting")
	UCurveFloat* MoonlightCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Lighting")
	UCurveFloat* SkyLightCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Lighting")
	UCurveFloat* SecondaryLightCurve;

	UPROPERTY(EditAnywhere, Category = "Lighting")
	ETimeOfDay TimeOfDay_Editor;

	/** Distance along the x-axis the roofs covering the night time windows have to travel during the day to night
	 *  transition. This value is reversed when the cycle goes from night to day */
	UPROPERTY(EditAnywhere, Category = "Lighting")
	float DayToNightRoofXTravelDistance = -1000.f;

	/** Distance along the z-axis the roofs covering the night time windows have to travel during the day to night
	 *  transition. This value is reversed when the cycle goes from night to day */
	UPROPERTY(EditAnywhere, Category = "Lighting")
	float DayToNightRoofZTravelDistance = 20.f;

	UPROPERTY(EditAnywhere, Category = "Lighting")
	bool bUseRectLight = true;

	UPROPERTY(EditAnywhere, Category = "Lighting")
	bool bUseSpotlight = true;

	UPROPERTY(EditAnywhere, Category = "Lighting")
	float LowGISettingSkyLightIntensity = 1.f;

	/** Constant Light Intensity of the sun */
	UPROPERTY(EditAnywhere, Category = "Lighting|Day")
	float DayDirectionalLightIntensity = 10.f;

	UPROPERTY(EditAnywhere, Category = "Lighting|Day")
	float DaySkylightIntensity = 3.f;

	UPROPERTY(EditAnywhere, Category = "Lighting|Day")
	float DayRectLightIntensity = 0.f;

	UPROPERTY(EditAnywhere, Category = "Lighting|Day")
	float DaySpotlightIntensity = 0.f;

	/** Dynamic Light Intensity of the moon's directional light */
	UPROPERTY(EditAnywhere, Category = "Lighting|Night")
	float NightDirectionalLightIntensity = 1.f;

	UPROPERTY(EditAnywhere, Category = "Lighting|Night")
	float NightSkylightIntensity = 3.f;

	UPROPERTY(EditAnywhere, Category = "Lighting|Night")
	float NightRectLightIntensity = 160.f;

	UPROPERTY(EditAnywhere, Category = "Lighting|Night")
	float NightSpotlightIntensity = 2000.f;

	/** Reference to SkySphere dynamic material instance */
	UPROPERTY()
	TSoftObjectPtr<UMaterialInstanceDynamic> SkySphereMaterial;

	/** The current time of day */
	ETimeOfDay TimeOfDay;

	/** Timeline bound to Position Curve */
	FTimeline TransitionTimeline;

	/** Delegate that binds to TransitionTimeline's InterpFloat, using the TransitionCurve and TransitionTimeOfDay function */
	FOnTimelineFloat OnTransitionTimelineTick;

	/** Delegate that binds to TransitionTimeline's TimelineFinishedFunc, using the OnTimelineCompletedCallback function */
	FOnTimelineEvent OnTimelineCompleted;

	/** Changes TimeOfDay after TransitionTimeline completes */
	UFUNCTION()
	void OnTimelineCompletedCallback();

	/** Executes on every tick of TransitionTimeline, reads from TransitionCurve */
	UFUNCTION()
	void TransitionTimeOfDay(const float Value);

	FVector DaytimeLeftRoofLocation = {2980, 0, 3371};
	FVector DaytimeRightRoofLocation = {2020, 0, 3371};
	FVector NighttimeLeftRoofLocation;
	FVector NighttimeRightRoofLocation;

	float LastLerpRotation;

	bool bUsingLowGISettings = false;
};
