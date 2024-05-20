// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "BSAssetManager.generated.h"

/** The most basic of Asset Managers. */
UCLASS()
class BEATSHOT_API UBSAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	UBSAssetManager();

	// Returns the AssetManager singleton object.
	static UBSAssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
};
