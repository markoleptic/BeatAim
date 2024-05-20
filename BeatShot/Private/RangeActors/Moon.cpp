// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "RangeActors/Moon.h"
#include "BSConstants.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SphereComponent.h"

AMoon::AMoon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;
	SphereComponent->SetMobility(EComponentMobility::Movable);
	SphereComponent->SetSphereRadius(Constants::MoonOrbitRadius);

	MoonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoonMesh"));
	MoonMesh->SetupAttachment(SphereComponent);
	MoonMesh->SetRelativeLocation(Constants::MoonMeshOffset);
	MoonMesh->SetWorldScale3D(Constants::MoonMeshScale);
	MoonMesh->SetMobility(EComponentMobility::Movable);

	MoonGlowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoonGlowMesh"));
	MoonGlowMesh->SetupAttachment(MoonMesh);
	MoonGlowMesh->SetWorldScale3D(Constants::MoonGlowMeshScale);
	MoonGlowMesh->SetMobility(EComponentMobility::Movable);

	MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
	MoonLight->SetupAttachment(MoonMesh);
	MoonLight->SetWorldScale3D(Constants::MoonLightScale);
	MoonLight->SetMobility(EComponentMobility::Movable);
}

void AMoon::BeginPlay()
{
	Super::BeginPlay();
}

void AMoon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
