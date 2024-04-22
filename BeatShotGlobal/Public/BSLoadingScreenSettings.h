// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "UObject/SoftObjectPtr.h"
#include "Engine/DeveloperSettings.h"
#include "BSLoadingScreenSettings.generated.h"

class UObject;
class USlateWidgetStyleAsset;

/** Loading screen settings that are editable within the Unreal Editor. */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "BeatShotLoadingScreenSettings"))
class BEATSHOTGLOBAL_API UBSLoadingScreenSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** The style of the loading screen. */
	UPROPERTY(config, EditAnywhere)
	TSoftObjectPtr<USlateWidgetStyleAsset> LoadingScreenStyle;

	/** The minimum duration to show the loading screen for. */
	UPROPERTY(config, EditAnywhere)
	float MinimumLoadingScreenDisplayTime = 2.f;

	/** The time the screen fade widget plays its animations for. */
	UPROPERTY(config, EditAnywhere)
	float ScreenFadeWidgetAnimationDuration = 0.75f;
};
