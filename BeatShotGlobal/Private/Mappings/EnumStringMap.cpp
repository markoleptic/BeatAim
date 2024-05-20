// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Mappings/EnumStringMap.h"

UEnumStringMap::UEnumStringMap()
{
}

void UEnumStringMap::PostLoad()
{
	for (auto& [Key, Value] : EnumStringMap)
	{
		// Add any Enum Values not present in EnumTagPairs
		for (int64 i = 0; i < Key->GetMaxEnumValue(); i++)
		{
			if (!Value.EnumStringPairs.Find(i))
			{
				const FText EnumValueText = Key->GetDisplayNameTextByValue(i);
				Value.EnumStringPairs.Emplace(i, FEnumStringPair(EnumValueText.ToString(), i));
			}
		}
	}

	Super::PostLoad();
}

const TMap<UEnum*, FEnumStringMapping>& UEnumStringMap::GetEnumStringMap() const
{
	return EnumStringMap;
}

void UEnumStringMap::PopulateEnumTypes(const TSet<UEnum*>& InTypes)
{
	for (UEnum* Enum : InTypes)
	{
		if (!EnumStringMap.Contains(Enum))
		{
			EnumStringMap.Emplace(Enum, FEnumStringMapping(Enum));
		}
	}
}
