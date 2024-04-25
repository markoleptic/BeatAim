// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "EnumTagMap.h"
#include "UObject/ObjectSaveContext.h"

UEnumTagMap::UEnumTagMap()
{
}

void UEnumTagMap::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	for (FEnumTagMapping& TagMapping : EnumTagMappings)
	{
		for (FEnumTagPair& TagPair : TagMapping.EnumTagPairs)
		{
			TagPair.AddParentTags(TagMapping.ParentTags);
		}
	}
	Super::PreSave(ObjectSaveContext);
}

void UEnumTagMap::PostLoad()
{
	// Add any Enum Types not present in EnumTagMappings
	for (const UEnum* Enum : EnumsTypes)
	{
		FEnumTagMapping TagMapping(Enum);
		const int32 Index = EnumTagMappings.Find(TagMapping);
		if (Index == INDEX_NONE)
		{
			EnumTagMappings.Add(TagMapping);
		}
		else
		{
			// Add any Enum Values not present in EnumTagPairs
			TArray<FEnumTagPair>& EnumTagPairs = EnumTagMappings[Index].EnumTagPairs;
			for (int64 i = 0; i < Enum->GetMaxEnumValue(); i++)
			{
				if (EnumTagPairs.Find(FEnumTagPair(i)) == INDEX_NONE)
				{
					const FText EnumValueText = Enum->GetDisplayNameTextByValue(i);
					EnumTagPairs.Emplace(EnumValueText.ToString(), i);
				}
			}
		}
	}
	EnumTagMappings.Sort();
	Super::PostLoad();
}

const TArray<FEnumTagMapping>* UEnumTagMap::GetEnumTagMappings() const
{
	return &EnumTagMappings;
}

void UEnumTagMap::PopulateEnumTypes(const TSet<UEnum*>& InTypes)
{
	for (const UEnum* Enum : InTypes)
	{
		EnumTagMappings.AddUnique(Enum);
	}
	EnumTagMappings.Sort();
}
