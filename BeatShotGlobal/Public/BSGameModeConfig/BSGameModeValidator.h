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
	None,
	Caution,
	Warning,
	Error
};

USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FValidationCheck
{
	GENERATED_BODY()

	FValidationCheck() : Property(nullptr), WarningType(EGameModeWarningType::None)
	{
	}

	~FValidationCheck()
	{
		Property = nullptr;
	}

	const FProperty* Property;
	TSet<const FProperty*> Dependencies;
	EGameModeWarningType WarningType;
	FString StringTableKey;
	FValidationDelegate ValidationDelegate;
};

USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FValidationResult
{
	GENERATED_BODY()

	bool bSuccess;
	const FProperty* Property;
	TSet<const FProperty*> Dependencies;
	EGameModeWarningType WarningType;
	FString StringTableKey;

	FValidationResult() : bSuccess(false), Property(nullptr), WarningType(EGameModeWarningType::None)
	{
	}

	FValidationResult(const bool bInSuccess, const FValidationCheck& Check) : bSuccess(bInSuccess),
	                                                                          Property(Check.Property),
	                                                                          Dependencies(Check.Dependencies),
	                                                                          WarningType(Check.WarningType),
	                                                                          StringTableKey(Check.StringTableKey)
	{
	}

	~FValidationResult()
	{
		Property = nullptr;
	}
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

	TArray<FValidationResult> Validate(const TSharedPtr<FBSConfig>& InConfig);

	TArray<FValidationResult> Validate(const TSharedPtr<FBSConfig>& InConfig, const FName SubStructName,
		const FName PropertyName);

	static const FProperty* FindProperty(const UStruct* Owner, const FName SubStructName, const FName PropertyName);

private:
	TMap<const FProperty*, TArray<FValidationCheck>> ValidationChecks;
};
