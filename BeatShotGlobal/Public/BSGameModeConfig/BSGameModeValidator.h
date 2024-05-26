// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BSGameModeValidator.generated.h"

struct FValidationCheckResult;
struct FBSConfig;
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
	FValidationCheck() = default;
	FValidationCheck(TSet<const FProperty*>&& InProperties, EGameModeWarningType InWarningType,
		const FString& InStringTableKey);

	TSet<const FProperty*> Dependents;
	EGameModeWarningType WarningType;
	FString StringTableKey;
	TDelegate<bool(const TSharedPtr<FBSConfig>&)> ValidationDelegate;
};

/** A property containing one or multiple validation checks. */
USTRUCT()
struct BEATSHOTGLOBAL_API FValidationProperty
{
	GENERATED_BODY()
	FValidationProperty() = default;

	explicit FValidationProperty(const FProperty* InProperty, const EGameModeCategory InGameModeCategory) :
		Property(InProperty), GameModeCategory(InGameModeCategory)
	{
	}

	void AddCheck(const FValidationCheck& Check);

	/** Execute all validation checks for this property.
	 *  @param Config configuration to use for executing validation delegates.
	 *  @return a set of validation check results for each check executed.
	 */
	TArray<FValidationCheckResult> ExecuteAll(const TSharedPtr<FBSConfig>& Config) const;

	/** Execute validation checks involving only those found in Properties.
	 *  @param Properties only look for validation checks with these as dependents.
	 *  @param Config configuration to use for executing validation delegates.
	 *  @return a set of validation check results for each check executed.
	 */
	TArray<FValidationCheckResult> Execute(const TSet<const FProperty*>& Properties,
		const TSharedPtr<FBSConfig>& Config) const;

	const FProperty* Property;
	TMap<const FProperty*, TSet<int32>> PropertyMap;
	EGameModeCategory GameModeCategory;
	TArray<FValidationCheck> ValidationChecks;

	FORCEINLINE bool operator ==(const FValidationProperty* Other) const
	{
		return Property == Other->Property;
	}

	FORCEINLINE bool operator <(const FValidationProperty* Other) const
	{
		return GameModeCategory < Other->GameModeCategory;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FValidationProperty& Object)
	{
		return PointerHash(Object.Property);
	}
};

struct FValidationPropertyKeyFuncs : BaseKeyFuncs<FValidationProperty, const FProperty*, false>
{
	static const FProperty* GetSetKey(const FValidationProperty& Element)
	{
		return Element.Property;
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

	bool bSuccess;
	EGameModeWarningType WarningType;
	FString StringTableKey;
	FValidationCheckResult() = default;

	FValidationCheckResult(const bool Success, const FValidationCheck& InValidationCheck) : bSuccess(Success),
		WarningType(InValidationCheck.WarningType), StringTableKey(InValidationCheck.StringTableKey)
	{
	}
};

USTRUCT()
struct BEATSHOTGLOBAL_API FValidationResult
{
	GENERATED_BODY()

	FValidationResult()
	{
	}

	void AddValidationCheckResult(const FValidationCheckResult& Check, EGameModeCategory GameModeCategory);


	const TMap<EGameModeCategory, TMap<EGameModeWarningType, TArray<FValidationCheckResult>>>& GetSucceeded() const;
	const TMap<EGameModeCategory, TMap<EGameModeWarningType, TArray<FValidationCheckResult>>>& GetFailed() const;

private:
	TMap<EGameModeCategory, TMap<EGameModeWarningType, TArray<FValidationCheckResult>>> SucceededValidationCheckResults;
	TMap<EGameModeCategory, TMap<EGameModeWarningType, TArray<FValidationCheckResult>>> FailedValidationCheckResults;
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

private:
	class FPrivate;
	TPimplPtr<FPrivate> Impl;
};
