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

	/** Finds a nested property in RootStruct.
	 *  @param SubStructName name of the inner struct.
	 *  @param SubSubStructName name of the inner inner struct.
	 *  @param PropertyName name of property to find.
	 *	@return a property if found, otherwise null.
	 */
	template <typename RootStruct>
	const FProperty* FindProperty(const FName SubStructName, const FName SubSubStructName, const FName PropertyName)
	{
		if (const FStructProperty* StructProperty = FindFProperty<FStructProperty>(RootStruct::StaticStruct(),
			SubStructName))
		{
			if (const FStructProperty* SubStructProperty = FindFProperty<FStructProperty>(StructProperty->Struct,
				SubSubStructName))
			{
				return FindFProperty<FProperty>(SubStructProperty->Struct, PropertyName);
			}
		}
		return nullptr;
	}

	/** Creates a FPropertyHash object using the substruct and property found in RootStruct.
	 *  @param SubStructName name of the inner struct.
	 *  @param PropertyName name of property to find.
	 *	@return a FPropertyHash object containing all properties leading to innermost property.
	 */
	template <typename RootStruct>
	FPropertyHash CreatePropertyHash(const FName SubStructName, const FName PropertyName)
	{
		FPropertyHash Hash;
		if (const FStructProperty* StructProperty = FindFProperty<FStructProperty>(RootStruct::StaticStruct(),
			SubStructName))
		{
			Hash.Properties.Add(StructProperty);
			if (const FProperty* Property = FindFProperty<FProperty>(StructProperty->Struct, PropertyName))
			{
				Hash.Properties.Add(Property);
			}
		}
		return Hash;
	}

	/** Creates a FPropertyHash object using the substruct and property found in RootStruct.
	 *  @param SubStructName name of the inner struct.
	 *  @param SubSubStructName name of the inner inner struct.
	 *  @param PropertyName name of property to find.
	 *	@return a FPropertyHash object containing all properties leading to innermost property.
	 */
	template <typename RootStruct>
	FPropertyHash CreatePropertyHash(const FName SubStructName, const FName SubSubStructName, const FName PropertyName)
	{
		FPropertyHash Hash;
		if (const FStructProperty* StructProperty = FindFProperty<FStructProperty>(RootStruct::StaticStruct(),
			SubStructName))
		{
			Hash.Properties.Add(StructProperty);
			if (const FStructProperty* SubStructProperty = FindFProperty<FStructProperty>(StructProperty->Struct,
				SubSubStructName))
			{
				Hash.Properties.Add(SubStructProperty);
				if (const FProperty* Property = FindFProperty<FProperty>(SubStructProperty->Struct, PropertyName))
				{
					Hash.Properties.Add(Property);
				}
			}
		}
		return Hash;
	}

	/** Creates a FPropertyHash object using the substruct and property found in RootStruct.
	 *  @param SubStructName name of the inner struct.
	 *  @param PropertyName name of property to find.
	 *	@return a FPropertyHash object containing all properties leading to innermost property.
	 */
	template <typename RootStruct>
	uint32 GetPropertyHash(const FName SubStructName, const FName PropertyName)
	{
		FPropertyHash Hash;
		if (const FStructProperty* StructProperty = FindFProperty<FStructProperty>(RootStruct::StaticStruct(),
			SubStructName))
		{
			Hash.Properties.Add(StructProperty);
			if (const FProperty* Property = FindFProperty<FProperty>(StructProperty->Struct, PropertyName))
			{
				Hash.Properties.Add(Property);
			}
		}
		return GetTypeHash(Hash);
	}

	/** Creates a FPropertyHash object using the substruct and property found in RootStruct.
	 *  @param SubStructName name of the inner struct.
	 *  @param SubSubStructName name of the inner inner struct.
	 *  @param PropertyName name of property to find.
	 *	@return a FPropertyHash object containing all properties leading to innermost property.
	 */
	template <typename RootStruct>
	uint32 GetPropertyHash(const FName SubStructName, const FName SubSubStructName, const FName PropertyName)
	{
		FPropertyHash Hash;
		if (const FStructProperty* StructProperty = FindFProperty<FStructProperty>(RootStruct::StaticStruct(),
			SubStructName))
		{
			Hash.Properties.Add(StructProperty);
			if (const FStructProperty* SubStructProperty = FindFProperty<FStructProperty>(StructProperty->Struct,
				SubSubStructName))
			{
				Hash.Properties.Add(SubStructProperty);
				if (const FProperty* Property = FindFProperty<FProperty>(SubStructProperty->Struct, PropertyName))
				{
					Hash.Properties.Add(Property);
				}
			}
		}
		return GetTypeHash(Hash);
	}
}

