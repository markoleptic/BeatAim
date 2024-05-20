// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "BSGameModeConfig/BSGameModeDataAsset.h"


UBSGameModeDataAsset::UBSGameModeDataAsset()
{
	if (!DefaultGameModes.IsEmpty())
	{
		return;
	}
	for (const EBaseGameMode& GameMode : TEnumRange<EBaseGameMode>())
	{
		for (const EGameModeDifficulty& Difficulty : TEnumRange<EGameModeDifficulty>())
		{
			if (Difficulty == EGameModeDifficulty::None)
			{
				continue;
			}
			DefaultGameModes.Add(FBSConfig::GetConfigForPreset(GameMode, Difficulty), FBSConfig(GameMode, Difficulty));
		}
	}
}

TArray<FBSConfig> UBSGameModeDataAsset::GetDefaultGameModes() const
{
	TArray<FBSConfig> ReturnArray;
	for (const TTuple<FBS_DefiningConfig, FBSConfig>& KeyValue : DefaultGameModes)
	{
		ReturnArray.Add(KeyValue.Value);
	}
	return ReturnArray;
}

const TMap<FBS_DefiningConfig, FBSConfig>& UBSGameModeDataAsset::GetGameModesMap() const
{
	return DefaultGameModes;
}
