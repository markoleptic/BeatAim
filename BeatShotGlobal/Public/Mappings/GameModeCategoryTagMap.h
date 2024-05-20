// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GameModeCategoryTagMap.generated.h"

/** Associates GameModeCategory GameplayTags to unique widgets. */
UCLASS(Blueprintable, BlueprintType, Const)
class BEATSHOTGLOBAL_API UGameModeCategoryTagMap : public UDataAsset
{
	GENERATED_BODY()

public:
	UGameModeCategoryTagMap();

	/** Returns the GameModeCategoryTagWidget associated with a GameplayTag. */
	TSubclassOf<UUserWidget> GetWidgetByGameModeCategoryTag(const FGameplayTag& GameplayTag);

	/** Returns the GameplayTagWidgetMap. */
	TMap<FGameplayTag, TSubclassOf<UUserWidget>>& GetMap() { return GameplayTagWidgetMap; }

protected:
	UPROPERTY(EditDefaultsOnly, meta = (Categories="GameModeCategory"))
	TMap<FGameplayTag, TSubclassOf<UUserWidget>> GameplayTagWidgetMap;
};
