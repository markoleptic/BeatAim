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

DECLARE_DELEGATE_RetVal_OneParam(bool, FValidationDelegate, const TSharedPtr<FBSConfig>&);

UENUM(BlueprintType)
enum class EGameModeWarningType: uint8
{
	None, Caution, Warning, Error
};

UENUM(BlueprintType)
enum class EGameModeCategory : uint8
{
	None, Start, General, SpawnArea, TargetSpawning, TargetActivation, TargetBehavior, TargetSizing, Preview
};

/** A single validation check for a property. */
USTRUCT()
struct BEATSHOTGLOBAL_API FValidationCheck
{
	GENERATED_BODY()

	FValidationCheck(): WarningType(EGameModeWarningType::None), Id(GId++)
	{
	}

	explicit FValidationCheck(TSet<const FProperty*>&& InProperties, const EGameModeWarningType InWarningType) :
		InvolvedProperties(MoveTemp(InProperties)), WarningType(InWarningType), Id(GId++)
	{
	}

	/** All properties involved in the calculation of the validation result. */
	TSet<const FProperty*> InvolvedProperties;

	/** The type of warning associated with the validation check. */
	EGameModeWarningType WarningType;

	/** Must return true in order to validate successfully. */
	TDelegate<bool(const TSharedPtr<FBSConfig>&, TArray<int32>&)> ValidationDelegate;

	int32 Id;
	static int32 GId;

	FORCEINLINE bool operator ==(const FValidationCheck& Other) const
	{
		return Id == Other.Id;
	}

	FORCEINLINE bool operator <(const FValidationCheck& Other) const
	{
		return Id < Other.Id;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FValidationCheck& Object)
	{
		return GetTypeHash(Object.Id);
	}
};

/** Specific data associated with an FValidationProperty and an FValidationCheck. */
USTRUCT()
struct BEATSHOTGLOBAL_API FUniqueValidationCheckData
{
	GENERATED_BODY()

	FUniqueValidationCheckData() = default;

	FUniqueValidationCheckData(const FString& InStringTableKey, const FString& InDynamicStringTableKey) :
		StringTableKey(InStringTableKey), DynamicStringTableKey(InDynamicStringTableKey),
		WarningType(EGameModeWarningType::None)
	{
	}

	/** The string table key to find tooltip text from if validation fails. Also serves as backup if Dynamic. */
	FString StringTableKey;

	/** The dynamic string table key to find tooltip text from if validation fails. */
	FString DynamicStringTableKey;

	/** The tooltip text populated from either one of the string table keys. */
	FText TooltipText;

	/** The type of warning associated with the validation check. */
	EGameModeWarningType WarningType;

	FORCEINLINE bool operator ==(const FUniqueValidationCheckData& Other) const
	{
		return StringTableKey == Other.StringTableKey && DynamicStringTableKey == Other.DynamicStringTableKey;
	}

	FORCEINLINE bool operator <(const FUniqueValidationCheckData& Other) const
	{
		return StringTableKey < Other.StringTableKey;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FUniqueValidationCheckData& Object)
	{
		return GetTypeHash(Object.StringTableKey);
	}
};

/** A property containing any number of validation checks. */
USTRUCT()
struct BEATSHOTGLOBAL_API FValidationProperty
{
	GENERATED_BODY()
	FValidationProperty() = default;

	explicit FValidationProperty(const FProperty* InProperty, const EGameModeCategory InGameModeCategory) :
		Property(InProperty), GameModeCategory(InGameModeCategory)
	{
	}

	/** Adds a pointer to a validation check involving this property to its stored checks, and adds data unique to this
	 *  property and check to its stored check data.
	 *  @param Check validation check to associate with this property.
	 *  @param Data unique data to associate with this property and the validation check.
	 */
	void AddCheck(const FValidationCheckPtr& Check, const FUniqueValidationCheckData& Data);

	/** The single property this validation property represents. */
	const FProperty* Property;

	/** The game mode category this property falls under. Used to sync with user interface. */
	EGameModeCategory GameModeCategory;

