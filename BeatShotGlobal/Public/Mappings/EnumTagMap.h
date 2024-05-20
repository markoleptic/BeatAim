// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnumStringMap.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "EnumTagMap.generated.h"

/** Display name and associated GameplayTags for an enum. */
USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct BEATSHOTGLOBAL_API FEnumTagPair : public FEnumStringPair
{
	GENERATED_BODY()

	/** Gameplay Tags inherited from the Enum Class. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FGameplayTagContainer ParentTags;

	/** Gameplay Tags associated with the Enum Value. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FGameplayTagContainer Tags;

	FEnumTagPair();

	FEnumTagPair(const FString& InEnumValue, const uint8 InIndex);

	void AddParentTags(const FGameplayTagContainer& InParentTags);

	FORCEINLINE bool operator==(const FEnumTagPair& Other) const
	{
		return Index == Other.Index;
	}
};

/** A collection of FEnumTagPair for a particular enum type. */
USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct BEATSHOTGLOBAL_API FEnumTagMapping
{
	GENERATED_BODY()

	/** UEnum static Enum. */
	UPROPERTY(BlueprintReadOnly)
	const UEnum* Enum;

	/** String version of the Enum Class. */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, meta = (NoResetToDefault))
	FString EnumClass;

	/** Any EnumTagPairs inherit these tags. Must save to show changes in editor. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FGameplayTagContainer ParentTags;

	/** Gameplay Tags and Display Names associated with an Enum Value. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (TitleProperty="{DisplayName}"))
	TMap<uint8, FEnumTagPair> EnumTagPairs;

	FEnumTagMapping() = default;

	explicit FEnumTagMapping(const UEnum* InEnum);

	FORCEINLINE bool operator==(const FEnumTagMapping& Other) const
	{
		return Enum == Other.Enum;
	}

	FORCEINLINE bool operator<(const FEnumTagMapping& Other) const
	{
		return EnumClass < Other.EnumClass;
	}
};

/** Since the combo boxes in the custom game mode menu are usually populated with enums, this data asset allows editing
 *  GameplayTags associated with each enum in blueprint. */
UCLASS(Blueprintable, BlueprintType, Const)
class BEATSHOTGLOBAL_API UEnumTagMap : public UDataAsset
{
	GENERATED_BODY()

public:
	UEnumTagMap();

	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
	virtual void PostLoad() override;

	/** Returns a pointer to the entire EnumTagMappings array. */
	const TMap<UEnum*, FEnumTagMapping>& GetEnumTagMap() const;

	/** Returns the EnumTagMapping associated with a specific enum class. */
	template <typename T>
	const FEnumTagMapping* GetEnumTagMapping() const;

	/** Returns the string associated with a specific full enum name. */
	template <typename T>
	FString FindStringFromEnum(const T& InEnum) const;

	/** Finds the full enum type from the Display Name. */
	template <typename T>
	T FindEnumFromString(const FString& EnumString) const;

protected:
	void PopulateEnumTypes(const TSet<UEnum*>& InTypes);

	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty="{EnumClass}"))
	TMap<UEnum*, FEnumTagMapping> EnumTagMap;
};

template <typename T>
const FEnumTagMapping* UEnumTagMap::GetEnumTagMapping() const
{
	const UEnum* EnumClass = StaticEnum<T>();
	if (!EnumClass)
	{
		return nullptr;
	}
	return EnumTagMap.Find(EnumClass);
}

template <typename T>
FString UEnumTagMap::FindStringFromEnum(const T& InEnum) const
{
	const FEnumTagMapping* EnumTagMapping = GetEnumTagMapping<T>();
	if (!EnumTagMapping)
	{
		return FString();
	}
	if (const auto Found = EnumTagMapping->EnumTagPairs.Find(static_cast<uint8>(InEnum)))
	{
		return Found->DisplayName;
	}

	UE_LOG(LogTemp, Display, TEXT("Didn't find mapping for %d"), static_cast<uint8>(InEnum));
	return FString();
}

template <typename T>
T UEnumTagMap::FindEnumFromString(const FString& EnumString) const
{
	if (const FEnumTagMapping* EnumTagMapping = GetEnumTagMapping<T>())
	{
		for (const auto& [Key, Value] : EnumTagMapping->EnumTagPairs)
		{
			if (Value.DisplayName.Equals(EnumString))
			{
				return static_cast<T>(Value.Index);
			}
		}
	}
	return static_cast<T>(0);
}
