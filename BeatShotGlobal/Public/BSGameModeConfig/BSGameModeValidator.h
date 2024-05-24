// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BSGameModeValidator.generated.h"

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

USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FValidationCheck
{
	GENERATED_BODY()

	FValidationCheck() = default;

	explicit FValidationCheck(const FProperty* InProperty);

	~FValidationCheck();

	FValidationCheck& AddDependent(const FProperty* InProperty);
	FValidationCheck& AddDependents(const TSet<const FProperty*>& InProperties);
	FValidationCheck& SetWarningType(EGameModeWarningType InWarningType);
	FValidationCheck& SetStringTableKey(const FString& InKey);
	FValidationCheck& SetGameModeCategory(EGameModeCategory InGameModeCategory);
	FValidationCheck& BindLambda(const TFunction<bool(const TSharedPtr<FBSConfig>&)>& InLambda);

	bool Execute(const TSharedPtr<FBSConfig>& Config) const;

	const FProperty* GetProperty() const;

private:
	friend struct FValidationCheckResult;
	friend class UBSGameModeValidator;
	const FProperty* Property;
	TSet<const FProperty*> Dependents;
	EGameModeWarningType WarningType;
	FString StringTableKey;
	TDelegate<bool(const TSharedPtr<FBSConfig>&)> ValidationDelegate;
	EGameModeCategory GameModeCategory;
};

USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FValidationCheckResult
{
	GENERATED_BODY()

	bool bSuccess;
	const FProperty* Property;
	TSet<const FProperty*> Dependencies;
	EGameModeWarningType WarningType;
	FString StringTableKey;


	FValidationCheckResult() : bSuccess(false), Property(nullptr), WarningType(EGameModeWarningType::None)
	{
	}

	FValidationCheckResult(const bool bInSuccess, const FValidationCheck& Check) : bSuccess(bInSuccess),
		Property(Check.GetProperty()), Dependencies(Check.Dependents), WarningType(Check.WarningType),
		StringTableKey(Check.StringTableKey)
	{
	}

	~FValidationCheckResult()
	{
		Property = nullptr;
	}
};

using FGameModeWarningResultMap = TMap<EGameModeWarningType, TArray<FValidationCheckResult>>;
using FGameModeCategoryResultMap = TMap<EGameModeCategory, FGameModeWarningResultMap>;

USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FValidationResult
{
	GENERATED_BODY()

public:
	FValidationResult()
	{
	}

	void AddValidationCheckResult(const FValidationCheckResult&& Check, EGameModeCategory GameModeCategory);

	~FValidationResult()
	{
	}

	const FGameModeCategoryResultMap& GetSucceeded() const;
	const FGameModeCategoryResultMap& GetFailed() const;

private:
	FGameModeCategoryResultMap SucceededValidationCheckResults;
	FGameModeCategoryResultMap FailedValidationCheckResults;
};

/** Validates a BSConfig. */
UCLASS()
class BEATSHOTGLOBAL_API UBSGameModeValidator : public UObject
{
	GENERATED_BODY()

public:
	UBSGameModeValidator();

	void SetupValidationChecks();

	void AddValidationCheck(const FValidationCheck& InValidationCheck);

	FValidationResult Validate(const TSharedPtr<FBSConfig>& InConfig);

	FValidationResult Validate(const TSharedPtr<FBSConfig>& InConfig, FName SubStructName, FName PropertyName);

	FValidationResult Validate(const TSharedPtr<FBSConfig>& InConfig, const TSet<const FProperty*>& Properties);

	static const FProperty* FindPropertyByName(const UStruct* Owner, const FName SubStructName,
		const FName PropertyName);

	//template <typename RootStruct, typename SubStruct>
	//static const FStructProperty* FindSubStruct(SubStruct RootStruct::* SubStructMember);
	template <typename RootStruct>
	static const FProperty* FindProperty(const FName SubStructProperty, const FName PropertyInSubStruct);

	//template <typename RootStruct, typename SubStruct, typename... PropertyTypes>
	//static TSet<const FProperty*> FindProperties(SubStruct RootStruct::* SubStructMember,
	//	PropertyTypes SubStruct::*... PropertiesInSubStruct);

private:
	TMap<const FProperty*, TArray<FValidationCheck>> ValidationChecks;
};

/*template <typename RootStruct, typename SubStruct>
const FStructProperty* UBSGameModeValidator::FindSubStruct(SubStruct RootStruct::* SubStructMember)
{
	return FindFProperty<FStructProperty>(RootStruct::StaticStruct(),
		GET_MEMBER_NAME_CHECKED(RootStruct, SubStructMember));
}*/

template <typename RootStruct>
const FProperty* UBSGameModeValidator::FindProperty(const FName SubStructProperty, const FName PropertyInSubStruct)
{
	if (const FStructProperty* StructProperty = FindFProperty<FStructProperty>(RootStruct::StaticStruct(),
		SubStructProperty))
	{
		return FindFProperty<FProperty>(StructProperty->Struct, PropertyInSubStruct);
	}
	return nullptr;
}

/*template <typename RootStruct, typename SubStruct, typename... PropertyTypes>
TSet<const FProperty*> UBSGameModeValidator::FindProperties(SubStruct RootStruct::* SubStructMember,
	PropertyTypes SubStruct::*... PropertiesInSubStruct)
{
	TSet<const FProperty*> FoundProperties;
	// Expand the parameter pack and use FindProperty for each property
	((FoundProperties.Add(FindProperty(SubStructMember, PropertiesInSubStruct))), ...);
	return FoundProperties;
}*/
