// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BSGameModeValidator.generated.h"

struct FValidationProperty;
struct FValidationCheck;
struct FValidationCheckResult;
struct FBSConfig;
enum class EGameModeCategory : uint8;
enum class EGameModeWarningType : uint8;

using FValidationPropertyPtr = TSharedPtr<FValidationProperty>;
using FValidationCheckPtr = TSharedPtr<FValidationCheck>;

struct FPropertyHash
{
	FPropertyHash() = default;
	~FPropertyHash() = default;

	TArray<const FProperty*> Properties;

	FORCEINLINE bool operator ==(const FPropertyHash& Other) const
	{
		return Properties == Other.Properties;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FPropertyHash& Object)
	{
		uint32 Hash = 0;
		for (const FProperty* Property : Object.Properties)
		{
			Hash = HashCombineFast(Hash, PointerHash(Property));
		}
		return Hash;
	}
};

DECLARE_DELEGATE_RetVal_OneParam(bool, FValidationDelegate, const TSharedPtr<FBSConfig>&);

UENUM(BlueprintType)
enum class EGameModeWarningType: uint8
{
	None, Caution, Warning, Error
};

UENUM(BlueprintType)
enum class EGameModeCategory : uint8
{
	None, Start, General, SpawnArea, TargetSpawning, TargetActivation, TargetBehavior, TargetMovement, TargetSizing,
	Preview
};

/** A single validation check for a property. */
USTRUCT()
struct BEATSHOTGLOBAL_API FValidationCheck
{
	GENERATED_BODY()

	FValidationCheck(): WarningType(EGameModeWarningType::None)
	{
	}

	explicit FValidationCheck(const TSet<FValidationPropertyPtr>& InProperties,
		const EGameModeWarningType InWarningType) : InvolvedProperties(InProperties), WarningType(InWarningType)
	{
	}

	/** All properties involved in the calculation of the validation result. */
	TSet<FValidationPropertyPtr> InvolvedProperties;

	/** The type of warning associated with the validation check. */
	EGameModeWarningType WarningType;

	/** Must return true in order to validate successfully. */
	TDelegate<bool(const TSharedPtr<FBSConfig>&, TArray<int32>&)> ValidationDelegate;


	FORCEINLINE bool operator ==(const FValidationCheck& Other) const
	{
		return InvolvedProperties.Intersect(Other.InvolvedProperties).Num() == InvolvedProperties.Num() &&
			ValidationDelegate.GetHandle() == Other.ValidationDelegate.GetHandle();
	}

	friend FORCEINLINE uint32 GetTypeHash(const FValidationCheck& Object)
	{
		uint32 Hash = GetTypeHash(Object.ValidationDelegate.GetHandle());
		for (const auto Obj : Object.InvolvedProperties)
		{
			Hash = HashCombineFast(Hash, GetTypeHash(Obj));
		}
		return Hash;
	}
};

/** Specific data associated with an FValidationProperty and an FValidationCheck. */
USTRUCT()
struct BEATSHOTGLOBAL_API FUniqueValidationCheckData
{
	GENERATED_BODY()

	FUniqueValidationCheckData() = default;

	FUniqueValidationCheckData(const FString& InStringTableKey, const FString& InDynamicStringTableKey,
		const uint32 InHash) : StringTableKey(InStringTableKey), DynamicStringTableKey(InDynamicStringTableKey),
		                       WarningType(EGameModeWarningType::None), Hash(InHash)
	{
	}

	/** @return true if there is no relevant data. */
	bool IsEmpty() const;

	/** The string table key to find tooltip text from if validation fails. Also serves as backup if Dynamic. */
	FString StringTableKey;

	/** The dynamic string table key to find tooltip text from if validation fails. */
	FString DynamicStringTableKey;

	/** The tooltip text populated from either one of the string table keys. */
	FText TooltipText;

	/** The type of warning associated with the validation check. */
	EGameModeWarningType WarningType;

	/** Unique has created from the FValidationPropertyPtr and FValidationCheckPtr. */
	uint32 Hash;

