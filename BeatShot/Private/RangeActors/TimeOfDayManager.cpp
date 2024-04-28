// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "RangeActors/TimeOfDayManager.h"

#include "BSGameInstance.h"
#include "BSGameMode.h"
#include "BSGameUserSettings.h"
#include "Components/StaticMeshComponent.h"
#include "RangeActors/Moon.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/RectLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "GlobalConstants.h"
#include "Components/SpotLightComponent.h"
#include "RangeActors/SpawnAreaSpotLight.h"

using namespace Constants;

ATimeOfDayManager::ATimeOfDayManager()
{
	PrimaryActorTick.bCanEverTick = true;
	TransitionCurve = nullptr;
	SkyMaterialCurve = nullptr;
	MoonlightCurve = nullptr;
	SkyLightCurve = nullptr;
	SecondaryLightCurve = nullptr;
	TimeOfDay_Editor = ETimeOfDay::Day;
	TimeOfDay = ETimeOfDay::Day;
	LastLerpRotation = 0;
	NighttimeLeftRoofLocation = DaytimeLeftRoofLocation + FVector(DayToNightRoofXTravelDistance, 0,
		DayToNightRoofZTravelDistance);
	NighttimeRightRoofLocation = DaytimeRightRoofLocation + FVector(DayToNightRoofXTravelDistance, 0,
		DayToNightRoofZTravelDistance);
}

void ATimeOfDayManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	OnTransitionTimelineTick.BindUFunction(this, FName("TransitionTimeOfDay"));
	OnTimelineCompleted.BindUFunction(this, FName("OnTimelineCompletedCallback"));

	if (TransitionCurve)
	{
		TransitionTimeline.AddInterpFloat(TransitionCurve, OnTransitionTimelineTick);
		TransitionTimeline.SetTimelineFinishedFunc(OnTimelineCompleted);
	}

	if (SkySphere)
	{
		SkySphereMaterial = Cast<UMaterialInstanceDynamic>(
			Cast<UStaticMeshComponent>(SkySphere->GetComponentByClass(UStaticMeshComponent::StaticClass()))->
			GetMaterial(0));
	}

	if (DayDirectionalLight && DayDirectionalLight->GetLightComponent())
	{
		DayDirectionalLight->GetLightComponent()->SetIntensity(DayDirectionalLightIntensity);
	}

	const auto GameUserSettings = UBSGameUserSettings::Get();
	HandleGameUserSettingsChanged(GameUserSettings);
	GameUserSettings->OnSettingsChanged.AddUObject(this, &ATimeOfDayManager::HandleGameUserSettingsChanged);

	if (UBSGameInstance* GI = Cast<UBSGameInstance>(GetGameInstance()))
	{
		GI->RegisterPlayerSettingsSubscriber<ATimeOfDayManager, FPlayerSettings_Game>(this,
			&ATimeOfDayManager::OnPlayerSettingsChanged);
		GI->SetTimeOfDayManager(this);

		// Initialize the time of day
		const FPlayerSettings Settings = LoadPlayerSettings();
		if (Settings.User.bNightModeUnlocked && Settings.Game.bNightModeSelected)
		{
			SetTimeOfDay(ETimeOfDay::Night);
		}
		else
		{
			SetTimeOfDay(ETimeOfDay::Day);
		}
	}
}

void ATimeOfDayManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TransitionTimeline.IsPlaying())
	{
		TransitionTimeline.TickTimeline(DeltaTime);
	}
}

void ATimeOfDayManager::PostLoad()
{
	Super::PostLoad();
}

void ATimeOfDayManager::BeginTransitionToNight()
{
	if (GetTimeOfDay() == ETimeOfDay::DayToNight || GetTimeOfDay() == ETimeOfDay::NightToDay)
	{
		return;
	}
	LastLerpRotation = 0;
	TimeOfDay = ETimeOfDay::DayToNight;
	TransitionTimeline.SetPlayRate(1.f / DayNightCycleSpeed);
	TransitionTimeline.PlayFromStart();
}

