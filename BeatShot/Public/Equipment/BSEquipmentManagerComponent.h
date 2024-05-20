// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Globals/BSAbilitySet.h"
#include "Components/PawnComponent.h"
#include "Equipment/BSEquipmentDefinition.h"
#include "Equipment/BSEquipmentInstance.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "UObject/UObjectBaseUtility.h"
#include "BSEquipmentManagerComponent.generated.h"

class UBSAbilitySystemComponent;
class UBSEquipmentDefinition;

/** A single piece of applied equipment, which contains a pointer to EquipmentInstance and the EquipmentDefinition. */
USTRUCT(BlueprintType)
struct FBSAppliedEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FBSAppliedEquipmentEntry()
	{
	}

	FString GetDebugString() const
	{
		return FString::Printf(TEXT("%s of %s"), *GetNameSafe(Instance.Get()), *GetNameSafe(EquipmentDefinition.Get()));
	};

	friend struct FBSEquipmentList;
	friend class UBSEquipmentManagerComponent;

	/** The class default object to create the equipment from. */
	UPROPERTY()
	TSubclassOf<UBSEquipmentDefinition> EquipmentDefinition;

	/** The equipment instance created from the class default object. */
	UPROPERTY()
	TObjectPtr<UBSEquipmentInstance> Instance = nullptr;

	/** Authority-only list of granted handles. */
	UPROPERTY(NotReplicated)
	FBSGrantedAbilitySet GrantedHandles;
};


/** An array of FBSAppliedEquipmentEntries, which each contain a pointer to EquipmentInstance and the
 *  EquipmentDefinition. */
USTRUCT(BlueprintType)
struct FBSEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	FBSEquipmentList() : OwnerComponent(nullptr)
	{
	}

	FBSEquipmentList(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent)
	{
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FBSAppliedEquipmentEntry, FBSEquipmentList>(Items,
			DeltaParms, *this);
	}

	/** Creates and adds a new FBSAppliedEquipmentEntry, also granting any associated abilities. */
	UBSEquipmentInstance* AddEntry(TSubclassOf<UBSEquipmentDefinition> EquipmentDefinition);

	/** Removes an existing FBSAppliedEquipmentEntry, also removing any associated abilities. */
	void RemoveEntry(UBSEquipmentInstance* Instance);

private:
	UBSAbilitySystemComponent* GetAbilitySystemComponent() const;

	friend UBSEquipmentManagerComponent;

	/** Replicated list of equipment entries. */
	UPROPERTY()
	TArray<FBSAppliedEquipmentEntry> Items;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template <>
struct TStructOpsTypeTraits<FBSEquipmentList> : public TStructOpsTypeTraitsBase2<FBSEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};


/** The component responsible for equipment management. Contains an FBSEquipmentList, which holds an array of
 *  FBSAppliedEquipmentEntries. Each FBSInventoryEntry contains a pointer to EquipmentInstance and the
 *  EquipmentDefinition. Each InventoryItemInstance contains the item definition and the stack count. */
UCLASS(BlueprintType, Const)
class BEATSHOT_API UBSEquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UBSEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UBSEquipmentInstance* EquipItem(TSubclassOf<UBSEquipmentDefinition> EquipmentDefinition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void UnequipItem(UBSEquipmentInstance* ItemInstance, const bool bCallOnUnequipped = true);

	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
		FReplicationFlags* RepFlags) override;
	//~End of UObject interface

	//~UActorComponent interface
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void ReadyForReplication() override;
	//~End of UActorComponent interface

	/** Returns the first equipped instance of a given type, or nullptr if none are found. */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UBSEquipmentInstance* GetFirstInstanceOfType(TSubclassOf<UBSEquipmentInstance> InstanceType);

	/** Returns all equipped instances of a given type, or an empty array if none are found. */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<UBSEquipmentInstance*> GetEquipmentInstancesOfType(TSubclassOf<UBSEquipmentInstance> InstanceType) const;

	template <typename T>
	T* GetFirstInstanceOfType()
	{
		return static_cast<T*>(GetFirstInstanceOfType(T::StaticClass()));
	}

private:
	UPROPERTY(Replicated)
	FBSEquipmentList EquipmentList;
};
