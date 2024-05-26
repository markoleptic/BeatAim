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

/* ---------------------- */
/* -- FValidationCheck -- */
/* ---------------------- */

int32 FValidationCheck::GId = 0;

void FValidationProperty::AddCheck(const FValidationCheckPtr& Check, const FUniqueValidationCheckData& Data)
{
	Checks.Add(Check);
	CheckData.Add(Check, Data);
}

void FValidationResult::AddValidationCheckResult(FValidationCheckResult&& Check)
{
	if (Check.bSuccess)
	{
		SucceededValidationCheckResults[Check.WarningType].Add(MoveTemp(Check));
	}
	else
	{
		FailedValidationCheckResults[Check.WarningType].Add(MoveTemp(Check));
	}
}

const FValidationResultMap& FValidationResult::GetSucceeded() const
{
	return SucceededValidationCheckResults;
}

const FValidationResultMap& FValidationResult::GetFailed() const
{
	return FailedValidationCheckResults;
}

/* ------------------------------------ */
/* -- UBSGameModeValidator::FPrivate -- */
/* ------------------------------------ */

class UBSGameModeValidator::FPrivate
{
public:
	FPrivate();

	void SetupValidationChecks();

	FValidationPropertyPtr CreateValidationProperty(const FProperty* InProperty,
		const EGameModeCategory InGameModeCategory);

	static FValidationCheckPtr CreateValidationCheck(TSet<const FProperty*>&& InvolvedProperties,
		EGameModeWarningType WarningType, TFunction<bool(const TSharedPtr<FBSConfig>&, TArray<int32>&)> Lambda);

	static void AddValidationCheckToProperty(const FValidationPropertyPtr& PropPtr, const FValidationCheckPtr& CheckPtr,
		const FUniqueValidationCheckData& Data);

	static float GetMinRequiredHorizontalSpread(const TSharedPtr<FBSConfig>& Config);