void ATimeOfDayManager::BeginTransitionToDay()
{
	if (GetTimeOfDay() == ETimeOfDay::DayToNight || GetTimeOfDay() == ETimeOfDay::NightToDay)
	{
		return;
	}

	if (!SkyLight || !Moon || !DayDirectionalLight || !SkySphereMaterial || !RectLight || !SpawnAreaSpotLight || !
		LeftWindowCover || !RightWindowCover)
	{
		UE_LOG(LogTemp, Warning, TEXT("One of the Soft References for TimeOfDayManager is invalid."));
		return;
	}

	LastLerpRotation = 0;
	TimeOfDay = ETimeOfDay::NightToDay;
	TransitionTimeline.SetPlayRate(1.f / DayNightCycleSpeed);
	TransitionTimeline.PlayFromStart();
}

void ATimeOfDayManager::UpdateTimeOfDay_Editor()
{
	UE_LOG(LogTemp, Warning, TEXT("UpdateTimeOfDay_Editor"));
	if (SkySphere)
	{
		if (!SkySphereMaterial)
		{
			SkySphereMaterial = Cast<UMaterialInstanceDynamic>(
				Cast<UStaticMeshComponent>(SkySphere.Get()->GetComponentByClass(UStaticMeshComponent::StaticClass()))->
				GetMaterial(0));
		}
		if (SkySphereMaterial)
		{
			SetTimeOfDay(TimeOfDay_Editor);
		}
	}
}

void ATimeOfDayManager::BeginTransitionToNight_Editor()
{
	if (SkySphere)
	{
		if (!SkySphereMaterial)
		{
			SkySphereMaterial = Cast<UMaterialInstanceDynamic>(
				Cast<UStaticMeshComponent>(SkySphere.Get()->GetComponentByClass(UStaticMeshComponent::StaticClass()))->
				GetMaterial(0));
		}
		if (SkySphereMaterial)
		{
			BeginTransitionToNight();
		}
	}
}

void ATimeOfDayManager::BeginTransitionToDay_Editor()
{
	if (SkySphere)
	{
		if (!SkySphereMaterial)
		{
			SkySphereMaterial = Cast<UMaterialInstanceDynamic>(
				Cast<UStaticMeshComponent>(SkySphere.Get()->GetComponentByClass(UStaticMeshComponent::StaticClass()))->
				GetMaterial(0));
		}
		if (SkySphereMaterial)
		{
			BeginTransitionToDay();
		}
	}
}

void ATimeOfDayManager::SetTimeOfDay(const ETimeOfDay InTimeOfDay)
{
	TimeOfDay = InTimeOfDay;

	if (!SkyLight || !Moon || !DayDirectionalLight || !SkySphereMaterial || !RectLight || !SpawnAreaSpotLight || !
		LeftWindowCover || !RightWindowCover)
	{
		return;
	}

	// Always the same intensity
	DayDirectionalLight->GetLightComponent()->SetIntensity(DayDirectionalLightIntensity);

	if (InTimeOfDay == ETimeOfDay::Night)
	{
		SkySphereMaterial->SetScalarParameterValue("NightAlpha", 1);

		// Need to add world rotation instead of setting so things don't get weird
		if (FMath::IsNearlyEqual(
			static_cast<float>(DayDirectionalLight->GetLightComponent()->GetComponentRotation().GetNormalized().Roll),
			0.f, 0.1f))
		{
			DayDirectionalLight->GetLightComponent()->AddWorldRotation(FRotator(0, 0, 180));
		}

		Moon->MoonMaterialInstance->SetScalarParameterValue("Opacity", 1);
		Moon->MoonGlowMaterialInstance->SetScalarParameterValue("Opacity", 1);
		Moon->MoonLight->SetIntensity(NightDirectionalLightIntensity);
		LeftWindowCover->GetStaticMeshComponent()->SetRelativeLocation(NighttimeLeftRoofLocation);
		RightWindowCover->GetStaticMeshComponent()->SetRelativeLocation(NighttimeRightRoofLocation);
		SkyLight->GetLightComponent()->SetIntensity(bUsingLowGISettings
			? LowGISettingSkyLightIntensity
			: NightSkylightIntensity);
		RectLight->GetLightComponent()->SetIntensity(bUseRectLight ? NightRectLightIntensity : 0.f);
		SpawnAreaSpotLight->GetLightComponent()->SetIntensity(bUseSpotlight ? NightSpotlightIntensity : 0.f);
	}
	else if (InTimeOfDay == ETimeOfDay::Day)
	{
		SkySphereMaterial->SetScalarParameterValue("NightAlpha", 0);

		// Need to add world rotation instead of setting so things don't get weird
		if (FMath::IsNearlyEqual(
			static_cast<float>(DayDirectionalLight->GetLightComponent()->GetComponentRotation().GetNormalized().Roll),
			180.f, 0.1f))
		{
			DayDirectionalLight->GetLightComponent()->AddWorldRotation(FRotator(0, 0, 180));
		}

		Moon->MoonMaterialInstance->SetScalarParameterValue("Opacity", 0);
		Moon->MoonGlowMaterialInstance->SetScalarParameterValue("Opacity", 0);
		Moon->MoonLight->SetIntensity(0);
		LeftWindowCover->GetStaticMeshComponent()->SetRelativeLocation(DaytimeLeftRoofLocation);
		RightWindowCover->GetStaticMeshComponent()->SetRelativeLocation(DaytimeRightRoofLocation);
		SkyLight->GetLightComponent()->SetIntensity(bUsingLowGISettings
			? LowGISettingSkyLightIntensity
			: DaySkylightIntensity);
		RectLight->GetLightComponent()->SetIntensity(bUseRectLight ? DayRectLightIntensity : 0.f);
		SpawnAreaSpotLight->GetLightComponent()->SetIntensity(bUseSpotlight ? DaySpotlightIntensity : 0.f);
	}

	RefreshSkySphereMaterial();
}

