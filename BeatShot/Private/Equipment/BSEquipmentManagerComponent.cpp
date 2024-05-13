// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Equipment/BSEquipmentManagerComponent.h"
#include "Equipment/BSEquipmentDefinition.h"
#include "Equipment/BSEquipmentInstance.h"
#include "AbilitySystem/Globals/BSAbilitySet.h"
#include "AbilitySystem/BSAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/Actor.h"
#include "Misc/AssertionMacros.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/Object.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectBaseUtility.h"

class FLifetimeProperty;
struct FReplicationFlags;

UBSEquipmentInstance* FBSEquipmentList::AddEntry(TSubclassOf<UBSEquipmentDefinition> EquipmentDefinition)
{
	check(EquipmentDefinition);
	check(OwnerComponent);
	check(OwnerComponent->GetOwner()->HasAuthority());

	const UBSEquipmentDefinition* EquipmentCDO = GetDefault<UBSEquipmentDefinition>(EquipmentDefinition);

	TSubclassOf<UBSEquipmentInstance> InstanceType = EquipmentCDO->InstanceType;
	if (InstanceType == nullptr)
	{
		InstanceType = UBSEquipmentInstance::StaticClass();
	}

	FBSAppliedEquipmentEntry& NewEntry = Items.AddDefaulted_GetRef();
	NewEntry.EquipmentDefinition = EquipmentDefinition;
	NewEntry.Instance = NewObject<UBSEquipmentInstance>(OwnerComponent->GetOwner(), InstanceType);
	UBSEquipmentInstance* Result = NewEntry.Instance.Get();

	if (UBSAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		for (const TObjectPtr<const UBSAbilitySet> AbilitySet : EquipmentCDO->AbilitySetsToGrant)
		{
			if (AbilitySet)
			{
				AbilitySet->GiveToAbilitySystem(ASC, &NewEntry.GrantedHandles, Result);
			}
		}

		// Apply Equipped gameplay tags
		ASC->AddLooseGameplayTags(EquipmentCDO->EquippedTags);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent failed to get in FBSEquipmentList::AddEntry"));
	}
	Result->SpawnEquipmentActors(EquipmentCDO->ActorsToSpawn);
	MarkItemDirty(NewEntry);
	return Result;
}

void FBSEquipmentList::RemoveEntry(UBSEquipmentInstance* Instance)
{
	for (auto EntryIt = Items.CreateIterator(); EntryIt; ++EntryIt)
	{
		FBSAppliedEquipmentEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			if (UBSAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				Entry.GrantedHandles.TakeFromAbilitySystem(ASC);

				// Remove Equipped gameplay tags
				const UBSEquipmentDefinition* EquipmentCDO = GetDefault<UBSEquipmentDefinition>(
					Entry.EquipmentDefinition);
				ASC->RemoveLooseGameplayTags(EquipmentCDO->EquippedTags);
			}
			Instance->DestroyEquipmentActors();
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

UBSAbilitySystemComponent* FBSEquipmentList::GetAbilitySystemComponent() const
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	return Cast<UBSAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor));
}

/* ------------------------------------ */
/* -- BS Equipment Manager Component -- */
/* ------------------------------------ */

UBSEquipmentManagerComponent::UBSEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer), EquipmentList(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UBSEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentList);
}

UBSEquipmentInstance* UBSEquipmentManagerComponent::EquipItem(TSubclassOf<UBSEquipmentDefinition> EquipmentDefinition)
{
	UBSEquipmentInstance* Result = nullptr;
	if (EquipmentDefinition != nullptr)
	{
		Result = EquipmentList.AddEntry(EquipmentDefinition);
		if (Result != nullptr)
		{
			Result->OnEquipped();
			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
			{
				AddReplicatedSubObject(Result);
			}
		}
	}
	return Result;
}

void UBSEquipmentManagerComponent::UnequipItem(UBSEquipmentInstance* ItemInstance, const bool bCallOnUnequipped)
{
	if (ItemInstance != nullptr)
	{
		if (bCallOnUnequipped)
		{
			ItemInstance->OnUnequipped();
		}
		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(ItemInstance);
		}
		EquipmentList.RemoveEntry(ItemInstance);
	}
}

bool UBSEquipmentManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FBSAppliedEquipmentEntry& Entry : EquipmentList.Items)
	{
		UBSEquipmentInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UBSEquipmentManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UBSEquipmentManagerComponent::UninitializeComponent()
{
	TArray<UBSEquipmentInstance*> AllEquipmentInstances;

	// gathering all instances before removal to avoid side effects affecting the equipment list iterator	
	for (const FBSAppliedEquipmentEntry& Entry : EquipmentList.Items)
	{
		AllEquipmentInstances.Add(Entry.Instance.Get());
	}

	for (UBSEquipmentInstance* EquipInstance : AllEquipmentInstances)
	{
		UnequipItem(EquipInstance);
	}

	Super::UninitializeComponent();
}

void UBSEquipmentManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing BSEquipmentInstances
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FBSAppliedEquipmentEntry& Entry : EquipmentList.Items)
		{
			UBSEquipmentInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

UBSEquipmentInstance* UBSEquipmentManagerComponent::GetFirstInstanceOfType(
	TSubclassOf<UBSEquipmentInstance> InstanceType)
{
	for (FBSAppliedEquipmentEntry& Entry : EquipmentList.Items)
	{
		if (UBSEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

TArray<UBSEquipmentInstance*> UBSEquipmentManagerComponent::GetEquipmentInstancesOfType(
	TSubclassOf<UBSEquipmentInstance> InstanceType) const
{
	TArray<UBSEquipmentInstance*> Results;
	for (const FBSAppliedEquipmentEntry& Entry : EquipmentList.Items)
	{
		if (UBSEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				Results.Add(Instance);
			}
		}
	}
	return Results;
}
