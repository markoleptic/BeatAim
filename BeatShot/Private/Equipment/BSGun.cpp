// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Equipment/BSGun.h"
#include "BSGameInstance.h"
#include "BSGameMode.h"
#include "BeatShot/BSGameplayTags.h"
#include "Character/BSCharacterBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

ABSGun::ABSGun()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("Gun");
	MeshComp->SetOnlyOwnerSee(false);
	MeshComp->CastShadow = false;
	RootComponent = MeshComp;
	MuzzleLocationComp = CreateDefaultSubobject<USceneComponent>("Muzzle Location");
	MuzzleLocationComp->SetupAttachment(MeshComp, "Muzzle");
	bCanFire = true;
	bIsFiring = false;
}

void ABSGun::BeginPlay()
{
	Super::BeginPlay();

	OnPlayerSettingsChanged(LoadPlayerSettings().Game);

	UBSGameInstance* GI = Cast<UBSGameInstance>(GetGameInstance());
	GI->RegisterPlayerSettingsSubscriber<ABSGun, FPlayerSettings_Game>(this, &ABSGun::OnPlayerSettingsChanged);
}

void ABSGun::OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings)
{
	SetShouldRecoil(GameSettings.bShouldRecoil);
	SetFireRate(GameSettings.bAutomaticFire);
	SetShowDecals(GameSettings.bShowBulletDecals);
	SetShowTracers(GameSettings.bShowBulletTracers && GameSettings.bShowWeaponMesh && GameSettings.bShowCharacterMesh);
	SetShowMuzzleFlash(
		GameSettings.bShowMuzzleFlash && GameSettings.bShowWeaponMesh && GameSettings.bShowCharacterMesh);
	SetShowWeaponMesh(GameSettings.bShowWeaponMesh);
}

void ABSGun::Fire()
{
	if (!bCanFire)
	{
		return;
	}
	bIsFiring = true;
	if (OnShotFired.IsBound())
	{
		check(GetOwner());
		if (const ABSCharacterBase* Character = Cast<ABSCharacterBase>(GetOwner()))
		{
			OnShotFired.Execute(Character->GetBSPlayerController());
		}
	}
}

void ABSGun::StopFire()
{
	bIsFiring = false;
}

FVector ABSGun::GetMuzzleLocation() const
{
	return MuzzleLocationComp->GetComponentLocation();
}

void ABSGun::SetFireRate(const bool bAutomatic)
{
	StopFire();

	if (bAutomatic)
	{
		AddGameplayTag(BSGameplayTags::State_Weapon_AutomaticFire);
		return;
	}
	RemoveGameplayTag(BSGameplayTags::State_Weapon_AutomaticFire);
}

void ABSGun::SetShouldRecoil(const bool bRecoil)
{
	StopFire();

	if (bRecoil)
	{
		AddGameplayTag(BSGameplayTags::State_Weapon_Recoil);
		return;
	}
	RemoveGameplayTag(BSGameplayTags::State_Weapon_Recoil);
}

void ABSGun::SetShowDecals(const bool bShowDecals)
{
	if (bShowDecals)
	{
		AddGameplayTag(BSGameplayTags::State_Weapon_ShowDecals);
		return;
	}
	RemoveGameplayTag(BSGameplayTags::State_Weapon_ShowDecals);
}

void ABSGun::SetShowTracers(const bool bShowTracers)
{
	if (bShowTracers)
	{
		AddGameplayTag(BSGameplayTags::State_Weapon_ShowTracers);
		return;
	}
	RemoveGameplayTag(BSGameplayTags::State_Weapon_ShowTracers);
}

void ABSGun::SetShowWeaponMesh(const bool bShow)
{
	SetActorHiddenInGame(!bShow);
	if (bShow)
	{
		AddGameplayTag(BSGameplayTags::State_Weapon_ShowMesh);
		return;
	}
	RemoveGameplayTag(BSGameplayTags::State_Weapon_ShowMesh);
}

void ABSGun::SetShowMuzzleFlash(const bool bShow)
{
	if (bShow)
	{
		AddGameplayTag(BSGameplayTags::State_Weapon_ShowMuzzleFlash);
		return;
	}
	RemoveGameplayTag(BSGameplayTags::State_Weapon_ShowMuzzleFlash);
}

void ABSGun::TriggerFireAudio(USoundBase* Sound, AActor* OwningActor)
{
	if (!FireAudioComponent)
	{
		if (USkeletalMeshComponent* MeshComponent = OwningActor->GetComponentByClass<USkeletalMeshComponent>())
		{
			FireAudioComponent = UGameplayStatics::SpawnSoundAttached(Sound, MeshComponent, FireAttachPointName,
				FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset,
				false, 1.f, 1.f, 0.f, nullptr, nullptr, true);
		}
	}
	if (FireAudioComponent)
	{
		FireAudioComponent->SetTriggerParameter(FireTriggerParameterName);
	}
}
