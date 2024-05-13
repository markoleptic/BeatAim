// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Audio/WeaponAudioFunctions.h"
#include "GameplayCueNotifyTypes.h"
#include "Camera/CameraComponent.h"
#include "Character/BSCharacterBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
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
	UWorld* World = Target->GetWorld();
	const FVector ListenerLoc = Camera->GetComponentLocation();
	const FVector PawnLoc = BSCharacter->GetActorLocation();
	const FVector HitLoc = Params.Location;
	const FVector HitNormal = Params.Normal;
	const float DirectHitDist = (HitLoc - ListenerLoc).Length();

	if (DirectHitDist >= 4000.f && Target != UGameplayStatics::GetPlayerPawn(Target->GetWorld(), 0))
	{
		return;
	}

	TArray<int32> TapIDs;
	SubmixEffect->GetTapIds(TapIDs);

	FCollisionQueryParams CParams;
	CParams.AddIgnoredActor(Target);

	// Direct hit (HitResult from GCN)
	CalculateTapProperties("Direct.Primary", SubmixEffect, Camera, ListenerLoc, HitLoc, TapIDs[0], DirectHitDist, bHit);

	// Direct reflection from line of sight (Center)
	FHitResult DirectLOS;
	const FVector DirectFromLOSEnd = HitLoc + HitNormal * 4000.f;
	bHit = World->LineTraceSingleByChannel(DirectLOS, HitLoc, DirectFromLOSEnd, ECC_Visibility, CParams);
	DirectLOS.Distance += DirectHitDist;
	CalculateTapProperties("Direct.Second.C", SubmixEffect, Camera, ListenerLoc, DirectLOS.Location, TapIDs[1],
		DirectLOS.Distance, bHit);

	// Side Reflections from line of sight
	FVector LeftAngle, RightAngle;
	FVector UpVector = FRotationMatrix(HitNormal.ToOrientationRotator()).GetScaledAxis(EAxis::Z);
	GetSideReflectionAngles(HitLoc, HitNormal, 4000.f, UpVector, LeftAngle, RightAngle);

	FHitResult LeftLOS;
	bHit = Target->GetWorld()->LineTraceSingleByChannel(LeftLOS, HitLoc, LeftAngle, ECC_Visibility, CParams);
	LeftLOS.Distance += DirectHitDist;
	CalculateTapProperties("Side.Second.L", SubmixEffect, Camera, ListenerLoc, LeftLOS.Location, TapIDs[4],
		LeftLOS.Distance, bHit);

	FHitResult RightLOS;
	bHit = Target->GetWorld()->LineTraceSingleByChannel(RightLOS, HitLoc, RightAngle, ECC_Visibility, CParams);
	RightLOS.Distance += DirectHitDist;
	CalculateTapProperties("Side.Second.R", SubmixEffect, Camera, ListenerLoc, RightLOS.Location, TapIDs[5],
		RightLOS.Distance, bHit);

	// Side Reflections from Line of Sight's Direct Reflection
	UpVector = FRotationMatrix(DirectLOS.Normal.ToOrientationRotator()).GetScaledAxis(EAxis::Z);
	GetSideReflectionAngles(DirectLOS.Location, DirectLOS.Normal, 4000.f, UpVector, LeftAngle, RightAngle);

	FHitResult LeftLOSDirect;
	bHit = Target->GetWorld()->LineTraceSingleByChannel(LeftLOSDirect, DirectLOS.Location, LeftAngle, ECC_Visibility,
		CParams);
	LeftLOSDirect.Distance += DirectLOS.Distance;
	CalculateTapProperties("Direct.Second.L", SubmixEffect, Camera, ListenerLoc, LeftLOSDirect.Location, TapIDs[2],
		LeftLOSDirect.Distance, bHit);

	FHitResult RightLOSDirect;
	bHit = Target->GetWorld()->LineTraceSingleByChannel(RightLOSDirect, DirectLOS.Location, RightAngle, ECC_Visibility,
		CParams);
	RightLOSDirect.Distance += DirectLOS.Distance;
	CalculateTapProperties("Direct.Second.R", SubmixEffect, Camera, ListenerLoc, RightLOSDirect.Location, TapIDs[3],
		RightLOSDirect.Distance, bHit);

	// Side Reflections from Weapon
	FVector CameraNormal = Camera->GetComponentLocation() - HitLoc;
	CameraNormal.Normalize();
	FVector UpVectorWeapon = FRotationMatrix(Camera->GetComponentRotation()).GetScaledAxis(EAxis::Z);
	GetSideReflectionAngles(PawnLoc, CameraNormal, 4000.f, UpVectorWeapon, LeftAngle, RightAngle);

	FHitResult LeftWeapon;
	bHit = Target->GetWorld()->LineTraceSingleByChannel(LeftWeapon, PawnLoc, LeftAngle, ECC_Visibility, CParams);
	CalculateTapProperties("Side.First.L", SubmixEffect, Camera, FVector(0.f), LeftWeapon.Location, TapIDs[6],
		LeftWeapon.Distance, bHit);

	FHitResult RightWeapon;
	bHit = Target->GetWorld()->LineTraceSingleByChannel(RightWeapon, PawnLoc, RightAngle, ECC_Visibility, CParams);
	CalculateTapProperties("Side.First.R", SubmixEffect, Camera, FVector(0.f), RightWeapon.Location, TapIDs[7],
		RightWeapon.Distance, bHit);
}

void UWeaponAudioFunctions::CalculateTapProperties(const FString& DebugString,
	USubmixEffectTapDelayPreset* SubmixEffect, UCameraComponent* CameraComponent, const FVector& ListenerLocation,
	const FVector& HitLocation, const int32 TapID, const float TravelDistance, const bool bHit)
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

void UWeaponAudioFunctions::GetSideReflectionAngles(const FVector& Origin, const FVector& Normal,
	const float TraceDistance, const FVector& Axis, FVector& OutAngleLeft, FVector& OutAngleRight)
{
	OutAngleLeft = UKismetMathLibrary::RotateAngleAxis(Normal, 45.f, Axis) * TraceDistance + Origin;
	OutAngleRight = UKismetMathLibrary::RotateAngleAxis(Normal, -45.f, Axis) * TraceDistance + Origin;
}