/* ---------------------- */
/* -- FValidationCheck -- */
/* ---------------------- */

bool FUniqueValidationCheckData::IsEmpty() const
{
	return StringTableKey.IsEmpty() && DynamicStringTableKey.IsEmpty();
}

void FValidationProperty::AddCheck(const FValidationCheckPtr& Check, const FUniqueValidationCheckData& Data)
{
	Checks.Add(Check);
	if (!Data.IsEmpty())
	{
		CheckData.Add(Check, Data).WarningType = Check->WarningType;
	}
}

void FValidationResult::AddValidationCheckResult(FValidationCheckResult&& Check)
{
	if (Check.bSuccess)
	{
		SucceededValidationCheckResults.Add(MoveTemp(Check));
	}
	else
	{
		FailedValidationCheckResults.Add(MoveTemp(Check));
	}
}

TSet<FValidationCheckResult, FValidationCheckKeyFuncs> FValidationResult::GetSucceeded() const
{
	return SucceededValidationCheckResults;
}

TSet<FValidationCheckResult, FValidationCheckKeyFuncs> FValidationResult::GetFailed() const
{
	return FailedValidationCheckResults;
}

bool FValidationResult::Contains(const FValidationCheckPtr& Check) const
{
	return SucceededValidationCheckResults.Contains(Check) || FailedValidationCheckResults.Contains(Check);
}

/* ------------------------------------ */
/* -- UBSGameModeValidator::FPrivate -- */
/* ------------------------------------ */

class UBSGameModeValidator::FPrivate
{
public:
	FPrivate();

	void SetupValidationChecks();

	FValidationPropertyPtr CreateValidationProperty(const FPropertyHash& InProperty,
		const EGameModeCategory InGameModeCategory);

	FValidationCheckPtr CreateValidationCheck(const TSet<FPropertyHash>& InvolvedProperties,
		EGameModeWarningType WarningType, TFunction<bool(const TSharedPtr<FBSConfig>&, TArray<int32>&)> Lambda);

	static void AddValidationCheckToProperty(const FValidationPropertyPtr& PropPtr, const FValidationCheckPtr& CheckPtr,
		const FString& TooltipKey = FString(), const FString& DynamicTooltipKey = FString());

	static float GetMinRequiredHorizontalSpread(const TSharedPtr<FBSConfig>& Config);

