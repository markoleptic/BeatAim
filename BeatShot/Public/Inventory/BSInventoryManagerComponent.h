// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "BSInventoryManagerComponent.generated.h"

class UBSEquipmentManagerComponent;
class UBSEquipmentInstance;
class UBSInventoryItemDefinition;
class UBSInventoryItemInstance;
struct FGameplayTag;


/** A single entry in an inventory, which contains a pointer to the InventoryItemInstance and the stack count. */
USTRUCT(BlueprintType)
struct FBSInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FBSInventoryEntry()
	{
	}

	FString GetDebugString() const;

private:
	friend struct FBSInventoryList;
	friend class UBSInventoryManagerComponent;

	UPROPERTY()
	TObjectPtr<UBSInventoryItemInstance> Instance = nullptr;

	UPROPERTY()
	int32 StackCount = 0;

	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
};


/**
 *  An array of FBSInventoryEntries, which each contain a pointer to the InventoryItemInstance and the stack count
 */
USTRUCT(BlueprintType)
struct FBSInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FBSInventoryList() : OwnerComponent(nullptr)
	{
	}

	FBSInventoryList(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent)
	{
	}

	TArray<UBSInventoryItemInstance*> GetAllItems() const;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FBSInventoryEntry, FBSInventoryList>(Items, DeltaParms,
			*this);
	}

	UBSInventoryItemInstance* AddEntry(TSubclassOf<UBSInventoryItemDefinition> ItemDef, int32 StackCount);
	void RemoveEntry(UBSInventoryItemInstance* Instance);

private:
	friend UBSInventoryManagerComponent;

	// Replicated list of items
	UPROPERTY()
	TArray<FBSInventoryEntry> Items;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template <>
struct TStructOpsTypeTraits<FBSInventoryList> : public TStructOpsTypeTraitsBase2<FBSInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};


/**
 *  The component responsible for inventory management. Contains an FBSInventoryList, which holds an array of FBSInventoryEntries.
 *  Each FBSInventoryEntry contains a pointer to InventoryItemInstance and the stack count. Each InventoryItemInstance contains the
 *  item definition and the stack count
 */
UCLASS(BlueprintType)
class BEATSHOT_API UBSInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBSInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Creates and returns an instance of the inventory item. Adds to the replicated sub object list. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	UBSInventoryItemInstance* AddItemInstance(TSubclassOf<UBSInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	/** Removes the item from the inventory list and removes it from the replicated sub object list. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void RemoveItemInstance(UBSInventoryItemInstance* ItemInstance);

	/** Returns all item instances the inventory list. */
	UFUNCTION(BlueprintCallable, Category = "Inventory", BlueprintPure=false)
	TArray<UBSInventoryItemInstance*> GetAllItems() const;

	/** Returns the first item matching the item definition in the inventory list. */
	UFUNCTION(BlueprintCallable, Category = "Inventory", BlueprintPure)
	UBSInventoryItemInstance* FindFirstItemStackByDefinition(TSubclassOf<UBSInventoryItemDefinition> ItemDef) const;

	/** Returns the total count of item instances matching the item definition. */
	int32 GetTotalItemCountByDefinition(TSubclassOf<UBSInventoryItemDefinition> ItemDef) const;

	/** Removes the specified number of item instances. */
	bool ConsumeItemsByDefinition(TSubclassOf<UBSInventoryItemDefinition> ItemDef, int32 NumToConsume);

	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
		FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	//~End of UObject interface

private:
	UPROPERTY(Replicated)
	FBSInventoryList InventoryList;

public:
	/** Cycles the active slot index forward, wrapping around if necessary. */
	UFUNCTION(BlueprintCallable, Category = "Inventory | Slots")
	void CycleActiveSlotForward();

	/** Cycles the active slot index backwards, wrapping around if necessary. */
	UFUNCTION(BlueprintCallable, Category = "Inventory | Slots")
	void CycleActiveSlotBackward();

	/** Unequips the item instance in the current slot and equips the item in the NewIndex slot.
	 *  Updates the ActiveSlotIndex */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category= "Inventory | Slots")
	void SetActiveSlotIndex(int32 NewIndex);

	/** Returns the array of Inventory Item Instances. */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category= "Inventory | Slots")
	TArray<UBSInventoryItemInstance*> GetSlots() const { return Slots; }

	/** Returns the index of the currently active slot. */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category= "Inventory | Slots")
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	/** Returns the index of the previously active slot. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= "Inventory | Slots")
	int32 GetLastSlotIndex() const { return LastSlotIndex; }

	/** Returns item instance corresponding to the current slot. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category= "Inventory | Slots")
	UBSInventoryItemInstance* GetActiveSlotItem() const;

	/** Returns the first empty slot (null Inventory Item Instance in Slots) */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category= "Inventory | Slots")
	int32 GetNextFreeItemSlot() const;

	/** Adds an Inventory Item Instance to the Slots array. Call AddItemDefinition to get an
	 *  instance from a definition before calling this function. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= "Inventory | Slots")
	void AddItemToSlot(int32 SlotIndex, UBSInventoryItemInstance* Item);

	/** Unequips and returns an item instance. Call RemoveItemInstance to remove from the inventory list. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= "Inventory | Slots")
	UBSInventoryItemInstance* RemoveItemFromSlot(int32 SlotIndex);

	/** Returns the first spawned equipment actor for the currently equipped item. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= "Inventory | Equipment")
	AActor* GetEquippedItemFirstSpawnedActor() const;

	/** Returns whether or not the currently equipped item has the tag. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= "Inventory | Tags")
	bool EquippedContainsTag(const FGameplayTag& Tag) const;

	/** Returns whether or not the slot index item has the tag. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= "Inventory | Tags")
	bool SlotIndexContainsTag(const int32 SlotIndex, const FGameplayTag& Tag) const;

	virtual void BeginPlay() override;

private:
	/** Calls UnequipItem using the Equipment Manager and clears the EquippedItem. */
	void ForceUnequipItemInSlot();

	/** Calls EquipItem using the Equipment Manager and sets the EquippedItem. */
	void EquipItemInSlot();

	UBSEquipmentManagerComponent* FindEquipmentManager() const;

protected:
	UPROPERTY()
	int32 NumSlots = 3;

	UFUNCTION()
	void OnRep_Slots();

	UFUNCTION()
	void OnRep_ActiveSlotIndex();

private:
	UPROPERTY(ReplicatedUsing=OnRep_Slots)
	TArray<TObjectPtr<UBSInventoryItemInstance>> Slots;

	UPROPERTY(ReplicatedUsing=OnRep_ActiveSlotIndex)
	int32 ActiveSlotIndex = -1;

	UPROPERTY()
	int32 LastSlotIndex = -1;

	int32 LastRequestedSlotEquipIndex = -1;

	UPROPERTY()
	TObjectPtr<UBSEquipmentInstance> EquippedItem;
};
