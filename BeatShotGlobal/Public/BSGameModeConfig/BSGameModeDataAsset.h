// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSGameModeConfig/BSConfig.h"
#include "Engine/DataAsset.h"
#include "BSGameModeDataAsset.generated.h"

UCLASS(Blueprintable, BlueprintType)
class BEATSHOTGLOBAL_API UBSGameModeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UBSGameModeDataAsset();

	/** @return all FBSConfig structs. */
	TArray<FBSConfig> GetDefaultGameModes() const;

	/** @return DefiningConfig structs mapped to BSConfig structs. */
	const TMap<FBS_DefiningConfig, FBSConfig>& GetGameModesMap() const;

protected:
	/** The game modes contained in this data asset. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ForceInlineRow))
	TMap<FBS_DefiningConfig, FBSConfig> DefaultGameModes;
};
