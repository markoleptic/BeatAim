// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "EnumTagMap.generated.h"

/** Display name and associated GameplayTags for an enum */
USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct FEnumTagPair
{
	GENERATED_BODY()

	/** DisplayName */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, meta = (NoResetToDefault))
	uint8 Index;

	/** Gameplay Tags inherited from the Enum Class */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FGameplayTagContainer ParentTags;

	/** Gameplay Tags associated with the Enum Value */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FGameplayTagContainer Tags;

	FEnumTagPair() = default;

	FEnumTagPair(const FString& InEnumValue, const uint8 InIndex) : DisplayName(InEnumValue), Index(InIndex)
	{
	}

	void AddParentTags(const FGameplayTagContainer& InParentTags)
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

	FORCEINLINE bool operator==(const FEnumTagPair& Other) const
	{
		return Index == Other.Index;
	}
};

/** A collection of FEnumTagPair for a particular enum type */
USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct FEnumTagMapping
{
	GENERATED_BODY()

	/** UEnum static Enum */
	UPROPERTY(BlueprintReadOnly)
	const UEnum* Enum;

	/** String version of the Enum Class */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, meta = (NoResetToDefault))
	FString EnumClass;

	/** Any EnumTagPairs inherit these tags. Must save to show changes in editor */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FGameplayTagContainer ParentTags;

	/** Gameplay Tags associated with an Enum Value */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (TitleProperty="{DisplayName}"))
	TMap<uint8, FEnumTagPair> NewEnumTagPairs;

	FEnumTagMapping() = default;

	explicit FEnumTagMapping(const UEnum* InEnum)
	{
		Enum = InEnum;
		EnumClass = Enum->CppType;
		CreateEnumTagPairs();
	}

	void CreateEnumTagPairs()
	{
		for (int64 i = 0; i < Enum->GetMaxEnumValue(); i++)
		{
			const FText EnumValueText = Enum->GetDisplayNameTextByValue(i);
			NewEnumTagPairs.Emplace(i, FEnumTagPair(EnumValueText.ToString(), i));
		}
	}

	FORCEINLINE bool operator==(const FEnumTagMapping& Other) const
	{
		return Enum == Other.Enum;
	}

	FORCEINLINE bool operator<(const FEnumTagMapping& Other) const
	{
		return EnumClass < Other.EnumClass;
	}
};

/** Since the combo boxes in the custom game mode menu are usually populated with enums,
 *  this data asset allows editing GameplayTags associated with each enum in blueprint.
 *  Enums are populated in the constructor with one line of code. */
UCLASS(Blueprintable, BlueprintType, Const)
class BEATSHOTGLOBAL_API UEnumTagMap : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Adds enums to the EnumTagMappings array */
	UEnumTagMap();

	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
	virtual void PostLoad() override;

	/** Returns the GameplayTags associated with a specific full enum name */
	template <typename T>
	FGameplayTagContainer GetTagsForEnum(const T& InEnum);

	/** Returns the EnumTagMapping associated with a specific enum class,
	 *  which contains an array of FEnumTagPairs for each enum value in the class */
	template <typename T>
	const FEnumTagMapping* GetEnumTagMapping();

	/** Returns a pointer to the entire EnumTagMappings array */
	const TMap<UEnum*, FEnumTagMapping>& GetEnumTagMap() const;

	/** Returns the string associated with a specific full enum name */
	template <typename T>
	FString GetStringFromEnumTagPair(const T& InEnum);

	/** Finds the full enum type from the Display Name */
	template <typename T>
	T FindEnumFromString(const FString& EnumString);

protected:
	void PopulateEnumTypes(const TSet<UEnum*>& InTypes);

	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty="{UEnum}"))
	TMap<UEnum*, FEnumTagMapping> EnumTagMap;

	TSet<UEnum*> EnumTypes;

	template <typename T>
	FEnumTagMapping* GetEditableEnumTagMapping();
};

template <typename T>
FGameplayTagContainer UEnumTagMap::GetTagsForEnum(const T& InEnum)
{
	const FEnumTagMapping* EnumTagMapping = GetEnumTagMapping<T>();
	if (!EnumTagMapping)
	{
		return FGameplayTagContainer();
	}

	if (const auto Found = EnumTagMapping->NewEnumTagPairs.Find(static_cast<uint8>(InEnum)))
	{
		return Found->Tags;
	}

	return FGameplayTagContainer();
}

template <typename T>
const FEnumTagMapping* UEnumTagMap::GetEnumTagMapping()
{
	const UEnum* EnumClass = StaticEnum<T>();
	if (!EnumClass)
	{
		return nullptr;
	}
	return EnumTagMap.Find(EnumClass);
}

template <typename T>
FString UEnumTagMap::GetStringFromEnumTagPair(const T& InEnum)
{
	const FEnumTagMapping* EnumTagMapping = GetEnumTagMapping<T>();
	if (!EnumTagMapping)
	{
		return FString();
	}
	if (const auto Found = EnumTagMapping->NewEnumTagPairs.Find(static_cast<uint8>(InEnum)))
	{
		return Found->DisplayName;
	}

	UE_LOG(LogTemp, Display, TEXT("Didn't find mapping for %d"), static_cast<uint8>(InEnum));
	return FString();
}

template <typename T>
T UEnumTagMap::FindEnumFromString(const FString& EnumString)
{
	if (const FEnumTagMapping* EnumTagMapping = GetEnumTagMapping<T>())
	{
		for (const auto& [Key, Value] : EnumTagMapping->NewEnumTagPairs)
		{
			if (Value.DisplayName.Equals(EnumString))
			{
				return static_cast<T>(Value.Index);
			}
		}
	}
	return static_cast<T>(0);
}

template <typename T>
FEnumTagMapping* UEnumTagMap::GetEditableEnumTagMapping()
{
	const UEnum* EnumClass = StaticEnum<T>();
	if (!EnumClass)
	{
		return nullptr;
	}

	return EnumTagMap.Find(EnumClass);
}