	/** An array of pointers to all validation checks that involve this property. */
	TSet<FValidationCheckPtr> Checks;

	/** Maps each validation check to unique validation check data for this property. */
	TMap<FValidationCheckPtr, FUniqueValidationCheckData> CheckData;

	FORCEINLINE bool operator ==(const FValidationProperty& Other) const
	{
		return Property == Other.Property;
	}

	FORCEINLINE bool operator <(const FValidationProperty& Other) const
	{
		return GameModeCategory < Other.GameModeCategory;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FValidationProperty& Object)
	{
		return PointerHash(Object.Property);
	}
};

/** Struct used to allow efficient retrieval of validation properties in TSets using the FProperty as a key. */
struct FValidationPropertyKeyFuncs : BaseKeyFuncs<FValidationPropertyPtr, const FProperty*, false>
{
	static const FProperty* GetSetKey(const FValidationPropertyPtr& Element)
	{
		return Element->Property;
	}

	static bool Matches(const FProperty* A, const FProperty* B)
	{
		return A == B;
	}

	static uint32 GetKeyHash(const FProperty* Key)
	{
		return PointerHash(Key);
	}
};

/** The result from executing a validation check. */
USTRUCT()
struct BEATSHOTGLOBAL_API FValidationCheckResult
{
	GENERATED_BODY()

	/** What the validation function returned. */
	bool bSuccess;

	/** The type of warning associated with the validation check. */
	EGameModeWarningType WarningType;

	/** Calculated values from the validation function. */
	TArray<int32> CalculatedValues;

	/** All properties involved in the calculation of this validation check result. */
	TSet<const FProperty*> InvolvedProperties;

	/** Maps each involved property to their property data unique to the validation check. */
	TMap<const FProperty*, FUniqueValidationCheckData> PropertyData;


	FValidationCheckResult() : bSuccess(false), WarningType(EGameModeWarningType::None), Id(GId++)
	{
	}

	FValidationCheckResult(const bool Success, const FValidationCheckPtr& InValidationCheck, TArray<int32>&& Values,
		TMap<const FProperty*, FUniqueValidationCheckData>&& Data) : bSuccess(Success),
		                                                             WarningType(InValidationCheck->WarningType),
		                                                             CalculatedValues(MoveTemp(Values)),
		                                                             InvolvedProperties(
			                                                             InValidationCheck->InvolvedProperties),
		                                                             PropertyData(MoveTemp(Data)), Id(GId++)
	{
	}

	FORCEINLINE bool operator ==(const FValidationCheckResult& Other) const
	{
		return Id == Other.Id;
	}

	FORCEINLINE bool operator <(const FValidationCheckResult& Other) const
	{
		return Id < Other.Id;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FValidationCheckResult& Object)
	{
		return GetTypeHash(Object.Id);
	}

private:
	int32 Id;
	static int32 GId;
};


USTRUCT()
struct BEATSHOTGLOBAL_API FValidationResult
{
	GENERATED_BODY()

	FValidationResult()
	{
	}

	void AddValidationCheckResult(FValidationCheckResult&& Check);


	TSet<FValidationCheckResult> GetSucceeded() const;
	TSet<FValidationCheckResult> GetFailed() const;

private:
	TSet<FValidationCheckResult> SucceededValidationCheckResults;
	TSet<FValidationCheckResult> FailedValidationCheckResults;
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
	FValidationResult Validate(const TSharedPtr<FBSConfig>& InConfig, const TSet<const FProperty*>& Properties) const;

	/** Finds a property in BSConfig.
	 *  @param SubStructName name of the inner struct.
	 *  @param PropertyName name of property to find.
	 *	@return a property if found, otherwise null.
	 */
	static const FProperty* FindBSConfigProperty(const FName SubStructName, const FName PropertyName);

	/** Finds a validation property based on a BSConfig FProperty.
	 *  @param Property the property to look for.
	 *	@return a validation property pointer if found, otherwise null.
	 */
	FValidationPropertyPtr FindValidationProperty(const FProperty* Property) const;

private:
	class FPrivate;
	TPimplPtr<FPrivate> Impl;
};
