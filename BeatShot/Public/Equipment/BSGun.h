// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSEquipmentActor.h"
#include "BSPlayerSettingsInterface.h"
#include "GameFramework/Actor.h"
#include "BSGun.generated.h"

class USkeletalMeshComponent;

DECLARE_DELEGATE_OneParam(FOnShotFired, class ABSPlayerController*);

/** The base gun used in this game. */
UCLASS()
class BEATSHOT_API ABSGun : public ABSEquipmentActor, public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

	ABSGun();

	virtual void BeginPlay() override;

	/** ~IBSPlayerSettingsInterface begin */
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings) override;
	/** ~IBSPlayerSettingsInterface end */

public:
	/** Increments ShotsFired, executes OnShotFired. */
	UFUNCTION(BlueprintCallable)
	virtual void Fire();

	/** Sets bIsFiring to false. */
	UFUNCTION(BlueprintCallable)
	virtual void StopFire();

	/** Returns the location of the muzzle. */
	UFUNCTION(BlueprintPure, BlueprintCallable)
	virtual FVector GetMuzzleLocation() const;

	UFUNCTION(BlueprintPure, BlueprintCallable)
	float GetFireRate() const { return FireRate; }

	/** Returns whether the weapon can fire. */
	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool CanFire() const { return bCanFire; }

	/** Returns whether the gun is currently firing (input is being held down). */
	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool IsFiring() const { return bIsFiring; }

	/** Sets whether this gun can be fired. */
	UFUNCTION(BlueprintCallable)
	void SetCanFire(const bool bNewFire) { bCanFire = bNewFire; }

	/** Sets the fire rate of this gun, updating its GameplayTags. */
	UFUNCTION(BlueprintCallable)
	void SetFireRate(const bool bAutomatic);

	/** Sets whether the gun should recoil, updating its GameplayTags. */
	UFUNCTION(BlueprintCallable)
	void SetShouldRecoil(const bool bRecoil);

	/** Sets whether the gun should show bullet decals, updating its GameplayTags. */
	UFUNCTION(BlueprintCallable)
	void SetShowDecals(const bool bShowDecals);

	/** Sets whether the gun should show bullet tracers, updating its GameplayTags. */
	UFUNCTION(BlueprintCallable)
	void SetShowTracers(const bool bShowTracers);

	/** Sets whether the weapon mesh is visible, updating its GameplayTags. */
	UFUNCTION(BlueprintCallable)
	void SetShowWeaponMesh(const bool bShow);

	/** Sets whether the muzzle flash is shown, updating its GameplayTags. */
	UFUNCTION(BlueprintCallable)
	void SetShowMuzzleFlash(const bool bShow);

	UFUNCTION(BlueprintCallable)
	void TriggerFireAudio(USoundBase* Sound, AActor* OwningActor);

	/** GameMode binds to this delegate to keep track of number of shots fired. */
	FOnShotFired OnShotFired;

protected:
	/** The skeletal mesh of the gun. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Components")
	USkeletalMeshComponent* MeshComp;

	/** The location of the muzzle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Components")
	USceneComponent* MuzzleLocationComp;

	UPROPERTY()
	UAudioComponent* FireAudioComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Audio")
	FName FireTriggerParameterName = "Fire";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Audio")
	FName FireAttachPointName = "hand_r";

	/** whether the player is holding down left click. */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon|State")
	bool bIsFiring;

	/** Determines if the player can fire. */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon|State")
	bool bCanFire;

	/** The fire rate of the weapon. */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon|Stats")
	float FireRate = 0.11f;
};
