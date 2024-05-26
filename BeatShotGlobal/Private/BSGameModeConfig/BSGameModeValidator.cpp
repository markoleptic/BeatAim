// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "BSGameModeConfig/BSGameModeValidator.h"
#include "BSGameModeConfig/BSConfig.h"

namespace
{
	/** Finds a property in RootStruct.
	 *  @param SubStructName name of the inner struct.
	 *  @param PropertyName name of property to find.
	 *	@return a property if found, otherwise null.
	 */
	template <typename RootStruct>
	const FProperty* FindProperty(const FName SubStructName, const FName PropertyName)
	{
		if (const FStructProperty* StructProperty = FindFProperty<FStructProperty>(RootStruct::StaticStruct(),
			SubStructName))
		{
			return FindFProperty<FProperty>(StructProperty->Struct, PropertyName);
		}
		return nullptr;
	}
}

FValidationCheck::FValidationCheck(TSet<const FProperty*>&& InProperties, const EGameModeWarningType InWarningType,
	const FString& InStringTableKey): Dependents(MoveTemp(InProperties)), WarningType(InWarningType),
	                                  StringTableKey(InStringTableKey)
{
}

void FValidationProperty::AddCheck(const FValidationCheck& Check)
{
	const int32 Index = ValidationChecks.Add(Check);
	for (const auto& Dependent : Check.Dependents)
	{
		PropertyMap[Dependent].Add(Index);
	}
}

TArray<FValidationCheckResult> FValidationProperty::ExecuteAll(const TSharedPtr<FBSConfig>& Config) const
{
	TArray<FValidationCheckResult> Out;
	for (const FValidationCheck& Check : ValidationChecks)
	{
		Out.Emplace(Check.ValidationDelegate.Execute(Config), Check);
	}
	return Out;
}

TArray<FValidationCheckResult> FValidationProperty::Execute(const TSet<const FProperty*>& Properties,
	const TSharedPtr<FBSConfig>& Config) const
{
	TArray<FValidationCheckResult> Out;
	TSet<int32> ValidationCheckIndices;

	for (const auto& Prop : Properties)
	{
		if (const TSet<int32>* Found = PropertyMap.Find(Prop))
		{
			ValidationCheckIndices.Append(*Found);
		}
	}

	for (const int32 CheckIndex : ValidationCheckIndices)
	{
		Out.Emplace(ValidationChecks[CheckIndex].ValidationDelegate.Execute(Config), ValidationChecks[CheckIndex]);
	}
	return Out;
}

