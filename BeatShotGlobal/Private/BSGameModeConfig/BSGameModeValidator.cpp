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

	template <typename... EnumValue>
	TFunction<bool(const TSharedPtr<FBSConfig>&, FValidationCheckData&)>
	CreateValidationFunction(const TArray<std::common_type_t<EnumValue...>> FBS_TargetConfig::* Container,
		const EnumValue... Values)
	{
		using EnumType = std::common_type_t<EnumValue...>;
		return [Container, Values...](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
		{
			const TArray<EnumType>& ContainerArray = Config->TargetConfig.*Container;
			const bool ContainsAnyValue = (... || ContainerArray.Contains(Values));
			return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None &&
				ContainsAnyValue);
		};
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
	const FName FVectorXName = GET_MEMBER_NAME_CHECKED(FVector, X);
	const FName FVectorYName = GET_MEMBER_NAME_CHECKED(FVector, Y);
	const FName FVectorZName = GET_MEMBER_NAME_CHECKED(FVector, Z);
	const FName FVector2dXName = GET_MEMBER_NAME_CHECKED(FVector2D, X);
	const FName FVector2dYName = GET_MEMBER_NAME_CHECKED(FVector2D, Y);

	// AI Config

	const FName AIConfigName = GET_MEMBER_NAME_CHECKED(FBSConfig, AIConfig);

	const FPropertyHash EnableReinforcementLearning = CreatePropertyHash<FBSConfig>(AIConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_AIConfig, bEnableReinforcementLearning));
	const FPropertyHash Alpha = CreatePropertyHash<FBSConfig>(AIConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_AIConfig, Alpha));
	const FPropertyHash Epsilon = CreatePropertyHash<FBSConfig>(AIConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_AIConfig, Epsilon));
	const FPropertyHash Gamma = CreatePropertyHash<FBSConfig>(AIConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_AIConfig, Gamma));
	const FPropertyHash ReinforcementLearningMode = CreatePropertyHash<FBSConfig>(AIConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_AIConfig, ReinforcementLearningMode));
	const FPropertyHash HyperParameterMode = CreatePropertyHash<FBSConfig>(AIConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_AIConfig, HyperParameterMode));

	FValidationPropertyPtr EnableReinforcementLearningPtr = CreateValidationProperty(EnableReinforcementLearning,
		EGameModeCategory::General);
	FValidationPropertyPtr AlphaPtr = CreateValidationProperty(Alpha, EGameModeCategory::General);
	FValidationPropertyPtr EpsilonPtr = CreateValidationProperty(Epsilon, EGameModeCategory::General);
	FValidationPropertyPtr GammaPtr = CreateValidationProperty(Gamma, EGameModeCategory::General);
	FValidationPropertyPtr ReinforcementLearningModePtr = CreateValidationProperty(ReinforcementLearningMode,
		EGameModeCategory::General);
	FValidationPropertyPtr HyperParameterModePtr = CreateValidationProperty(HyperParameterMode,
		EGameModeCategory::General);
	// Grid config

	const FName GridConfigName = GET_MEMBER_NAME_CHECKED(FBSConfig, GridConfig);
	const FName GridSpacingName = GET_MEMBER_NAME_CHECKED(FBS_GridConfig, GridSpacing);

	const FPropertyHash NumHorizontalGridTargets = CreatePropertyHash<FBSConfig>(GridConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_GridConfig, NumHorizontalGridTargets));
	const FPropertyHash NumVerticalGridTargets = CreatePropertyHash<FBSConfig>(GridConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_GridConfig, NumVerticalGridTargets));
	const FPropertyHash GridSpacingX = CreatePropertyHash<FBSConfig>(GridConfigName, GridSpacingName, FVector2dXName);
	const FPropertyHash GridSpacingY = CreatePropertyHash<FBSConfig>(GridConfigName, GridSpacingName, FVector2dYName);
	const FPropertyHash NumGridTargetsVisibleAtOnce = CreatePropertyHash<FBSConfig>(GridConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_GridConfig, NumGridTargetsVisibleAtOnce));

	FValidationPropertyPtr NumHorizontalGridTargetsPtr = CreateValidationProperty(NumHorizontalGridTargets,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr NumVerticalGridTargetsPtr = CreateValidationProperty(NumVerticalGridTargets,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr GridSpacingXPtr = CreateValidationProperty(GridSpacingX, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr GridSpacingYPtr = CreateValidationProperty(GridSpacingY, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr NumGridTargetsVisibleAtOncePtr = CreateValidationProperty(NumGridTargetsVisibleAtOnce,
		EGameModeCategory::SpawnArea);

	// Target Config

	const FName TargetConfigName = GET_MEMBER_NAME_CHECKED(FBSConfig, TargetConfig);
	const FName BoxBoundsName = GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, BoxBounds);

	const FPropertyHash AllowActivationWhileActivated = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bAllowActivationWhileActivated));
	const FPropertyHash AllowSpawnWithoutActivation = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bAllowSpawnWithoutActivation));
	const FPropertyHash SpawnAtOriginWheneverPossible = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bSpawnAtOriginWheneverPossible));
	const FPropertyHash SpawnEveryOtherTargetInCenter = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bSpawnEveryOtherTargetInCenter));
	const FPropertyHash BatchSpawning = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, bUseBatchSpawning));
	const FPropertyHash BoundsScalingPolicy = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, BoundsScalingPolicy));
	const FPropertyHash ConsecutiveTargetScalePolicy = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, ConsecutiveTargetScalePolicy));
	const FPropertyHash MovingTargetDirectionMode = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MovingTargetDirectionMode));
	const FPropertyHash RecentTargetMemoryPolicy = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, RecentTargetMemoryPolicy));
	const FPropertyHash TargetActivationSelectionPolicy = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetActivationSelectionPolicy));
	const FPropertyHash TargetDamageType = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDamageType));
	const FPropertyHash TargetDistributionPolicy = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDistributionPolicy));
	const FPropertyHash TargetSpawningPolicy = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetSpawningPolicy));
	const FPropertyHash RuntimeTargetSpawningLocationSelectionMode = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, RuntimeTargetSpawningLocationSelectionMode));
	const FPropertyHash TargetSpawnResponses = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetSpawnResponses));
	const FPropertyHash TargetActivationResponses = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetActivationResponses));
	const FPropertyHash TargetDeactivationResponses = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDeactivationResponses));
	const FPropertyHash TargetDestructionConditions = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetDestructionConditions));
	const FPropertyHash BasePlayerHitDamage = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, BasePlayerHitDamage));
	const FPropertyHash BasePlayerTrackingDamage = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, BasePlayerTrackingDamage));
	const FPropertyHash ConsecutiveChargeScaleMultiplier = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, ConsecutiveChargeScaleMultiplier));
	const FPropertyHash DeactivationHealthLostThreshold = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, DeactivationHealthLostThreshold));
	const FPropertyHash ExpirationHealthPenalty = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, ExpirationHealthPenalty));
	const FPropertyHash FloorDistance = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, FloorDistance));
	const FPropertyHash MinDistanceBetweenTargets = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinDistanceBetweenTargets));
	const FPropertyHash MaxHealth = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxHealth));
	const FPropertyHash LifetimeTargetScaleMultiplier = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, LifetimeTargetScaleMultiplier));
	const FPropertyHash MinSpawnedTargetScale = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinSpawnedTargetScale));
	const FPropertyHash MaxSpawnedTargetScale = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxSpawnedTargetScale));
	const FPropertyHash MinSpawnedTargetSpeed = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinSpawnedTargetSpeed));
	const FPropertyHash MaxSpawnedTargetSpeed = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxSpawnedTargetSpeed));
	const FPropertyHash MinActivatedTargetSpeed = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinActivatedTargetSpeed));
	const FPropertyHash MaxActivatedTargetSpeed = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxActivatedTargetSpeed));
	const FPropertyHash MinDeactivatedTargetSpeed = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinDeactivatedTargetSpeed));
	const FPropertyHash MaxDeactivatedTargetSpeed = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxDeactivatedTargetSpeed));
	const FPropertyHash RecentTargetTimeLength = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, RecentTargetTimeLength));
	const FPropertyHash SpawnBeatDelay = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, SpawnBeatDelay));
	const FPropertyHash TargetMaxLifeSpan = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetMaxLifeSpan));
	const FPropertyHash TargetSpawnCD = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, TargetSpawnCD));
	const FPropertyHash BoxBoundsX = CreatePropertyHash<FBSConfig>(TargetConfigName, BoxBoundsName, FVectorXName);
	const FPropertyHash BoxBoundsY = CreatePropertyHash<FBSConfig>(TargetConfigName, BoxBoundsName, FVectorYName);
	const FPropertyHash BoxBoundsZ = CreatePropertyHash<FBSConfig>(TargetConfigName, BoxBoundsName, FVectorZName);
	const FPropertyHash MaxNumActivatedTargetsAtOnce = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxNumActivatedTargetsAtOnce));
	const FPropertyHash MaxNumRecentTargets = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxNumRecentTargets));
	const FPropertyHash MaxNumTargetsAtOnce = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxNumTargetsAtOnce));
	const FPropertyHash MinNumTargetsToActivateAtOnce = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MinNumTargetsToActivateAtOnce));
	const FPropertyHash MaxNumTargetsToActivateAtOnce = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, MaxNumTargetsToActivateAtOnce));
	const FPropertyHash NumRuntimeTargetsToSpawn = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, NumRuntimeTargetsToSpawn));
	const FPropertyHash NumUpfrontTargetsToSpawn = CreatePropertyHash<FBSConfig>(TargetConfigName,
		GET_MEMBER_NAME_CHECKED(FBS_TargetConfig, NumUpfrontTargetsToSpawn));

	FValidationPropertyPtr AllowActivationWhileActivatedPtr = CreateValidationProperty(AllowActivationWhileActivated,
		EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr AllowSpawnWithoutActivationPtr = CreateValidationProperty(AllowSpawnWithoutActivation,
		EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr SpawnAtOriginWheneverPossiblePtr = CreateValidationProperty(SpawnAtOriginWheneverPossible,
		EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr SpawnEveryOtherTargetInCenterPtr = CreateValidationProperty(SpawnEveryOtherTargetInCenter,
		EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr BatchSpawningPtr =
		CreateValidationProperty(BatchSpawning, EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr BoundsScalingPolicyPtr = CreateValidationProperty(BoundsScalingPolicy,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr ConsecutiveTargetScalePolicyPtr = CreateValidationProperty(ConsecutiveTargetScalePolicy,
		EGameModeCategory::TargetSizing);
	FValidationPropertyPtr MovingTargetDirectionModePtr = CreateValidationProperty(MovingTargetDirectionMode,
		EGameModeCategory::TargetMovement);
	FValidationPropertyPtr RecentTargetMemoryPolicyPtr = CreateValidationProperty(RecentTargetMemoryPolicy,
		EGameModeCategory::General);
	FValidationPropertyPtr TargetActivationSelectionPolicyPtr = CreateValidationProperty(
		TargetActivationSelectionPolicy, EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr TargetDamageTypePtr = CreateValidationProperty(TargetDamageType, EGameModeCategory::General);
	FValidationPropertyPtr TargetDistributionPolicyPtr = CreateValidationProperty(TargetDistributionPolicy,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr TargetSpawningPolicyPtr = CreateValidationProperty(TargetSpawningPolicy,
		EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr RuntimeTargetSpawningLocationSelectionModePtr = CreateValidationProperty(
		RuntimeTargetSpawningLocationSelectionMode, EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr TargetSpawnResponsesPtr = CreateValidationProperty(TargetSpawnResponses,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr TargetActivationResponsesPtr = CreateValidationProperty(TargetActivationResponses,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr TargetDeactivationResponsesPtr = CreateValidationProperty(TargetDeactivationResponses,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr TargetDestructionConditionsPtr = CreateValidationProperty(TargetDestructionConditions,
		EGameModeCategory::TargetBehavior);
	FValidationPropertyPtr BasePlayerHitDamagePtr = CreateValidationProperty(BasePlayerHitDamage,
		EGameModeCategory::General);
	FValidationPropertyPtr BasePlayerTrackingDamagePtr = CreateValidationProperty(BasePlayerTrackingDamage,
		EGameModeCategory::General);
	FValidationPropertyPtr ConsecutiveChargeScaleMultiplierPtr = CreateValidationProperty(
		ConsecutiveChargeScaleMultiplier, EGameModeCategory::TargetSizing);
	FValidationPropertyPtr DeactivationHealthLostThresholdPtr = CreateValidationProperty(
		DeactivationHealthLostThreshold, EGameModeCategory::General);
	FValidationPropertyPtr ExpirationHealthPenaltyPtr = CreateValidationProperty(ExpirationHealthPenalty,
		EGameModeCategory::General);
	FValidationPropertyPtr FloorDistancePtr = CreateValidationProperty(FloorDistance, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr MinDistanceBetweenTargetsPtr = CreateValidationProperty(MinDistanceBetweenTargets,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr MaxHealthPtr = CreateValidationProperty(MaxHealth, EGameModeCategory::General);
	FValidationPropertyPtr LifetimeTargetScaleMultiplierPtr = CreateValidationProperty(LifetimeTargetScaleMultiplier,
		EGameModeCategory::TargetSizing);
	FValidationPropertyPtr MinSpawnedTargetScalePtr = CreateValidationProperty(MinSpawnedTargetScale,
		EGameModeCategory::TargetSizing);
	FValidationPropertyPtr MaxSpawnedTargetScalePtr = CreateValidationProperty(MaxSpawnedTargetScale,
		EGameModeCategory::TargetSizing);
	FValidationPropertyPtr MinSpawnedTargetSpeedPtr = CreateValidationProperty(MinSpawnedTargetSpeed,
		EGameModeCategory::TargetMovement);
	FValidationPropertyPtr MaxSpawnedTargetSpeedPtr = CreateValidationProperty(MaxSpawnedTargetSpeed,
		EGameModeCategory::TargetMovement);
	FValidationPropertyPtr MinActivatedTargetSpeedPtr = CreateValidationProperty(MinActivatedTargetSpeed,
		EGameModeCategory::TargetMovement);
	FValidationPropertyPtr MaxActivatedTargetSpeedPtr = CreateValidationProperty(MaxActivatedTargetSpeed,
		EGameModeCategory::TargetMovement);
	FValidationPropertyPtr MinDeactivatedTargetSpeedPtr = CreateValidationProperty(MinDeactivatedTargetSpeed,
		EGameModeCategory::TargetMovement);
	FValidationPropertyPtr MaxDeactivatedTargetSpeedPtr = CreateValidationProperty(MaxDeactivatedTargetSpeed,
		EGameModeCategory::TargetMovement);
	FValidationPropertyPtr RecentTargetTimeLengthPtr = CreateValidationProperty(RecentTargetTimeLength,
		EGameModeCategory::General);
	FValidationPropertyPtr SpawnBeatDelayPtr = CreateValidationProperty(SpawnBeatDelay, EGameModeCategory::General);
	FValidationPropertyPtr TargetMaxLifeSpanPtr = CreateValidationProperty(TargetMaxLifeSpan,
		EGameModeCategory::General);
	FValidationPropertyPtr TargetSpawnCDPtr = CreateValidationProperty(TargetSpawnCD, EGameModeCategory::General);
	FValidationPropertyPtr MaxNumActivatedTargetsAtOncePtr = CreateValidationProperty(MaxNumActivatedTargetsAtOnce,
		EGameModeCategory::TargetActivation);
	FValidationPropertyPtr MaxNumRecentTargetsPtr = CreateValidationProperty(MaxNumRecentTargets,
		EGameModeCategory::General);
	FValidationPropertyPtr MaxNumTargetsAtOncePtr = CreateValidationProperty(MaxNumTargetsAtOnce,
		EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr MinNumTargetsToActivateAtOncePtr = CreateValidationProperty(MinNumTargetsToActivateAtOnce,
		EGameModeCategory::TargetActivation);
	FValidationPropertyPtr MaxNumTargetsToActivateAtOncePtr = CreateValidationProperty(MaxNumTargetsToActivateAtOnce,
		EGameModeCategory::TargetActivation);
	FValidationPropertyPtr NumRuntimeTargetsToSpawnPtr = CreateValidationProperty(NumRuntimeTargetsToSpawn,
		EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr NumUpfrontTargetsToSpawnPtr = CreateValidationProperty(NumUpfrontTargetsToSpawn,
		EGameModeCategory::TargetSpawning);
	FValidationPropertyPtr BoxBoundsXPtr = CreateValidationProperty(BoxBoundsX, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr BoxBoundsYPtr = CreateValidationProperty(BoxBoundsY, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr BoxBoundsZPtr = CreateValidationProperty(BoxBoundsZ, EGameModeCategory::SpawnArea);

	// Dynamic Spawn Area Config

	const FName DynamicSpawnAreaName = GET_MEMBER_NAME_CHECKED(FBSConfig, DynamicSpawnAreaScaling);

	const FPropertyHash StartBoundsX = CreatePropertyHash<FBSConfig>(DynamicSpawnAreaName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, StartBounds), FVectorXName);
	const FPropertyHash StartBoundsY = CreatePropertyHash<FBSConfig>(DynamicSpawnAreaName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, StartBounds), FVectorYName);
	const FPropertyHash StartBoundsZ = CreatePropertyHash<FBSConfig>(DynamicSpawnAreaName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, StartBounds), FVectorZName);
	const FPropertyHash StartThresholdSpawnArea = CreatePropertyHash<FBSConfig>(DynamicSpawnAreaName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, StartThreshold));
	const FPropertyHash EndThresholdSpawnArea = CreatePropertyHash<FBSConfig>(DynamicSpawnAreaName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, EndThreshold));
	const FPropertyHash IsCubicInterpolationSpawnArea = CreatePropertyHash<FBSConfig>(DynamicSpawnAreaName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, bIsCubicInterpolation));
	const FPropertyHash DecrementAmountSpawnArea = CreatePropertyHash<FBSConfig>(DynamicSpawnAreaName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, DecrementAmount));

	FValidationPropertyPtr StartBoundsXPtr = CreateValidationProperty(StartBoundsX, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr StartBoundsYPtr = CreateValidationProperty(StartBoundsY, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr StartBoundsZPtr = CreateValidationProperty(StartBoundsZ, EGameModeCategory::SpawnArea);
	FValidationPropertyPtr StartThresholdSpawnAreaPtr = CreateValidationProperty(StartThresholdSpawnArea,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr EndThresholdSpawnAreaPtr = CreateValidationProperty(EndThresholdSpawnArea,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr IsCubicInterpolationSpawnAreaPtr = CreateValidationProperty(IsCubicInterpolationSpawnArea,
		EGameModeCategory::SpawnArea);
	FValidationPropertyPtr DecrementAmountSpawnAreaPtr = CreateValidationProperty(DecrementAmountSpawnArea,
		EGameModeCategory::SpawnArea);

	// Dynamic Target Scaling Config

	const FName DynamicTargetScalingName = GET_MEMBER_NAME_CHECKED(FBSConfig, DynamicTargetScaling);

	const FPropertyHash StartThresholdTarget = CreatePropertyHash<FBSConfig>(DynamicTargetScalingName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, StartThreshold));
	const FPropertyHash EndThresholdTarget = CreatePropertyHash<FBSConfig>(DynamicTargetScalingName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, EndThreshold));
	const FPropertyHash IsCubicInterpolationTarget = CreatePropertyHash<FBSConfig>(DynamicTargetScalingName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, bIsCubicInterpolation));
	const FPropertyHash DecrementAmountTarget = CreatePropertyHash<FBSConfig>(DynamicTargetScalingName,
		GET_MEMBER_NAME_CHECKED(FBS_Dynamic_SpawnArea, DecrementAmount));

	FValidationPropertyPtr StartThresholdTargetPtr = CreateValidationProperty(StartThresholdTarget,
		EGameModeCategory::TargetSizing);
	FValidationPropertyPtr EndThresholdTargetPtr = CreateValidationProperty(EndThresholdTarget,
		EGameModeCategory::TargetSizing);
	FValidationPropertyPtr IsCubicInterpolationTargetPtr = CreateValidationProperty(IsCubicInterpolationTarget,
		EGameModeCategory::TargetSizing);
	FValidationPropertyPtr DecrementAmountTargetPtr = CreateValidationProperty(DecrementAmountTarget,
		EGameModeCategory::TargetSizing);

	FValidationCheckPtr CheckPtr = nullptr;

	// SpawnEveryOtherTargetInCenter & BatchSpawning checks
	auto SpawnEveryOtherAndBatchLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.bSpawnEveryOtherTargetInCenter && Config->TargetConfig.bUseBatchSpawning);
	};

	CheckPtr = CreateValidationCheck(SpawnEveryOtherTargetInCenterPtr, EGameModeWarningType::Warning,
		SpawnEveryOtherAndBatchLambda);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning"));
	BatchSpawningPtr->AddDependent(SpawnEveryOtherTargetInCenterPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(BatchSpawningPtr, EGameModeWarningType::Warning, SpawnEveryOtherAndBatchLambda,
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_SpawnEveryOtherTargetInCenter_BatchSpawning2"));
	SpawnEveryOtherTargetInCenterPtr->AddDependent(BatchSpawningPtr, CheckPtr);

	// SpawnEveryOtherTargetInCenter & AllowSpawnWithoutActivation checks
	auto SpawnEveryOtherAndAllowSpawnLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.bSpawnEveryOtherTargetInCenter && Config->TargetConfig.
			bAllowSpawnWithoutActivation);
	};

	CheckPtr = CreateValidationCheck(SpawnEveryOtherTargetInCenterPtr, EGameModeWarningType::Warning,
		SpawnEveryOtherAndAllowSpawnLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_SpawnEveryOtherTargetInCenter_AllowSpawnWithoutActivation"));
	AllowSpawnWithoutActivationPtr->AddDependent(SpawnEveryOtherTargetInCenterPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(AllowSpawnWithoutActivationPtr, EGameModeWarningType::Warning,
		SpawnEveryOtherAndAllowSpawnLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_SpawnEveryOtherTargetInCenter_AllowSpawnWithoutActivation2"));
	SpawnEveryOtherTargetInCenterPtr->AddDependent(AllowSpawnWithoutActivationPtr, CheckPtr);

	// Moving Target Direction Mode & Target Spawn/Activation/Deactivation Responses
	auto VelocityCheckPtr = CreateValidationCheck(MovingTargetDirectionModePtr, EGameModeWarningType::Caution,
		[](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
		{
			return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && (Config->
				TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeVelocity) || Config->TargetConfig
				.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeVelocity) || Config->TargetConfig.
				TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeVelocity)));
		}, nullptr);
	auto DirectionCheckPtr = CreateValidationCheck(MovingTargetDirectionModePtr, EGameModeWarningType::Caution,
		[](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
		{
			return !(Config->TargetConfig.MovingTargetDirectionMode == EMovingTargetDirectionMode::None && (Config->
				TargetConfig.TargetSpawnResponses.Contains(ETargetSpawnResponse::ChangeDirection) || Config->
				TargetConfig.TargetActivationResponses.Contains(ETargetActivationResponse::ChangeDirection) || Config->
				TargetConfig.TargetDeactivationResponses.Contains(ETargetDeactivationResponse::ChangeDirection)));
		}, nullptr);
	AddValidationCheckData(VelocityCheckPtr, TEXT("Invalid_Velocity_MTDM_None_2"));
	AddValidationCheckData(DirectionCheckPtr, TEXT("Invalid_Direction_MTDM_None_2"));

	// Target Spawn Responses
	TargetSpawnResponsesPtr->AddDependent(MovingTargetDirectionModePtr, VelocityCheckPtr);
	CheckPtr = CreateValidationCheck(TargetSpawnResponsesPtr, EGameModeWarningType::Caution,
		CreateValidationFunction(&FBS_TargetConfig::TargetSpawnResponses, ETargetSpawnResponse::ChangeVelocity),
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Velocity_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetSpawnResponsesPtr, CheckPtr);

	TargetSpawnResponsesPtr->AddDependent(MovingTargetDirectionModePtr, DirectionCheckPtr);
	CheckPtr = CreateValidationCheck(TargetSpawnResponsesPtr, EGameModeWarningType::Caution,
		CreateValidationFunction(&FBS_TargetConfig::TargetSpawnResponses, ETargetSpawnResponse::ChangeDirection),
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Direction_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetSpawnResponsesPtr, CheckPtr);

	// Target Activation Responses
	TargetActivationResponsesPtr->AddDependent(MovingTargetDirectionModePtr, VelocityCheckPtr);
	CheckPtr = CreateValidationCheck(TargetActivationResponsesPtr, EGameModeWarningType::Caution,
		CreateValidationFunction(&FBS_TargetConfig::TargetActivationResponses,
			ETargetActivationResponse::ChangeVelocity), nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Velocity_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetActivationResponsesPtr, CheckPtr);

	TargetActivationResponsesPtr->AddDependent(MovingTargetDirectionModePtr, DirectionCheckPtr);
	CheckPtr = CreateValidationCheck(TargetActivationResponsesPtr, EGameModeWarningType::Caution,
		CreateValidationFunction(&FBS_TargetConfig::TargetActivationResponses,
			ETargetActivationResponse::ChangeDirection), nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Direction_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetActivationResponsesPtr, CheckPtr);

	// Target Deactivation Responses
	TargetDeactivationResponsesPtr->AddDependent(MovingTargetDirectionModePtr, VelocityCheckPtr);
	CheckPtr = CreateValidationCheck(TargetDeactivationResponsesPtr, EGameModeWarningType::Caution,
		CreateValidationFunction(&FBS_TargetConfig::TargetDeactivationResponses,
			ETargetDeactivationResponse::ChangeVelocity), nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Velocity_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetDeactivationResponsesPtr, CheckPtr);

	TargetDeactivationResponsesPtr->AddDependent(MovingTargetDirectionModePtr, DirectionCheckPtr);
	CheckPtr = CreateValidationCheck(TargetDeactivationResponsesPtr, EGameModeWarningType::Caution,
		CreateValidationFunction(&FBS_TargetConfig::TargetDeactivationResponses,
			ETargetDeactivationResponse::ChangeDirection), nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Direction_MTDM_None"));
	MovingTargetDirectionModePtr->AddDependent(TargetDeactivationResponsesPtr, CheckPtr);

	// Forward Only Moving Target Direction Mode & Box Bounds X
	auto BoxBoundsLambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.BoxBounds.X <= 0.f && Config->TargetConfig.MovingTargetDirectionMode ==
			EMovingTargetDirectionMode::ForwardOnly);
	};

	CheckPtr = CreateValidationCheck(MovingTargetDirectionModePtr, EGameModeWarningType::Caution, BoxBoundsLambda,
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Caution_ZeroForwardDistance_MTDM_ForwardOnly"));
	BoxBoundsXPtr->AddDependent(MovingTargetDirectionModePtr, CheckPtr);

	CheckPtr = CreateValidationCheck(BoxBoundsXPtr, EGameModeWarningType::Caution, BoxBoundsLambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Caution_ZeroForwardDistance_MTDM_ForwardOnly_2"));
	MovingTargetDirectionModePtr->AddDependent(BoxBoundsXPtr, CheckPtr);

	// Grid spacing, number of grid targets, and target sizing checks
	auto GridTargetDistributionPolicyPrerequisite = [](const TSharedPtr<FBSConfig>& Config)
	{
		return Config->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::Grid;
	};

	CheckPtr = CreateValidationCheck(NumHorizontalGridTargetsPtr, EGameModeWarningType::Warning,
		[](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
		{
			const int32 MaxAllowed = GetMaxAllowedNumHorizontalTargets(Config);
			if (MaxAllowed < Constants::MinValue_NumHorizontalGridTargets)
			{
				Data.bRequireOtherPropertiesToBeChanged = true;
				return false;
			}
			Data.CalculatedValues.Add(MaxAllowed);

			return MaxAllowed >= Config->GridConfig.NumHorizontalGridTargets;
		}, GridTargetDistributionPolicyPrerequisite);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Grid_NumHorizontalTargets_Fallback"),
		TEXT("Invalid_Grid_NumHorizontalTargets"), 0, true);
	TargetDistributionPolicyPtr->AddDependent(NumHorizontalGridTargetsPtr, CheckPtr);
	GridSpacingXPtr->AddDependent(NumHorizontalGridTargetsPtr, CheckPtr);
	MaxSpawnedTargetScalePtr->AddDependent(NumHorizontalGridTargetsPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(NumVerticalGridTargetsPtr, EGameModeWarningType::Warning,
		[](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
		{
			const int32 MaxAllowed = GetMaxAllowedNumVerticalTargets(Config);
			Data.CalculatedValues.Add(MaxAllowed);
			if (MaxAllowed < Constants::MinValue_NumVerticalGridTargets)
			{
				Data.bRequireOtherPropertiesToBeChanged = true;
				return false;
			}
			return MaxAllowed >= Config->GridConfig.NumVerticalGridTargets;
		}, GridTargetDistributionPolicyPrerequisite);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Grid_NumVerticalTargets_Fallback"),
		TEXT("Invalid_Grid_NumVerticalTargets"), 0, true);
	TargetDistributionPolicyPtr->AddDependent(NumVerticalGridTargetsPtr, CheckPtr);
	GridSpacingYPtr->AddDependent(NumVerticalGridTargetsPtr, CheckPtr);
	MaxSpawnedTargetScalePtr->AddDependent(NumVerticalGridTargetsPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(GridSpacingXPtr, EGameModeWarningType::Warning,
		[](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
		{
			const float MaxAllowed = GetMaxAllowedHorizontalSpacing(Config);
			Data.CalculatedValues.Add(FMath::RoundHalfToZero(MaxAllowed * 100) / 100.f);
			if (MaxAllowed < Constants::MinValue_HorizontalGridSpacing)
			{
				Data.bRequireOtherPropertiesToBeChanged = true;
				return false;
			}
			return MaxAllowed >= Config->GridConfig.GridSpacing.X;
		}, GridTargetDistributionPolicyPrerequisite);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Grid_HorizontalSpacing_Fallback"),
		TEXT("Invalid_Grid_HorizontalSpacing"), 10, false);
	TargetDistributionPolicyPtr->AddDependent(GridSpacingXPtr, CheckPtr);
	NumHorizontalGridTargetsPtr->AddDependent(GridSpacingXPtr, CheckPtr);
	MaxSpawnedTargetScalePtr->AddDependent(GridSpacingXPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(GridSpacingYPtr, EGameModeWarningType::Warning,
		[](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
		{
			const float MaxAllowed = GetMaxAllowedVerticalSpacing(Config);
			Data.CalculatedValues.Add(FMath::RoundHalfToZero(MaxAllowed * 100) / 100.f);
			if (MaxAllowed < Constants::MinValue_VerticalGridSpacing)
			{
				Data.bRequireOtherPropertiesToBeChanged = true;
				return false;
			}
			return MaxAllowed >= Config->GridConfig.GridSpacing.Y;
		}, GridTargetDistributionPolicyPrerequisite);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Grid_VerticalSpacing_Fallback"),
		TEXT("Invalid_Grid_VerticalSpacing"), 10, false);
	TargetDistributionPolicyPtr->AddDependent(GridSpacingYPtr, CheckPtr);
	NumVerticalGridTargetsPtr->AddDependent(GridSpacingYPtr, CheckPtr);
	MaxSpawnedTargetScalePtr->AddDependent(GridSpacingYPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(MaxSpawnedTargetScalePtr, EGameModeWarningType::Warning,
		[](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
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
		}, GridTargetDistributionPolicyPrerequisite);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Grid_MaxSpawnedTargetScale_Fallback"),
		TEXT("Invalid_Grid_MaxSpawnedTargetScale"), 0, false);
	TargetDistributionPolicyPtr->AddDependent(MaxSpawnedTargetScalePtr, CheckPtr);
	NumVerticalGridTargetsPtr->AddDependent(MaxSpawnedTargetScalePtr, CheckPtr);
	NumHorizontalGridTargetsPtr->AddDependent(MaxSpawnedTargetScalePtr, CheckPtr);
	GridSpacingXPtr->AddDependent(MaxSpawnedTargetScalePtr, CheckPtr);
	GridSpacingYPtr->AddDependent(MaxSpawnedTargetScalePtr, CheckPtr);

	// HeadshotHeightOnly Target Distribution Policy & AIConfig checks
	auto InvalidHeadshotAILambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->TargetConfig.TargetDistributionPolicy == ETargetDistributionPolicy::HeadshotHeightOnly &&
			Config->AIConfig.bEnableReinforcementLearning);
	};

	CheckPtr = CreateValidationCheck(TargetDistributionPolicyPtr, EGameModeWarningType::Warning,
		InvalidHeadshotAILambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_HeadshotHeightOnly_AI"));
	EnableReinforcementLearningPtr->AddDependent(TargetDistributionPolicyPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(EnableReinforcementLearningPtr, EGameModeWarningType::Warning,
		InvalidHeadshotAILambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_HeadshotHeightOnly_AI"));
	TargetDistributionPolicyPtr->AddDependent(EnableReinforcementLearningPtr, CheckPtr);

	// Tracking Target Damage Type & AIConfig checks
	auto InvalidTrackingAILambda = [](const TSharedPtr<FBSConfig>& Config, FValidationCheckData& Data)
	{
		return !(Config->AIConfig.bEnableReinforcementLearning && Config->TargetConfig.TargetDamageType ==
			ETargetDamageType::Tracking);
	};

	CheckPtr = CreateValidationCheck(EnableReinforcementLearningPtr, EGameModeWarningType::Warning,
		InvalidTrackingAILambda, nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Tracking_AI"));
	TargetDamageTypePtr->AddDependent(EnableReinforcementLearningPtr, CheckPtr);

	CheckPtr = CreateValidationCheck(TargetDamageTypePtr, EGameModeWarningType::Warning, InvalidTrackingAILambda,
		nullptr);
	AddValidationCheckData(CheckPtr, TEXT("Invalid_Tracking_AI"));
	EnableReinforcementLearningPtr->AddDependent(TargetDamageTypePtr, CheckPtr);
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
