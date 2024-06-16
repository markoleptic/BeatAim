// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BSAnimationSet.generated.h"

class UAimOffsetBlendSpace;

/** Highest level organizer. */
UENUM(BlueprintType)
enum class EBSAnimMovementMode : uint8
{
	None UMETA(DisplayName="None"),
	Crouch UMETA(DisplayName="Crouch"),
	Walk UMETA(DisplayName="Walk"),
	Jog UMETA(DisplayName="Jog"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EBSAnimMovementMode, EBSAnimMovementMode::None, EBSAnimMovementMode::Jog);

/** Second level organizer. */
UENUM(BlueprintType)
enum class EBSAnimStateType : uint8
{
	Normal UMETA(DisplayName="Normal"),
	Start UMETA(DisplayName="Start"),
	Stop UMETA(DisplayName="Stop")};

ENUM_RANGE_BY_FIRST_AND_LAST(EBSAnimStateType, EBSAnimStateType::Normal, EBSAnimStateType::Stop);

/** Third level organizer */
UENUM(BlueprintType)
enum class EBSAnimCardinal : uint8
{
	Forward UMETA(DisplayName="Forward"),
	Backward UMETA(DisplayName="Backward"),
	Left UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EBSAnimCardinal, EBSAnimCardinal::Forward, EBSAnimCardinal::Right);

USTRUCT(BlueprintType, meta=(ForceInlineRow))
struct FBSAnimCardinalSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ForceInlineRow))
	TMap<EBSAnimCardinal, UAnimSequence*> Map;
};

USTRUCT(BlueprintType, meta=(ForceInlineRow))
struct FBSAnimSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ForceInlineRow))
	TMap<EBSAnimStateType, FBSAnimCardinalSet> Map;
};

USTRUCT(BlueprintType)
struct FBSIdleAnimSet
{
	GENERATED_BODY()

	FBSIdleAnimSet(): Idle(nullptr), CrouchIdle(nullptr), CrouchIdleStart(nullptr), CrouchIdleStop(nullptr),
	                  LeftHandPoseOverride(nullptr), ArmIdlePoseOverride(nullptr)
	{
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* Idle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* CrouchIdle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* CrouchIdleStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* CrouchIdleStop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* LeftHandPoseOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* ArmIdlePoseOverride;
};

USTRUCT(BlueprintType)
struct FBSAimingAnimSet
{
	GENERATED_BODY()

	FBSAimingAnimSet(): IdleAimOffset(nullptr), RelaxedAimOffset(nullptr), HipFireOverridePose(nullptr),
	                    HipFireCrouchOverridePose(nullptr)
	{
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAimOffsetBlendSpace* IdleAimOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAimOffsetBlendSpace* RelaxedAimOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* HipFireOverridePose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* HipFireCrouchOverridePose;
};

USTRUCT(BlueprintType)
struct FBSJumpingAnimSet
{
	GENERATED_BODY()

	FBSJumpingAnimSet(): JumpStart(nullptr), JumpFallLoop(nullptr), JumpFallLand(nullptr), JumpApex(nullptr),
	                     JumpStartLoop(nullptr), JumpRecoveryAdditive(nullptr)
	{
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* JumpStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* JumpFallLoop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* JumpFallLand;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* JumpApex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* JumpStartLoop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* JumpRecoveryAdditive;
};

/**
 * A set of animation sequences and blend spaces to be used with UBSAnimationInstances.
 */
UCLASS(Blueprintable, Abstract, BlueprintType, EditInlineNew)
class BEATSHOT_API UBSAnimationSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EBSAnimMovementMode, FBSAnimSet> AnimMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBSIdleAnimSet IdleAnimSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBSAimingAnimSet AimingAnimSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBSJumpingAnimSet JumpingAnimSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<UAnimSequence*> IdleBreaks;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category="Animation")
	UAnimSequence* FindAnimSequence(const EBSAnimMovementMode MovementMode, const EBSAnimStateType StateType,
		const EBSAnimCardinal AnimCardinal) const;
};