void FValidationResult::AddValidationCheckResult(const FValidationCheckResult& Check,
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

const TMap<EGameModeCategory, TMap<EGameModeWarningType, TArray<FValidationCheckResult>>>&
FValidationResult::GetSucceeded() const
{
	return SucceededValidationCheckResults;
}

const TMap<EGameModeCategory, TMap<EGameModeWarningType, TArray<FValidationCheckResult>>>&
FValidationResult::GetFailed() const
{
	return FailedValidationCheckResults;
}

class UBSGameModeValidator::FPrivate
{
public:
	FPrivate();

	void SetupValidationChecks();

	void AddValidationProperty(const FValidationProperty& InValidationProperty);

	static float GetMinRequiredHorizontalSpread(const TSharedPtr<FBSConfig>& Config);

	static float GetMinRequiredVerticalSpread(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxTargetDiameter(const TSharedPtr<FBSConfig>& Config);

	static int32 GetMaxAllowedNumHorizontalTargets(const TSharedPtr<FBSConfig>& Config);

	static int32 GetMaxAllowedNumVerticalTargets(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedHorizontalSpacing(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedVerticalSpacing(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedTargetScale(const TSharedPtr<FBSConfig>& Config);

	TSet<FValidationProperty, FValidationPropertyKeyFuncs> ValidationProperties;
};

float UBSGameModeValidator::FPrivate::GetMinRequiredHorizontalSpread(const TSharedPtr<FBSConfig>& Config)
{
	return (Config->GridConfig.GridSpacing.X + GetMaxTargetDiameter(Config)) * (Config->GridConfig.
		NumHorizontalGridTargets - 1);
}

float UBSGameModeValidator::FPrivate::GetMinRequiredVerticalSpread(const TSharedPtr<FBSConfig>& Config)
{
	return (Config->GridConfig.GridSpacing.Y + GetMaxTargetDiameter(Config)) * (Config->GridConfig.
		NumVerticalGridTargets - 1);
}

float UBSGameModeValidator::FPrivate::GetMaxTargetDiameter(const TSharedPtr<FBSConfig>& Config)
{
	return FMath::Max(Config->TargetConfig.MinSpawnedTargetScale, Config->TargetConfig.MaxSpawnedTargetScale) *
		Constants::SphereTargetDiameter;
}

int32 UBSGameModeValidator::FPrivate::GetMaxAllowedNumHorizontalTargets(const TSharedPtr<FBSConfig>& Config)
{
	// Total = GridSpacing.X * (NumHorizontalGridTargets - 1) + (NumHorizontalGridTargets - 1) * MaxTargetDiameter;
	// Total = (NumHorizontalGridTargets - 1) * (GridSpacing.X + MaxTargetDiameter);
	// Total / (GridSpacing.X + MaxTargetDiameter) = NumHorizontalGridTargets - 1
	// NumHorizontalGridTargets = Total / (GridSpacing.X + MaxTargetDiameter) + 1
	return Constants::MaxValue_HorizontalSpread / (Config->GridConfig.GridSpacing.X + GetMaxTargetDiameter(Config)) + 1;
}

int32 UBSGameModeValidator::FPrivate::GetMaxAllowedNumVerticalTargets(const TSharedPtr<FBSConfig>& Config)
{
	// Total = GridSpacing.Y * (NumVerticalGridTargets - 1) + (NumVerticalGridTargets - 1) * MaxTargetDiameter;
	// Total = (NumVerticalGridTargets - 1) * (GridSpacing.Y + MaxTargetDiameter);
	// Total / (GridSpacing.Y + MaxTargetDiameter) = NumVerticalGridTargets - 1
	// NumVerticalGridTargets = Total / (GridSpacing.Y * MaxTargetDiameter) + 1
	return Constants::MaxValue_VerticalSpread / (Config->GridConfig.GridSpacing.Y + GetMaxTargetDiameter(Config)) + 1;
}

float UBSGameModeValidator::FPrivate::GetMaxAllowedHorizontalSpacing(const TSharedPtr<FBSConfig>& Config)
{
	// Total = GridSpacing.X * (NumHorizontalGridTargets - 1) + (NumHorizontalGridTargets - 1) * MaxTargetDiameter;
	// Total = (NumHorizontalGridTargets - 1) * (GridSpacing.X + MaxTargetDiameter);
	// Total / (NumHorizontalGridTargets - 1) = GridSpacing.X + MaxTargetDiameter;
	// Total / (NumHorizontalGridTargets - 1) - MaxTargetDiameter = GridSpacing.X;
	return Constants::MaxValue_HorizontalSpread / (Config->GridConfig.NumHorizontalGridTargets - 1) -
		GetMaxTargetDiameter(Config);
}

float UBSGameModeValidator::FPrivate::GetMaxAllowedVerticalSpacing(const TSharedPtr<FBSConfig>& Config)
{
	// Total = GridSpacing.Y * (NumVerticalGridTargets - 1) + (NumVerticalGridTargets - 1) * MaxTargetDiameter;
	// Total = (NumVerticalGridTargets - 1) * (GridSpacing.Y + MaxTargetDiameter);
	// Total / (NumVerticalGridTargets - 1) = GridSpacing.Y + MaxTargetDiameter;
	// Total / (NumVerticalGridTargets - 1) - MaxTargetDiameter = GridSpacing.Y;
	return Constants::MaxValue_VerticalSpread / (Config->GridConfig.NumVerticalGridTargets - 1) -
		GetMaxTargetDiameter(Config);
}

float UBSGameModeValidator::FPrivate::GetMaxAllowedTargetScale(const TSharedPtr<FBSConfig>& Config)
{
	// Total = GridSpacing.X * (NumHorizontalGridTargets - 1) + (NumHorizontalGridTargets - 1) * SphereTargetDiameter * Scale;
	// Total - (GridSpacing.X * (NumHorizontalGridTargets - 1)) = (NumHorizontalGridTargets - 1) * SphereTargetDiameter * Scale;
	// Total - (GridSpacing.X * (NumHorizontalGridTargets - 1)) = (NumHorizontalGridTargets - 1) * SphereTargetDiameter * Scale;
	// (Total - (GridSpacing.X * (NumHorizontalGridTargets - 1))) / ((NumHorizontalGridTargets - 1) * SphereTargetDiameter) = Scale;
	// Scale = (Total - (GridSpacing.X * (NumHorizontalGridTargets - 1))) / ((NumHorizontalGridTargets - 1) * SphereTargetDiameter)

	const float Horizontal = (Constants::MaxValue_HorizontalSpread - (Config->GridConfig.GridSpacing.X * (Config->
		GridConfig.NumHorizontalGridTargets - 1))) / ((Config->GridConfig.NumHorizontalGridTargets - 1) *
		Constants::SphereTargetDiameter);

	const float Vertical = (Constants::MaxValue_VerticalSpread - (Config->GridConfig.GridSpacing.Y * (Config->GridConfig
		.NumVerticalGridTargets - 1))) / ((Config->GridConfig.NumVerticalGridTargets - 1) *
		Constants::SphereTargetDiameter);
	return FMath::Min(Horizontal, Vertical);
}


UBSGameModeValidator::FPrivate::FPrivate()
{
	SetupValidationChecks();
}

void UBSGameModeValidator::FPrivate::SetupValidationChecks()
{
	const FProperty* BatchSpawningProperty = FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bUseBatchSpawning));
	const FProperty* SpawnEveryOtherTargetInCenterProperty = FindProperty<FBSConfig>(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig), GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bUseBatchSpawning));

	FValidationProperty Prop(SpawnEveryOtherTargetInCenterProperty, EGameModeCategory::TargetSpawning);
	FValidationCheck Check({BatchSpawningProperty}, EGameModeWarningType::Warning,
		TEXT("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning"));
	Check.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return !(Config->TargetConfig.bSpawnEveryOtherTargetInCenter && Config->TargetConfig.bUseBatchSpawning);
	});
	Prop.AddCheck(MoveTemp(Check));
	AddValidationProperty(MoveTemp(Prop));

	Prop = FValidationProperty(BatchSpawningProperty, EGameModeCategory::TargetSpawning);
	Check = FValidationCheck({SpawnEveryOtherTargetInCenterProperty}, EGameModeWarningType::Warning,
		TEXT("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning"));
	Check.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return !(Config->TargetConfig.bSpawnEveryOtherTargetInCenter && Config->TargetConfig.bUseBatchSpawning);
	});
	Prop.AddCheck(MoveTemp(Check));
	AddValidationProperty(MoveTemp(Prop));

	const FProperty* MovingTargetDirectionMode = FindProperty<FBSConfig>(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MovingTargetDirectionMode));
	const FProperty* TargetSpawnResponses = FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetSpawnResponses));
	const FProperty* TargetActivationResponses = FindProperty<FBSConfig>(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetActivationResponses));
	const FProperty* TargetDeactivationResponses = FindProperty<FBSConfig>(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDeactivationResponses));

	Prop = FValidationProperty(MovingTargetDirectionMode, EGameModeCategory::TargetBehavior);
	Check = FValidationCheck({TargetSpawnResponses}, EGameModeWarningType::Caution, TEXT("Invalid_Velocity_MTDM_None"));
	Check.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeVelocity));
	});
	Prop.AddCheck(MoveTemp(Check));
	Check = FValidationCheck({TargetSpawnResponses}, EGameModeWarningType::Caution,
		TEXT("Invalid_Direction_MTDM_None"));
	Check.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeDirection));
	});
	Prop.AddCheck(MoveTemp(Check));
	Check = FValidationCheck({TargetActivationResponses}, EGameModeWarningType::Caution,
		TEXT("Invalid_Velocity_MTDM_None"));
	Check.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeVelocity));
	});
	Prop.AddCheck(MoveTemp(Check));
	Check = FValidationCheck({TargetActivationResponses}, EGameModeWarningType::Caution,
		TEXT("Invalid_Direction_MTDM_None"));
	Check.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeDirection));
	});
	Prop.AddCheck(MoveTemp(Check));
	Check = FValidationCheck({TargetDeactivationResponses}, EGameModeWarningType::Caution,
		TEXT("Invalid_Velocity_MTDM_None"));
	Check.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeVelocity));
	});
	Prop.AddCheck(MoveTemp(Check));
	Check = FValidationCheck({TargetDeactivationResponses}, EGameModeWarningType::Caution,
		TEXT("Invalid_Direction_MTDM_None"));
	Check.ValidationDelegate.BindLambda([](const TSharedPtr<FBSConfig>& Config)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeDirection));
	});
	Prop.AddCheck(MoveTemp(Check));
	AddValidationProperty(MoveTemp(Prop));
}