	static float GetMinRequiredVerticalSpread(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxTargetDiameter(const TSharedPtr<FBSConfig>& Config);

	static int32 GetMaxAllowedNumHorizontalTargets(const TSharedPtr<FBSConfig>& Config);

	static int32 GetMaxAllowedNumVerticalTargets(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedHorizontalSpacing(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedVerticalSpacing(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedTargetScale(const TSharedPtr<FBSConfig>& Config);

	TSet<FValidationPropertyPtr, FValidationPropertyKeyFuncs> ValidationProperties;
};

UBSGameModeValidator::FPrivate::FPrivate()
{
	SetupValidationChecks();
}

void UBSGameModeValidator::FPrivate::SetupValidationChecks()
{
	FValidationCheckPtr CheckPtr = nullptr;

	const FProperty* BatchSpawning = FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bUseBatchSpawning));
	const FProperty* SpawnEveryOtherTargetInCenter = FindProperty<FBSConfig>(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig), GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bUseBatchSpawning));
	const FProperty* AllowSpawnWithoutActivation = FindProperty<FBSConfig>(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bAllowSpawnWithoutActivation));

	auto SpawnEveryOtherAndBatchLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.bSpawnEveryOtherTargetInCenter && Config->TargetConfig.bUseBatchSpawning);
	};
	auto SpawnEveryOtherAndAllowSpawnLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.bSpawnEveryOtherTargetInCenter && Config->TargetConfig.
			bAllowSpawnWithoutActivation);
	};

	FValidationPropertyPtr SpawnEveryOtherTargetInCenterPtr = CreateValidationProperty(SpawnEveryOtherTargetInCenter,
		EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr BatchSpawningPtr =
		CreateValidationProperty(BatchSpawning, EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr AllowSpawnWithoutActivationPtr = CreateValidationProperty(AllowSpawnWithoutActivation,
		EGameModeCategory::TargetSpawning);

	// SpawnEveryOtherTargetInCenter & BatchSpawning checks
	CheckPtr = CreateValidationCheck({SpawnEveryOtherTargetInCenter, BatchSpawning}, EGameModeWarningType::Warning,
		SpawnEveryOtherAndBatchLambda);
	AddValidationCheckToProperty(SpawnEveryOtherTargetInCenterPtr, CheckPtr, {
		TEXT("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning"), ""
	});
	AddValidationCheckToProperty(BatchSpawningPtr, CheckPtr, {
		TEXT("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning2"), ""
	});

	// SpawnEveryOtherTargetInCenter & AllowSpawnWithoutActivation checks
	CheckPtr = CreateValidationCheck({SpawnEveryOtherTargetInCenter, AllowSpawnWithoutActivation},
		EGameModeWarningType::Warning, SpawnEveryOtherAndAllowSpawnLambda);
	AddValidationCheckToProperty(SpawnEveryOtherTargetInCenterPtr, CheckPtr, {
		TEXT("Invalid_SpawnEveryOtherTargetInCenter_AllowSpawnWithoutActivation"), ""
	});
	AddValidationCheckToProperty(AllowSpawnWithoutActivationPtr, CheckPtr, {
		TEXT("Invalid_SpawnEveryOtherTargetInCenter_AllowSpawnWithoutActivation2"), ""
	});

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
	const FProperty* BoxBounds = FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, BoxBounds));

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

	FValidationPropertyPtr MovingTargetDirectionModePtr = CreateValidationProperty(MovingTargetDirectionMode,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr TargetSpawnResponsesPtr = CreateValidationProperty(TargetSpawnResponses,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr TargetActivationResponsesPtr = CreateValidationProperty(TargetActivationResponses,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr TargetDeactivationResponsesPtr = CreateValidationProperty(TargetDeactivationResponses,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr BoxBoundsPtr = CreateValidationProperty(BoxBounds, EGameModeCategory::SpawnArea);

	// MovingTargetDirectionMode & Target Spawn Responses
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetSpawnResponses}, EGameModeWarningType::Caution,
		SpawnVelocityLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, {TEXT("Invalid_Velocity_MTDM_None_2"), ""});
	AddValidationCheckToProperty(TargetSpawnResponsesPtr, CheckPtr, {TEXT("Invalid_Velocity_MTDM_None"), ""});
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetSpawnResponses}, EGameModeWarningType::Caution,
		SpawnDirectionLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, {TEXT("Invalid_Direction_MTDM_None_2"), ""});
	AddValidationCheckToProperty(TargetSpawnResponsesPtr, CheckPtr, {TEXT("Invalid_Direction_MTDM_None"), ""});

	// MovingTargetDirectionMode & Target Activation Responses
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetActivationResponses},
		EGameModeWarningType::Caution, ActivationVelocityLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, {TEXT("Invalid_Velocity_MTDM_None_2"), ""});
	AddValidationCheckToProperty(TargetActivationResponsesPtr, CheckPtr, {TEXT("Invalid_Velocity_MTDM_None"), ""});
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetActivationResponses},
		EGameModeWarningType::Caution, ActivationDirectionLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, {TEXT("Invalid_Direction_MTDM_None_2"), ""});
	AddValidationCheckToProperty(TargetActivationResponsesPtr, CheckPtr, {TEXT("Invalid_Direction_MTDM_None"), ""});

	// MovingTargetDirectionMode & Target Deactivation Responses
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetActivationResponses},
		EGameModeWarningType::Caution, DeactivationVelocityLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, {TEXT("Invalid_Velocity_MTDM_None_2"), ""});
	AddValidationCheckToProperty(TargetDeactivationResponsesPtr, CheckPtr, {TEXT("Invalid_Velocity_MTDM_None"), ""});
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, TargetDeactivationResponses},
		EGameModeWarningType::Caution, DeactivationDirectionLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr, {TEXT("Invalid_Direction_MTDM_None_2"), ""});
	AddValidationCheckToProperty(TargetDeactivationResponsesPtr, CheckPtr, {TEXT("Invalid_Direction_MTDM_None"), ""});

	// MovingTargetDirectionMode & Box Bounds
	CheckPtr = CreateValidationCheck({MovingTargetDirectionMode, BoxBounds}, EGameModeWarningType::Caution,
		BoxBoundsLambda);
	AddValidationCheckToProperty(MovingTargetDirectionModePtr, CheckPtr,
		{TEXT("Caution_ZeroForwardDistance_MTDM_ForwardOnly"), ""});
	AddValidationCheckToProperty(BoxBoundsPtr, CheckPtr, {TEXT("Caution_ZeroForwardDistance_MTDM_ForwardOnly_2"), ""});

	const FProperty* NumHorizontalGridTargets = FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, GridConfig),
		GET_MEMBER_NAME_CHECKED(FBS_GridConfig, NumHorizontalGridTargets));
	const FProperty* NumVerticalGridTargets = FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, GridConfig),
		GET_MEMBER_NAME_CHECKED(FBS_GridConfig, NumVerticalGridTargets));
	const FProperty* GridSpacing = FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, GridConfig),
		GET_MEMBER_NAME_CHECKED(FBS_GridConfig, GridSpacing));
	const FProperty* TargetDistributionPolicy = FindProperty<FBSConfig>(
		GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDistributionPolicy));
	const FProperty* EnableReinforcementLearning = FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, AIConfig),
		GET_MEMBER_NAME_CHECKED(FBS_AIConfig, bEnableReinforcementLearning));
	const FProperty* MinSpawnedTargetScale = FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinSpawnedTargetScale));
	const FProperty* MaxSpawnedTargetScale = FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxSpawnedTargetScale));
	const FProperty* TargetDamageType = FindProperty<FBSConfig>(GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig),
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDamageType));

	auto NumHorizontalGridTargetsLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		if (Config->TargetConfig.TargetDistributionPolicy != ETargetDistributionPolicy::Grid)
		{
			return true;
		}
		Values = {Config->GridConfig.NumHorizontalGridTargets, GetMaxAllowedNumHorizontalTargets(Config)};
		return Values[0] <= Values[1];
	};
	auto NumVerticalGridTargetsLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		if (Config->TargetConfig.TargetDistributionPolicy != ETargetDistributionPolicy::Grid)
		{
			return true;
		}
		Values = {Config->GridConfig.NumVerticalGridTargets, GetMaxAllowedNumVerticalTargets(Config)};
		return Values[0] <= Values[1];
	};
	auto GridSpacingHorizontalLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		if (Config->TargetConfig.TargetDistributionPolicy != ETargetDistributionPolicy::Grid)
		{
			return true;
		}
		Values = {
			static_cast<int32>(Config->GridConfig.GridSpacing.X),
			static_cast<int32>(GetMaxAllowedHorizontalSpacing(Config))
		};
		return Values[0] <= Values[1];
	};
	auto GridSpacingVerticalLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		if (Config->TargetConfig.TargetDistributionPolicy != ETargetDistributionPolicy::Grid)
		{
			return true;
		}
		Values = {
			static_cast<int32>(Config->GridConfig.GridSpacing.Y),
			static_cast<int32>(GetMaxAllowedVerticalSpacing(Config))
		};
		return Values[0] <= Values[1];
	};
	auto TargetDistributionPolicyLambda = [](const TSharedPtr<FBSConfig>& Config, TArray<int32>& Values)
	{
		return !(Config->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::HeadshotHeightOnly &&
			Config->AIConfig.bEnableReinforcementLearning);
	};

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
	FValidationPropertyPtr GridSpacingPtr = CreateValidationProperty(GridSpacing, EGameModeCategory::SpawnArea);

	// NumHorizontalGridTargets checks
	CheckPtr = CreateValidationCheck(
		{TargetDistributionPolicy, NumHorizontalGridTargets, GridSpacing, MinSpawnedTargetScale, MaxSpawnedTargetScale},
		EGameModeWarningType::Warning, NumHorizontalGridTargetsLambda);
	AddValidationCheckToProperty(NumHorizontalGridTargetsPtr, CheckPtr, {
		TEXT("Invalid_Grid_NumHorizontalTargets_Fallback"), TEXT("Invalid_Grid_NumHorizontalTargets")
	});
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(GridSpacingPtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(MinSpawnedTargetScalePtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(MaxSpawnedTargetScalePtr, CheckPtr, {"", ""});

	// NumVerticalGridTargets checks
	CheckPtr = CreateValidationCheck({
		TargetDistributionPolicy, NumVerticalGridTargets, GridSpacing, MinSpawnedTargetScale, MaxSpawnedTargetScale
	}, EGameModeWarningType::Warning, NumVerticalGridTargetsLambda);
	AddValidationCheckToProperty(NumVerticalGridTargetsPtr, CheckPtr, {
		TEXT("Invalid_Grid_NumVerticalTargets_Fallback"), TEXT("Invalid_Grid_NumVerticalTargets")
	});
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(GridSpacingPtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(MinSpawnedTargetScalePtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(MaxSpawnedTargetScalePtr, CheckPtr, {"", ""});

	// HorizontalSpacing checks
	CheckPtr = CreateValidationCheck({
		TargetDistributionPolicy, NumHorizontalGridTargets, GridSpacing, MinSpawnedTargetScale, MaxSpawnedTargetScale
	}, EGameModeWarningType::Warning, GridSpacingHorizontalLambda);
	AddValidationCheckToProperty(NumHorizontalGridTargetsPtr, CheckPtr, {
		TEXT("Invalid_Grid_HorizontalSpacing_Fallback"), TEXT("Invalid_Grid_HorizontalSpacing")
	});
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(GridSpacingPtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(MinSpawnedTargetScalePtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(MaxSpawnedTargetScalePtr, CheckPtr, {"", ""});

	// VerticalSpacing checks
	CheckPtr = CreateValidationCheck({
		TargetDistributionPolicy, NumVerticalGridTargets, GridSpacing, MinSpawnedTargetScale, MaxSpawnedTargetScale
	}, EGameModeWarningType::Warning, GridSpacingVerticalLambda);
	AddValidationCheckToProperty(NumVerticalGridTargetsPtr, CheckPtr, {
		TEXT("Invalid_Grid_VerticalSpacing_Fallback"), TEXT("Invalid_Grid_VerticalSpacing")
	});
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(GridSpacingPtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(MinSpawnedTargetScalePtr, CheckPtr, {"", ""});
	AddValidationCheckToProperty(MaxSpawnedTargetScalePtr, CheckPtr, {"", ""});

	FValidationPropertyPtr EnableReinforcementLearningPtr = CreateValidationProperty(EnableReinforcementLearning,
		EGameModeCategory::SpawnArea);

	// TargetDistributionPolicy checks
	CheckPtr = CreateValidationCheck({TargetDistributionPolicy, EnableReinforcementLearning},
		EGameModeWarningType::Warning, TargetDistributionPolicyLambda);
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr, {TEXT("Invalid_HeadshotHeightOnly_AI"), ""});
	AddValidationCheckToProperty(EnableReinforcementLearningPtr, CheckPtr, {TEXT("Invalid_HeadshotHeightOnly_AI"), ""});

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
	AddValidationCheckToProperty(EnableReinforcementLearningPtr, CheckPtr, {TEXT("Invalid_Tracking_AI"), ""});
	AddValidationCheckToProperty(TargetDamageTypePtr, CheckPtr, {TEXT("Invalid_Tracking_AI"), ""});

	CheckPtr = CreateValidationCheck({EnableReinforcementLearning, TargetDistributionPolicy},
		EGameModeWarningType::Warning, InvalidHeadshotAILambda);
	AddValidationCheckToProperty(EnableReinforcementLearningPtr, CheckPtr, {TEXT("Invalid_HeadshotHeightOnly_AI"), ""});
	AddValidationCheckToProperty(TargetDistributionPolicyPtr, CheckPtr, {TEXT("Invalid_HeadshotHeightOnly_AI"), ""});
}

FValidationCheckPtr UBSGameModeValidator::FPrivate::CreateValidationCheck(TSet<const FProperty*>&& InvolvedProperties,
	const EGameModeWarningType WarningType, TFunction<bool(const TSharedPtr<FBSConfig>&, TArray<int32>&)> Lambda)
{
	TSharedPtr<FValidationCheck> Check = MakeShareable(new FValidationCheck(MoveTemp(InvolvedProperties), WarningType));
	Check->ValidationDelegate.BindLambda(Lambda);
	return Check;
}

FValidationPropertyPtr UBSGameModeValidator::FPrivate::CreateValidationProperty(const FProperty* InProperty,
	const EGameModeCategory InGameModeCategory)
{
	FValidationPropertyPtr PropertyPtr = MakeShareable(new FValidationProperty(InProperty, InGameModeCategory));
	ValidationProperties.Add(PropertyPtr);
	return PropertyPtr;
}

void UBSGameModeValidator::FPrivate::AddValidationCheckToProperty(const FValidationPropertyPtr& PropPtr,
	const FValidationCheckPtr& CheckPtr, const FUniqueValidationCheckData& Data)
{
	PropPtr->AddCheck(CheckPtr, Data);
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
		for (const FValidationCheckPtr& Check : ValidationProperty->Checks)
		{
			TArray<int32> Values;
			const bool bResult = Check->ValidationDelegate.Execute(InConfig, Values);

			TMap<const FProperty*, FUniqueValidationCheckData> PropertyData;
			for (const FProperty* Property : Check->InvolvedProperties)
			{
				if (const FValidationPropertyPtr* FoundPtr = Impl->ValidationProperties.Find(Property))
				{
					if (const FUniqueValidationCheckData* FoundData = (*FoundPtr)->CheckData.Find(Check))
					{
						PropertyData.Add(Property, *FoundData);
					}
				}
			}

			Result.AddValidationCheckResult(FValidationCheckResult(bResult, Check, MoveTemp(Values),
				MoveTemp(PropertyData)));
		}
	}
	return Result;
}

FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig, const FName SubStructName,
	const FName PropertyName) const
{
	FValidationResult Result;

	if (const FValidationPropertyPtr& ValidationProperty = FindValidationProperty(
		FindProperty<FBSConfig>(SubStructName, PropertyName)))
	{
		for (const FValidationCheckPtr& Check : ValidationProperty->Checks)
		{
			TArray<int32> Values;
			const bool bResult = Check->ValidationDelegate.Execute(InConfig, Values);

			TMap<const FProperty*, FUniqueValidationCheckData> PropertyData;
			for (const FProperty* InvolvedProperty : Check->InvolvedProperties)
			{
				if (const FValidationPropertyPtr* FoundPtr = Impl->ValidationProperties.Find(InvolvedProperty))
				{
					if (const FUniqueValidationCheckData* FoundData = (*FoundPtr)->CheckData.Find(Check))
					{
						PropertyData.Add(InvolvedProperty, *FoundData);
					}
				}
			}

			Result.AddValidationCheckResult(FValidationCheckResult(bResult, Check, MoveTemp(Values),
				MoveTemp(PropertyData)));
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
		if (const FValidationPropertyPtr& ValidationProperty = FindValidationProperty(Property))
		{
			for (const FValidationCheckPtr& Check : ValidationProperty->Checks)
			{
				TArray<int32> Values;
				const bool bResult = Check->ValidationDelegate.Execute(InConfig, Values);

				TMap<const FProperty*, FUniqueValidationCheckData> PropertyData;
				for (const FProperty* InvolvedProperty : Check->InvolvedProperties)
				{
					if (const FValidationPropertyPtr* FoundPtr = Impl->ValidationProperties.Find(InvolvedProperty))
					{
						if (const FUniqueValidationCheckData* FoundData = (*FoundPtr)->CheckData.Find(Check))
						{
							PropertyData.Add(InvolvedProperty, *FoundData);
						}
					}
				}

				Result.AddValidationCheckResult(FValidationCheckResult(bResult, Check, MoveTemp(Values),
					MoveTemp(PropertyData)));
			}
		}
	}

	return Result;
}

const FProperty* UBSGameModeValidator::FindBSConfigProperty(const FName SubStructName, const FName PropertyName)
{
	return FindProperty<FBSConfig>(SubStructName, PropertyName);
}

FValidationPropertyPtr UBSGameModeValidator::FindValidationProperty(const FProperty* Property) const
{
	if (const FValidationPropertyPtr* Found = Impl->ValidationProperties.Find(Property))
	{
		return *Found;
	}
	return nullptr;
}
