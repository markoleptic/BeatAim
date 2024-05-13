// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BSEquipmentInstance.generated.h"

struct FBSEquipmentActorToSpawn;
class AActor;
class APawn;
struct FFrame;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnequipConfirmed, class UBSEquipmentInstance*);

/** Represents a piece of spawned equipment for a pawn. Contains an array of actors (equipment) */
UCLASS(BlueprintType, Blueprintable)
class BEATSHOT_API UBSEquipmentInstance : public UObject
{
	GENERATED_BODY()

public:
	UBSEquipmentInstance();

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual UWorld* GetWorld() const override final;
	//~End of UObject interface

	UFUNCTION(BlueprintPure, Category=Equipment)
	UObject* GetInstigator() const { return Instigator; }

	void SetInstigator(UObject* InInstigator) { Instigator = InInstigator; }

	UFUNCTION(BlueprintPure, Category=Equipment)
	APawn* GetPawn() const;

	UFUNCTION(BlueprintPure, Category=Equipment, meta=(DeterminesOutputType=PawnType))
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;

	UFUNCTION(BlueprintPure, Category=Equipment, meta=(DeterminesOutputType=ActorType))
	AActor* GetTypedSpawnedActor(TSubclassOf<AActor> ActorType) const;

	UFUNCTION(BlueprintPure, Category=Equipment)
	TArray<AActor*> GetSpawnedActors() const { return SpawnedActors; }

	UFUNCTION(BlueprintPure, Category=Equipment)
	AActor* GetFirstSpawnedActor() const;

	/** Spawns the equipment actors from the ActorsToSpawn array */
	virtual void SpawnEquipmentActors(const TArray<FBSEquipmentActorToSpawn>& ActorsToSpawn);

	/** Destroys the equipment actors spawned when equipped. */
	virtual void DestroyEquipmentActors();

	/** Called after adding to FBSEquipmentList. */
	virtual void OnEquipped();

	/** Called after removing from FBSEquipmentList. */
	virtual void OnUnequipped();

	UFUNCTION(BlueprintImplementableEvent, Category=Equipment, meta=(DisplayName="OnEquipped"))
	void K2_OnEquipped();

	UFUNCTION(BlueprintImplementableEvent, Category=Equipment, meta=(DisplayName="OnUnequipped"))
	void K2_OnUnequipped();

	UFUNCTION(BlueprintCallable, Category=Equipment)
	void ConfirmUnequip();

	UFUNCTION(BlueprintCallable, Category=Equipment)
	void CancelUnequip();

	template <class T>
	T* GetTypedSpawnedActor() const;

	FOnEnequipConfirmed OnUnequipConfirmed;

private:
	UFUNCTION()
	void OnRep_Instigator();

	UPROPERTY(ReplicatedUsing=OnRep_Instigator)
	TObjectPtr<UObject> Instigator;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<AActor>> SpawnedActors;
};
