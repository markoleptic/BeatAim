// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify_PlaySound.h"
#include "AnimNotify_PlayFootstepSound.generated.h"

/** Sound settings for Notify sounds. */
USTRUCT(BlueprintType)
struct FBSAnimNotifySoundSettings
{
	GENERATED_BODY()

	/** Volume Multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float VolumeMultiplier = 1.0f;

	/** Pitch Multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float PitchMultiplier = 1.0f;
};


/** Trace settings for Notify sounds. */
USTRUCT(BlueprintType)
struct FBSAnimNotifyTraceSettings
{
	GENERATED_BODY()

	/** Trace Channel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_Visibility;

	/** Vector offset from Effect Location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace)
	FVector EndTraceLocationOffset = FVector::ZeroVector;

	/** Ignore this Actor when getting trace result. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace)
	bool bIgnoreActor = true;
};

/** Anim notify that plays footstep sounds. */
UCLASS(const, hidecategories=Object, collapsecategories, Config = Game, meta=(DisplayName="Play Footstep Sound"))
class BEATSHOT_API UAnimNotify_PlayFootstepSound : public UAnimNotify_PlaySound
{
	GENERATED_BODY()

public:
	UAnimNotify_PlayFootstepSound();

	// Begin UAnimNotify interface
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	// End UAnimNotify interface

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable)
	void SetParameters(FGameplayTag EffectIn, FVector LocationOffsetIn, FRotator RotationOffsetIn,
		FBSAnimNotifySoundSettings AudioPropertiesIn, bool bAttachedIn, FName SocketNameIn, bool bPerformTraceIn,
		FBSAnimNotifyTraceSettings TracePropertiesIn);
#endif


	/** Effect to Play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify",
		meta = (DisplayName = "Effect", ExposeOnSpawn = true))
	FGameplayTag Effect;

	/** Location offset from the socket. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
	FVector LocationOffset = FVector::ZeroVector;

	/** Rotation offset from socket. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** Scale to spawn the particle system at. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
	FBSAnimNotifySoundSettings AudioProperties;

	/** Should attach to the bone/socket. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttachmentProperties", meta = (ExposeOnSpawn = true))
	uint32 bAttached : 1; //~ Does not follow coding standard due to redirection from BP

	/** SocketName to attach to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttachmentProperties",
		meta = (ExposeOnSpawn = true, EditCondition = "bAttached"))
	FName SocketName;

	/** Will perform a trace, required for SurfaceType to Context Conversion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
	uint32 bPerformTrace : 1;

	/** Scale to spawn the particle system at. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify",
		meta = (ExposeOnSpawn = true, EditCondition = "bPerformTrace"))
	FBSAnimNotifyTraceSettings TraceProperties;
};
