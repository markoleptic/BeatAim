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
	[[maybe_unused]] const FProperty* FindProperty(const FName SubStructName, const FName PropertyName)
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
	 *  @param SubSubStructName name of the inner struct inside the substruct.
	 *  @param PropertyName name of property to find.
	 *	@return a property if found, otherwise null.
	 */
	template <typename RootStruct>
	[[maybe_unused]] const FProperty* FindProperty(const FName SubStructName, const FName SubSubStructName,
		const FName PropertyName)
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
	 *  @param SubSubStructName name of the inner struct inside the substruct.
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
	 *  @param SubSubStructName name of the inner struct inside the substruct.
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

FValidationCheckData::FValidationCheckData(): WarningType(EGameModeWarningType::None), GridSnapSize(0),
                                              bCalculatedValuesAreIntegers(false),
                                              bRequireOtherPropertiesToBeChanged(false)
{
	NumberFormattingOptions.SetRoundingMode(HalfFromZero).SetMinimumFractionalDigits(0).SetMaximumFractionalDigits(2);
}

FValidationCheckData::FValidationCheckData(const EGameModeWarningType GameModeWarningType) :
	WarningType(GameModeWarningType), GridSnapSize(0), bCalculatedValuesAreIntegers(false),
	bRequireOtherPropertiesToBeChanged(false)
{
	NumberFormattingOptions.SetRoundingMode(HalfFromZero).SetMinimumFractionalDigits(0).SetMaximumFractionalDigits(2);
}

FValidationCheckData::FValidationCheckData(const EGameModeWarningType GameModeWarningType,
	const FString& InStringTableKey, const FString& InDynamicStringTableKey, const int32 InGridSnapSize,
	const bool InCalculatedValuesAreIntegers) : StringTableKey(InStringTableKey),
	                                            DynamicStringTableKey(InDynamicStringTableKey),
	                                            WarningType(GameModeWarningType), GridSnapSize(InGridSnapSize),
	                                            bCalculatedValuesAreIntegers(InCalculatedValuesAreIntegers),
	                                            bRequireOtherPropertiesToBeChanged(false)
{
	NumberFormattingOptions.SetRoundingMode(HalfFromZero).SetMinimumFractionalDigits(2).SetMaximumFractionalDigits(2);
}

void FValidationCheckData::ResetLiveData()
{
	CalculatedValues.Empty();
	bRequireOtherPropertiesToBeChanged = false;
}

bool FValidationCheckData::IsEmpty() const
{
	return StringTableKey.IsEmpty() && DynamicStringTableKey.IsEmpty();
}

FValidationCheck::FValidationCheck() : ValidationCheckData(FValidationCheckData()),
                                       ValidationPrerequisiteDelegate(nullptr), ValidationDelegate(nullptr),
                                       OwningPropertyHash(0)
{
}

FValidationCheck::FValidationCheck(const uint32 InOwningPropertyHash, const EGameModeWarningType GameModeWarningType):
	ValidationCheckData(FValidationCheckData(GameModeWarningType)), ValidationPrerequisiteDelegate(nullptr),
	ValidationDelegate(nullptr), OwningPropertyHash(InOwningPropertyHash)
{
}

void FValidationCheck::AddData(const FString& InStringTableKey, const FString& InDynamicStringTableKey,
	const int32 InGridSnapSize, const bool InCalculatedValuesAreIntegers)
{
	ValidationCheckData.StringTableKey = InStringTableKey;
	ValidationCheckData.DynamicStringTableKey = InDynamicStringTableKey;
	ValidationCheckData.GridSnapSize = InGridSnapSize;
	ValidationCheckData.bCalculatedValuesAreIntegers = InCalculatedValuesAreIntegers;
}

FValidationProperty::FValidationProperty(): GameModeCategory(EGameModeCategory::None), Hash(0)
{
}

FValidationProperty::FValidationProperty(const FPropertyHash& InPropertyHash,
	const EGameModeCategory InGameModeCategory) : GameModeCategory(InGameModeCategory),
	                                              Hash(GetTypeHash(InPropertyHash))
{
	for (const FProperty* Prop : InPropertyHash.Properties)
	{
		PropertyName += Prop->GetFullName() + ".";
	}
}

