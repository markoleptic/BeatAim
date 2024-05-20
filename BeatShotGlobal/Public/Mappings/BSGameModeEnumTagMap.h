// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mappings/EnumTagMap.h"
#include "BSGameModeEnumTagMap.generated.h"

UCLASS(Blueprintable, BlueprintType, Const)
class BEATSHOTGLOBAL_API UBSGameModeEnumTagMap : public UEnumTagMap
{
	GENERATED_BODY()

	UBSGameModeEnumTagMap();
};