	static float GetMinRequiredVerticalSpread(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxTargetDiameter(const TSharedPtr<FBSConfig>& Config);

	static int32 GetMaxAllowedNumHorizontalTargets(const TSharedPtr<FBSConfig>& Config);

	static int32 GetMaxAllowedNumVerticalTargets(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedHorizontalSpacing(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedVerticalSpacing(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedTargetScale(const TSharedPtr<FBSConfig>& Config);

	static void ValidateCheckGroup(const TSharedPtr<FBSConfig>& Config, TSet<FValidationCheckPtr>& Checks,
		FValidationResult& Result);

	TSet<FValidationPropertyPtr, FValidationPropertyKeyFuncs> ValidationProperties;
};

UBSGameModeValidator::FPrivate::FPrivate()
{
	SetupValidationChecks();
}

void UBSGameModeValidator::FPrivate::SetupValidationChecks()
{
	FValidationCheckPtr CheckPtr = nullptr;
	const FName TargetConfigName = GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig);
	const FName GridConfigName = GET_MEMBER_NAME_CHECKED(FBSConfig, GridConfig);
	const FName AIConfigName = GET_MEMBER_NAME_CHECKED(FBSConfig, AIConfig);
	const FName DynamicSpawnAreaName = GET_MEMBER_NAME_CHECKED(FBSConfig, DynamicSpawnAreaScaling);
	const FName DynamicTargetName = GET_MEMBER_NAME_CHECKED(FBSConfig, DynamicTargetScaling);
	const FName BoxBoundsName = GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, BoxBounds);
	const FName GridSpacingName = GET_MEMBER_NAME_CHECKED(FBS_GridConfig, GridSpacing);

	const FPropertyHash BatchSpawning = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bUseBatchSpawning));
	const FPropertyHash SpawnEveryOtherTargetInCenter = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bUseBatchSpawning));
	const FPropertyHash AllowSpawnWithoutActivation = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bAllowSpawnWithoutActivation));
	const FPropertyHash MovingTargetDirectionMode = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MovingTargetDirectionMode));
	const FPropertyHash TargetSpawnResponses = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetSpawnResponses));
	const FPropertyHash TargetActivationResponses = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetActivationResponses));
	const FPropertyHash TargetDeactivationResponses = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDeactivationResponses));
	const FPropertyHash BoxBounds = CreatePropertyHash<FBSConfig>(TargetConfigName, BoxBoundsName);
	const FPropertyHash BoxBoundsX = CreatePropertyHash<FBSConfig>(TargetConfigName, BoxBoundsName,
		GET_MEMBER_NAME_CHECKED(FVector, X));
	const FPropertyHash BoxBoundsY = CreatePropertyHash<FBSConfig>(TargetConfigName, BoxBoundsName,
		GET_MEMBER_NAME_CHECKED(FVector, Y));
	const FPropertyHash BoxBoundsZ = CreatePropertyHash<FBSConfig>(TargetConfigName, BoxBoundsName,
		GET_MEMBER_NAME_CHECKED(FVector, Z));
	const FPropertyHash NumHorizontalGridTargets = CreatePropertyHash<FBSConfig>(GridConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_GridConfig, NumHorizontalGridTargets));
	const FPropertyHash NumVerticalGridTargets = CreatePropertyHash<FBSConfig>(GridConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_GridConfig, NumVerticalGridTargets));
	const FPropertyHash GridSpacingX = CreatePropertyHash<FBSConfig>(GridConfigName, GridSpacingName,
		GET_MEMBER_NAME_CHECKED(FVector2d, X));
	const FPropertyHash GridSpacingY = CreatePropertyHash<FBSConfig>(GridConfigName, GridSpacingName,
		GET_MEMBER_NAME_CHECKED(FVector2d, Y));
	const FPropertyHash TargetDistributionPolicy = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDistributionPolicy));
	const FPropertyHash EnableReinforcementLearning = CreatePropertyHash<FBSConfig>(AIConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_AIConfig, bEnableReinforcementLearning));
	const FPropertyHash MinSpawnedTargetScale = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinSpawnedTargetScale));
	const FPropertyHash MaxSpawnedTargetScale = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxSpawnedTargetScale));
	const FPropertyHash TargetDamageType = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDamageType));
	const FPropertyHash StartBounds = CreatePropertyHash<FBSConfig>(DynamicSpawnAreaName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, StartBounds));
	const FPropertyHash StartBoundsX = CreatePropertyHash<FBSConfig>(DynamicSpawnAreaName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, StartBounds), GET_MEMBER_NAME_CHECKED(FVector, X));
	const FPropertyHash StartBoundsY = CreatePropertyHash<FBSConfig>(DynamicSpawnAreaName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, StartBounds), GET_MEMBER_NAME_CHECKED(FVector, Y));
	const FPropertyHash StartBoundsZ = CreatePropertyHash<FBSConfig>(DynamicSpawnAreaName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, StartBounds), GET_MEMBER_NAME_CHECKED(FVector, Z));

	FValidationPropertyPtr SpawnEveryOtherTargetInCenterPtr = CreateValidationProperty(SpawnEveryOtherTargetInCenter,
		EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr BatchSpawningPtr =
		CreateValidationProperty(BatchSpawning, EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr AllowSpawnWithoutActivationPtr = CreateValidationProperty(AllowSpawnWithoutActivation,
		EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr MovingTargetDirectionModePtr = CreateValidationProperty(MovingTargetDirectionMode,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr TargetSpawnResponsesPtr = CreateValidationProperty(TargetSpawnResponses,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr TargetActivationResponsesPtr = CreateValidationProperty(TargetActivationResponses,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr TargetDeactivationResponsesPtr = CreateValidationProperty(TargetDeactivationResponses,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr BoxBoundsPtr = CreateValidationProperty(BoxBounds, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr BoxBoundsXPtr = CreateValidationProperty(BoxBoundsX, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr BoxBoundsYPtr = CreateValidationProperty(BoxBoundsY, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr BoxBoundsZPtr = CreateValidationProperty(BoxBoundsZ, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr TargetDistributionPolicyPtr = CreateValidationProperty(TargetDistributionPolicy,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr NumHorizontalGridTargetsPtr = CreateValidationProperty(NumHorizontalGridTargets,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr NumVerticalGridTargetsPtr = CreateValidationProperty(NumVerticalGridTargets,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr MinSpawnedTargetScalePtr = CreateValidationProperty(MinSpawnedTargetScale,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr MaxSpawnedTargetScalePtr = CreateValidationProperty(MaxSpawnedTargetScale,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr GridSpacingXPtr = CreateValidationProperty(GridSpacingX, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr GridSpacingYPtr = CreateValidationProperty(GridSpacingY, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr EnableReinforcementLearningPtr = CreateValidationProperty(EnableReinforcementLearning,
		EGameModeCategory::SpawnArea);

	auto SpawnEveryOtherAndBatchLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.bSpawnEveryOtherTargetInCenter && Config->TargetConfig.bUseBatchSpawning);
	};
	auto SpawnEveryOtherAndAllowSpawnLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.bSpawnEveryOtherTargetInCenter && Config->TargetConfig.
			bAllowSpawnWithoutActivation);
	};

	// SpawnEveryOtherTargetInCenter & BatchSpawning checks
	CheckPtr = CreateValidationCheck({SpawnEveryOtherTargetInCenter, BatchSpawning}, EGameModeWarningType::Warning,
		SpawnEveryOtherAndBatchLambda);
	AddValidationCheckToProperty(SpawnEveryOtherTargetInCenterPtr, CheckPtr,
		TEXT("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning"));
	AddValidationCheckToProperty(BatchSpawningPtr, CheckPtr,
		TEXT("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning2"));

	// SpawnEveryOtherTargetInCenter & AllowSpawnWithoutActivation checks
	CheckPtr = CreateValidationCheck({SpawnEveryOtherTargetInCenter, AllowSpawnWithoutActivation},
		EGameModeWarningType::Warning, SpawnEveryOtherAndAllowSpawnLambda);
	AddValidationCheckToProperty(SpawnEveryOtherTargetInCenterPtr, CheckPtr,
		TEXT("Invalid_SpawnEveryOtherTargetInCenter_AllowSpawnWithoutActivation"));
	AddValidationCheckToProperty(AllowSpawnWithoutActivationPtr, CheckPtr,
		TEXT("Invalid_SpawnEveryOtherTargetInCenter_AllowSpawnWithoutActivation2"));

	auto SpawnVelocityLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeVelocity));
	};
	auto SpawnDirectionLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeDirection));
	};
	auto ActivationVelocityLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeVelocity));
	};
	auto ActivationDirectionLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeDirection));
	};
	auto DeactivationVelocityLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeVelocity));
	};
	auto DeactivationDirectionLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeDirection));
	};
	auto BoxBoundsLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.BoxBounds.X <= 0.f && Config->TargetConfig.MovingTargetDirectionMode ==
			EMovingTargetDirectionMode::ForwardOnly);
	};

	// MovingTargetDirectionMode & Target Spawn Responses
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetSpawnResponses}, EGameModeWarningType::Caution,
		SpawnVelocityLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, TEXT("Invalid_Velocity_MTDM_None_2"));
	AddValidationCheckToProperty(TargetSpawnResponsesPtr, CheckPtr, TEXT("Invalid_Velocity_MTDM_None"));
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetSpawnResponses}, EGameModeWarningType::Caution,
		SpawnDirectionLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, TEXT("Invalid_Direction_MTDM_None_2"));
	AddValidationCheckToProperty(TargetSpawnResponsesPtr, CheckPtr, TEXT("Invalid_Direction_MTDM_None"));

	// MovingTargetDirectionMode & Target Activation Responses
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetActivationResponses},
		EGameModeWarningType::Caution, ActivationVelocityLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, TEXT("Invalid_Velocity_MTDM_None_2"));
	AddValidationCheckToProperty(TargetActivationResponsesPtr, CheckPtr, TEXT("Invalid_Velocity_MTDM_None"));
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetActivationResponses},
		EGameModeWarningType::Caution, ActivationDirectionLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, TEXT("Invalid_Direction_MTDM_None_2"));
	AddValidationCheckToProperty(TargetActivationResponsesPtr, CheckPtr, TEXT("Invalid_Direction_MTDM_None"));

	// MovingTargetDirectionMode & Target Deactivation Responses
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetActivationResponses},
		EGameModeWarningType::Caution, DeactivationVelocityLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, TEXT("Invalid_Velocity_MTDM_None_2"));
	AddValidationCheckToProperty(TargetDeactivationResponsesPtr, CheckPtr, TEXT("Invalid_Velocity_MTDM_None"));
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetDeactivationResponses},
		EGameModeWarningType::Caution, DeactivationDirectionLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, TEXT("Invalid_Direction_MTDM_None_2"));
	AddValidationCheckToProperty(TargetDeactivationResponsesPtr, CheckPtr, TEXT("Invalid_Direction_MTDM_None"));

	// MovingTargetDirectionMode & Box Bounds
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, BoxBoundsX}, EGameModeWarningType::Caution,
		BoxBoundsLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr,
		TEXT("Caution_ZeroForwardDistance_MTDM_ForwardOnly"));
	AddValidationCheckToProperty(BoxBoundsXPtr, CheckPtr, TEXT("Caution_ZeroForwardDistance_MTDM_ForwardOnly_2"));

	auto NumHorizontalGridTargetsLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		if (Config->TargetConfig.TargetDistributionPolicy != ETargetDistributionPolicy::Grid)
		{
			return true;
		}
		Values.Add(GetMaxAllowedNumHorizontalTargets(Config));
		return Values[0] >= Config->GridConfig.NumHorizontalGridTargets;
	};
	auto NumVerticalGridTargetsLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		if (Config->TargetConfig.TargetDistributionPolicy != ETargetDistributionPolicy::Grid)
		{
			return true;
		}
		Values.Add(GetMaxAllowedNumVerticalTargets(Config));
		return Values[0] >= Config->GridConfig.NumVerticalGridTargets;
	};
	auto GridSpacingHorizontalLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		if (Config->TargetConfig.TargetDistributionPolicy != ETargetDistributionPolicy::Grid)
		{
			return true;
		}
		Values.Add(static_cast<int32>(GetMaxAllowedHorizontalSpacing(Config)));
		return Values[0] >= Config->GridConfig.GridSpacing.X;
	};
	auto GridSpacingVerticalLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		if (Config->TargetConfig.TargetDistributionPolicy != ETargetDistributionPolicy::Grid)
		{
			return true;
		}
		Values.Add(static_cast<int32>(GetMaxAllowedVerticalSpacing(Config)));
		return Values[0] >= Config->GridConfig.GridSpacing.Y;
	};
	auto TargetDistributionPolicyLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::HeadshotHeightOnly &&
			Config->AIConfig.bEnableReinforcementLearning);
	};

	// NumHorizontalGridTargets checks
	CheckPtr = CreateValidationCheck(
		{
			TargetDistributionPolicy, NumHorizontalGridTargets, GridSpacingX, MinSpawnedTargetScale,
			MaxSpawnedTargetScale
		}, EGameModeWarningType::Warning, NumHorizontalGridTargetsLambda);
	AddValidationCheckToProperty(NumHorizontalGridTargetsPtr, CheckPtr,
		TEXT("Invalid_Grid_NumHorizontalTargets_Fallback"), TEXT("Invalid_Grid_NumHorizontalTargets"));
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr);
	AddValidationCheckToProperty(GridSpacingXPtr, CheckPtr, TEXT("Invalid_Grid_HorizontalSpacing_Fallback"),
		TEXT("Invalid_Grid_HorizontalSpacing"));
	AddValidationCheckToProperty(MinSpawnedTargetScalePtr, CheckPtr,
		TEXT("Invalid_Grid_MaxSpawnedTargetScale_Fallback"), TEXT("Invalid_Grid_MaxSpawnedTargetScale"));
	AddValidationCheckToProperty(MaxSpawnedTargetScalePtr, CheckPtr);

	// NumVerticalGridTargets checks
	CheckPtr = CreateValidationCheck({
		TargetDistributionPolicy, NumVerticalGridTargets, GridSpacingY, MinSpawnedTargetScale, MaxSpawnedTargetScale
	}, EGameModeWarningType::Warning, NumVerticalGridTargetsLambda);
	AddValidationCheckToProperty(NumVerticalGridTargetsPtr, CheckPtr, TEXT("Invalid_Grid_NumVerticalTargets_Fallback"),
		TEXT("Invalid_Grid_NumVerticalTargets"));
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr);
	AddValidationCheckToProperty(GridSpacingYPtr, CheckPtr, TEXT("Invalid_Grid_VerticalSpacing_Fallback"),
		TEXT("Invalid_Grid_VerticalSpacing"));
	AddValidationCheckToProperty(MinSpawnedTargetScalePtr, CheckPtr,
		TEXT("Invalid_Grid_MaxSpawnedTargetScale_Fallback"), TEXT("Invalid_Grid_MaxSpawnedTargetScale"));
	AddValidationCheckToProperty(MaxSpawnedTargetScalePtr, CheckPtr);

	// HorizontalSpacing checks
	CheckPtr = CreateValidationCheck({
		TargetDistributionPolicy, NumHorizontalGridTargets, GridSpacingX, MinSpawnedTargetScale, MaxSpawnedTargetScale
	}, EGameModeWarningType::Warning, GridSpacingHorizontalLambda);
	AddValidationCheckToProperty(NumHorizontalGridTargetsPtr, CheckPtr, TEXT("Invalid_Grid_HorizontalSpacing_Fallback"),
		TEXT("Invalid_Grid_HorizontalSpacing"));
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr);
	AddValidationCheckToProperty(GridSpacingXPtr, CheckPtr, TEXT("Invalid_Grid_HorizontalSpacing_Fallback"),
		TEXT("Invalid_Grid_HorizontalSpacing"));
	AddValidationCheckToProperty(MinSpawnedTargetScalePtr, CheckPtr,
		TEXT("Invalid_Grid_MaxSpawnedTargetScale_Fallback"), TEXT("Invalid_Grid_MaxSpawnedTargetScale"));
	AddValidationCheckToProperty(MaxSpawnedTargetScalePtr, CheckPtr);

	// VerticalSpacing checks
	CheckPtr = CreateValidationCheck({
		TargetDistributionPolicy, NumVerticalGridTargets, GridSpacingY, MinSpawnedTargetScale, MaxSpawnedTargetScale
	}, EGameModeWarningType::Warning, GridSpacingVerticalLambda);
	AddValidationCheckToProperty(NumVerticalGridTargetsPtr, CheckPtr, TEXT("Invalid_Grid_VerticalSpacing_Fallback"),
		TEXT("Invalid_Grid_VerticalSpacing"));
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr);
	AddValidationCheckToProperty(GridSpacingYPtr, CheckPtr, TEXT("Invalid_Grid_VerticalSpacing_Fallback"),
		TEXT("Invalid_Grid_VerticalSpacing"));
	AddValidationCheckToProperty(MinSpawnedTargetScalePtr, CheckPtr,
		TEXT("Invalid_Grid_MaxSpawnedTargetScale_Fallback"), TEXT("Invalid_Grid_MaxSpawnedTargetScale"));
	AddValidationCheckToProperty(MaxSpawnedTargetScalePtr, CheckPtr);

	// TargetDistributionPolicy checks
	CheckPtr = CreateValidationCheck({TargetDistributionPolicy, EnableReinforcementLearning},
		EGameModeWarningType::Warning, TargetDistributionPolicyLambda);
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr, TEXT("Invalid_HeadshotHeightOnly_AI"));
	AddValidationCheckToProperty(EnableReinforcementLearningPtr, CheckPtr, TEXT("Invalid_HeadshotHeightOnly_AI"));

	FValidationPropertyPtr TargetDamageTypePtr = CreateValidationProperty(TargetDamageType,
		EGameModeCategory::SpawnArea);

	auto InvalidTrackingAILambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->AIConfig.bEnableReinforcementLearning && Config->TargetConfig.TargetDamageType ==
			ETargetDamageType::Tracking);
	};
	auto InvalidHeadshotAILambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->AIConfig.bEnableReinforcementLearning && Config->TargetConfig.TargetDistributionPolicy ==
			ETargetDistributionPolicy::HeadshotHeightOnly);
	};

	CheckPtr = CreateValidationCheck({EnableReinforcementLearning, TargetDamageType}, EGameModeWarningType::Warning,
		InvalidTrackingAILambda);
	AddValidationCheckToProperty(EnableReinforcementLearningPtr, CheckPtr, TEXT("Invalid_Tracking_AI"));
	AddValidationCheckToProperty(TargetDamageTypePtr, CheckPtr, TEXT("Invalid_Tracking_AI"));

	CheckPtr = CreateValidationCheck({EnableReinforcementLearning, TargetDistributionPolicy},
		EGameModeWarningType::Warning, InvalidHeadshotAILambda);
	AddValidationCheckToProperty(EnableReinforcementLearningPtr, CheckPtr, TEXT("Invalid_HeadshotHeightOnly_AI"));
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr, TEXT("Invalid_HeadshotHeightOnly_AI"));
}

