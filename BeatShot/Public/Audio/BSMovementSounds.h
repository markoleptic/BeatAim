// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "BSMovementSounds.generated.h"

USTRUCT(BlueprintType)
struct FBSFootstepSoundKey
{
	GENERATED_BODY()

	/** The AnimEffect GameplayTag associated with the effect (e.g. AnimEffect.Footstep.Walk). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag EffectTag;

	/** The context (GameplayTag associated with the physical surface type (e.g. SurfaceType.Concrete). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer Context;

	FBSFootstepSoundKey()
	{
	}

	FBSFootstepSoundKey(const FGameplayTag& InEffectTag, const FGameplayTagContainer& InContext)
		: EffectTag(InEffectTag), Context(InContext)
	{
	}

	FBSFootstepSoundKey(const FGameplayTag& InEffectTag, const FGameplayTag& InContext)
		: EffectTag(InEffectTag), Context(InContext)
	{
	}

	FORCEINLINE bool operator==(const FBSFootstepSoundKey& Other) const
	{
		return EffectTag.MatchesTagExact(Other.EffectTag) && Context.HasAllExact(Other.Context);
	}

	FORCEINLINE bool operator<(const FBSFootstepSoundKey& Other) const
	{
		if (Context.IsEmpty())
		{
			return EffectTag.ToString() < Other.EffectTag.ToString();
		}
		if (Other.Context.IsEmpty()) return false;
		return Context.First().ToString() < Other.Context.First().ToString();
	}

	friend FORCEINLINE uint32 GetTypeHash(const FBSFootstepSoundKey& Object)
	{
		return HashCombine(GetTypeHash(Object.EffectTag), GetTypeHash(Object.Context.ToString()));
	}
};

USTRUCT(BlueprintType)
struct FBSFootstepSoundValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ForceInlineRow))
	TArray<TObjectPtr<USoundBase>> Sounds;
};

/** Move and Step Sounds to play that CharacterMovementComponent uses */
UCLASS(Blueprintable, Const)
class BEATSHOT_API UBSMovementSounds : public UDataAsset
{
	GENERATED_BODY()

public:
	virtual void PostLoad() override;

	void GetFootstepSounds(const FGameplayTag Effect, const FGameplayTagContainer& Context,
		TArray<TObjectPtr<USoundBase>>& Sounds) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FBSFootstepSoundKey, FBSFootstepSoundValue> FootstepSounds;
};
