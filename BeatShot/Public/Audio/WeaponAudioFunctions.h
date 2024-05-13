// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WeaponAudioFunctions.generated.h"

class UCameraComponent;
struct FGameplayCueParameters;
struct FGameplayCueNotify_SpawnResult;
class USubmixEffectTapDelayPreset;

UCLASS()
class BEATSHOT_API UWeaponAudioFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Audio")
	static void SetWeaponSoundParams(AActor* Actor, const FGameplayCueNotify_SpawnResult& SpawnResult);

	UFUNCTION(BlueprintCallable, Category="Audio")
	static void SetEarlyReflections(AActor* Target, const FGameplayCueParameters& Params,
		USubmixEffectTapDelayPreset* SubmixEffect);

protected:
	static void CalculateTapProperties(const FString& DebugString, USubmixEffectTapDelayPreset* SubmixEffect,
		UCameraComponent* CameraComponent, const FVector& ListenerLocation, const FVector& HitLocation,
		const int32 TapID, const float TravelDistance, const bool bHit);

	static void GetSideReflectionAngles(const FVector& Origin, const FVector& Normal, const float TraceDistance,
		const FVector& Axis, FVector& OutAngleLeft, FVector& OutAngleRight);
};
