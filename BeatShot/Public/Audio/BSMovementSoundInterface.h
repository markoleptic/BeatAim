// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "BSMovementSoundInterface.generated.h"

UINTERFACE()
class UBSMovementSoundInterface : public UInterface
{
	GENERATED_BODY()
};

/** */
class BEATSHOT_API IBSMovementSoundInterface
{
	GENERATED_BODY()

public:
	/** */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlayMovementSound(const FName Bone, const FGameplayTag MotionEffect, USceneComponent* StaticMeshComponent,
		const FVector LocationOffset, const FRotator RotationOffset, const UAnimSequenceBase* AnimationSequence,
		const FHitResult HitResult, FGameplayTagContainer Context, float AudioVolume = 1, float AudioPitch = 1);
};
