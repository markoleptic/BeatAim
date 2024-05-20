// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Equipment/BSEquipmentInstance.h"
#include "BSGameMode.h"
#include "Character/BSCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "Equipment/BSEquipmentDefinition.h"
#include "Equipment/BSGun.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UBSEquipmentInstance::UBSEquipmentInstance()
{
}

UWorld* UBSEquipmentInstance::GetWorld() const
{
	if (const APawn* OwningPawn = GetPawn())
	{
		return OwningPawn->GetWorld();
	}
	return nullptr;
}

void UBSEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, SpawnedActors);
}

APawn* UBSEquipmentInstance::GetPawn() const
{
	return Cast<APawn>(GetOuter());
}

APawn* UBSEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* Result = nullptr;
	if (UClass* ActualPawnType = PawnType)
	{
		if (GetOuter()->IsA(ActualPawnType))
		{
			Result = Cast<APawn>(GetOuter());
		}
	}
	return Result;
}

AActor* UBSEquipmentInstance::GetTypedSpawnedActor(TSubclassOf<AActor> ActorType) const
{
	UClass* ActualActorType = ActorType;
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor->IsA(ActualActorType))
		{
			return Cast<AActor>(Actor);
		}
	}
	return nullptr;
}


template <class T>
T* UBSEquipmentInstance::GetTypedSpawnedActor() const
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor->IsA<T>())
		{
			return Cast<T>(Actor);
		}
	}
	return nullptr;
}

AActor* UBSEquipmentInstance::GetFirstSpawnedActor() const
{
	if (!SpawnedActors.IsEmpty())
	{
		return SpawnedActors[0];
	}
	return nullptr;
}

void UBSEquipmentInstance::SpawnEquipmentActors(const TArray<FBSEquipmentActorToSpawn>& ActorsToSpawn)
{
	if (APawn* OwningPawn = GetPawn())
	{
		USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
		if (const ABSCharacter* Character = Cast<ABSCharacter>(OwningPawn))
		{
			AttachTarget = Character->GetHandsMesh();
		}
		else if (const ABSCharacterBase* CharacterBase = Cast<ABSCharacterBase>(OwningPawn))
		{
			AttachTarget = CharacterBase->GetMesh();
		}

		for (const FBSEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity,
				OwningPawn);
			if (ABSGun* Gun = Cast<ABSGun>(NewActor))
			{
				if (ABSGameMode* GameMode = Cast<ABSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
				{
					GameMode->RegisterGun(Gun);
				}
			}
			NewActor->FinishSpawning(FTransform::Identity, true);
			NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
			NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform,
				SpawnInfo.AttachSocket);
			SpawnedActors.Add(NewActor);
		}
	}
}

void UBSEquipmentInstance::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}

void UBSEquipmentInstance::OnEquipped()
{
	K2_OnEquipped();
}

void UBSEquipmentInstance::OnUnequipped()
{
	K2_OnUnequipped();
}

void UBSEquipmentInstance::ConfirmUnequip()
{
	OnUnequipConfirmed.Broadcast(this);
}

void UBSEquipmentInstance::CancelUnequip()
{
	OnUnequipConfirmed.Clear();
}

void UBSEquipmentInstance::OnRep_Instigator()
{
}
