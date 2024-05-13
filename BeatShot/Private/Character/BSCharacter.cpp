// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "Character/BSCharacter.h"
#include "Character/BSCharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"


ABSCharacter::ABSCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.SetDefaultSubobjectClass<UBSCharacterMovementComponent>(CharacterMovementComponentName).
	                  DoNotCreateDefaultSubobject(MeshComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bAlwaysRelevant = true;
	bReplicates = true;

	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, 64.f));

	HandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Character Mesh");
	HandsMesh->SetupAttachment(CameraComponent);
	HandsMesh->bCastDynamicShadow = false;
	HandsMesh->CastShadow = false;
	HandsMesh->SetRelativeRotation(FRotator(1.53f, -15.20f, 8.32f));
	HandsMesh->SetRelativeLocation(FVector(-17.69f, -10.50f, -149.11f));

	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.f);
	// Set collision settings. We are the invisible player with no 3rd person mesh.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);

	// Camera eye level
	BaseEyeHeight = 64.f;
	CrouchedEyeHeight = 44.f;
	MinSpeedForFallDamage = 1002.9825f;
	MinLandBounceSpeed = 329.565f;
	CapDamageMomentumZ = 476.25f;
}

USkeletalMeshComponent* ABSCharacter::GetHandsMesh() const
{
	return Cast<USkeletalMeshComponent>(HandsMesh);
}