FValidationCheckPtr UBSGameModeValidator::FPrivate::CreateValidationCheck(const TSet<FPropertyHash>& InvolvedProperties,
	const EGameModeWarningType WarningType, TFunction<bool(const TSharedPtr<FBSConfig>&, TArray<int32>&)> Lambda)
{
	TSet<FValidationPropertyPtr> Properties;
	for (const FPropertyHash& PropertyHash : InvolvedProperties)
	{
		Properties.Add(*ValidationProperties.Find(GetTypeHash(PropertyHash)));
	}
	TSharedPtr<FValidationCheck> Check = MakeShareable(new FValidationCheck(MoveTemp(Properties), WarningType));
	Check->ValidationDelegate.BindLambda(Lambda);
	return Check;
}

FValidationPropertyPtr UBSGameModeValidator::FPrivate::CreateValidationProperty(const FPropertyHash& InProperty,
	const EGameModeCategory InGameModeCategory)
{
	FValidationPropertyPtr PropertyPtr = MakeShareable(new FValidationProperty(InProperty, InGameModeCategory));
	ValidationProperties.Add(PropertyPtr);
	return PropertyPtr;
}

void UBSGameModeValidator::FPrivate::AddValidationCheckToProperty(const FValidationPropertyPtr& PropPtr,
	const FValidationCheckPtr& CheckPtr, const FString& TooltipKey, const FString& DynamicTooltipKey)
{
	PropPtr->AddCheck(CheckPtr, FUniqueValidationCheckData(TooltipKey, DynamicTooltipKey,
		HashCombine(PointerHash(PropPtr.Get()), PointerHash(CheckPtr.Get()))));
}

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

