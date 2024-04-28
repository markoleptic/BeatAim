// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "RangeActors/RangeLevelScriptActor.h"
#include "BSGameUserSettings.h"
#include "Engine/PostProcessVolume.h"

ARangeLevelScriptActor::ARangeLevelScriptActor()
{
}

void ARangeLevelScriptActor::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		return;
	}

	const auto GameUserSettings = UBSGameUserSettings::Get();
	HandleGameUserSettingsChanged(GameUserSettings);
	GameUserSettings->OnSettingsChanged.AddUObject(this, &ARangeLevelScriptActor::HandleGameUserSettingsChanged);
}

void ARangeLevelScriptActor::HandleGameUserSettingsChanged(const UBSGameUserSettings* InGameUserSettings)
{
	if (PostProcessVolume)
	{
		PostProcessVolume->Settings.bOverride_AutoExposureBias = true;
		PostProcessVolume->Settings.AutoExposureBias = InGameUserSettings->GetPostProcessBiasFromBrightness();
	}
}
