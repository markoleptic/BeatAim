// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Visualizers/SimpleBeamLight.h"
#include "BSConstants.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/TimelineComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

using namespace Constants;

ASimpleBeamLight::ASimpleBeamLight()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpotlightBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpotlightBase"));
	RootComponent = SpotlightBase;

	SpotlightLimb = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpotlightLimb"));
	SpotlightLimb->SetupAttachment(SpotlightBase);
	SpotlightLimb->SetRelativeLocation(DefaultSpotlightLimbOffset);

	SpotlightHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpotlightHead"));
	SpotlightHead->SetupAttachment(SpotlightLimb);
	SpotlightHead->SetRelativeLocation(DefaultSpotlightHeadOffset);

	Spotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spotlight"));
	Spotlight->SetupAttachment(SpotlightHead);
	Spotlight->SetRelativeLocation(DefaultSpotlightOffset);
	Spotlight->SetAutoActivate(false);
	Spotlight->SetInnerConeAngle(SimpleBeamLightConfig.InnerConeAngle);
	Spotlight->SetOuterConeAngle(SimpleBeamLightConfig.OuterConeAngle);

	LightPositionComponent = CreateDefaultSubobject<USceneComponent>("LightPosition");
	LightPositionComponent->SetupAttachment(SpotlightBase);

	BeamEndLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("BeamEndLight"));
	BeamEndLight->SetupAttachment(LightPositionComponent);
	BeamEndLight->SetAutoActivate(false);

	SimpleBeamComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SimpleBeam"));
	SimpleBeamComponent->SetupAttachment(SpotlightHead);
	SimpleBeamComponent->SetAutoActivate(false);
}

void ASimpleBeamLight::BeginPlay()
{
	Super::BeginPlay();
}

void ASimpleBeamLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!SimpleBeamLightConfig.bIsMovingLight)
	{
		return;
	}
	if (LightPositionTimeline.IsPlaying() || LightPositionTimeline.IsReversing())
	{
		LightPositionTimeline.TickTimeline(DeltaTime);
	}
}

void ASimpleBeamLight::InitSimpleBeamLight(const FSimpleBeamLightConfig& InConfig)
{
	SimpleBeamLightConfig = InConfig;

	if (SimpleBeamComponent)
	{
		if (SimpleBeamLightConfig.NiagaraSystem)
		{
			SimpleBeamComponent->SetAsset(SimpleBeamLightConfig.NiagaraSystem);
		}
		// SimpleBeamComponent->InitializeSystem();
		SimpleBeamLightConfig.NiagaraColorParameters.FindOrAdd("User.BeamColor") = SimpleBeamLightConfig.LightColor;

		for (TTuple<FString, float>& Elem : SimpleBeamLightConfig.NiagaraFloatParameters)
		{
			SimpleBeamComponent->SetFloatParameter(FName(Elem.Key), Elem.Value);
		}
		for (TTuple<FString, FLinearColor>& Elem : SimpleBeamLightConfig.NiagaraColorParameters)
		{
			SimpleBeamComponent->SetColorParameter(FName(Elem.Key), Elem.Value);
		}
		SimpleBeamComponent->OnSystemFinished.AddUniqueDynamic(this, &ASimpleBeamLight::OnNiagaraBeamFinished);
	}

	Spotlight->SetIntensity(0.f);
	BeamEndLight->SetIntensity(0.f);
	Spotlight->SetLightColor(SimpleBeamLightConfig.LightColor);
	BeamEndLight->SetLightColor(SimpleBeamLightConfig.LightColor);

	EmissiveLightBulb = UMaterialInstanceDynamic::Create(SpotlightHead->GetMaterial(1), this);
	SpotlightHead->SetMaterial(1, EmissiveLightBulb);
	if (EmissiveLightBulb)
	{
		EmissiveLightBulb->SetScalarParameterValue(TEXT("Intensity"), 0.f);
		EmissiveLightBulb->SetVectorParameterValue(TEXT("Color"), SimpleBeamLightConfig.LightColor);
	}

	FHitResult Hit;
	LineTraceFromSpotlightHead(SpotlightHead->GetForwardVector(), Hit);
	LightPositionComponent->SetWorldLocation(Hit.Location);
	UpdateSpotlightHeadAndLimbRotation(Hit.Location, SpotlightHead->GetComponentLocation());
	UpdateBeamEndLightTransform(Hit);

	if (SimpleBeamLightConfig.bAutoCalculateBeamLength)
	{
		UpdateNiagaraBeamLength(Hit.Distance);
	}

	if (SimpleBeamLightConfig.bIsMovingLight)
	{
		TimelineVectorDelegate.BindUFunction(this, FName("LightMovementCurveCallback"));
		LightPositionTimeline.AddInterpVector(SimpleBeamLightConfig.LightMovementCurve, TimelineVectorDelegate);
	}
}

void ASimpleBeamLight::ActivateLightComponents()
{
	if (LightPositionTimeline.IsPlaying() || LightPositionTimeline.IsReversing())
	{
		return;
	}

	if (SimpleBeamComponent)
	{
		SimpleBeamComponent->Activate();
	}
	if (SimpleBeamLightConfig.bUseSpotlight)
	{
		Spotlight->Activate();
	}
	if (SimpleBeamLightConfig.bUseBeamEndLight)
	{
		BeamEndLight->Activate();
	}

	if (SimpleBeamLightConfig.bIsMovingLight)
	{
		LightPositionTimeline.SetPlayRate(1.f / SimpleBeamLightConfig.LightDuration);
		if (SimpleBeamLightConfig.Index % 2 == 0)
		{
			LightPositionTimeline.ReverseFromEnd();
		}
		else
		{
			LightPositionTimeline.PlayFromStart();
		}
	}
}

