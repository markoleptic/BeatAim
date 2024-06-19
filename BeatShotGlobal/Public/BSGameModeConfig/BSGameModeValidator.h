// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BSGameModeValidator.generated.h"

struct FValidationProperty;
struct FValidationCheck;
struct FValidationCheckResult;
struct FBSConfig;
struct FValidationPropertyKeyFuncs;
struct FValidationCheckResultKeyFuncs;
enum class EGameModeCategory : uint8;
enum class EGameModeWarningType : uint8;

using FValidationPropertyPtr = TSharedPtr<FValidationProperty>;
using FValidationCheckPtr = TSharedPtr<FValidationCheck>;
using FValidationPropertySet = TSet<TSharedPtr<FValidationProperty>, FValidationPropertyKeyFuncs>;
using FValidationCheckResultSet = TSet<FValidationCheckResult, FValidationCheckResultKeyFuncs>;
using FValidationCheckSet = TSet<TSharedPtr<FValidationCheck>>;

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


/** Specific data associated with a FValidationCheck. */
USTRUCT()
struct BEATSHOTGLOBAL_API FValidationCheckData
{
	GENERATED_BODY()

	FValidationCheckData();

	explicit FValidationCheckData(EGameModeWarningType GameModeWarningType);

	FValidationCheckData(EGameModeWarningType GameModeWarningType, const FString& InStringTableKey,
		const FString& InDynamicStringTableKey, int32 InGridSnapSize, bool InCalculatedValuesAreIntegers);

	void ResetLiveData();

	/** @return true if there is no relevant data. */
	bool IsEmpty() const;

	/** The string table key to find tooltip text from if validation fails. Also serves as backup if Dynamic. */
	FString StringTableKey;

	/** The dynamic string table key to find tooltip text from if validation fails. */
	FString DynamicStringTableKey;

	/** The tooltip text populated from StringTableKey as a fallback. */
	FText FallbackTooltipText;

	/** The tooltip text populated from either one of the string table keys. */
	FText TooltipText;

	/** The type of warning associated with the validation check. */
	EGameModeWarningType WarningType;

	/** Snap size for calculated values. If zero, no grid snapping occurs. Always snaps away from zero. */
	int32 GridSnapSize;

	/** Whether the calculated results should be treated as integers. */
	bool bCalculatedValuesAreIntegers;

	/** How to format number arguments for formatted text. */
	FNumberFormattingOptions NumberFormattingOptions;

	/** Values calculated during the validation check. */
	TArray<float> CalculatedValues;

	/** Whether the calculated value(s) are not valid, and another property must also be modified to validate. */
	bool bRequireOtherPropertiesToBeChanged;
};

/** A single validation check for a property. */
USTRUCT()
struct BEATSHOTGLOBAL_API FValidationCheck
{
	GENERATED_BODY()

	FValidationCheck();

	explicit FValidationCheck(uint32 InOwningPropertyHash, EGameModeWarningType GameModeWarningType);

	void AddData(const FString& InStringTableKey, const FString& InDynamicStringTableKey, int32 InGridSnapSize,
		bool InCalculatedValuesAreIntegers);

	FValidationCheckData ValidationCheckData;

	/** The prerequisite must return true in order for the validation check to be executed, otherwise validation is
	 *  skipped and marked as passing. */
	TDelegate<bool(const TSharedPtr<FBSConfig>&)> ValidationPrerequisiteDelegate;

	/** Must return true in order to validate successfully. */
	TDelegate<bool(const TSharedPtr<FBSConfig>&, FValidationCheckData&)> ValidationDelegate;

	/** The hash of the property associated with this validation check. */
	uint32 OwningPropertyHash;

	FORCEINLINE bool operator ==(const FValidationCheck& Other) const
	{
		return OwningPropertyHash == Other.OwningPropertyHash && ValidationDelegate.GetHandle() == Other.
			ValidationDelegate.GetHandle();
	}

	friend FORCEINLINE uint32 GetTypeHash(const FValidationCheck& Object)
	{
		return HashCombine(Object.OwningPropertyHash, GetTypeHash(Object.ValidationDelegate.GetHandle()));
	}
};

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

/** A property containing any number of validation checks. */
USTRUCT()
struct BEATSHOTGLOBAL_API FValidationProperty
{
	GENERATED_BODY()

	FValidationProperty();

	explicit FValidationProperty(const FPropertyHash& InPropertyHash, const EGameModeCategory InGameModeCategory);

	/** Adds a property as a dependent to this property.
	 *  @param Dependent the property that becomes dependent on this property
	 *  @param ValidationCheck the validation check that makes the property dependent on this property
	 */
	void AddDependent(const FValidationPropertyPtr& Dependent, const FValidationCheckPtr& ValidationCheck);

	/** Full property name constructed from the property's path. */
	FString PropertyName;

	/** The game mode category this property falls under. Used to sync with user interface. */
	EGameModeCategory GameModeCategory;

	/** Dependents mapped to the validation check(s) that makes them dependent on this property. */
	TMap<FValidationPropertyPtr, TSet<FValidationCheckPtr>> Dependents;

	/** Validation checks for this property. */
	TSet<FValidationCheckPtr> ValidationChecks;

private:
	/** Unique has created from the FProperty this Validation Property represents. */
	uint32 Hash;

public:
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
		return GetTypeHash(*Element.Get());
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

	FValidationCheckResult();

	FValidationCheckResult(bool Success, bool Bypassed, const FValidationCheckPtr& InValidationCheck);

	/** What the validation function returned. */
	bool bSuccess;

	/** Whether the validation check bypassed validation due to failing prerequisites. */
	bool bBypassed;

	/** The hash of the property associated with the validation check. */
	uint32 OwningPropertyHash;

	/** Pointer to the validation check that was performed to create this validation check result. */
	FValidationCheckPtr ValidationCheckPtr;

	/** The type of warning associated with the validation check. */
	EGameModeWarningType WarningType;

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
struct FValidationCheckResultKeyFuncs : BaseKeyFuncs<FValidationCheckResult, FValidationCheckPtr, false>
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

	FValidationResult();

	/** Adds a validation check result to the validation result.
	 *  @param Check The validation check result to move and emplace into either succeeded or failed check results.
	 */
	void AddValidationCheckResult(FValidationCheckResult&& Check);

	/** @return Succeeded validation checks. */
	FValidationCheckResultSet GetSucceeded() const;

	/** @return Failed validation checks. */
	FValidationCheckResultSet GetFailed() const;

	/** @return true if the validation check already exists. */
	bool Contains(const FValidationCheckPtr& Check) const;

private:
	FValidationCheckResultSet SucceededValidationCheckResults;
	FValidationCheckResultSet FailedValidationCheckResults;
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
	 *  @param SubSubStructName name of the inner struct inside the substruct.
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
	FValidationPropertySet& GetValidationProperties() const;

private:
	class FPrivate;
	TPimplPtr<FPrivate> Impl;
};
