﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "SaveLoadInterface.h"
#include "SaveGameCustomGameMode.h"
#include "SaveGamePlayerScore.h"
#include "SaveGamePlayerSettings.h"
#include "Kismet/GameplayStatics.h"

FAASettingsStruct ISaveLoadInterface::LoadAASettings() const
{
	USaveGamePlayerSettings* SaveGamePlayerSettings;
	if (UGameplayStatics::DoesSaveGameExist(TEXT("SettingsSlot"), 0))
	{
		SaveGamePlayerSettings = Cast<USaveGamePlayerSettings>(UGameplayStatics::LoadGameFromSlot(TEXT("SettingsSlot"), 0));
	}
	else
	{
		SaveGamePlayerSettings = Cast<USaveGamePlayerSettings>(UGameplayStatics::CreateSaveGameObject(USaveGamePlayerSettings::StaticClass()));
	}
	return SaveGamePlayerSettings->AASettings;
}

void ISaveLoadInterface::SaveAASettings(const FAASettingsStruct AASettingsToSave)
{
	if (USaveGamePlayerSettings* SaveGameObject = Cast<USaveGamePlayerSettings>(UGameplayStatics::CreateSaveGameObject(USaveGamePlayerSettings::StaticClass())))
	{
		SaveGameObject->PlayerSettings = LoadPlayerSettings();
		SaveGameObject->AASettings = AASettingsToSave;
		UGameplayStatics::SaveGameToSlot(SaveGameObject, TEXT("SettingsSlot"), 0);
	}
}

FPlayerSettings ISaveLoadInterface::LoadPlayerSettings() const
{
	USaveGamePlayerSettings* SaveGamePlayerSettings;
	if (UGameplayStatics::DoesSaveGameExist(TEXT("SettingsSlot"), 0))
	{
		SaveGamePlayerSettings = Cast<USaveGamePlayerSettings>(UGameplayStatics::LoadGameFromSlot(TEXT("SettingsSlot"), 0));
	}
	else
	{
		SaveGamePlayerSettings = Cast<USaveGamePlayerSettings>(UGameplayStatics::CreateSaveGameObject(USaveGamePlayerSettings::StaticClass()));
	}
	return SaveGamePlayerSettings->PlayerSettings;
}

void ISaveLoadInterface::SavePlayerSettings(const FPlayerSettings PlayerSettingsToSave)
{
	if (USaveGamePlayerSettings* SaveGameObject = Cast<USaveGamePlayerSettings>(UGameplayStatics::CreateSaveGameObject(USaveGamePlayerSettings::StaticClass())))
	{
		SaveGameObject->PlayerSettings = PlayerSettingsToSave;
		SaveGameObject->AASettings = LoadAASettings();
		UGameplayStatics::SaveGameToSlot(SaveGameObject, TEXT("SettingsSlot"), 0);
	}
}

void ISaveLoadInterface::SaveCustomGameMode(const TArray<FBSConfig> GameModeArrayToSave)
{
	if (USaveGameCustomGameMode* SaveCustomGameModeObject = Cast<USaveGameCustomGameMode>(UGameplayStatics::CreateSaveGameObject(USaveGameCustomGameMode::StaticClass())))
	{
		SaveCustomGameModeObject->CustomGameModes = GameModeArrayToSave;
		UGameplayStatics::SaveGameToSlot(SaveCustomGameModeObject, TEXT("CustomGameModesSlot"), 3);
	}
}

TArray<FBSConfig> ISaveLoadInterface::LoadCustomGameModes() const
{
	USaveGameCustomGameMode* SaveGameCustomGameMode;
	if (UGameplayStatics::DoesSaveGameExist(TEXT("CustomGameModesSlot"), 3))
	{
		SaveGameCustomGameMode = Cast<USaveGameCustomGameMode>(UGameplayStatics::LoadGameFromSlot(TEXT("CustomGameModesSlot"), 3));
	}
	else
	{
		SaveGameCustomGameMode = Cast<USaveGameCustomGameMode>(UGameplayStatics::CreateSaveGameObject(USaveGameCustomGameMode::StaticClass()));
	}
	if (SaveGameCustomGameMode)
	{
		return SaveGameCustomGameMode->CustomGameModes;
	}
	return TArray<FBSConfig>();
}

