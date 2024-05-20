// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "BSGameModeConfig/BSGameModeValidator.h"
#include "BSGameModeConfig/BSConfig.h"

UBSGameModeValidator::UBSGameModeValidator()
{
}

void UBSGameModeValidator::SetupValidationChecks()
{
	FValidationCheck ValidationCheck;
	ValidationCheck.Property = FindProperty(FBSConfig::StaticStruct(), GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.MovingTargetDirectionMode));
	ValidationCheck.Dependencies.Add(FindProperty(FBSConfig::StaticStruct(),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.TargetSpawnResponses)));
	ValidationCheck.WarningType = EGameModeWarningType::Caution;
	ValidationCheck.StringTableKey = "Invalid_Velocity_MTDM_None";
	ValidationCheck.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeVelocity);
	});
	AddValidationCheck(ValidationCheck);

	ValidationCheck = FValidationCheck();
	ValidationCheck.Property = FindProperty(FBSConfig::StaticStruct(), GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.MovingTargetDirectionMode));
	ValidationCheck.Dependencies.Add(FindProperty(FBSConfig::StaticStruct(),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.TargetSpawnResponses)));
	ValidationCheck.WarningType = EGameModeWarningType::Caution;
	ValidationCheck.StringTableKey = "Invalid_Direction_MTDM_None";
	ValidationCheck.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeDirection);
	});
	AddValidationCheck(ValidationCheck);

	ValidationCheck = FValidationCheck();
	ValidationCheck.Property = FindProperty(FBSConfig::StaticStruct(), GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.MovingTargetDirectionMode));
	ValidationCheck.Dependencies.Add(FindProperty(FBSConfig::StaticStruct(),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.TargetDeactivationResponses)));
	ValidationCheck.WarningType = EGameModeWarningType::Caution;
	ValidationCheck.StringTableKey = "Invalid_Velocity_MTDM_None";
	ValidationCheck.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeVelocity);
	});
	AddValidationCheck(ValidationCheck);

	ValidationCheck = FValidationCheck();
	ValidationCheck.Property = FindProperty(FBSConfig::StaticStruct(), GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.MovingTargetDirectionMode));
	ValidationCheck.Dependencies.Add(FindProperty(FBSConfig::StaticStruct(),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.TargetDeactivationResponses)));
	ValidationCheck.WarningType = EGameModeWarningType::Caution;
	ValidationCheck.StringTableKey = "Invalid_Direction_MTDM_None";
	ValidationCheck.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeDirection);
	});
	AddValidationCheck(ValidationCheck);

	ValidationCheck = FValidationCheck();
	ValidationCheck.Property = FindProperty(FBSConfig::StaticStruct(), GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.MovingTargetDirectionMode));
	ValidationCheck.Dependencies.Add(FindProperty(FBSConfig::StaticStruct(),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.TargetActivationResponses)));
	ValidationCheck.WarningType = EGameModeWarningType::Caution;
	ValidationCheck.StringTableKey = "Invalid_Direction_MTDM_None";
	ValidationCheck.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeDirection);
	});
	AddValidationCheck(ValidationCheck);

	ValidationCheck = FValidationCheck();
	ValidationCheck.Property = FindProperty(FBSConfig::StaticStruct(), GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.MovingTargetDirectionMode));
	ValidationCheck.Dependencies.Add(FindProperty(FBSConfig::StaticStruct(),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig.TargetActivationResponses)));
	ValidationCheck.WarningType = EGameModeWarningType::Caution;
	ValidationCheck.StringTableKey = "Invalid_Direction_MTDM_None";
	ValidationCheck.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeDirection);
	});
	AddValidationCheck(ValidationCheck);
}

void UBSGameModeValidator::AddValidationCheck(const FValidationCheck& InValidationCheck)
{
	if (const auto Found = ValidationChecks.Find(InValidationCheck.Property))
	{
		Found->Add(InValidationCheck);
	}
	else
	{
		ValidationChecks.Emplace(InValidationCheck.Property, TArray{InValidationCheck});
	}
}

TArray<FValidationResult> UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig)
{
	TArray<FValidationResult> Results;
	for (const auto& [Property, Checks] : ValidationChecks)
	{
		for (const FValidationCheck& Check : Checks)
		{
			Results.Emplace(Check.ValidationDelegate.Execute(InConfig), Check);
		}
	}
	return Results;
}

TArray<FValidationResult> UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig,
	const FName SubStructName, const FName PropertyName)
{
	TArray<FValidationResult> Results;
	if (const auto Found = ValidationChecks.Find(FindProperty(FBSConfig::StaticStruct(), SubStructName, PropertyName)))
	{
		for (const FValidationCheck& Check : *Found)
		{
			Results.Emplace(Check.ValidationDelegate.Execute(InConfig), Check);
		}
	}
	return Results;
}

const FProperty* UBSGameModeValidator::FindProperty(const UStruct* Owner, const FName SubStructName,
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
