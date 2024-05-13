// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "BSGameModeInterface.h"
#include "SaveGameCustomGameMode.h"
#include "SaveGamePlayerScore.h"
#include "SaveLoadCommon.h"

using namespace SaveLoadCommon;

/* --------------------------- */
/* ---- Custom Game Modes ---- */
/* --------------------------- */

TArray<FBSConfig> IBSGameModeInterface::LoadCustomGameModes()
{
	if (const USaveGameCustomGameMode* SaveGameCustomGameMode = LoadFromSlot<USaveGameCustomGameMode>(
		TEXT("CustomGameModesSlot"), 3))
	{
		return SaveGameCustomGameMode->GetCustomGameModes();
	}
	return TArray<FBSConfig>();
}

bool IBSGameModeInterface::FindCustomGameMode(const FString& CustomGameModeName, FBSConfig& OutConfig)
{
	if (const USaveGameCustomGameMode* SaveGameCustomGameMode = LoadFromSlot<USaveGameCustomGameMode>(
		TEXT("CustomGameModesSlot"), 3))
	{
		return SaveGameCustomGameMode->FindCustomGameMode(CustomGameModeName, OutConfig);
	}
	return false;
}

void IBSGameModeInterface::SaveCustomGameMode(const FBSConfig& ConfigToSave)
{
	if (USaveGameCustomGameMode* SaveGameCustomGameMode = LoadFromSlot<USaveGameCustomGameMode>(
		TEXT("CustomGameModesSlot"), 3))
	{
		SaveGameCustomGameMode->SaveCustomGameMode(ConfigToSave);
		SaveToSlot(SaveGameCustomGameMode, TEXT("CustomGameModesSlot"), 3);
	}
}

int32 IBSGameModeInterface::RemoveCustomGameMode(const FBSConfig& ConfigToRemove)
{
	int32 NumCustomGameModesRemoved = 0;
	if (USaveGameCustomGameMode* SaveGameCustomGameMode = LoadFromSlot<USaveGameCustomGameMode>(
		TEXT("CustomGameModesSlot"), 3))
	{
		NumCustomGameModesRemoved = SaveGameCustomGameMode->RemoveCustomGameMode(ConfigToRemove);
		SaveToSlot(SaveGameCustomGameMode, TEXT("CustomGameModesSlot"), 3);
	}
	if (USaveGamePlayerScore* SaveGamePlayerScore = LoadFromSlot<USaveGamePlayerScore>(TEXT("ScoreSlot"), 1))
	{
		const int32 NumCommonScoreInfosRemoved = SaveGamePlayerScore->RemoveCommonScoreInfo(
			ConfigToRemove.DefiningConfig);
		UE_LOG(LogTemp, Display, TEXT("%d Common Score Infos removed when removing a custom game mode."),
			NumCommonScoreInfosRemoved);
		SaveToSlot(SaveGamePlayerScore, TEXT("ScoreSlot"), 1);
	}
	return NumCustomGameModesRemoved;
}

int32 IBSGameModeInterface::RemoveAllCustomGameModes()
{
	int32 NumCustomGameModesRemoved = 0;
	if (USaveGameCustomGameMode* SaveGameCustomGameMode = LoadFromSlot<USaveGameCustomGameMode>(
		TEXT("CustomGameModesSlot"), 3))
	{
		NumCustomGameModesRemoved = SaveGameCustomGameMode->RemoveAll();
		SaveToSlot(SaveGameCustomGameMode, TEXT("CustomGameModesSlot"), 3);
	}
	if (USaveGamePlayerScore* SaveGamePlayerScore = LoadFromSlot<USaveGamePlayerScore>(TEXT("ScoreSlot"), 1))
	{
		const int32 NumCommonScoreInfosRemoved = SaveGamePlayerScore->RemoveAllCustomGameModeCommonScoreInfo();
		UE_LOG(LogTemp, Display, TEXT("%d Common Score Infos removed when removing all custom game modes."),
			NumCommonScoreInfosRemoved);
		SaveToSlot(SaveGamePlayerScore, TEXT("ScoreSlot"), 1);
	}
	return NumCustomGameModesRemoved;
}

