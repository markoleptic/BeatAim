﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Inventory/BSInventoryManagerComponent.h"
#include "Inventory/BSInventoryItemDefinition.h"
#include "Inventory/BSInventoryItemInstance.h"
#include "Inventory/Fragments/EquippableFragment.h"
#include "Equipment/BSEquipmentDefinition.h"
#include "Equipment/BSEquipmentManagerComponent.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "Equipment/BSEquipmentInstance.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Misc/AssertionMacros.h"
#include "Net/UnrealNetwork.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectBaseUtility.h"

class FLifetimeProperty;
struct FReplicationFlags;

FString FBSInventoryEntry::GetDebugString() const
{
	TSubclassOf<UBSInventoryItemDefinition> ItemDef;
	if (Instance != nullptr)
	{
		ItemDef = Instance->GetItemDef();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Instance), StackCount, *GetNameSafe(ItemDef));
}

TArray<UBSInventoryItemInstance*> FBSInventoryList::GetAllItems() const
{
	TArray<UBSInventoryItemInstance*> Results;
	Results.Reserve(Entries.Num());
	for (const FBSInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance != nullptr) //@TODO: Would prefer to not deal with this here and hide it further?
		{
			Results.Add(Entry.Instance);
		}
	}
	return Results;
}

void FBSInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Index : RemovedIndices)
	{
		FBSInventoryEntry& Stack = Entries[Index];
		Stack.LastObservedCount = 0;
	}
}

void FBSInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FBSInventoryEntry& Stack = Entries[Index];
		Stack.LastObservedCount = Stack.StackCount;
	}
}

void FBSInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Index : ChangedIndices)
	{
		FBSInventoryEntry& Stack = Entries[Index];
		check(Stack.LastObservedCount != INDEX_NONE);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

UBSInventoryItemInstance* FBSInventoryList::AddEntry(TSubclassOf<UBSInventoryItemDefinition> ItemDef, int32 StackCount)
{
	check(ItemDef != nullptr);
	check(OwnerComponent);

	const AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	FBSInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewObject<UBSInventoryItemInstance>(OwnerComponent->GetOwner());
	NewEntry.Instance->SetItemDef(ItemDef);
	NewEntry.Instance->AddIdentifierTags(GetDefault<UBSInventoryItemDefinition>(ItemDef)->ItemTags);
	for (const UBSInventoryItemFragment* Fragment : GetDefault<UBSInventoryItemDefinition>(ItemDef)->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.Instance);
		}
	}
	NewEntry.StackCount = StackCount;
	UBSInventoryItemInstance* Result = NewEntry.Instance;

	MarkItemDirty(NewEntry);

	return Result;
}

void FBSInventoryList::RemoveEntry(UBSInventoryItemInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FBSInventoryEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

UBSInventoryManagerComponent::UBSInventoryManagerComponent(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer), InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

void UBSInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

UBSInventoryItemInstance* UBSInventoryManagerComponent::AddItemInstance(
	TSubclassOf<UBSInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UBSInventoryItemInstance* Result = nullptr;
	if (ItemDef != nullptr)
	{
		Result = InventoryList.AddEntry(ItemDef, StackCount);

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			AddReplicatedSubObject(Result);
		}
	}
	return Result;
}

void UBSInventoryManagerComponent::RemoveItemInstance(UBSInventoryItemInstance* ItemInstance)
{
	InventoryList.RemoveEntry(ItemInstance);

	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

TArray<UBSInventoryItemInstance*> UBSInventoryManagerComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

UBSInventoryItemInstance* UBSInventoryManagerComponent::FindFirstItemStackByDefinition(
	TSubclassOf<UBSInventoryItemDefinition> ItemDef) const
{
	for (const FBSInventoryEntry& Entry : InventoryList.Entries)
	{
		UBSInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

int32 UBSInventoryManagerComponent::GetTotalItemCountByDefinition(TSubclassOf<UBSInventoryItemDefinition> ItemDef) const
{
	int32 TotalCount = 0;
	for (const FBSInventoryEntry& Entry : InventoryList.Entries)
	{
		UBSInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				++TotalCount;
			}
		}
	}

	return TotalCount;
}

bool UBSInventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<UBSInventoryItemDefinition> ItemDef,
	int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return false;
	}

	//@TODO: N squared right now as there's no acceleration structure
	int32 TotalConsumed = 0;
	while (TotalConsumed < NumToConsume)
	{
		if (UBSInventoryItemInstance* Instance = UBSInventoryManagerComponent::FindFirstItemStackByDefinition(ItemDef))
		{
			InventoryList.RemoveEntry(Instance);
			++TotalConsumed;
		}
		else
		{
			return false;
		}
	}

	return TotalConsumed == NumToConsume;
}

