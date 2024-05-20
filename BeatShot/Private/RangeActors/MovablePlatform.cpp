// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "RangeActors/MovablePlatform.h"
#include "Character/BSCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/BSPlayerController.h"
#include "RangeActors/WallMenu.h"

AMovablePlatform::AMovablePlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	Floor = CreateDefaultSubobject<UStaticMeshComponent>("Floor");
	RootComponent = Floor;

	ControlTriggerVolume = CreateDefaultSubobject<UBoxComponent>("ControlTriggerVolume");
	ControlTriggerVolume->SetupAttachment(Floor);

	FloorTriggerVolume = CreateDefaultSubobject<UBoxComponent>("FloorTriggerVolume");
	FloorTriggerVolume->SetupAttachment(Floor);

	ControlBase = CreateDefaultSubobject<UStaticMeshComponent>("ControlBase");
	ControlBase->SetupAttachment(Floor);

	ControlBox = CreateDefaultSubobject<UStaticMeshComponent>("ControlBox");
	ControlBox->SetupAttachment(ControlBase);

	WallMenuComponent = CreateDefaultSubobject<UChildActorComponent>("WallMenuComponent");
	WallMenuComponent->SetupAttachment(Floor);

	PlatformTransitionType = EPlatformTransitionType::None;
}

void AMovablePlatform::BeginPlay()
{
	Super::BeginPlay();
	if (ABSCharacterBase* Character = Cast<ABSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		Character->OnInteractDelegate.BindUObject(this, &AMovablePlatform::MovePlatformUp);
		Character->OnShiftInteractDelegate.BindUObject(this, &AMovablePlatform::MovePlatformDown);
	}

	WallMenu = Cast<AWallMenu>(WallMenuComponent->GetChildActor());

	ControlTriggerVolume->OnComponentBeginOverlap.
	                      AddUniqueDynamic(this, &AMovablePlatform::OnTriggerVolumeBeginOverlap);
	ControlTriggerVolume->OnComponentEndOverlap.AddUniqueDynamic(this, &AMovablePlatform::OnTriggerVolumeEndOverlap);
	FloorTriggerVolume->OnComponentBeginOverlap.AddUniqueDynamic(this, &AMovablePlatform::OnCharacterStepOnFloor);
	FloorTriggerVolume->OnComponentEndOverlap.AddUniqueDynamic(this, &AMovablePlatform::OnCharacterStepOffFloor);
}

void AMovablePlatform::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (PlatformTransitionType == EPlatformTransitionType::None)
	{
		return;
	}
	if (!bPlayerIsOverlappingControl && bPlayerIsOverlappingFloor)
	{
		return;
	}
	InterpFloorElevation(DeltaSeconds);
}

void AMovablePlatform::MovePlatformUp(const int32 Stop)
{
	if (!bPlayerIsOverlappingControl)
	{
		return;
	}
	switch (Stop)
	{
	case 0:
		PlatformTransitionType = EPlatformTransitionType::MoveUpByInteract;
		break;
	case 1:
		PlatformTransitionType = EPlatformTransitionType::None;
		break;
	default:
		break;
	}
}

void AMovablePlatform::MovePlatformDown(const int32 Stop)
{
	if (!bPlayerIsOverlappingControl)
	{
		return;
	}
	switch (Stop)
	{
	case 0:
		PlatformTransitionType = EPlatformTransitionType::MoveDownByInteract;
		break;
	case 1:
		PlatformTransitionType = EPlatformTransitionType::None;
		break;
	default:
		break;
	}
}

void AMovablePlatform::InterpFloorElevation(const float DeltaSeconds)
{
	FVector Location;
	switch (PlatformTransitionType)
	{
	case EPlatformTransitionType::MoveUpByInteract:
		Location = UKismetMathLibrary::VInterpTo_Constant(Floor->GetComponentLocation(), Constants::MaxFloorHeight,
			DeltaSeconds, 75);
		break;
	case EPlatformTransitionType::MoveDownByInteract:
		Location = UKismetMathLibrary::VInterpTo_Constant(Floor->GetComponentLocation(), Constants::MinFloorHeight,
			DeltaSeconds, 75);
		break;
	case EPlatformTransitionType::MoveDownByStepOff:
		Location = UKismetMathLibrary::VInterpTo_Constant(Floor->GetComponentLocation(), Constants::MinFloorHeight,
			DeltaSeconds, 75);
		break;
	case EPlatformTransitionType::None:
		return;
	default:
		return;
	}
	Floor->SetWorldLocation(Location);
}

void AMovablePlatform::OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<ABSCharacterBase>(OtherActor))
	{
		Cast<ABSPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->ShowInteractInfo();
		bPlayerIsOverlappingControl = true;
	}
}

void AMovablePlatform::OnTriggerVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<ABSCharacterBase>(OtherActor))
	{
		Cast<ABSPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->HideInteractInfo();
		bPlayerIsOverlappingControl = false;
		PlatformTransitionType = EPlatformTransitionType::None;
	}
}

void AMovablePlatform::OnCharacterStepOnFloor(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<ABSCharacterBase>(OtherActor))
	{
		bPlayerIsOverlappingFloor = true;
	}
}

void AMovablePlatform::OnCharacterStepOffFloor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<ABSCharacterBase>(OtherActor))
	{
		PlatformTransitionType = EPlatformTransitionType::MoveDownByStepOff;
		bPlayerIsOverlappingFloor = false;
	}
}