void FValidationProperty::AddDependent(const FValidationPropertyPtr& Dependent,
	const FValidationCheckPtr& ValidationCheck)
{
	ensure(Dependent->ValidationChecks.Contains(ValidationCheck));
	if (Dependents.Contains(Dependent))
	{
		Dependents.Find(Dependent)->Add(ValidationCheck);
	}
	else
	{
		Dependents.Add(Dependent, {ValidationCheck});
	}
}

FValidationCheckResult::FValidationCheckResult() : bSuccess(false), bBypassed(false), OwningPropertyHash(0),
                                                   WarningType(EGameModeWarningType::None)
{
}

FValidationCheckResult::FValidationCheckResult(const bool Success, const bool Bypassed,
	const FValidationCheckPtr& InValidationCheck) : bSuccess(Success), bBypassed(Bypassed),
	                                                OwningPropertyHash(InValidationCheck->OwningPropertyHash),
	                                                ValidationCheckPtr(InValidationCheck),
	                                                WarningType(InValidationCheck->ValidationCheckData.WarningType)
{
}

FValidationResult::FValidationResult()
{
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

FValidationCheckResultSet FValidationResult::GetSucceeded() const
{
	return SucceededValidationCheckResults;
}

FValidationCheckResultSet FValidationResult::GetFailed() const
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
		EGameModeCategory InGameModeCategory);

	static FValidationCheckPtr CreateValidationCheck(const FValidationPropertyPtr& PropPtr,
		EGameModeWarningType GameModeWarningType,
		const TFunction<bool(const TSharedPtr<FBSConfig>&, FValidationCheckData&)>& InValidationFunction,
		const TFunction<bool(const TSharedPtr<FBSConfig>&)>& InValidationPrerequisite = nullptr);

	static void AddValidationCheckData(const FValidationCheckPtr& Check, const FString& InStringTableKey,
		const FString& InDynamicStringTableKey = FString(), int32 InGridSnapSize = 0,
		bool InCalculatedValuesAreIntegers = false);

	static float GetMinRequiredHorizontalSpread(const TSharedPtr<FBSConfig>& Config);

	static float GetMinRequiredVerticalSpread(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxTargetDiameter(const TSharedPtr<FBSConfig>& Config);

	static int32 GetMaxAllowedNumHorizontalTargets(const TSharedPtr<FBSConfig>& Config);

	static int32 GetMaxAllowedNumVerticalTargets(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedHorizontalSpacing(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedVerticalSpacing(const TSharedPtr<FBSConfig>& Config);

	static float GetMaxAllowedTargetScale(const TSharedPtr<FBSConfig>& Config);

	static void ValidateCheckSet(const FValidationCheckSet& Checks, const TSharedPtr<FBSConfig>& Config,
		FValidationResult& Result);

	void Validate(const FValidationPropertySet& Properties, const TSharedPtr<FBSConfig>& Config,
		FValidationResult& Result);

	void Validate(const TSet<uint32>& Properties, const TSharedPtr<FBSConfig>& Config, FValidationResult& Result);

	FValidationPropertySet ValidationProperties;

private:
	void TopologicalSort(const FValidationPropertyPtr& Property,
		TSet<TPair<FValidationPropertyPtr, FValidationCheckPtr>>& Visited,
		TSet<TPair<FValidationPropertyPtr, FValidationCheckPtr>>& Visiting, TArray<FValidationCheckPtr>& Sorted);
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
	// const FName DynamicTargetName = GET_MEMBER_NAME_CHECKED(FBSConfig, DynamicTargetScaling);
	const FName BoxBoundsName = GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, BoxBounds);
	const FName GridSpacingName = GET_MEMBER_NAME_CHECKED(FBS_GridConfig, GridSpacing);

	const FPropertyHash BatchSpawning = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bUseBatchSpawning));
	const FPropertyHash SpawnEveryOtherTargetInCenter = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bSpawnEveryOtherTargetInCenter));
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
	/*const FPropertyHash MinSpawnedTargetScale = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinSpawnedTargetScale));*/
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
		EGameModeCategory::TargetMovement);
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
	FValidationPropertyPtr MaxSpawnedTargetScalePtr = CreateValidationProperty(MaxSpawnedTargetScale,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr GridSpacingXPtr = CreateValidationProperty(GridSpacingX, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr GridSpacingYPtr = CreateValidationProperty(GridSpacingY, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr EnableReinforcementLearningPtr = CreateValidationProperty(EnableReinforcementLearning,
		EGameModeCategory::General);

	auto SpawnEveryOtherAndBatchLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.bSpawnEveryOtherTargetInCenter && Config->TargetConfig.bUseBatchSpawning);
	};
	auto SpawnEveryOtherAndAllowSpawnLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.bSpawnEveryOtherTargetInCenter && Config->TargetConfig.
			bAllowSpawnWithoutActivation);
	};

	// SpawnEveryOtherTargetInCenter & BatchSpawning checks
	CheckPtr = CreateValidationCheck(SpawnEveryOtherTargetInCenterPtr, EGameModeWarningType::Warning,
		SpawnEveryOtherAndBatchLambda);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning"));
	BatchSpawningPtr->AddDependent(SpawnEveryOtherTargetInCenterPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(BatchSpawningPtr, EGameModeWarningType::Warning, SpawnEveryOtherAndBatchLambda,
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning2"));
	SpawnEveryOtherTargetInCenterPtr->AddDependent(BatchSpawningPtr, CheckPtr);

	// SpawnEveryOtherTargetInCenter & AllowSpawnWithoutActivation checks
	CheckPtr = CreateValidationCheck(SpawnEveryOtherTargetInCenterPtr, EGameModeWarningType::Warning,
		SpawnEveryOtherAndAllowSpawnLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_SpawnEveryOtherTargetInCenter_AllowSpawnWithoutActivation"));
	AllowSpawnWithoutActivationPtr->AddDependent(SpawnEveryOtherTargetInCenterPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(AllowSpawnWithoutActivationPtr, EGameModeWarningType::Warning,
		SpawnEveryOtherAndAllowSpawnLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_SpawnEveryOtherTargetInCenter_AllowSpawnWithoutActivation2"));
	SpawnEveryOtherTargetInCenterPtr->AddDependent(AllowSpawnWithoutActivationPtr, CheckPtr);

	auto SpawnVelocityLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeVelocity));
	};
	auto SpawnDirectionLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeDirection));
	};
	auto ActivationVelocityLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeVelocity));
	};
	auto ActivationDirectionLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeDirection));
	};
	auto DeactivationVelocityLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeVelocity));
	};
	auto DeactivationDirectionLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && Config->
			TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeDirection));
	};
	auto BoxBoundsLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.BoxBounds.X <= 0.f && Config->TargetConfig.MovingTargetDirectionMode ==
			EMovingTargetDirectionMode::ForwardOnly);
	};

	// MovingTargetDirectionMode & Target Spawn Responses
	CheckPtr = CreateValidationCheck(MovingTargetDirectionModePtr, EGameModeWarningType::Caution, SpawnVelocityLambda,
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Velocity_MTDM_None_2"));
	TargetSpawnResponsesPtr->AddDependent(MovingTargetDirectionModePtr, CheckPtr);
	CheckPtr = CreateValidationCheck(TargetSpawnResponsesPtr, EGameModeWarningType::Caution, SpawnVelocityLambda,
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Velocity_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetSpawnResponsesPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(MovingTargetDirectionModePtr, EGameModeWarningType::Caution, SpawnDirectionLambda,
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Direction_MTDM_None_2"));
	TargetSpawnResponsesPtr->AddDependent(MovingTargetDirectionModePtr, CheckPtr);
	CheckPtr = CreateValidationCheck(TargetSpawnResponsesPtr, EGameModeWarningType::Caution, SpawnDirectionLambda,
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Direction_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetSpawnResponsesPtr, CheckPtr);

	// MovingTargetDirectionMode & Target Activation Responses
	CheckPtr = CreateValidationCheck(MovingTargetDirectionModePtr, EGameModeWarningType::Caution,
		ActivationVelocityLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Velocity_MTDM_None_2"));
	TargetActivationResponsesPtr->AddDependent(MovingTargetDirectionModePtr, CheckPtr);
	CheckPtr = CreateValidationCheck(TargetActivationResponsesPtr, EGameModeWarningType::Caution,
		ActivationVelocityLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Velocity_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetActivationResponsesPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(MovingTargetDirectionModePtr, EGameModeWarningType::Caution,
		ActivationDirectionLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Direction_MTDM_None_2"));
	TargetActivationResponsesPtr->AddDependent(MovingTargetDirectionModePtr, CheckPtr);
	CheckPtr = CreateValidationCheck(TargetActivationResponsesPtr, EGameModeWarningType::Caution,
		ActivationDirectionLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Direction_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetActivationResponsesPtr, CheckPtr);

	// MovingTargetDirectionMode & Target Deactivation Responses
	CheckPtr = CreateValidationCheck(MovingTargetDirectionModePtr, EGameModeWarningType::Caution,
		DeactivationVelocityLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Velocity_MTDM_None_2"));
	TargetDeactivationResponsesPtr->AddDependent(MovingTargetDirectionModePtr, CheckPtr);
	CheckPtr = CreateValidationCheck(TargetDeactivationResponsesPtr, EGameModeWarningType::Caution,
		DeactivationVelocityLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Velocity_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetDeactivationResponsesPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(MovingTargetDirectionModePtr, EGameModeWarningType::Caution,
		DeactivationDirectionLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Direction_MTDM_None_2"));
	TargetDeactivationResponsesPtr->AddDependent(MovingTargetDirectionModePtr, CheckPtr);
	CheckPtr = CreateValidationCheck(TargetDeactivationResponsesPtr, EGameModeWarningType::Caution,
		DeactivationDirectionLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Direction_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetDeactivationResponsesPtr, CheckPtr);

	// MovingTargetDirectionMode & Box Bounds
	CheckPtr = CreateValidationCheck(MovingTargetDirectionModePtr, EGameModeWarningType::Caution, BoxBoundsLambda,
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Caution_ZeroForwardDistance_MTDM_ForwardOnly"));
	BoxBoundsXPtr->AddDependent(MovingTargetDirectionModePtr, CheckPtr);
	CheckPtr = CreateValidationCheck(BoxBoundsXPtr, EGameModeWarningType::Caution, BoxBoundsLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Caution_ZeroForwardDistance_MTDM_ForwardOnly_2"));
	MovingTargetDirectionModePtr->AddDependent(BoxBoundsXPtr, CheckPtr);

	auto GridTargetDistributionPolicyPrerequisite = [](const TSharedPtr<FBSConfig>& Config)
	{
		return Config->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid;
	};
	auto NumHorizontalGridTargetsLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		const int32 MaxAllowed = GetMaxAllowedNumHorizontalTargets(Config);
		if (MaxAllowed < Constants::MinValue_NumHorizontalGridTargets)
		{
			Data.bRequireOtherPropertiesToBeChanged = true;
			return false;
		}
		Data.CalculatedValues.Add(MaxAllowed);

		return MaxAllowed >= Config->GridConfig.NumHorizontalGridTargets;
	};
	auto NumVerticalGridTargetsLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		const int32 MaxAllowed = GetMaxAllowedNumVerticalTargets(Config);
		Data.CalculatedValues.Add(MaxAllowed);
		if (MaxAllowed < Constants::MinValue_NumVerticalGridTargets)
		{
			Data.bRequireOtherPropertiesToBeChanged = true;
			return false;
		}
		return MaxAllowed >= Config->GridConfig.NumVerticalGridTargets;
	};
	auto GridSpacingHorizontalLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		const float MaxAllowed = GetMaxAllowedHorizontalSpacing(Config);
		Data.CalculatedValues.Add(FMath::RoundHalfToZero(MaxAllowed * 100) / 100.f);
		if (MaxAllowed < Constants::MinValue_HorizontalGridSpacing)
		{
			Data.bRequireOtherPropertiesToBeChanged = true;
			return false;
		}
		return MaxAllowed >= Config->GridConfig.GridSpacing.X;
	};
	auto GridSpacingVerticalLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		const float MaxAllowed = GetMaxAllowedVerticalSpacing(Config);
		Data.CalculatedValues.Add(FMath::RoundHalfToZero(MaxAllowed * 100) / 100.f);
		if (MaxAllowed < Constants::MinValue_VerticalGridSpacing)
		{
			Data.bRequireOtherPropertiesToBeChanged = true;
			return false;
		}
		return MaxAllowed >= Config->GridConfig.GridSpacing.Y;
	};
	auto TargetDistributionPolicyLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::HeadshotHeightOnly &&
			Config->AIConfig.bEnableReinforcementLearning);
	};
	auto TargetSizeLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		const float MaxAllowed = GetMaxAllowedTargetScale(Config);
		Data.CalculatedValues.Add(FMath::RoundHalfToZero(MaxAllowed * 100) / 100.f);
		if (MaxAllowed < Constants::MinValue_TargetScale)
		{
			Data.bRequireOtherPropertiesToBeChanged = true;
			return false;
		}
		return MaxAllowed >= FMath::Max(Config->TargetConfig.MinSpawnedTargetScale,
			Config->TargetConfig.MaxSpawnedTargetScale);
	};

	CheckPtr = CreateValidationCheck(NumHorizontalGridTargetsPtr, EGameModeWarningType::Warning,
		NumHorizontalGridTargetsLambda, GridTargetDistributionPolicyPrerequisite);
	AddValidationCheckData(CheckPtr,TEXT("Invalid_Grid_NumHorizontalTargets_Fallback"),
		TEXT("Invalid_Grid_NumHorizontalTargets"), 0, true);
	TargetDistributionPolicyPtr->AddDependent(NumHorizontalGridTargetsPtr, CheckPtr);
	GridSpacingXPtr->AddDependent(NumHorizontalGridTargetsPtr, CheckPtr);
	MaxSpawnedTargetScalePtr->AddDependent(NumHorizontalGridTargetsPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(NumVerticalGridTargetsPtr, EGameModeWarningType::Warning,
		NumVerticalGridTargetsLambda, GridTargetDistributionPolicyPrerequisite);
	AddValidationCheckData(CheckPtr,TEXT("Invalid_Grid_NumVerticalTargets_Fallback"),
		TEXT("Invalid_Grid_NumVerticalTargets"), 0, true);
	TargetDistributionPolicyPtr->AddDependent(NumVerticalGridTargetsPtr, CheckPtr);
	GridSpacingYPtr->AddDependent(NumVerticalGridTargetsPtr, CheckPtr);
	MaxSpawnedTargetScalePtr->AddDependent(NumVerticalGridTargetsPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(GridSpacingXPtr, EGameModeWarningType::Warning, GridSpacingHorizontalLambda,
		GridTargetDistributionPolicyPrerequisite);
	AddValidationCheckData(CheckPtr,TEXT("Invalid_Grid_HorizontalSpacing_Fallback"),
		TEXT("Invalid_Grid_HorizontalSpacing"), 10, false);
	TargetDistributionPolicyPtr->AddDependent(GridSpacingXPtr, CheckPtr);
	NumHorizontalGridTargetsPtr->AddDependent(GridSpacingXPtr, CheckPtr);
	MaxSpawnedTargetScalePtr->AddDependent(GridSpacingXPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(GridSpacingYPtr, EGameModeWarningType::Warning, GridSpacingVerticalLambda,
		GridTargetDistributionPolicyPrerequisite);
	AddValidationCheckData(CheckPtr,TEXT("Invalid_Grid_VerticalSpacing_Fallback"), TEXT("Invalid_Grid_VerticalSpacing"),
		10, false);
	TargetDistributionPolicyPtr->AddDependent(GridSpacingYPtr, CheckPtr);
	NumVerticalGridTargetsPtr->AddDependent(GridSpacingYPtr, CheckPtr);
	MaxSpawnedTargetScalePtr->AddDependent(GridSpacingYPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(MaxSpawnedTargetScalePtr, EGameModeWarningType::Warning, TargetSizeLambda,
		GridTargetDistributionPolicyPrerequisite);
	AddValidationCheckData(CheckPtr,TEXT("Invalid_Grid_MaxSpawnedTargetScale_Fallback"),
		TEXT("Invalid_Grid_MaxSpawnedTargetScale"), 0, false);
	TargetDistributionPolicyPtr->AddDependent(MaxSpawnedTargetScalePtr, CheckPtr);
	NumVerticalGridTargetsPtr->AddDependent(MaxSpawnedTargetScalePtr, CheckPtr);
	NumHorizontalGridTargetsPtr->AddDependent(MaxSpawnedTargetScalePtr, CheckPtr);
	GridSpacingXPtr->AddDependent(MaxSpawnedTargetScalePtr, CheckPtr);
	GridSpacingYPtr->AddDependent(MaxSpawnedTargetScalePtr, CheckPtr);

	// TargetDistributionPolicy checks
	CheckPtr = CreateValidationCheck(TargetDistributionPolicyPtr, EGameModeWarningType::Warning,
		TargetDistributionPolicyLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_HeadshotHeightOnly_AI"));
	EnableReinforcementLearningPtr->AddDependent(TargetDistributionPolicyPtr, CheckPtr);
	CheckPtr = CreateValidationCheck(EnableReinforcementLearningPtr, EGameModeWarningType::Warning,
		TargetDistributionPolicyLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_HeadshotHeightOnly_AI"));
	TargetDistributionPolicyPtr->AddDependent(EnableReinforcementLearningPtr, CheckPtr);

	FValidationPropertyPtr TargetDamageTypePtr = CreateValidationProperty(TargetDamageType,
		EGameModeCategory::SpawnArea);

	auto InvalidTrackingAILambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->AIConfig.bEnableReinforcementLearning && Config->TargetConfig.TargetDamageType ==
			ETargetDamageType::Tracking);
	};
	auto InvalidHeadshotAILambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->AIConfig.bEnableReinforcementLearning && Config->TargetConfig.TargetDistributionPolicy ==
			ETargetDistributionPolicy::HeadshotHeightOnly);
	};

	CheckPtr = CreateValidationCheck(EnableReinforcementLearningPtr, EGameModeWarningType::Warning,
		InvalidTrackingAILambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Tracking_AI"));
	TargetDamageTypePtr->AddDependent(EnableReinforcementLearningPtr, CheckPtr);
	CheckPtr = CreateValidationCheck(TargetDamageTypePtr, EGameModeWarningType::Warning, InvalidTrackingAILambda,
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Tracking_AI"));
	EnableReinforcementLearningPtr->AddDependent(TargetDamageTypePtr, CheckPtr);

	CheckPtr = CreateValidationCheck(EnableReinforcementLearningPtr, EGameModeWarningType::Warning,
		InvalidHeadshotAILambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_HeadshotHeightOnly_AI"));
	TargetDistributionPolicyPtr->AddDependent(EnableReinforcementLearningPtr, CheckPtr);
	CheckPtr = CreateValidationCheck(TargetDistributionPolicyPtr, EGameModeWarningType::Warning,
		InvalidHeadshotAILambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_HeadshotHeightOnly_AI"));
	EnableReinforcementLearningPtr->AddDependent(TargetDistributionPolicyPtr, CheckPtr);
}

FValidationPropertyPtr UBSGameModeValidator::FPrivate::CreateValidationProperty(const FPropertyHash& InProperty,
	const EGameModeCategory InGameModeCategory)
{
	FValidationPropertyPtr PropertyPtr = MakeShareable(new FValidationProperty(InProperty, InGameModeCategory));
	ValidationProperties.Add(PropertyPtr);
	return PropertyPtr;
}

FValidationCheckPtr UBSGameModeValidator::FPrivate::CreateValidationCheck(const FValidationPropertyPtr& PropPtr,
	const EGameModeWarningType GameModeWarningType,
	const TFunction<bool(const TSharedPtr<FBSConfig>&, FValidationCheckData&)>& InValidationFunction,
	const TFunction<bool(const TSharedPtr<FBSConfig>&)>& InValidationPrerequisite)
{
	FValidationCheckPtr Check = MakeShareable(new FValidationCheck(GetTypeHash(*PropPtr.Get()), GameModeWarningType));
	if (InValidationPrerequisite)
	{
		Check->ValidationPrerequisiteDelegate.BindLambda(InValidationPrerequisite);
	}
	Check->ValidationDelegate.BindLambda(InValidationFunction);
	PropPtr->ValidationChecks.Add(Check);
	return Check;
}

void UBSGameModeValidator::FPrivate::AddValidationCheckData(const FValidationCheckPtr& Check,
	const FString& InStringTableKey, const FString& InDynamicStringTableKey, const int32 InGridSnapSize,
	const bool InCalculatedValuesAreIntegers)
{
	Check->AddData(InStringTableKey, InDynamicStringTableKey, InGridSnapSize, InCalculatedValuesAreIntegers);
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
	// Total = GridSpacing.X * (HorizontalTargets - 1) + (HorizontalTargets - 1) * MaxTargetDiameter;
	// Total = (HorizontalTargets - 1) * (GridSpacing.X + MaxTargetDiameter);
	// Total / (GridSpacing.X + MaxTargetDiameter) = HorizontalTargets - 1
	// HorizontalTargets = Total / (GridSpacing.X + MaxTargetDiameter) + 1

	return Constants::MaxValue_HorizontalSpread / (Config->GridConfig.GridSpacing.X + GetMaxTargetDiameter(Config)) + 1;
}

int32 UBSGameModeValidator::FPrivate::GetMaxAllowedNumVerticalTargets(const TSharedPtr<FBSConfig>& Config)
{
	// Total = GridSpacing.Y * (VerticalTargets - 1) + (VerticalTargets - 1) * MaxTargetDiameter;
	// Total = (VerticalTargets - 1) * (GridSpacing.Y + MaxTargetDiameter);
	// Total / (GridSpacing.Y + MaxTargetDiameter) = VerticalTargets - 1
	// VerticalTargets = Total / (GridSpacing.Y * MaxTargetDiameter) + 1

	return Constants::MaxValue_VerticalSpread / (Config->GridConfig.GridSpacing.Y + GetMaxTargetDiameter(Config)) + 1;
}

float UBSGameModeValidator::FPrivate::GetMaxAllowedHorizontalSpacing(const TSharedPtr<FBSConfig>& Config)
{
	// Total = GridSpacing.X * (HorizontalTargets - 1) + (HorizontalTargets - 1) * MaxTargetDiameter;
	// Total = (HorizontalTargets - 1) * (GridSpacing.X + MaxTargetDiameter);
	// Total / (HorizontalTargets - 1) = GridSpacing.X + MaxTargetDiameter;
	// Total / (HorizontalTargets - 1) - MaxTargetDiameter = GridSpacing.X;

	return Constants::MaxValue_HorizontalSpread / (Config->GridConfig.NumHorizontalGridTargets - 1) -
		GetMaxTargetDiameter(Config);
}

float UBSGameModeValidator::FPrivate::GetMaxAllowedVerticalSpacing(const TSharedPtr<FBSConfig>& Config)
{
	// Total = GridSpacing.Y * (VerticalTargets - 1) + (VerticalTargets - 1) * MaxTargetDiameter;
	// Total = (VerticalTargets - 1) * (GridSpacing.Y + MaxTargetDiameter);
	// Total / (VerticalTargets - 1) = GridSpacing.Y + MaxTargetDiameter;
	// Total / (VerticalTargets - 1) - MaxTargetDiameter = GridSpacing.Y;

	return Constants::MaxValue_VerticalSpread / (Config->GridConfig.NumVerticalGridTargets - 1) -
		GetMaxTargetDiameter(Config);
}

float UBSGameModeValidator::FPrivate::GetMaxAllowedTargetScale(const TSharedPtr<FBSConfig>& Config)
{
	// Total = GridSpacing.X * (HorizontalTargets - 1) + (HorizontalTargets - 1) * SphereTargetDiameter * Scale;
	// Total - (GridSpacing.X * (HorizontalTargets - 1)) = (HorizontalTargets - 1) * SphereTargetDiameter * Scale;
	// Total - (GridSpacing.X * (HorizontalTargets - 1)) = (HorizontalGTargets - 1) * SphereTargetDiameter * Scale;
	// (Total - (GridSpacing.X * (HorizontalTargets - 1))) / ((HorizontalTargets - 1) * SphereTargetDiameter) = Scale;
	// Scale = (Total - (GridSpacing.X * (HorizontalTargets - 1))) / ((HorizontalTargets - 1) * SphereTargetDiameter)

	const float Horizontal = (Constants::MaxValue_HorizontalSpread - (Config->GridConfig.GridSpacing.X * (Config->
		GridConfig.NumHorizontalGridTargets - 1))) / ((Config->GridConfig.NumHorizontalGridTargets - 1) *
		Constants::SphereTargetDiameter);

	const float Vertical = (Constants::MaxValue_VerticalSpread - (Config->GridConfig.GridSpacing.Y * (Config->GridConfig
		.NumVerticalGridTargets - 1))) / ((Config->GridConfig.NumVerticalGridTargets - 1) *
		Constants::SphereTargetDiameter);
	return FMath::Min(Horizontal, Vertical);
}

void UBSGameModeValidator::FPrivate::ValidateCheckSet(const FValidationCheckSet& Checks,
	const TSharedPtr<FBSConfig>& Config, FValidationResult& Result)
{
	for (const FValidationCheckPtr& Check : Checks)
	{
		if (Result.Contains(Check))
		{
			continue;
		}
		TArray<float> Values;
		bool bBypassed = false;
		bool bResult = true;

		if (Check->ValidationPrerequisiteDelegate.IsBound())
		{
			bBypassed = !Check->ValidationPrerequisiteDelegate.Execute(Config);
		}
		if (!bBypassed)
		{
			Check->ValidationCheckData.ResetLiveData();
			bResult = Check->ValidationDelegate.Execute(Config, Check->ValidationCheckData);
		}

		Result.AddValidationCheckResult(FValidationCheckResult(bResult, bBypassed, Check));
	}
}

void UBSGameModeValidator::FPrivate::Validate(const FValidationPropertySet& Properties,
	const TSharedPtr<FBSConfig>& Config, FValidationResult& Result)
{
	TSet<TPair<FValidationPropertyPtr, FValidationCheckPtr>> Visited;
	TSet<TPair<FValidationPropertyPtr, FValidationCheckPtr>> Visiting;
	TArray<FValidationCheckPtr> Sorted;

	for (const FValidationPropertyPtr& Property : Properties)
	{
		TopologicalSort(Property, Visited, Visiting, Sorted);
	}

	TArray<FValidationCheckPtr> NewSorted;
	while (!Sorted.IsEmpty())
	{
		const auto Elem = Sorted.Pop();
		NewSorted.Add(Elem);
	}


	for (const FValidationCheckPtr& Check : NewSorted)
	{
		if (Result.Contains(Check))
		{
			continue;
		}
		TArray<float> Values;
		bool bBypassed = false;
		bool bResult = true;

		if (Check->ValidationPrerequisiteDelegate.IsBound())
		{
			bBypassed = !Check->ValidationPrerequisiteDelegate.Execute(Config);
		}
		if (!bBypassed)
		{
			Check->ValidationCheckData.ResetLiveData();
			bResult = Check->ValidationDelegate.Execute(Config, Check->ValidationCheckData);
		}
		Result.AddValidationCheckResult(FValidationCheckResult(bResult, bBypassed, Check));
	}
}

void UBSGameModeValidator::FPrivate::Validate(const TSet<uint32>& Properties, const TSharedPtr<FBSConfig>& Config,
	FValidationResult& Result)
{
	FValidationPropertySet ValidationPropertySet;
	for (const uint32 Hash : Properties)
	{
		if (const FValidationPropertyPtr* Found = ValidationProperties.Find(Hash))
		{
			ValidationPropertySet.Add(*Found);
		}
	}
	Validate(ValidationPropertySet, Config, Result);
}

void UBSGameModeValidator::FPrivate::TopologicalSort(const FValidationPropertyPtr& Property,
	TSet<TPair<FValidationPropertyPtr, FValidationCheckPtr>>& Visited,
	TSet<TPair<FValidationPropertyPtr, FValidationCheckPtr>>& Visiting, TArray<FValidationCheckPtr>& Sorted)
{
	for (const auto& [Dependent, Checks] : Property->Dependents)
	{
		for (const FValidationCheckPtr& Check : Checks)
		{
			const TPair<FValidationPropertyPtr, FValidationCheckPtr> Pair = {Dependent, Check};

			if (Visited.Contains(Pair))
			{
				continue;
			}

			if (Visiting.Contains(Pair))
			{
				// Handle cyclic dependency
				continue;
			}

			Visiting.Add(Pair);

			TopologicalSort(Dependent, Visited, Visiting, Sorted);

			Visiting.Remove(Pair);
			Visited.Add(Pair);

			if (!Sorted.Contains(Check))
			{
				Sorted.Push(Check);
			}
		}
	}
	// Ensure property’s own validation checks are added after its dependencies
	for (const FValidationCheckPtr& Check : Property->ValidationChecks)
	{
		if (const TPair<FValidationPropertyPtr, FValidationCheckPtr> Pair = {Property, Check}; !Visited.Contains(Pair))
		{
			Visited.Add(Pair);
			Sorted.Push(Check);
		}
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
	Impl->Validate(Impl->ValidationProperties, InConfig, Result);
	return Result;
}

FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig, const FName SubStructName,
	const FName PropertyName) const
{
	FValidationResult Result;
	if (const FValidationPropertyPtr& ValidationProperty = FindValidationProperty(
		GetPropertyHash<FBSConfig>(SubStructName, PropertyName)))
	{
		Impl->Validate({ValidationProperty}, InConfig, Result);
	}
	return Result;
}

FValidationResult UBSGameModeValidator::Validate(const TSharedPtr<FBSConfig>& InConfig,
	const TSet<uint32>& Properties) const
{
	FValidationResult Result;
	Impl->Validate(Properties, InConfig, Result);
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

FValidationPropertySet& UBSGameModeValidator::GetValidationProperties() const
{
	return Impl->ValidationProperties;
}
