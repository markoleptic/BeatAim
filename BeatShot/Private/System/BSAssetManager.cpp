// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "System/BSAssetManager.h"

UBSAssetManager::UBSAssetManager()
{
}

UBSAssetManager& UBSAssetManager::Get()
{
	check(GEngine);
	auto This = Cast<UBSAssetManager>(GEngine->AssetManager);

	UBSAssetManager* MyAssetManager = Cast<UBSAssetManager>(GEngine->AssetManager);
	return *MyAssetManager;
}

void UBSAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
}
