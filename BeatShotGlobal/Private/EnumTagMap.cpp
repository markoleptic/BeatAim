// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "EnumTagMap.h"
#include "UObject/ObjectSaveContext.h"

UEnumTagMap::UEnumTagMap()
{
}

void UEnumTagMap::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	for (auto& [Enum, EnumTageMapping] : EnumTagMap)
	{
		for (auto& [Index, EnumTagPair] : EnumTageMapping.NewEnumTagPairs)
		{
			EnumTagPair.AddParentTags(EnumTageMapping.ParentTags);
		}
	}

	Super::PreSave(ObjectSaveContext);
}

void UEnumTagMap::PostLoad()
{
	// Add any Enum Types not present in EnumTagMappings
	for (UEnum* Enum : EnumTypes)
	{
		FEnumTagMapping* Found = EnumTagMap.Find(Enum);
		if (!Found)
		{
			EnumTagMap.Emplace(Enum, FEnumTagMapping(Enum));
		}

		// Add any Enum Values not present in EnumTagPairs
		for (int64 i = 0; i < Enum->GetMaxEnumValue(); i++)
		{
			if (!Found->NewEnumTagPairs.Find(i))
			{
				const FText EnumValueText = Enum->GetDisplayNameTextByValue(i);
				Found->NewEnumTagPairs.Emplace(i, FEnumTagPair(EnumValueText.ToString(), i));
			}
		}
	}

	Super::PostLoad();
}

const TMap<UEnum*, FEnumTagMapping>& UEnumTagMap::GetEnumTagMap() const
{
	return EnumTagMap;
}

void UEnumTagMap::PopulateEnumTypes(const TSet<UEnum*>& InTypes)
{
	for (UEnum* Enum : InTypes)
	{
		if (!EnumTagMap.Contains(Enum))
		{
			EnumTagMap.Emplace(Enum, FEnumTagMapping(Enum));
		}
	}
}
