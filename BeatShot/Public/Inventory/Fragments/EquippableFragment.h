// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/BSInventoryItemDefinition.h"
#include "EquippableFragment.generated.h"

class UBSEquipmentDefinition;

/** An inventory fragment that can be equipped. */
UCLASS()
class BEATSHOT_API UEquippableFragment : public UBSInventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Equipment")
	TSubclassOf<UBSEquipmentDefinition> EquipmentDefinition;
};