bool UBSInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FBSInventoryEntry& Entry : InventoryList.Entries)
	{
		UBSInventoryItemInstance* Instance = Entry.Instance;

		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UBSInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing UBSInventoryItemInstance
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FBSInventoryEntry& Entry : InventoryList.Entries)
		{
			UBSInventoryItemInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

void UBSInventoryManagerComponent::CycleActiveSlotForward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num() - 1 : ActiveSlotIndex);
	int32 NewIndex = ActiveSlotIndex;
	do
	{
		NewIndex = (NewIndex + 1) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	}
	while (NewIndex != OldIndex);
	LastSlotIndex = OldIndex;
}

void UBSInventoryManagerComponent::CycleActiveSlotBackward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num() - 1 : ActiveSlotIndex);
	int32 NewIndex = ActiveSlotIndex;
	do
	{
		NewIndex = (NewIndex - 1 + Slots.Num()) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	}
	while (NewIndex != OldIndex);
	LastSlotIndex = OldIndex;
}

void UBSInventoryManagerComponent::SetActiveSlotIndex_Implementation(int32 NewIndex)
{
	if (Slots.IsValidIndex(NewIndex) && (ActiveSlotIndex != NewIndex))
	{
		const int32 OldIndex = ActiveSlotIndex;
		
		UnequipItemInSlot();

		ActiveSlotIndex = NewIndex;
		LastSlotIndex = OldIndex;

		EquipItemInSlot();

		OnRep_ActiveSlotIndex();
	}
}

UBSInventoryItemInstance* UBSInventoryManagerComponent::GetActiveSlotItem() const
{
	return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}

int32 UBSInventoryManagerComponent::GetNextFreeItemSlot() const
{
	int32 SlotIndex = 0;
	for (TObjectPtr<UBSInventoryItemInstance> ItemPtr : Slots)
	{
		if (ItemPtr == nullptr)
		{
			return SlotIndex;
		}
		++SlotIndex;
	}

	return INDEX_NONE;
}

void UBSInventoryManagerComponent::AddItemToSlot(int32 SlotIndex, UBSInventoryItemInstance* Item)
{
	if (!Slots.IsValidIndex(SlotIndex))
	{
		Slots.AddDefaulted(1);
	}

	if (Slots.IsValidIndex(SlotIndex) && (Item != nullptr))
	{
		if (Slots[SlotIndex] == nullptr)
		{
			Slots[SlotIndex] = Item;
			OnRep_Slots();
		}
	}
}

UBSInventoryItemInstance* UBSInventoryManagerComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	UBSInventoryItemInstance* Result = nullptr;

	if (ActiveSlotIndex == SlotIndex)
	{
		UnequipItemInSlot();
		ActiveSlotIndex = -1;
	}

	if (Slots.IsValidIndex(SlotIndex))
	{
		Result = Slots[SlotIndex];

		if (Result != nullptr)
		{
			Slots[SlotIndex] = nullptr;
			OnRep_Slots();
		}
	}

	return Result;
}

AActor* UBSInventoryManagerComponent::GetEquippedItemFirstSpawnedActor() const
{
	if (EquippedItem)
	{
		return EquippedItem->GetFirstSpawnedActor();
	}
	return nullptr;
}

bool UBSInventoryManagerComponent::EquippedContainsTag(const FGameplayTag& Tag) const
{
	if (GetActiveSlotItem())
	{
		return GetActiveSlotItem()->HasIdentifierTag(Tag);
	}
	return false;
}

bool UBSInventoryManagerComponent::SlotIndexContainsTag(const int32 SlotIndex, const FGameplayTag& Tag) const
{
	if (Slots.IsValidIndex(SlotIndex))
	{
		return Slots[SlotIndex]->HasIdentifierTag(Tag);
	}
	return false;
}

void UBSInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Slots.Num() < NumSlots)
	{
		Slots.AddDefaulted(NumSlots - Slots.Num());
	}
}

void UBSInventoryManagerComponent::UnequipItemInSlot()
{
	if (UBSEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
	{
		if (EquippedItem)
		{
			EquipmentManager->UnequipItem(EquippedItem);
			EquippedItem = nullptr;
		}
	}
}

void UBSInventoryManagerComponent::EquipItemInSlot()
{
	check(Slots.IsValidIndex(ActiveSlotIndex));
	check(EquippedItem == nullptr);

	if (UBSInventoryItemInstance* SlotItem = Slots[ActiveSlotIndex])
	{
		if (const UEquippableFragment* EquipInfo = SlotItem->FindFragmentByClass<UEquippableFragment>())
		{
			const TSubclassOf<UBSEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition;
			if (EquipDef != nullptr)
			{
				if (UBSEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
				{
					EquippedItem = EquipmentManager->EquipItem(EquipDef);
					if (EquippedItem != nullptr)
					{
						EquippedItem->SetInstigator(SlotItem);
					}
				}
			}
		}
	}
}

UBSEquipmentManagerComponent* UBSInventoryManagerComponent::FindEquipmentManager() const
{
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		return Pawn->FindComponentByClass<UBSEquipmentManagerComponent>();
	}
	return nullptr;
}

void UBSInventoryManagerComponent::OnRep_Slots()
{
}

void UBSInventoryManagerComponent::OnRep_ActiveSlotIndex()
{
}
