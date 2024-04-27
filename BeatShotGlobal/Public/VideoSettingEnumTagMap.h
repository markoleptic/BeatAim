// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnumTagMap.h"
#include "VideoSettingEnumTagMap.generated.h"

UCLASS()
class BEATSHOTGLOBAL_API UVideoSettingEnumTagMap : public UEnumTagMap
{
	GENERATED_BODY()

public:
	UVideoSettingEnumTagMap();

	template <typename T>
	TMap<FString, uint8> GetNvidiaSettingModes(const TArray<T>& NvidiaTypes) const;
};

template <typename T>
TMap<FString, uint8> UVideoSettingEnumTagMap::GetNvidiaSettingModes(const TArray<T>& NvidiaTypes) const
{
	TMap<FString, uint8> Map;
	const FEnumTagMapping* EnumTagMapping = GetEnumTagMapping<T>();

	if (!EnumTagMapping)
	{
		return Map;
	}

	for (const auto Type : NvidiaTypes)
	{
		if (const FEnumTagPair* EnumTagPair = EnumTagMapping->NewEnumTagPairs.Find(static_cast<uint8>(Type)))
		{
			Map.Add(EnumTagPair->DisplayName, EnumTagPair->Index);
		}
	}

	return Map;
}