void UBSGameModeValidator::FPrivate::ValidateCheckGroup(const TSharedPtr<FBSConfig>& Config,
	TSet<FValidationCheckPtr>& Checks, FValidationResult& Result)
{
	for (const FValidationCheckPtr& Check : Checks)
	{
		if (Result.Contains(Check))
		{
			continue;
		}
		TArray<int32> Values;
		const bool bResult = Check->ValidationDelegate.Execute(Config, Values);

		TMap<FValidationPropertyPtr, FUniqueValidationCheckData> PropertyData;
		for (const FValidationPropertyPtr& InvolvedProperty : Check->InvolvedProperties)
		{
			if (const FUniqueValidationCheckData* FoundData = InvolvedProperty->CheckData.Find(Check))
			{
				PropertyData.Add(InvolvedProperty, *FoundData);
			}
		}

		Result.AddValidationCheckResult(
			FValidationCheckResult(bResult, Check, MoveTemp(Values), MoveTemp(PropertyData)));
	}
}

/* -------------------------- */
/* -- UBSGameModeValidator -- */
/* -------------------------- */

UBSGameModeValidator::UBSGameModeValidator() : Impl(MakePimpl<FPrivate>())
{
}

FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig) const
{
	FValidationResult Result;
	for (const FValidationPropertyPtr& ValidationProperty : Impl->ValidationProperties)
	{
		Impl->ValidateCheckGroup(InConfig, ValidationProperty->Checks, Result);
	}
	return Result;
}

FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig, const FName SubStructName,
	const FName PropertyName) const
{
	FValidationResult Result;
	if (const FValidationPropertyPtr& ValidationProperty = FindValidationProperty(
		GetPropertyHash<FBSConfig>(SubStructName, PropertyName)))
	{
		Impl->ValidateCheckGroup(InConfig, ValidationProperty->Checks, Result);
	}
	return Result;
}

FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig,
	const TSet<uint32>& Properties) const
{
	FValidationResult Result;
	for (const uint32 PropertyHash : Properties)
	{
		if (const FValidationPropertyPtr& ValidationProperty = FindValidationProperty(PropertyHash))
		{
			Impl->ValidateCheckGroup(InConfig, ValidationProperty->Checks, Result);
		}
	}

	return Result;
}

uint32 UBSGameModeValidator::FindBSConfigProperty(const FName SubStructName, const FName PropertyName)
{
	return GetPropertyHash<FBSConfig>(SubStructName, PropertyName);
}

uint32 UBSGameModeValidator::FindBSConfigProperty(const FName SubStructName, const FName SubSubStructName,
	const FName PropertyName)
{
	return GetPropertyHash<FBSConfig>(SubStructName, SubSubStructName, PropertyName);
}

FValidationPropertyPtr UBSGameModeValidator::FindValidationProperty(const uint32 PropertyHash) const
{
	if (const FValidationPropertyPtr* Found = Impl->ValidationProperties.Find(PropertyHash))
	{
		return *Found;
	}
	return nullptr;
}

TSet<FValidationPropertyPtr, FValidationPropertyKeyFuncs>& UBSGameModeValidator::GetValidationProperties() const
{
	return Impl->ValidationProperties;
}