	FORCEINLINE bool operator ==(const FUniqueValidationCheckData& Other) const
	{
		return Hash == Other.Hash;
	}

	FORCEINLINE bool operator <(const FUniqueValidationCheckData& Other) const
	{
		return Hash < Other.Hash;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FUniqueValidationCheckData& Object)
	{
		return Object.Hash;
	}
};

/** A property containing any number of validation checks. */
USTRUCT()
struct BEATSHOTGLOBAL_API FValidationProperty
{
	GENERATED_BODY()

	FValidationProperty() = default;

	explicit FValidationProperty(const FPropertyHash& InPropertyHash, const EGameModeCategory InGameModeCategory) :
		Hash(GetTypeHash(InPropertyHash)), GameModeCategory(InGameModeCategory)
	{
		for (const FProperty* Prop : InPropertyHash.Properties)
		{
			PropertyName += Prop->GetFullName() + ".";
		}
	}

	/** Adds a pointer to a validation check involving this property to its stored checks, and adds data unique to this
	 *  property and check to its stored check data.
	 *  @param Check validation check to associate with this property.
	 *  @param Data unique data to associate with this property and the validation check.
	 */
	void AddCheck(const FValidationCheckPtr& Check, const FUniqueValidationCheckData& Data);

	/** Unique has created from the FProperty this Validation Property represents. */
	uint32 Hash;

	/** Full property name constructed from the property's path. */
	FString PropertyName;

	/** The game mode category this property falls under. Used to sync with user interface. */
	EGameModeCategory GameModeCategory;

	/** An array of pointers to all validation checks that involve this property. */
	TSet<FValidationCheckPtr> Checks;

	/** Maps each validation check to unique validation check data for this property. */
	TMap<FValidationCheckPtr, FUniqueValidationCheckData> CheckData;

	FORCEINLINE bool operator ==(const FValidationProperty& Other) const
	{
		return Hash == Other.Hash;
	}

	FORCEINLINE bool operator <(const FValidationProperty& Other) const
	{
		return GameModeCategory < Other.GameModeCategory;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FValidationProperty& Object)
	{
		return Object.Hash;
	}
};

/** Struct used to allow efficient retrieval of validation properties in TSets using FProperty as a key. */
struct FValidationPropertyKeyFuncs : BaseKeyFuncs<FValidationPropertyPtr, uint32, false>
{
	static uint32 GetSetKey(const FValidationPropertyPtr& Element)
	{
		return Element->Hash;
	}

	static bool Matches(const uint32 A, const uint32 B)
	{
		return A == B;
	}

	static uint32 GetKeyHash(const uint32 Key)
	{
		return Key;
	}
};

/** The result from executing a validation check. */
USTRUCT()
struct BEATSHOTGLOBAL_API FValidationCheckResult
{
	GENERATED_BODY()

	/** What the validation function returned. */
	bool bSuccess;

	FValidationCheckPtr ValidationCheckPtr;

	/** The type of warning associated with the validation check. */
	EGameModeWarningType WarningType;

	/** Calculated values from the validation function. */
	TArray<int32> CalculatedValues;

	/** All properties involved in the calculation of this validation check result. */
	TSet<FValidationPropertyPtr> InvolvedProperties;

	/** Maps each involved property to their property data unique to the validation check. */
	TMap<FValidationPropertyPtr, FUniqueValidationCheckData> PropertyData;


	FValidationCheckResult() : bSuccess(false), WarningType(EGameModeWarningType::None)
	{
	}

	FValidationCheckResult(const bool Success, const FValidationCheckPtr& InValidationCheck, TArray<int32>&& Values,
		TMap<FValidationPropertyPtr, FUniqueValidationCheckData>&& Data) : bSuccess(Success),
		                                                                   ValidationCheckPtr(InValidationCheck),
		                                                                   WarningType(InValidationCheck->WarningType),
		                                                                   CalculatedValues(MoveTemp(Values)),
		                                                                   InvolvedProperties(
			                                                                   InValidationCheck->InvolvedProperties),
		                                                                   PropertyData(MoveTemp(Data))
	{
	}

