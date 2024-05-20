// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "Utilities/SaveLoadCommon.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGames/SaveGameCustomGameMode.h"
#include "SaveGames/SaveGamePlayerScore.h"
#include "SaveGames/SaveGamePlayerSettings.h"

template <typename T>
T* SaveLoadCommon::LoadFromSlot(const FString& InSlotName, const int32 InSlotIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(InSlotName, InSlotIndex))
	{
		if (T* SaveGameObject = Cast<T>(UGameplayStatics::LoadGameFromSlot(InSlotName, InSlotIndex)))
		{
			return SaveGameObject;
		}
	}
	else
	{
		if (T* SaveGameObject = Cast<T>(UGameplayStatics::CreateSaveGameObject(T::StaticClass())))
		{
			return SaveGameObject;
		}
	}
	return nullptr;
}

template <typename T>
bool SaveLoadCommon::SaveToSlot(T* SaveGameClass, const FString& InSlotName, const int32 InSlotIndex)
{
	if (UGameplayStatics::SaveGameToSlot(SaveGameClass, InSlotName, InSlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Save Succeeded"));
		return true;
	}
	return false;
}

template <>
USaveGameCustomGameMode* SaveLoadCommon::LoadFromSlot(const FString& InSlotName, const int32 InSlotIndex)
{
	USaveGameCustomGameMode* SaveGameObject;
	if (UGameplayStatics::DoesSaveGameExist(InSlotName, InSlotIndex))
	{
		SaveGameObject = Cast<USaveGameCustomGameMode>(UGameplayStatics::LoadGameFromSlot(InSlotName, InSlotIndex));
	}
	else
	{
		SaveGameObject = Cast<USaveGameCustomGameMode>(
			UGameplayStatics::CreateSaveGameObject(USaveGameCustomGameMode::StaticClass()));
	}

	if (!SaveGameObject)
	{
		return nullptr;
	}

	if (SaveGameObject->GetLastLoadedVersion() < Constants::CustomGameModeVersion)
	{
		const int32 Old = SaveGameObject->GetLastLoadedVersion();
		SaveGameObject->UpgradeCustomGameModes();
		SaveToSlot(SaveGameObject, InSlotName, InSlotIndex);
		const int32 New = SaveGameObject->GetVersion();
		UE_LOG(LogTemp, Warning, TEXT("Upgraded USaveGameCustomGameMode from Version %d to %d"), Old, New);
	}
	return SaveGameObject;
}