bool IBSGameModeInterface::IsCustomGameMode(const FString& GameModeName)
{
	if (const USaveGameCustomGameMode* SaveGameCustomGameMode = LoadFromSlot<USaveGameCustomGameMode>(
		TEXT("CustomGameModesSlot"), 3))
	{
		return SaveGameCustomGameMode->IsCustomGameMode(GameModeName);
	}
	return false;
}

bool IBSGameModeInterface::DoesCustomGameModeMatchConfig(const FString& CustomGameModeName, const FBSConfig& InConfig)
{
	if (!IsCustomGameMode(CustomGameModeName))
	{
		return false;
	}
	FBSConfig FoundCustom;
	if (!FindCustomGameMode(CustomGameModeName, FoundCustom))
	{
		return false;
	}
	if (FoundCustom.AIConfig == InConfig.AIConfig && FoundCustom.GridConfig == InConfig.GridConfig && FoundCustom.
		TargetConfig == InConfig.TargetConfig && FoundCustom.DynamicTargetScaling == InConfig.DynamicTargetScaling &&
		FoundCustom.DynamicSpawnAreaScaling == InConfig.DynamicSpawnAreaScaling)
	{
		return true;
	}
	return false;
}

bool IBSGameModeInterface::ImportCustomGameMode(const FString& InSerializedJsonString, FBSConfig& OutConfig,
	FText& OutFailureReason)
{
	if (!FBSConfig::DecodeFromString(InSerializedJsonString, OutConfig, &OutFailureReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to import custom game mode: %s"), *OutFailureReason.ToString());
		OutFailureReason = FText::FromString("Invalid import string");
		return false;
	}

	if (IsPresetGameMode(OutConfig.DefiningConfig.CustomGameModeName) || OutConfig.DefiningConfig.GameModeType ==
		EGameModeType::Preset)
	{
		OutFailureReason = FText::FromString("Default Game Modes cannot be imported");
		return false;
	}

	if (OutConfig.DefiningConfig.CustomGameModeName.IsEmpty())
	{
		OutFailureReason = FText::FromString("Failed to import game mode with empty CustomGameModeName");
		return false;
	}

	if (IsCustomGameMode(OutConfig.DefiningConfig.CustomGameModeName))
	{
		OutFailureReason = FText::FromString("Existing");
		return false;
	}

	return true;
}

FString IBSGameModeInterface::ExportCustomGameMode(const FBSConfig& InConfig)
{
	return InConfig.EncodeToString();
}

/* --------------------------- */
/* ---- Preset Game Modes ---- */
/* --------------------------- */

bool IBSGameModeInterface::FindPresetGameMode(const FString& GameModeName, const EGameModeDifficulty& Difficulty,
	const UBSGameModeDataAsset* PresetGameModeDataAsset, FBSConfig& OutConfig)
{
	EBaseGameMode BaseGameMode = EBaseGameMode::None;

	for (const EBaseGameMode& Preset : TEnumRange<EBaseGameMode>())
	{
		if (GameModeName.Equals(UEnum::GetDisplayValueAsText(Preset).ToString(), ESearchCase::IgnoreCase))
		{
			BaseGameMode = Preset;
			break;
		}
	}

	if (BaseGameMode == EBaseGameMode::None) return false;

	return FindPresetGameMode(BaseGameMode, Difficulty, PresetGameModeDataAsset, OutConfig);
}

bool IBSGameModeInterface::FindPresetGameMode(const EBaseGameMode& BaseGameMode, const EGameModeDifficulty& Difficulty,
	const UBSGameModeDataAsset* PresetGameModeDataAsset, FBSConfig& OutConfig)
{
	if (BaseGameMode != EBaseGameMode::None)
	{
		const FBS_DefiningConfig DefiningConfig = FBSConfig::GetConfigForPreset(BaseGameMode, Difficulty);
		const TMap<FBS_DefiningConfig, FBSConfig> Map = PresetGameModeDataAsset->GetDefaultGameModesMap();
		if (const FBSConfig* Found = Map.Find(DefiningConfig))
		{
			OutConfig = *Found;
			return true;
		}
	}

	return false;
}

bool IBSGameModeInterface::IsPresetGameMode(const FString& GameModeName)
{
	for (const EBaseGameMode& Preset : TEnumRange<EBaseGameMode>())
	{
		if (GameModeName.Equals(UEnum::GetDisplayValueAsText(Preset).ToString(), ESearchCase::IgnoreCase))
		{
			return true;
		}
	}
	return false;
}
