// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveLoadInterface.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "BSInputConfig.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "GameplayAbility/BSAbilitySet.h"
#include "BSCharacter.generated.h"

class UBSRecoilComponent;
class UBSCharacterMovementComponent;
class ABSPlayerState;
class UBSMoveStepSound;
class UBSInventoryManagerComponent;
class UBSInventoryItemDefinition;
class UBSEquipmentManagerComponent;
class ABSPlayerController;
class USceneComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UAnimInstance;
class UInputMappingContext;
class UBSAttributeSetBase;
class UBSAbilitySystemComponent;
struct FInputActionValue;

DECLARE_DELEGATE_OneParam(FOnInteractDelegate, const int32);
DECLARE_DELEGATE_OneParam(FOnShiftInteractDelegate, const int32);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargetAddedToQueue);

inline float SimpleSpline(const float Value)
{
	const float ValueSquared = Value * Value;
	return (3.0f * ValueSquared - 2.0f * ValueSquared * Value);
}

/** Base Character for this game */
UCLASS()
class BEATSHOT_API ABSCharacter : public ACharacter, public ISaveLoadInterface, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
public:
	ABSCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** The spring arm component */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BeatShot|Components")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	/** The skeletal mesh for hands that hold the gun */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BeatShot|Components")
	TObjectPtr<USkeletalMeshComponent> HandsMesh;

	/** Camera component */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BeatShot|Components")
	TObjectPtr<UCameraComponent> CameraComponent;

	/** Additional layer of rotation to use for more realistic recoil */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BeatShot|Components")
	TObjectPtr<USceneComponent> CameraRecoilComponent;

	/** The Equipment Manager component */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BeatShot|Components")
	TObjectPtr<UBSEquipmentManagerComponent> EquipmentManagerComponent;

	/** The Equipment Manager component */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BeatShot|Components")
	TObjectPtr<UBSInventoryManagerComponent> InventoryManagerComponent;

	/** The component responsible for handling recoil generated by weapons */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BeatShot|Components")
	TObjectPtr<UBSRecoilComponent> RecoilComponent;

	/** Input configuration used by player controlled pawns to create input mappings and bind input actions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Input")
	TObjectPtr<UBSInputConfig> InputConfig;

	/** Default abilities, attributes, and effects */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BeatShot|Abilities")
	TArray<TObjectPtr<UBSAbilitySet>> AbilitySets;

	/** Default inventory items. Each entry should contain an inventory item definition composed of inventory fragments */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BeatShot|Inventory")
	TArray<TSubclassOf<UBSInventoryItemDefinition>> InitialInventoryItems;

	/** Move step sounds by physical surface */
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "BeatShot|Sounds")
	TMap<TEnumAsByte<EPhysicalSurface>, TSubclassOf<UBSMoveStepSound>> MoveStepSounds;
	
	TWeakObjectPtr<UBSAbilitySystemComponent> AbilitySystemComponent;
	TWeakObjectPtr<UBSAttributeSetBase> AttributeSetBase;
	FBSAbilitySet_GrantedHandles AbilitySet_GrantedHandles;

public:
	
#pragma region Getters
	/** Implement IAbilitySystemInterface */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	ABSPlayerController* GetBSPlayerController() const;
	
	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	ABSPlayerState* GetBSPlayerState() const;
	
	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	UBSAbilitySystemComponent* GetBSAbilitySystemComponent() const;

	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	UBSRecoilComponent* GetRecoilComponent() const;
	
	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	USkeletalMeshComponent* GetHandsMesh() const;

	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	UBSEquipmentManagerComponent* GetEquipmentManager() const;

	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	UBSInventoryManagerComponent* GetInventoryManager() const;

	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	UBSCharacterMovementComponent* GetBSCharacterMovement() const;

	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	float GetMinSpeedForFallDamage() const { return MinSpeedForFallDamage; };

	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	float GetMinLandBounceSpeed() const { return MinLandBounceSpeed; }

	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	float GetDefaultBaseEyeHeight() const { return DefaultBaseEyeHeight; }

	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	bool DoesWantToWalk() const { return bWantsToWalk; }

	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	float GetLastJumpTime() const { return LastJumpTime; }
	
	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	FORCEINLINE bool GetAutoBunnyHop() const { return bAutoBunnyHop; }

	FORCEINLINE TSubclassOf<UBSMoveStepSound>* GetMoveStepSound(const TEnumAsByte<EPhysicalSurface> Surface)
	{
		return MoveStepSounds.Find(Surface);
	}

#pragma endregion

#pragma region Aimbot

public:
	UFUNCTION(BlueprintCallable, Category = "BeatShot|Character")
	ASphereTarget* PeekActiveTargets();

	UFUNCTION(BlueprintCallable, Category = "BeatShot|Character")
	void PopActiveTargets();

	UFUNCTION(BlueprintCallable, Category = "BeatShot|Character")
	bool IsEnabled_AimBot() const { return bEnabled_AimBot; }
	
	/** Sets whether or not to enable AimBot */
	UFUNCTION(BlueprintCallable, Category = "BeatShot|Character")
	void SetEnabled_AimBot(const bool bEnable) { bEnabled_AimBot = bEnable; }

	/** Bound to DefaultGameMode's OnTargetSpawned delegate, executes when a target has been spawned and adds the spawned target to the ActiveTargetLocations_AimBot queue. */
	UFUNCTION()
	void OnTargetSpawned_AimBot(ASphereTarget* SpawnedTarget);

	UFUNCTION(BlueprintCallable, Category = "BeatShot|Character")
	float GetAimBotPlaybackSpeed() const;
	
	UPROPERTY(BlueprintAssignable, Category = "BeatShot|Character")
	FOnTargetAddedToQueue OnTargetAddedToQueue;

