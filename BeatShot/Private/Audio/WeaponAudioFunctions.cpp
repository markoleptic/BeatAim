// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Audio/WeaponAudioFunctions.h"
#include "GameplayCueNotifyTypes.h"
#include "Camera/CameraComponent.h"
#include "Character/BSCharacterBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SubmixEffects/SubmixEffectTapDelay.h"

void UWeaponAudioFunctions::SetWeaponSoundParams(AActor* Actor, const FGameplayCueNotify_SpawnResult& SpawnResult)
{
	if (Actor->IsA<APawn>())
	{
		for (const TObjectPtr<UAudioComponent>& AudioComponent : SpawnResult.AudioComponents)
		{
			if (AudioComponent)
			{
				AudioComponent->SetIntParameter(FName("PawnSeed"), -1);
			}
		}
	}
}

void UWeaponAudioFunctions::SetEarlyReflections(AActor* Target, const FGameplayCueParameters& Params,
	USubmixEffectTapDelayPreset* SubmixEffect)
{
	if (!Target) return;
	ABSCharacterBase* BSCharacter = Cast<ABSCharacterBase>(Target);
	
	if (!BSCharacter) return;

	bool bHit = Params.PhysicalMaterial.IsValid();
	if (bHit)
	{
		// Hitting another character
		if (Params.PhysicalMaterial->SurfaceType == SurfaceType1) return;
	}

	UCameraComponent* Camera = BSCharacter->GetCamera();
	FVector ListenerLocation = Camera->GetComponentLocation();
	FVector HitLocation = Params.Location;
	float DirectHitDistance = (HitLocation - ListenerLocation).Length();

	if (DirectHitDistance >= 4000.f && Target != UGameplayStatics::GetPlayerPawn(Target->GetWorld(), 0))
	{
		return;
	}

	FVector PawnLocation = BSCharacter->GetActorLocation();
	FVector HitNormal = Params.Normal;

	TArray<int32> TapIDs;
	SubmixEffect->GetTapIds(TapIDs);
	
	// Direct hit (HitResult from GCN)
	CalculateTapProperties("Direct.Primary", SubmixEffect, Camera,
		ListenerLocation, HitLocation, TapIDs[0], DirectHitDistance, bHit);
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Target);
	
	FHitResult DirectReflectionFromLOSHitResult;
	bHit = Target->GetWorld()->LineTraceSingleByChannel(DirectReflectionFromLOSHitResult, HitLocation,
		HitLocation + HitNormal * 4000.f, ECC_Visibility, CollisionParams);
	DirectReflectionFromLOSHitResult.Distance += DirectHitDistance;

	// Direct reflection from line of sight
	CalculateTapProperties("Direct.Second.C", SubmixEffect, Camera,
		ListenerLocation, DirectReflectionFromLOSHitResult.Location, TapIDs[1],
		DirectReflectionFromLOSHitResult.Distance, bHit);
	
	// TODO: 5 or 6 other traces
	
}

void UWeaponAudioFunctions::CalculateTapProperties(const FString& DebugString, USubmixEffectTapDelayPreset* SubmixEffect,
	UCameraComponent* CameraComponent, const FVector& ListenerLocation, const FVector& HitLocation, const int32 TapID,
	const float TravelDistance, const bool bHit)
{
	// Carry over settings on hit to minimize artifacts
	if (!bHit)
	{
		FTapDelayInfo TapDelayInfo;
		SubmixEffect->GetTap(TapID, TapDelayInfo);
		TapDelayInfo.TapLineMode = ETapLineMode::Panning;
		TapDelayInfo.Gain = -60.f;
		TapDelayInfo.OutputChannel = 0;
		SubmixEffect->SetTap(TapID, TapDelayInfo);
		return;
	}

	FTapDelayInfo TapDelayInfo;
	TapDelayInfo.OutputChannel = 0;
	TapDelayInfo.TapLineMode = ETapLineMode::Panning;
	
	const float HitDistance = (ListenerLocation - HitLocation).Length();

	const float Alpha = FMath::GetMappedRangeValueUnclamped(FVector2f(0, 10000.f), FVector2f(0, 1.f),
		TravelDistance + HitDistance);
	
	// Attenuate when direct hits are nearby
	const float AttenuatedHitDistance = FMath::GetMappedRangeValueClamped(FVector2f(0, 1000.f), FVector2f(0, 1.f),
		HitDistance);
	TapDelayInfo.Gain = FMath::GetMappedRangeValueClamped(FVector2f(0, 1.f), FVector2f(-9.f, -60.f),
		AttenuatedHitDistance * Alpha);

	// Ease in
	TapDelayInfo.DelayLength = FMath::GetMappedRangeValueUnclamped(FVector2f(0, 1.f), FVector2f(10.f, 600.f),
		Alpha * Alpha);
	
	FVector CameraDistance = CameraComponent->GetComponentLocation() - HitLocation;
	CameraDistance.Normalize();
	const FVector RightVector = FRotationMatrix(CameraComponent->GetComponentRotation()).GetScaledAxis(EAxis::Y);
	const float DotProduct = RightVector.Dot(CameraDistance);
	TapDelayInfo.PanInDegrees = DotProduct * 180.f;
	
	SubmixEffect->SetTap(TapID, TapDelayInfo);
}
