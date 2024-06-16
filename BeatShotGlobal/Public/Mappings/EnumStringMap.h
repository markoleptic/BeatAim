// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnumStringMap.generated.h"

/** Display name for an enum. */
USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct BEATSHOTGLOBAL_API FEnumStringPair
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, meta = (NoResetToDefault))
	uint8 Index;

	FEnumStringPair(): Index(MAX_uint8)
	{
	}

	FEnumStringPair(const FString& InEnumValue, const uint8 InIndex) : DisplayName(InEnumValue), Index(InIndex)
	{
	}

	FORCEINLINE bool operator==(const FEnumStringPair& Other) const
	{
		return Index == Other.Index;
	}
};

/** A collection of FEnumStringPair for a particular enum type. */
USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct BEATSHOTGLOBAL_API FEnumStringMapping
{
	GENERATED_BODY()

	/** UEnum static Enum. */
	UPROPERTY(BlueprintReadOnly)
	const UEnum* Enum;

	/** String version of the Enum Class. */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, meta = (NoResetToDefault))
	FString EnumClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (TitleProperty="{DisplayName}"))
	TMap<uint8, FEnumStringPair> EnumStringPairs;

	FEnumStringMapping(): Enum(nullptr)
	{
	}

	virtual ~FEnumStringMapping() = default;

	explicit FEnumStringMapping(const UEnum* InEnum)
	{
		Enum = InEnum;
		EnumClass = Enum->CppType;
		for (int64 i = 0; i < Enum->GetMaxEnumValue(); i++)
		{
			const FText EnumValueText = Enum->GetDisplayNameTextByValue(i);
			EnumStringPairs.Emplace(i, FEnumStringPair(EnumValueText.ToString(), i));
		}
	}

	FORCEINLINE bool operator==(const FEnumStringMapping& Other) const
	{
		return Enum == Other.Enum;
	}

	FORCEINLINE bool operator<(const FEnumStringMapping& Other) const
	{
		return EnumClass < Other.EnumClass;
	}
};

UCLASS()
class BEATSHOTGLOBAL_API UEnumStringMap : public UDataAsset
{
	GENERATED_BODY()

public:
	UEnumStringMap();

	virtual void PostLoad() override;

	/** Returns a pointer to the entire EnumStringMap. */
	const TMap<UEnum*, FEnumStringMapping>& GetEnumStringMap() const;

	/** Returns the EnumStringMapping associated with a specific enum class. */
	template <typename T>
	const FEnumStringMapping* GetEnumStringMapping() const;

	/** Returns the string associated with a specific full enum name. */
	template <typename T>
	FString FindStringFromEnum(const T& InEnum) const;

	/** Finds the full enum type from the Display Name. */
	template <typename T>
	T FindEnumFromString(const FString& EnumString) const;

protected:
	void PopulateEnumTypes(const TSet<UEnum*>& InTypes);

	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty="{UEnum}"))
	TMap<UEnum*, FEnumStringMapping> EnumStringMap;
};

template <typename T>
const FEnumStringMapping* UEnumStringMap::GetEnumStringMapping() const
{
	const UEnum* EnumClass = StaticEnum<T>();
	if (!EnumClass)
	{
		return nullptr;
	}
	return EnumStringMap.Find(EnumClass);
}

template <typename T>
FString UEnumStringMap::FindStringFromEnum(const T& InEnum) const
{
	const FEnumStringMapping* EnumTagMapping = GetEnumStringMapping<T>();
	if (!EnumTagMapping)
	{
		return FString();
	}
	if (const auto Found = EnumTagMapping->EnumStringPairs.Find(static_cast<uint8>(InEnum)))
	{
		return Found->DisplayName;
	}

	UE_LOG(LogTemp, Display, TEXT("Didn't find mapping for %d"), static_cast<uint8>(InEnum));
	return FString();
}

template <typename T>
T UEnumStringMap::FindEnumFromString(const FString& EnumString) const
{
	if (const FEnumStringMapping* EnumTagMapping = GetEnumStringMapping<T>())
	{
		for (const auto& [Key, Value] : EnumTagMapping->EnumStringPairs)
		{
			if (Value.DisplayName.Equals(EnumString))
			{
				return static_cast<T>(Value.Index);
			}
		}
	}
	return static_cast<T>(0);
}