void ATimeOfDayManager::SetSpotLightFrontEnabledState(const bool bEnable)
{
	if (bEnable)
	{
		if (TimeOfDay == ETimeOfDay::Day)
		{
			SpawnAreaSpotLight->GetLightComponent()->SetIntensity(DaySpotlightIntensity);
		}
		if (TimeOfDay == ETimeOfDay::Night)
		{
			SpawnAreaSpotLight->GetLightComponent()->SetIntensity(NightSpotlightIntensity);
		}
	}
	else
	{
		SpawnAreaSpotLight->GetLightComponent()->SetIntensity(0.f);
	}
}

void ATimeOfDayManager::OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings)
{
	if (LoadPlayerSettings().User.bNightModeUnlocked)
	{
		if (GameSettings.bNightModeSelected)
		{
			if (GetTimeOfDay() == ETimeOfDay::Day)
			{
				BeginTransitionToNight();
			}
		}
		else
		{
			if (GetTimeOfDay() == ETimeOfDay::Night)
			{
				BeginTransitionToDay();
			}
		}
	}
}

void ATimeOfDayManager::HandleGameUserSettingsChanged(const UBSGameUserSettings* InGameUserSettings)
{
	// Adjust SkyLight brightness if using Low or Medium Global Illumination Settings
	if (InGameUserSettings->GetGlobalIlluminationQuality() < 2)
	{
		bUsingLowGISettings = true;
		if (SkyLight)
		{
			SkyLight->GetLightComponent()->SetIntensity(LowGISettingSkyLightIntensity);
		}
	}
	else
	{
		bUsingLowGISettings = false;
		if (SkyLight)
		{
			if (TimeOfDay == ETimeOfDay::Day)
			{
				SkyLight->GetLightComponent()->SetIntensity(DaySkylightIntensity);
			}
			else if (TimeOfDay == ETimeOfDay::Night)
			{
				SkyLight->GetLightComponent()->SetIntensity(NightSkylightIntensity);
			}
		}
	}
}

void ATimeOfDayManager::OnTimelineCompletedCallback()
{
	TimeOfDay = (TimeOfDay == ETimeOfDay::DayToNight) ? ETimeOfDay::Night : ETimeOfDay::Day;;
	if (OnTimeOfDayTransitionCompleted.IsBound())
	{
		OnTimeOfDayTransitionCompleted.Execute(TimeOfDay);
	}
}