TArray<FPlayerScore> ISaveLoadInterface::LoadPlayerScores() const
{
	USaveGamePlayerScore* SaveGamePlayerScore;
	if (UGameplayStatics::DoesSaveGameExist(TEXT("ScoreSlot"), 1))
	{
		SaveGamePlayerScore = Cast<USaveGamePlayerScore>(UGameplayStatics::LoadGameFromSlot(TEXT("ScoreSlot"), 1));
		return SaveGamePlayerScore->PlayerScoreArray;
	}
	SaveGamePlayerScore = Cast<USaveGamePlayerScore>(UGameplayStatics::CreateSaveGameObject(USaveGamePlayerScore::StaticClass()));
	return SaveGamePlayerScore->PlayerScoreArray;
}

void ISaveLoadInterface::SavePlayerScores(const TArray<FPlayerScore> PlayerScoreArrayToSave)
{
	if (USaveGamePlayerScore* SaveGamePlayerScores = Cast<USaveGamePlayerScore>(UGameplayStatics::CreateSaveGameObject(USaveGamePlayerScore::StaticClass())))
	{
		SaveGamePlayerScores->PlayerScoreArray = PlayerScoreArrayToSave;
		if (UGameplayStatics::SaveGameToSlot(SaveGamePlayerScores, TEXT("ScoreSlot"), 1))
		{
			UE_LOG(LogTemp, Warning, TEXT("SavePlayerScores Succeeded"));
		}
	}
}

TArray<FPlayerScore> ISaveLoadInterface::GetMatchingPlayerScores(const FPlayerScore& PlayerScore) const
{
	return LoadPlayerScores().FilterByPredicate([&PlayerScore] (const FPlayerScore& ComparePlayerScore) {
		if (ComparePlayerScore == PlayerScore)
		{
			return true;
		}
		return false;
	});
}

TMap<FBS_DefiningConfig, FCommonScoreInfo> ISaveLoadInterface::LoadCommonScoreInfo() const
{
	USaveGamePlayerScore* SaveGamePlayerScore;
	if (UGameplayStatics::DoesSaveGameExist(TEXT("ScoreSlot"), 1))
	{
		SaveGamePlayerScore = Cast<USaveGamePlayerScore>(UGameplayStatics::LoadGameFromSlot(TEXT("ScoreSlot"), 1));
		return SaveGamePlayerScore->CommonScoreInfo;
	}
	SaveGamePlayerScore = Cast<USaveGamePlayerScore>(UGameplayStatics::CreateSaveGameObject(USaveGamePlayerScore::StaticClass()));
	return SaveGamePlayerScore->CommonScoreInfo;
}

void ISaveLoadInterface::SaveCommonScoreInfo(const TMap<FBS_DefiningConfig, FCommonScoreInfo>& MapToSave)
{
	if (USaveGamePlayerScore* SaveGamePlayerScores = Cast<USaveGamePlayerScore>(UGameplayStatics::CreateSaveGameObject(USaveGamePlayerScore::StaticClass())))
	{
		SaveGamePlayerScores->CommonScoreInfo = MapToSave;
		if (UGameplayStatics::SaveGameToSlot(SaveGamePlayerScores, TEXT("ScoreSlot"), 1))
		{
			UE_LOG(LogTemp, Warning, TEXT("SaveCommonScoreInfo Succeeded"));
		}
	}
}

FCommonScoreInfo ISaveLoadInterface::GetScoreInfoFromDefiningConfig(const FBS_DefiningConfig& DefiningConfig)
{
	TMap<FBS_DefiningConfig, FCommonScoreInfo> Map = LoadCommonScoreInfo();
	if (Map.Find(DefiningConfig) == nullptr)
	{
		return Map.Add(DefiningConfig);
	}
	return *Map.Find(DefiningConfig);
}

void ISaveLoadInterface::UpdateCommonScoreInfo(const FBS_DefiningConfig& DefiningConfig, const FCommonScoreInfo& CommonScoreInfoToSave)
{
	TMap<FBS_DefiningConfig, FCommonScoreInfo> Map = LoadCommonScoreInfo();
	FCommonScoreInfo FoundOrAddedScoreInfo = Map.FindOrAdd(DefiningConfig);
	FoundOrAddedScoreInfo.UpdateCommonValues(CommonScoreInfoToSave.TotalHits, CommonScoreInfoToSave.TotalSpawns);
	SaveCommonScoreInfo(Map);
}