private:
	/** A queue of target locations that have not yet been destroyed */
	TQueue<ASphereTarget*> ActiveTargets_AimBot;

	/** Whether the AimBot is active */
	bool bEnabled_AimBot;

#pragma endregion

public:
	/** Implement IGameplayTagAssetInterface */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	/** End Implement IGameplayTagAssetInterface */

	/** Implement ISaveLoadInterface */
	virtual void OnPlayerSettingsChanged_Game(const FPlayerSettings_Game& GameSettings) override;
	virtual void OnPlayerSettingsChanged_User(const FPlayerSettings_User& UserSettings) override;
	/** End Implement ISaveLoadInterface */

	virtual bool IsSprinting() const;

	UFUNCTION(Category = "BeatShot|Character", BlueprintCallable)
	void SetAutoBunnyHop(bool Value) { bAutoBunnyHop = Value; }
	
protected:
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	virtual void ApplyDamageMomentum(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser) override;
	virtual void ClearJumpInput(float DeltaTime) override;
	virtual void Jump() override;
	virtual void StopJumping() override;
	virtual void OnJumped_Implementation() override;
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void RecalculateBaseEyeHeight() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual bool CanCrouch() const override;
	
	/** Grant abilities on the Server. The Ability Specs will be replicated to the owning client. Called from inside PossessedBy(). */
	virtual void AddCharacterAbilities();

	/** Grant equipment on the Server. Called from inside PossessedBy() */
	virtual void AddCharacterInventoryItems();

	/** Removes all CharacterAbilities. Can only be called by the Server. Removing on the Server will remove from Client too. */
	virtual void RemoveCharacterAbilities();

	/** Removes all equipment on the Server. Called from inside PossessedBy() */
	virtual void RemoveCharacterInventoryItems();

#pragma region Input

public:
	/** Executed when interact is pressed */
	FOnInteractDelegate OnInteractDelegate;

	/** Executed when shift interact is pressed */
	FOnShiftInteractDelegate OnShiftInteractDelegate;
	
private:
	/** Move the character left/right and forward/back */
	void Input_Move(const FInputActionValue& Value);

	/** Look left/right and up/down */
	void Input_Look(const FInputActionValue& Value);

	/** Toggles crouching */
	void Input_Crouch(const FInputActionValue& Value);

	/** Walk instead of default sprint */
	void Input_WalkStart(const FInputActionValue& Value);

	/** Walk instead of default sprint */
	void Input_WalkEnd(const FInputActionValue& Value);

	/** Crouches or un-crouches based on current state */
	void ToggleCrouch();

	/** Triggered on pressing E */
	void OnInteractStarted(const FInputActionValue& Value);

	/** Triggered on releasing E */
	void OnInteractCompleted(const FInputActionValue& Value);

	/** Triggered on pressing Shift + E */
	void OnShiftInteractStarted(const FInputActionValue& Value);

	/** Triggered on releasing Shift + E */
	void OnShiftInteractCompleted(const FInputActionValue& Value);

	void Input_OnInspectStarted(const FInputActionValue& Value);

	void Input_OnMeleeStarted(const FInputActionValue& Value);

	void Input_OnEquipmentSlot1Started(const FInputActionValue& Value);

	void Input_OnEquipmentSlot2Started(const FInputActionValue& Value);

	void Input_OnEquipmentSlot3Started(const FInputActionValue& Value);

	void Input_OnEquipmentSlotLastEquippedStarted(const FInputActionValue& Value);

	/** Let ASC know an ability bound to an input was pressed. */
	UFUNCTION(BlueprintCallable)
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);

	UFUNCTION(BlueprintCallable)
	/** Let ASC know an ability bound to an input was released. */
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
	/** Multiplier to controller pitch and yaw */
	float Sensitivity;

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Input")
	// ReSharper disable once UnrealHeaderToolError
	UInputMappingContext* BaseMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Input")
	int32 BaseMappingPriority = 0;

	const float SensitivityMultiplier = 14.2789148024750118991f;
#pragma endregion

protected:
	/** Automatic bunny-hopping */
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "PB Player|Gameplay")
	bool bAutoBunnyHop;

	/** Minimum speed to play the camera shake for landing */
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "PB Player|Damage")
	float MinLandBounceSpeed;

	/** Don't take damage below this speed - so jumping doesn't damage */
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "PB Player|Damage")
	float MinSpeedForFallDamage;

	// In HL2, the player has the Z component for applying momentum to the capsule capped
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "PB Player|Damage")
	float CapDamageMomentumZ = 0.f;

	bool bWantsToWalk;

	/** defer the jump stop for a frame (for early jumps) */
	bool bDeferJumpStop;
	
	/** cached default eye height */
	float DefaultBaseEyeHeight;

	/** when we last jumped */
	float LastJumpTime;

	/** throttle jump boost when going up a ramp, so we don't spam it */
	float LastJumpBoostTime;

	/** maximum time it takes to jump */
	float MaxJumpTime;
};
