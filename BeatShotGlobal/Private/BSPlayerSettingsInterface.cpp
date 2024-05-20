// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "BSPlayerSettingsInterface.h"
#include "SaveGames/SaveGamePlayerSettings.h"
#include "Utilities/SaveLoadCommon.h"

FPlayerSettings IBSPlayerSettingsInterface::LoadPlayerSettings()
{
	if (const USaveGamePlayerSettings* SaveGamePlayerSettings = SaveLoadCommon::LoadFromSlot<USaveGamePlayerSettings>(
		TEXT("SettingsSlot"), 0))
	{
		return SaveGamePlayerSettings->GetPlayerSettings();
	}
	return FPlayerSettings();
}

void IBSPlayerSettingsInterface::SavePlayerSettings(const FPlayerSettings_AudioAnalyzer& InSettingsStruct)
{
	if (USaveGamePlayerSettings* Settings = SaveLoadCommon::LoadFromSlot<USaveGamePlayerSettings>(TEXT("SettingsSlot"),
		0))
	{
		Settings->SavePlayerSettings(InSettingsStruct);
		SaveLoadCommon::SaveToSlot(Settings, TEXT("SettingsSlot"), 0);
		OnPlayerSettingsChangedDelegate_AudioAnalyzer.Broadcast(InSettingsStruct);
	}
}

void IBSPlayerSettingsInterface::SavePlayerSettings(const FPlayerSettings_CrossHair& InSettingsStruct)
{
	if (USaveGamePlayerSettings* Settings = SaveLoadCommon::LoadFromSlot<USaveGamePlayerSettings>(TEXT("SettingsSlot"),
		0))
	{
		Settings->SavePlayerSettings(InSettingsStruct);
		SaveLoadCommon::SaveToSlot(Settings, TEXT("SettingsSlot"), 0);
		OnPlayerSettingsChangedDelegate_CrossHair.Broadcast(InSettingsStruct);
	}
}

void IBSPlayerSettingsInterface::SavePlayerSettings(const FPlayerSettings_Game& InSettingsStruct)
{
	if (USaveGamePlayerSettings* Settings = SaveLoadCommon::LoadFromSlot<USaveGamePlayerSettings>(TEXT("SettingsSlot"),
		0))
	{
		Settings->SavePlayerSettings(InSettingsStruct);
		SaveLoadCommon::SaveToSlot(Settings, TEXT("SettingsSlot"), 0);
		OnPlayerSettingsChangedDelegate_Game.Broadcast(InSettingsStruct);
	}
}

void IBSPlayerSettingsInterface::SavePlayerSettings(const FPlayerSettings_User& InSettingsStruct)
{
	if (USaveGamePlayerSettings* Settings = SaveLoadCommon::LoadFromSlot<USaveGamePlayerSettings>(TEXT("SettingsSlot"),
		0))
	{
		Settings->SavePlayerSettings(InSettingsStruct);
		SaveLoadCommon::SaveToSlot(Settings, TEXT("SettingsSlot"), 0);
		OnPlayerSettingsChangedDelegate_User.Broadcast(InSettingsStruct);
	}
}
