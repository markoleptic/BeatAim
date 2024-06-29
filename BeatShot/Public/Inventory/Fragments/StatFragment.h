// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/BSInventoryItemDefinition.h"
#include "StatFragment.generated.h"

struct FGameplayTag;

/** An inventory fragment that contains a map of item stats. */
UCLASS()
class BEATSHOT_API UStatFragment : public UBSInventoryItemFragment
{
	GENERATED_BODY()

	virtual void OnInstanceCreated(UBSInventoryItemInstance* Instance) const override;

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Equipment")
	TMap<FGameplayTag, int32> InitialItemStats;

	int32 GetItemStatByTag(const FGameplayTag& Tag) const;
};
