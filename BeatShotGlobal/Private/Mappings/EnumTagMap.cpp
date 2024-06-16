// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Mappings/EnumTagMap.h"
#include "UObject/ObjectSaveContext.h"

FEnumTagPair::FEnumTagPair()
{
}

FEnumTagPair::FEnumTagPair(const FString& InEnumValue, const uint8 InIndex): FEnumStringPair(InEnumValue, InIndex)
{
}

void FEnumTagPair::AddParentTags(const FGameplayTagContainer& InParentTags)
{
	ParentTags.AppendTags(InParentTags);

	for (const FGameplayTag& Tag : InParentTags)
	{
		if (Tags.HasTagExact(Tag))
		{
			Tags.RemoveTag(Tag);
		}
	}
}

FEnumTagMapping::FEnumTagMapping(): Enum(nullptr)
{
}

FEnumTagMapping::FEnumTagMapping(const UEnum* InEnum)
{
	Enum = InEnum;
	EnumClass = Enum->CppType;
	for (int64 i = 0; i < Enum->GetMaxEnumValue(); i++)
	{
		const FText EnumValueText = Enum->GetDisplayNameTextByValue(i);
		EnumTagPairs.Emplace(i, FEnumTagPair(EnumValueText.ToString(), i));
	}
}

UEnumTagMap::UEnumTagMap()
{
}

void UEnumTagMap::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	for (auto& [Enum, EnumTageMapping] : EnumTagMap)
	{
		for (auto& [Index, EnumTagPair] : EnumTageMapping.EnumTagPairs)
		{
			EnumTagPair.AddParentTags(EnumTageMapping.ParentTags);
		}
	}

	Super::PreSave(ObjectSaveContext);
}

void UEnumTagMap::PostLoad()
{
	for (auto& [Key, Value] : EnumTagMap)
	{
		// Add any Enum Values not present in EnumTagPairs
		for (int64 i = 0; i < Key->GetMaxEnumValue(); i++)
		{
			if (!Value.EnumTagPairs.Find(i))
			{
				const FText EnumValueText = Key->GetDisplayNameTextByValue(i);
				Value.EnumTagPairs.Emplace(i, FEnumTagPair(EnumValueText.ToString(), i));
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
