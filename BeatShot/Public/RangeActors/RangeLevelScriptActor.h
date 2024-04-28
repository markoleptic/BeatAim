// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "RangeLevelScriptActor.generated.h"

class UBSGameUserSettings;
class APostProcessVolume;

/** The base level used for this game */
UCLASS()
class BEATSHOT_API ARangeLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()

protected:
	ARangeLevelScriptActor();

	virtual void BeginPlay() override;

	/** Callback function to respond to video setting changes */
	void HandleGameUserSettingsChanged(const UBSGameUserSettings* InGameUserSettings);

	UPROPERTY(EditDefaultsOnly, Category = "Lighting|References")
	TSoftObjectPtr<APostProcessVolume> PostProcessVolume;
};
