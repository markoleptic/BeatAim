// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Mappings/GameModeCategoryTagMap.h"

UGameModeCategoryTagMap::UGameModeCategoryTagMap()
{
}

TSubclassOf<UUserWidget> UGameModeCategoryTagMap::GetWidgetByGameModeCategoryTag(const FGameplayTag& GameplayTag)
{
	const TSubclassOf<UUserWidget>* Class = GameplayTagWidgetMap.Find(GameplayTag);
	return Class ? *Class : nullptr;
}
