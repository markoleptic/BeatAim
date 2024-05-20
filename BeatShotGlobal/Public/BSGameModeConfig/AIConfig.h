// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSConstants.h"
#include "AIConfig.generated.h"

/** Enum representing the modes in which the Reinforcement Learning Component operates. */
UENUM(BlueprintType)
enum class EReinforcementLearningMode : uint8
{
	None UMETA(DisplayName="None"),
	Training UMETA(DisplayName="Training"),
	Exploration UMETA(DisplayName="Exploration"),
	ActiveAgent UMETA(DisplayName="ActiveAgent")};

ENUM_RANGE_BY_FIRST_AND_LAST(EReinforcementLearningMode, EReinforcementLearningMode::None,
	EReinforcementLearningMode::ActiveAgent);

/** Enum representing auto or custom hyper-parameters. */
UENUM(BlueprintType)
enum class EReinforcementLearningHyperParameterMode: uint8
{
	None UMETA(DisplayName="None"),
	Auto UMETA(DisplayName="Auto"),
	Custom UMETA(DisplayName="Custom")};

ENUM_RANGE_BY_FIRST_AND_LAST(EReinforcementLearningHyperParameterMode, EReinforcementLearningHyperParameterMode::Auto,
	EReinforcementLearningHyperParameterMode::Custom);

USTRUCT(BlueprintType)
struct BEATSHOTGLOBAL_API FBS_AIConfig
{
	GENERATED_BODY()

	/** Whether to enable the reinforcement learning agent to handle target spawning. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bEnableReinforcementLearning;

	/** Learning rate, or how much to update the Q-Table rewards when a reward is received. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Alpha;

	/** The exploration/exploitation balance factor. A value = 1 will result in only choosing random values (explore),
	 *  while a value of zero will result in only choosing the max Q-value (exploitation). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Epsilon;

	/** Discount factor, or how much to value future rewards vs immediate rewards. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Gamma;

	/** The mode to operate the Reinforcement Learning Component. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EReinforcementLearningMode ReinforcementLearningMode;

	/** Auto adjust Alpha and Epsilon or never change and let user choose. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EReinforcementLearningHyperParameterMode HyperParameterMode;

	FBS_AIConfig()
	{
		bEnableReinforcementLearning = false;
		Alpha = Constants::DefaultAlpha;
		Epsilon = Constants::DefaultEpsilon;
		Gamma = Constants::DefaultGamma;
		ReinforcementLearningMode = EReinforcementLearningMode::None;
		HyperParameterMode = EReinforcementLearningHyperParameterMode::Auto;
	}

	FORCEINLINE bool operator==(const FBS_AIConfig& Other) const
	{
		if (bEnableReinforcementLearning != Other.bEnableReinforcementLearning)
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(Alpha, Other.Alpha))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(Epsilon, Other.Epsilon))
		{
			return false;
		}
		if (!FMath::IsNearlyEqual(Gamma, Other.Gamma))
		{
			return false;
		}
		if (ReinforcementLearningMode != Other.ReinforcementLearningMode)
		{
			return false;
		}
		if (HyperParameterMode != Other.HyperParameterMode)
		{
			return false;
		}
		return true;
	}
};