void ASimpleBeamLight::DeactivateLightComponents()
{
	if (LightPositionTimeline.IsPlaying() || LightPositionTimeline.IsReversing())
	{
		LightPositionTimeline.Stop();
	}

	if (EmissiveLightBulb)
	{
		EmissiveLightBulb->SetScalarParameterValue(TEXT("Intensity"), 0.f);
	}

	if (SimpleBeamLightConfig.bIsMovingLight)
	{
		LightPositionComponent->SetRelativeLocation(FVector(0));
	}

	if (SpotlightLimb)
	{
		SpotlightLimb->SetRelativeRotation(FRotator(0));
		SpotlightHead->SetRelativeRotation(FRotator(0));
	}

	if (SimpleBeamComponent)
	{
		SimpleBeamComponent->Deactivate();
	}
	if (Spotlight->IsActive())
	{
		Spotlight->SetIntensity(0.f);
		Spotlight->Deactivate();
	}
	if (BeamEndLight->IsActive())
	{
		BeamEndLight->SetIntensity(0.f);
		BeamEndLight->Deactivate();
	}
}

void ASimpleBeamLight::OnNiagaraBeamFinished(UNiagaraComponent* NiagaraComponent)
{
	DeactivateLightComponents();
	OnBeamLightLifetimeCompleted.Broadcast(SimpleBeamLightConfig.Index);
}

void ASimpleBeamLight::LineTraceFromSpotlightHead(const FVector& EndLocation, FHitResult& OutHitResult) const
{
	GetWorld()->LineTraceSingleByChannel(OutHitResult, SpotlightHead->GetComponentLocation(),
		EndLocation * FVector(999999999), ECC_Camera, FCollisionQueryParams::DefaultQueryParam);
}

void ASimpleBeamLight::UpdateBeamEndLightTransform(const FHitResult& HitResult) const
{
	BeamEndLight->SetWorldLocation(HitResult.Location + SpotlightHead->GetComponentRotation().Vector() * 5);
	const FVector SpotlightHeadFV = SpotlightHead->GetForwardVector() * FVector(999999999);
	BeamEndLight->SetWorldRotation(
		(SpotlightHeadFV - HitResult.Normal.Dot(SpotlightHeadFV) * 2 * HitResult.Normal).Rotation());
}

void ASimpleBeamLight::UpdateSpotlightHeadAndLimbRotation(const FVector& HitLocation,
	const FVector& SpotlightHeadLocation) const
{
	//SpotlightLimb->SetRelativeRotation(FRotator(0, 0, 0));
	SpotlightHead->SetWorldRotation((HitLocation - SpotlightHeadLocation).Rotation());
	//SpotlightHead->SetRelativeRotation((HitLocation - SpotlightHeadLocation).Rotation());
}

void ASimpleBeamLight::UpdateSpotlightIntensityAndAttRadius(const float InPlaybackPosition,
	const float HitResultDistance) const
{
	Spotlight->SetAttenuationRadius(HitResultDistance + 2000);
	Spotlight->SetIntensity(InPlaybackPosition * SimpleBeamLightConfig.MaxSpotlightIntensity);
}

void ASimpleBeamLight::UpdateEmissiveLightBulbIntensity(const float Intensity) const
{
	EmissiveLightBulb->SetScalarParameterValue(TEXT("Intensity"),
		Intensity * SimpleBeamLightConfig.MaxEmissiveLightBulbLightIntensity);
}

void ASimpleBeamLight::UpdateBeamEndLightIntensity(const float InPlaybackPosition) const
{
	BeamEndLight->SetIntensity(InPlaybackPosition * SimpleBeamLightConfig.MaxBeamEndLightIntensity);
}

void ASimpleBeamLight::UpdateNiagaraBeamLength(const float HitResultDistance)
{
	//SimpleBeamLightConfig.NiagaraFloatParameters.FindOrAdd("User.BeamLength") = HitResultDistance + 100.f;
	SimpleBeamComponent->SetFloatParameter(TEXT("User.BeamLength"), HitResultDistance + 100.f);
}

void ASimpleBeamLight::LightMovementCurveCallback(const FVector& Position)
{
	FHitResult Hit;
	float PlaybackPosition;

	if (LightPositionTimeline.IsReversing())
	{
		PlaybackPosition = LightPositionTimeline.GetPlaybackPosition();
	}
	else
	{
		PlaybackPosition = 1 - LightPositionTimeline.GetPlaybackPosition();
	}

	LineTraceFromSpotlightHead(Position, Hit);
	LightPositionComponent->SetWorldLocation(Hit.Location);
	UpdateSpotlightHeadAndLimbRotation(Hit.Location, SpotlightHead->GetComponentLocation());

	if (SimpleBeamLightConfig.bAutoCalculateBeamLength)
	{
		UpdateNiagaraBeamLength(Hit.Distance);
	}
	if (SimpleBeamLightConfig.bUseBeamEndLight)
	{
		UpdateBeamEndLightTransform(Hit);
		UpdateBeamEndLightIntensity(PlaybackPosition);
	}
	if (SimpleBeamLightConfig.bUseSpotlight)
	{
		UpdateSpotlightIntensityAndAttRadius(PlaybackPosition, Hit.Distance);
	}

	UpdateEmissiveLightBulbIntensity(PlaybackPosition);
}

void ASimpleBeamLight::UpdateBeamEndLightLocation()
{
	FHitResult Hit;
	LineTraceFromSpotlightHead(SpotlightHead->GetForwardVector(), Hit);
	LightPositionComponent->SetWorldLocation(Hit.Location);
	UpdateSpotlightHeadAndLimbRotation(Hit.Location, SpotlightHead->GetComponentLocation());
	UpdateBeamEndLightTransform(Hit);
}