	FORCEINLINE bool operator ==(const FValidationCheckResult& Other) const
	{
		return ValidationCheckPtr == Other.ValidationCheckPtr;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FValidationCheckResult& Object)
	{
		return GetTypeHash(Object.ValidationCheckPtr);
	}
};

/** Struct used to allow efficient retrieval of FValidationCheckResults in TSets using FValidationCheckPtr as a key. */
struct FValidationCheckKeyFuncs : BaseKeyFuncs<FValidationCheckResult, FValidationCheckPtr, false>
{
	static FValidationCheckPtr GetSetKey(const FValidationCheckResult& Element)
	{
		return Element.ValidationCheckPtr;
	}

	static bool Matches(const FValidationCheckPtr& A, const FValidationCheckPtr& B)
	{
		return A == B;
	}

	static uint32 GetKeyHash(const FValidationCheckPtr& Key)
	{
		return GetTypeHash(Key);
	}
};

USTRUCT()
struct BEATSHOTGLOBAL_API FValidationResult
{
	GENERATED_BODY()

	FValidationResult()
	{
	}

	void AddValidationCheckResult(FValidationCheckResult&& Check);

	TSet<FValidationCheckResult, FValidationCheckKeyFuncs> GetSucceeded() const;
	TSet<FValidationCheckResult, FValidationCheckKeyFuncs> GetFailed() const;
	bool Contains(const FValidationCheckPtr& Check) const;

private:
	TSet<FValidationCheckResult, FValidationCheckKeyFuncs> SucceededValidationCheckResults;
	TSet<FValidationCheckResult, FValidationCheckKeyFuncs> FailedValidationCheckResults;
};

/** Validates a BSConfig. */
UCLASS()
class BEATSHOTGLOBAL_API UBSGameModeValidator : public UObject
{
	GENERATED_BODY()

public:
	UBSGameModeValidator();

	/** Performs all validation checks against the config.
	 *  @param InConfig the config to validate.
	 *	@return a validation result container object.
	 */
	FValidationResult Validate(const TSharedPtr<FBSConfig>& InConfig) const;

	/** Performs validation checks against one property.
	 *  @param InConfig the config containing the substruct and property.
	 *  @param SubStructName the substruct name within FBSConfig.
	 *  @param PropertyName the substruct property name.
	 *	@return a validation result container object
	 */
	FValidationResult Validate(const TSharedPtr<FBSConfig>& InConfig, FName SubStructName, FName PropertyName) const;

	/** Performs validation checks against a set of properties.
	 *  @param InConfig the config containing the substruct and property.
	 *  @param Properties a set of specific properties to validate.
	 *	@return a validation result container object
	 */
	FValidationResult Validate(const TSharedPtr<FBSConfig>& InConfig, const TSet<uint32>& Properties) const;

	/** Finds a property in BSConfig.
	 *  @param SubStructName name of the inner struct.
	 *  @param PropertyName name of property to find.
	 *	@return a property if found, otherwise null.
	 */
	static uint32 FindBSConfigProperty(const FName SubStructName, const FName PropertyName);

	/** Finds a property in BSConfig.
	 *  @param SubStructName name of the inner struct.
	 *  @param SubSubStructName name of the inner inner struct.
	 *  @param PropertyName name of property to find.
	 *	@return a property if found, otherwise null.
	 */
	static uint32 FindBSConfigProperty(const FName SubStructName, const FName SubSubStructName,
		const FName PropertyName);

	/** Finds a validation property based on a BSConfig FProperty.
	 *  @param PropertyHash the property to look for.
	 *	@return a validation property pointer if found, otherwise null.
	 */
	FValidationPropertyPtr FindValidationProperty(uint32 PropertyHash) const;

	/** @return All validation properties. */
	TSet<FValidationPropertyPtr, FValidationPropertyKeyFuncs>& GetValidationProperties() const;

private:
	class FPrivate;
	TPimplPtr<FPrivate> Impl;
};
