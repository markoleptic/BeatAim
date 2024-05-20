// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

class USaveGamePlayerScore;
class USaveGamePlayerSettings;
class USaveGameCustomGameMode;

namespace SaveLoadCommon
{
	template <typename T>
	T* LoadFromSlot(const FString& InSlotName, const int32 InSlotIndex);

	template <typename T>
	bool SaveToSlot(T* SaveGameClass, const FString& InSlotName, const int32 InSlotIndex);
}

template bool SaveLoadCommon::SaveToSlot(USaveGameCustomGameMode* SaveGameClass, const FString& InSlotName,
	const int32 InSlotIndex);
template bool SaveLoadCommon::SaveToSlot(USaveGamePlayerSettings* SaveGameClass, const FString& InSlotName,
	const int32 InSlotIndex);
template bool SaveLoadCommon::SaveToSlot(USaveGamePlayerScore* SaveGameClass, const FString& InSlotName,
	const int32 InSlotIndex);

template <>
USaveGameCustomGameMode* SaveLoadCommon::LoadFromSlot(const FString& InSlotName, const int32 InSlotIndex);

template USaveGamePlayerSettings* SaveLoadCommon::LoadFromSlot(const FString& InSlotName, const int32 InSlotIndex);

template USaveGamePlayerScore* SaveLoadCommon::LoadFromSlot(const FString& InSlotName, const int32 InSlotIndex);