void ATimeOfDayManager::TransitionTimeOfDay(const float Value)
{
	float PositionAlpha;
	float MoonlightIntensity;
	float SkylightIntensity;
	float NightAlpha;
	float SpotlightIntensity;
	float RectLightIntensity;

	if (TimeOfDay == ETimeOfDay::DayToNight)
	{
		PositionAlpha = UKismetMathLibrary::Lerp(0, 1, Value);
		NightAlpha = UKismetMathLibrary::Lerp(0, 1, Value);
		MoonlightIntensity = UKismetMathLibrary::Lerp(0, NightDirectionalLightIntensity,
			MoonlightCurve->GetFloatValue(Value));
		SkylightIntensity = bUsingLowGISettings
			? LowGISettingSkyLightIntensity
			: UKismetMathLibrary::Lerp(DaySkylightIntensity, NightSkylightIntensity,
				SkyLightCurve->GetFloatValue(Value));
		SpotlightIntensity = bUseSpotlight
			? UKismetMathLibrary::Lerp(DaySpotlightIntensity, NightSpotlightIntensity,
				SecondaryLightCurve->GetFloatValue(Value))
			: 0.f;
		RectLightIntensity = bUseRectLight
			? UKismetMathLibrary::Lerp(DayRectLightIntensity, NightRectLightIntensity,
				SecondaryLightCurve->GetFloatValue(Value))
			: 0.f;
	}
	else
	{
		PositionAlpha = UKismetMathLibrary::Lerp(1, 0, Value);
		NightAlpha = UKismetMathLibrary::Lerp(1, 0, Value);
		MoonlightIntensity = UKismetMathLibrary::Lerp(NightDirectionalLightIntensity, 0,
			MoonlightCurve->GetFloatValue(Value));
		SkylightIntensity = bUsingLowGISettings
			? LowGISettingSkyLightIntensity
			: UKismetMathLibrary::Lerp(NightSkylightIntensity, DaySkylightIntensity,
				SkyLightCurve->GetFloatValue(Value));
		SpotlightIntensity = bUseSpotlight
			? UKismetMathLibrary::Lerp(NightSpotlightIntensity, DaySpotlightIntensity,
				SecondaryLightCurve->GetFloatValue(Value))
			: 0.f;
		RectLightIntensity = bUseRectLight
			? UKismetMathLibrary::Lerp(NightRectLightIntensity, DayRectLightIntensity,
				SecondaryLightCurve->GetFloatValue(Value))
			: 0.f;
	}

	if (PositionAlpha <= 0.2f)
	{
		const float CurrentRoofZTravelDistance = UKismetMathLibrary::Lerp(0, DayToNightRoofZTravelDistance,
			PositionAlpha / 0.2f);
		LeftWindowCover->GetStaticMeshComponent()->SetRelativeLocation(
			DaytimeLeftRoofLocation + FVector(0, 0, CurrentRoofZTravelDistance));
		RightWindowCover->GetStaticMeshComponent()->SetRelativeLocation(
			DaytimeRightRoofLocation + FVector(0, 0, CurrentRoofZTravelDistance));
	}
	else
	{
		const float CurrentRoofXTravelDistance = UKismetMathLibrary::Lerp(0, DayToNightRoofXTravelDistance,
			(PositionAlpha - 0.2f) / (0.8f));
		LeftWindowCover->GetStaticMeshComponent()->SetRelativeLocation(
			DaytimeLeftRoofLocation + FVector(CurrentRoofXTravelDistance, 0, DayToNightRoofZTravelDistance));
		RightWindowCover->GetStaticMeshComponent()->SetRelativeLocation(
			DaytimeRightRoofLocation + FVector(CurrentRoofXTravelDistance, 0, DayToNightRoofZTravelDistance));
	}

	const float CurrentLerpRotation = UKismetMathLibrary::Lerp(0, 180, Value);
	DayDirectionalLight->GetLightComponent()->AddWorldRotation(FRotator(0, 0, CurrentLerpRotation - LastLerpRotation));
	LastLerpRotation = CurrentLerpRotation;

	Moon->MoonLight->SetIntensity(MoonlightIntensity);
	Moon->MoonMaterialInstance->SetScalarParameterValue("Opacity", PositionAlpha);
	Moon->MoonGlowMaterialInstance->SetScalarParameterValue("Opacity", PositionAlpha);
	SkyLight->GetLightComponent()->SetIntensity(SkylightIntensity);
	SkySphereMaterial->SetScalarParameterValue("NightAlpha", NightAlpha);
	SpawnAreaSpotLight->GetLightComponent()->SetIntensity(SpotlightIntensity);
	RectLight->GetLightComponent()->SetIntensity(RectLightIntensity);

	RefreshSkySphereMaterial();
}
