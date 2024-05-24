// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "BSGameModeConfig/BSGameModeValidator.h"
#include "BSGameModeConfig/BSConfig.h"

FValidationCheck::FValidationCheck(const FProperty* InProperty) : Property(InProperty),
                                                                  WarningType(EGameModeWarningType::None),
                                                                  GameModeCategory(EGameModeCategory::None)
{
}

FValidationCheck::~FValidationCheck()
{
}

FValidationCheck& FValidationCheck::AddDependent(const FProperty* InProperty)
{
	Dependents.Add(InProperty);
	return *this;
}

FValidationCheck& FValidationCheck::AddDependents(const TSet<const FProperty*>& InProperties)
{
	Dependents.Append(InProperties);
	return *this;
}

FValidationCheck& FValidationCheck::SetWarningType(const EGameModeWarningType InWarningType)
{
	WarningType = InWarningType;
	return *this;
}

FValidationCheck& FValidationCheck::SetStringTableKey(const FString& InKey)
{
	StringTableKey = InKey;
	return *this;
}

FValidationCheck& FValidationCheck::SetGameModeCategory(const EGameModeCategory InGameModeCategory)
{
	GameModeCategory = InGameModeCategory;
	return *this;
}

FValidationCheck& FValidationCheck::BindLambda(const TFunction<bool(const TSharedPtr<FBSConfig>&)>& InLambda)
{
	ValidationDelegate.BindLambda(InLambda);
	return *this;
}

bool FValidationCheck::Execute(const TSharedPtr<FBSConfig>& Config) const
{
	return ValidationDelegate.Execute(Config);
}

const FProperty* FValidationCheck::GetProperty() const
{
	return Property;
}

void FValidationResult::AddValidationCheckResult(const FValidationCheckResult&& Check,
	const EGameModeCategory GameModeCategory)
{
	if (Check.bSuccess)
	{
		SucceededValidationCheckResults[GameModeCategory][Check.WarningType].Emplace(Check);
	}
	else
	{
		FailedValidationCheckResults[GameModeCategory][Check.WarningType].Emplace(Check);
	}
}

const FGameModeCategoryResultMap& FValidationResult::GetSucceeded() const
{
	return SucceededValidationCheckResults;
}

const FGameModeCategoryResultMap& FValidationResult::GetFailed() const
{
	return FailedValidationCheckResults;
}

UBSGameModeValidator::UBSGameModeValidator()
{
}

void UBSGameModeValidator::SetupValidationChecks()
{
	AddValidationCheck(
		FValidationCheck(FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MovingTargetDirectionMode))).AddDependent(
			FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetSpawnResponses))).
		SetWarningType(EGameModeWarningType::Caution).SetStringTableKey(TEXT("Invalid_Velocity_MTDM_None")).BindLambda(
			[](const TSharedPtr<FBSConfig>& Config)
			{
				return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
					TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeVelocity));
			}));

	AddValidationCheck(
		FValidationCheck(FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MovingTargetDirectionMode))).AddDependent(
			FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetSpawnResponses))).
		SetWarningType(EGameModeWarningType::Caution).SetStringTableKey(TEXT("Invalid_Direction_MTDM_None")).BindLambda(
			[](const TSharedPtr<FBSConfig>& Config)
			{
				return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
					TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeDirection));
			}));

	AddValidationCheck(
		FValidationCheck(FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MovingTargetDirectionMode))).AddDependent(
			FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDeactivationResponses))).
		SetWarningType(EGameModeWarningType::Caution).SetStringTableKey(TEXT("Invalid_Velocity_MTDM_None")).BindLambda(
			[](const TSharedPtr<FBSConfig>& Config)
			{
				return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
					TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeVelocity));
			}));

	AddValidationCheck(
		FValidationCheck(FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MovingTargetDirectionMode))).AddDependent(
			FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDeactivationResponses))).
		SetWarningType(EGameModeWarningType::Caution).SetStringTableKey(TEXT("Invalid_Direction_MTDM_None")).BindLambda(
			[](const TSharedPtr<FBSConfig>& Config)
			{
				return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
					TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeDirection));
			}));

	AddValidationCheck(
		FValidationCheck(FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MovingTargetDirectionMode))).AddDependent(
			FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetActivationResponses))).
		SetWarningType(EGameModeWarningType::Caution).SetStringTableKey(TEXT("Invalid_Velocity_MTDM_None")).BindLambda(
			[](const TSharedPtr<FBSConfig>& Config)
			{
				return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
					TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeVelocity));
			}));

	AddValidationCheck(
		FValidationCheck(FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
			GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MovingTargetDirectionMode))).AddDependent(
			FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
				GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetActivationResponses))).
		SetWarningType(EGameModeWarningType::Caution).SetStringTableKey(TEXT("Invalid_Direction_MTDM_None")).BindLambda(
			[](const TSharedPtr<FBSConfig>& Config)
			{
				return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
					TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeDirection));
			}));
}

void UBSGameModeValidator::AddValidationCheck(const FValidationCheck& InValidationCheck)
{
	if (const auto Found = ValidationChecks.Find(InValidationCheck.GetProperty()))
	{
		Found->Add(InValidationCheck);
	}
	else
	{
		ValidationChecks.Emplace(InValidationCheck.GetProperty(), {InValidationCheck});
	}
}

FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig)
{
	FValidationResult Result;
	for (const auto& [Property, Checks] : ValidationChecks)
	{
		for (const FValidationCheck& Check : Checks)
		{
			Result.AddValidationCheckResult(FValidationCheckResult(Check.Execute(InConfig), Check),
				Check.GameModeCategory);
		}
	}
	return Result;
}


FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig, const FName SubStructName,
	const FName PropertyName)
{
	FValidationResult Result;
	if (const auto Found = ValidationChecks.Find(FindPropertyByName(FBSConfig::StaticStruct(), SubStructName,
		PropertyName)))
	{
		for (const FValidationCheck& Check : *Found)
		{
			Result.AddValidationCheckResult(FValidationCheckResult(Check.Execute(InConfig), Check),
				Check.GameModeCategory);
		}
	}
	return Result;
}

FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig,
	const TSet<const FProperty*>& Properties)
{
	FValidationResult Result;
	for (const FProperty* Property : Properties)
	{
		if (const auto Found = ValidationChecks.Find(Property))
		{
			for (const FValidationCheck& Check : *Found)
			{
				Result.AddValidationCheckResult(FValidationCheckResult(Check.Execute(InConfig), Check),
					Check.GameModeCategory);
			}
		}
	}

	return Result;
}


const FProperty* UBSGameModeValidator::FindPropertyByName(const UStruct* Owner, const FName SubStructName,
	const FName PropertyName)
{
	if (const auto StructProperty = FindFProperty<FStructProperty>(Owner, SubStructName))
	{
		if (const auto Property = FindFProperty<FProperty>(StructProperty->Struct, PropertyName))
		{
			return Property;
		}
	}
	return nullptr;
}
