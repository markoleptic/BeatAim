// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mappings/EnumStringMap.h"
#include "VideoSettingEnumStringMap.generated.h"

UCLASS()
class BEATSHOTGLOBAL_API UVideoSettingEnumStringMap : public UEnumStringMap
{
	GENERATED_BODY()

public:
	UVideoSettingEnumStringMap();

	template <typename T>
	TMap<FString, uint8> GetNvidiaSettingModes(const TArray<T>& NvidiaTypes) const;
};

template <typename T>
TMap<FString, uint8> UVideoSettingEnumStringMap::GetNvidiaSettingModes(const TArray<T>& NvidiaTypes) const
{
	TMap<FString, uint8> Map;
	const FEnumStringMapping* EnumStringMapping = GetEnumStringMapping<T>();

	if (!EnumStringMapping)
	{
		return Map;
	}

	for (const auto Type : NvidiaTypes)
	{
		if (const FEnumStringPair* EnumTagPair = EnumStringMapping->EnumStringPairs.Find(static_cast<uint8>(Type)))
		{
			Map.Add(EnumTagPair->DisplayName, EnumTagPair->Index);
		}
	}

	return Map;
}
