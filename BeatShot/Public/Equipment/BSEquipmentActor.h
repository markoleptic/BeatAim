// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Actor.h"
#include "BSEquipmentActor.generated.h"

UCLASS()
class BEATSHOT_API ABSEquipmentActor : public AActor, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	ABSEquipmentActor();

protected:
	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** ~IGameplayTagAssetInterface begin */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	/** ~IGameplayTagAssetInterface end */

	void AddGameplayTag(const FGameplayTag& Tag);
	void RemoveGameplayTag(const FGameplayTag& Tag);

private:
	FGameplayTagContainer GameplayTags;
};