void UBSGameModeValidator::FPrivate::AddValidationProperty(const FValidationProperty& InValidationProperty)
{
	ValidationProperties.Add(InValidationProperty);
}

UBSGameModeValidator::UBSGameModeValidator() : Impl(MakePimpl<FPrivate>())
{
}

FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig) const
{
	FValidationResult Result;
	for (const FValidationProperty& ValidationProperty : Impl->ValidationProperties)
	{
		for (const FValidationCheckResult& Elem : ValidationProperty.ExecuteAll(InConfig))
		{
			Result.AddValidationCheckResult(Elem, ValidationProperty.GameModeCategory);
		}
	}
	return Result;
}

FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig, const FName SubStructName,
	const FName PropertyName) const
{
	FValidationResult Result;

	if (const FValidationProperty* ValidationProperty = Impl->ValidationProperties.Find(
		FindProperty<FBSConfig>(SubStructName, PropertyName)))
	{
		for (const FValidationCheckResult& Elem : ValidationProperty->ExecuteAll(InConfig))
		{
			Result.AddValidationCheckResult(Elem, ValidationProperty->GameModeCategory);
		}
	}
	return Result;
}

FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig,
	const TSet<const FProperty*>& Properties) const
{
	FValidationResult Result;
	for (const FProperty* Property : Properties)
	{
		if (const FValidationProperty* ValidationProperty = Impl->ValidationProperties.Find(Property))
		{
			for (const FValidationCheckResult& Elem : ValidationProperty->ExecuteAll(InConfig))
			{
				Result.AddValidationCheckResult(Elem, ValidationProperty->GameModeCategory);
			}
		}
	}

	return Result;
}

const FProperty* UBSGameModeValidator::FindBSConfigProperty(const FName SubStructName, const FName PropertyName)
{
	return FindProperty<FBSConfig>(SubStructName, PropertyName);
}
