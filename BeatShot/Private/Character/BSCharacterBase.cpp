// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Character/BSCharacterBase.h"

#include "BSGameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "SaveGamePlayerSettings.h"
#include "AbilitySystem/BSAbilitySystemComponent.h"
#include "AbilitySystem/Globals/BSAttributeSetBase.h"
#include "BeatShot/BSGameplayTags.h"
#include "Camera/CameraComponent.h"
#include "Character/BSCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Equipment/BSEquipmentManagerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Character/BSRecoilComponent.h"
#include "Input/BSInputComponent.h"
#include "Inventory/BSInventoryManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BSPlayerController.h"
#include "Player/BSPlayerState.h"

static TAutoConsoleVariable CVarAutoBHop(TEXT("move.Pogo"), 0,
	TEXT("If holding spacebar should make the player jump whenever possible.\n"), ECVF_Default);
static TAutoConsoleVariable CVarJumpBoost(TEXT("move.JumpBoost"), 1, TEXT(
	"If the player should boost in a movement direction while jumping.\n0 - disables jump boosting entirely\n"
	"1 - boosts in the direction of input, even when moving in another direction\n2 - boosts in the direction "
	"of input when moving in the same direction\n"), ECVF_Default);
static TAutoConsoleVariable CVarBunnyHop(TEXT("move.BunnyHopping"), 0, TEXT("Enable normal bunnyhopping.\n"),
	ECVF_Default);

ABSCharacterBase::ABSCharacterBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UBSCharacterMovementComponent>(CharacterMovementComponentName)),
	BaseMappingContext(nullptr), Sensitivity(0), bAutoBunnyHop(false), MinLandBounceSpeed(0), MinSpeedForFallDamage(0),
	bWantsToWalk(false), bDeferJumpStop(false), LastJumpTime(0), LastJumpBoostTime(0), MaxJumpTime(0)
{
	PrimaryActorTick.bCanEverTick = true;
	bAlwaysRelevant = true;
	bReplicates = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Component"));
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->TargetArmLength = 0.f;
	SpringArmComponent->SetupAttachment(GetMesh(), HeadSocket);

	RecoilComponent = CreateDefaultSubobject<UBSRecoilComponent>("Recoil Component");
	RecoilComponent->SetupAttachment(SpringArmComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("Camera Component");
	CameraComponent->SetupAttachment(RecoilComponent);
	CameraComponent->SetFieldOfView(103);
	CameraComponent->PostProcessSettings.MotionBlurAmount = 0;
	CameraComponent->PostProcessSettings.bOverride_MotionBlurMax = 0;

	EquipmentManagerComponent = CreateDefaultSubobject<UBSEquipmentManagerComponent>("Equipment Manager Component");
	InventoryManagerComponent = CreateDefaultSubobject<UBSInventoryManagerComponent>("Inventory Manager Component");

	// Camera eye level
	BaseEyeHeight = 64.f;
	CrouchedEyeHeight = 44.f;
	MinSpeedForFallDamage = 1002.9825f;
	MinLandBounceSpeed = 329.565f;
	CapDamageMomentumZ = 476.25f;
}

UAbilitySystemComponent* ABSCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

ABSPlayerController* ABSCharacterBase::GetBSPlayerController() const
{
	return CastChecked<ABSPlayerController>(Controller, ECastCheckedType::NullAllowed);
}

ABSPlayerState* ABSCharacterBase::GetBSPlayerState() const
{
	return CastChecked<ABSPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

UBSAbilitySystemComponent* ABSCharacterBase::GetBSAbilitySystemComponent() const
{
	return CastChecked<UBSAbilitySystemComponent>(GetAbilitySystemComponent(), ECastCheckedType::NullAllowed);
}

UBSEquipmentManagerComponent* ABSCharacterBase::GetEquipmentManager() const
{
	return EquipmentManagerComponent.Get();
}

UBSInventoryManagerComponent* ABSCharacterBase::GetInventoryManager() const
{
	return InventoryManagerComponent.Get();
}

UBSCharacterMovementComponent* ABSCharacterBase::GetBSCharacterMovement() const
{
	return CastChecked<UBSCharacterMovementComponent>(GetCharacterMovement(), ECastCheckedType::NullAllowed);
}

void ABSCharacterBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (GetBSAbilitySystemComponent())
	{
		GetBSAbilitySystemComponent()->GetOwnedGameplayTags(TagContainer);
	}
}

void ABSCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled() && IsPlayerControlled())
	{
		GetBSPlayerController()->SetInputMode(FInputModeGameOnly());
	}

	// Max jump time to get to the top of the arc
	MaxJumpTime = -4.0f * GetCharacterMovement()->JumpZVelocity / (3.0f * GetCharacterMovement()->GetGravityZ());

	const FPlayerSettings Settings = LoadPlayerSettings();
	OnPlayerSettingsChanged(Settings.Game);
	OnPlayerSettingsChanged(Settings.User);

	UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	GI->RegisterPlayerSettingsSubscriber<ABSCharacterBase, FPlayerSettings_Game>(this,
		&ABSCharacterBase::OnPlayerSettingsChanged);
	GI->RegisterPlayerSettingsSubscriber<ABSCharacterBase, FPlayerSettings_User>(this,
		&ABSCharacterBase::OnPlayerSettingsChanged);
}

void ABSCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDeferJumpStop)
	{
		bDeferJumpStop = false;
		Super::StopJumping();
	}
}

void ABSCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		PC->GetLocalPlayer());
	check(Subsystem);

	// PawnClientRestart can run more than once in an Actor's lifetime, so start by clearing out any leftover mappings
	Subsystem->ClearAllMappings();

	// Add each mapping context, along with their priority values. Higher values out-prioritize lower values
	Subsystem->AddMappingContext(BaseMappingContext, BaseMappingPriority);
	Subsystem->InitalizeUserSettings();

	UEnhancedInputUserSettings* EnhancedInputUserSettings = Subsystem->GetUserSettings();
	if (!EnhancedInputUserSettings->IsMappingContextRegistered(BaseMappingContext))
	{
		EnhancedInputUserSettings->RegisterInputMappingContext(BaseMappingContext);
	}

	if (const UBSInputConfig* LoadedConfig = InputConfig)
	{
		UBSInputComponent* BSInputComponent = CastChecked<UBSInputComponent>(PlayerInputComponent);

		BSInputComponent->BindAbilityActions(LoadedConfig, this, &ThisClass::Input_AbilityInputTagPressed,
			&ThisClass::Input_AbilityInputTagReleased);

		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_Move_Forward, this,
			&ThisClass::Input_Move, true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_Move_Backward, this,
			&ThisClass::Input_Move, true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_Move_Left, this, &ThisClass::Input_Move,
			true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_Move_Right, this, &ThisClass::Input_Move,
			true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_Look, this, &ThisClass::Input_Look,
			true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_Jump, this, &ThisClass::Jump, true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_Pause, this, &ThisClass::Input_OnPause,
			true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_Walk, this, &ThisClass::Input_WalkStart,
			&ThisClass::Input_WalkEnd, true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_Crouch, this, &ThisClass::Input_Crouch,
			&ThisClass::Input_Crouch, true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_Interact, this,
			&ThisClass::Input_OnInteractStarted, &ThisClass::Input_OnInteractCompleted, true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_ShiftInteract, this,
			&ThisClass::Input_OnShiftInteractStarted, &ThisClass::Input_OnShiftInteractCompleted, true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_EquipmentSlot_1, this,
			&ThisClass::Input_OnEquipmentSlot1Started, true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_EquipmentSlot_2, this,
			&ThisClass::Input_OnEquipmentSlot2Started, true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_EquipmentSlot_3, this,
			&ThisClass::Input_OnEquipmentSlot3Started, true);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_EquipmentSlot_LastEquipped, this,
			&ThisClass::Input_OnEquipmentSlotLastEquippedStarted, true);
	}
}

void ABSCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ABSCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ABSPlayerState* PS = GetPlayerState<ABSPlayerState>();
	if (PS)
	{
		// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
		AbilitySystemComponent = Cast<UBSAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// AI won't have PlayerControllers so we can init again here just to be sure.
		// No harm in initing twice for heroes that have PlayerControllers.
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

		// Set the AttributeSetBase for convenience attribute functions
		AttributeSetBase = PS->GetAttributeSetBase();

		// If we handle players disconnecting and rejoining in the future, we'll have to change this so that possession
		// from rejoining doesn't reset attributes. For now assume possession = spawn/respawn.
		AddCharacterAbilities();

		AddCharacterInventoryItems();
	}
}

void ABSCharacterBase::UnPossessed()
{
	Super::UnPossessed();
	RemoveCharacterAbilities();
	RemoveCharacterInventoryItems();
}

void ABSCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	ABSPlayerState* PS = GetPlayerState<ABSPlayerState>();
	if (PS)
	{
		UE_LOG(LogTemp, Display, TEXT("OnRep_PlayerState() called and Player State Exists"));
		// Set the ASC for clients. Server does this in PossessedBy.
		AbilitySystemComponent = Cast<UBSAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// Init ASC Actor Info for clients. Server will init its ASC when it possesses a new Actor.
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);

		// Set the AttributeSetBase for convenience attribute functions
		AttributeSetBase = PS->GetAttributeSetBase();
	}
}

void ABSCharacterBase::ApplyDamageMomentum(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator,
	AActor* DamageCauser)
{
	Super::ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);
	UDamageType const* const DmgTypeCDO = DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>();

	if (GetCharacterMovement())
	{
		FVector ImpulseDir;

		if (IsValid(DamageCauser))
		{
			ImpulseDir = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
		}
		else
		{
			FHitResult HitInfo;
			DamageEvent.GetBestHitInfo(this, DamageCauser, HitInfo, ImpulseDir);
		}

		const float SizeFactor = (60.96f * 60.96f * 137.16f) / (FMath::Square(
				GetCapsuleComponent()->GetScaledCapsuleRadius() * 2.0f) * GetCapsuleComponent()->
			GetScaledCapsuleHalfHeight() * 2.0f);

		float Magnitude = 1.905f * DamageTaken * SizeFactor * 5.0f;
		Magnitude = FMath::Min(Magnitude, 1905.0f);

		FVector Impulse = ImpulseDir * Magnitude;
		bool const bMassIndependentImpulse = !DmgTypeCDO->bScaleMomentumByMass;
		float MassScale = 1.f;
		if (!bMassIndependentImpulse && GetCharacterMovement()->Mass > SMALL_NUMBER)
		{
			MassScale = 1.f / GetCharacterMovement()->Mass;
		}
		if (CapDamageMomentumZ > 0.f)
		{
			Impulse.Z = FMath::Min(Impulse.Z * MassScale, CapDamageMomentumZ) / MassScale;
		}

		GetCharacterMovement()->AddImpulse(Impulse, bMassIndependentImpulse);
	}
}

void ABSCharacterBase::ClearJumpInput(float DeltaTime)
{
	// Don't clear jump input right away if we're auto hopping or no-clipping (holding to go up), or if we are deferring a jump stop
	if (CVarAutoBHop.GetValueOnGameThread() != 0 || bAutoBunnyHop || GetCharacterMovement()->bCheatFlying ||
		bDeferJumpStop)
	{
		return;
	}
	Super::ClearJumpInput(DeltaTime);
}

void ABSCharacterBase::Jump()
{
	if (GetCharacterMovement()->IsFalling())
	{
		bDeferJumpStop = true;
	}
	Super::Jump();
}

void ABSCharacterBase::StopJumping()
{
	if (!bDeferJumpStop)
	{
		Super::StopJumping();
	}
}

void ABSCharacterBase::OnJumped_Implementation()
{
	const int32 JumpBoost = CVarJumpBoost->GetInt();
	if (GetBSCharacterMovement()->IsOnLadder())
	{
		return;
	}

	if (GetWorld()->GetTimeSeconds() >= LastJumpBoostTime + MaxJumpTime && JumpBoost)
	{
		LastJumpBoostTime = GetWorld()->GetTimeSeconds();
		// Boost forward speed on jump
		const FVector Facing = GetActorForwardVector();
		// Use input direction
		FVector Input = GetCharacterMovement()->GetCurrentAcceleration();
		if (JumpBoost != 1)
		{
			// Only boost input in the direction of current movement axis (prevents ABH).
			Input *= FMath::Max(Input.GetSafeNormal2D() | GetCharacterMovement()->Velocity.GetSafeNormal2D(), 0.0f);
		}
		const float ForwardSpeed = Input | Facing;
		// Adjust how much the boost is
		const float SpeedBoostPerc = IsSprinting() || bIsCrouched ? 0.1f : 0.5f;
		// How much we are boosting by
		float SpeedAddition = FMath::Abs(ForwardSpeed * SpeedBoostPerc);
		// We can only boost up to this much
		const float MaxBoostedSpeed = GetCharacterMovement()->GetMaxSpeed() + GetCharacterMovement()->GetMaxSpeed() *
			SpeedBoostPerc;
		// Calculate new speed
		const float NewSpeed = SpeedAddition + GetMovementComponent()->Velocity.Size2D();
		float SpeedAdditionNoClamp = SpeedAddition;

		// Scale the boost down if we are going over
		if (NewSpeed > MaxBoostedSpeed)
		{
			SpeedAddition -= NewSpeed - MaxBoostedSpeed;
		}

		if (ForwardSpeed < -GetBSCharacterMovement()->GetMaxAcceleration() * FMath::Sin(0.6981f))
		{
			// Boost backwards if we're going backwards
			SpeedAddition *= -1.0f;
			SpeedAdditionNoClamp *= -1.0f;
		}

		// Boost our velocity
		FVector JumpBoostedVel = GetMovementComponent()->Velocity + Facing * SpeedAddition;
		float JumpBoostedSizeSq = JumpBoostedVel.SizeSquared2D();
		if (CVarBunnyHop.GetValueOnGameThread() != 0)
		{
			const FVector JumpBoostedUnClampVel = GetMovementComponent()->Velocity + Facing * SpeedAdditionNoClamp;
			const float JumpBoostedUnClampSizeSq = JumpBoostedUnClampVel.SizeSquared2D();
			if (JumpBoostedUnClampSizeSq > JumpBoostedSizeSq)
			{
				JumpBoostedVel = JumpBoostedUnClampVel;
				JumpBoostedSizeSq = JumpBoostedUnClampSizeSq;
			}
		}
		if (GetMovementComponent()->Velocity.SizeSquared2D() < JumpBoostedSizeSq)
		{
			GetMovementComponent()->Velocity = JumpBoostedVel;
		}
	}
}

bool ABSCharacterBase::CanJumpInternal_Implementation() const
{
	bool bCanJump = GetCharacterMovement() && GetCharacterMovement()->IsJumpAllowed();
	if (bCanJump)
	{
		// Ensure JumpHoldTime and JumpCount are valid.
		if (!bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
		{
			if (JumpCurrentCount == 0 && GetCharacterMovement()->IsFalling())
			{
				bCanJump = JumpCurrentCount + 1 < JumpMaxCount;
			}
			else
			{
				bCanJump = JumpCurrentCount < JumpMaxCount;
			}
		}
		else
		{
			// Only consider JumpKeyHoldTime as long as:
			// A) We are on the ground
			// B) The jump limit hasn't been met OR
			// C) The jump limit has been met AND we were already jumping
			const bool bJumpKeyHeld = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());
			bCanJump = bJumpKeyHeld && (GetCharacterMovement()->IsMovingOnGround() || (JumpCurrentCount < JumpMaxCount)
				|| (bWasJumping && JumpCurrentCount == JumpMaxCount));
		}
		if (GetCharacterMovement()->IsMovingOnGround())
		{
			const float FloorZ = FVector(0.0f, 0.0f, 1.0f) | GetCharacterMovement()->CurrentFloor.HitResult.
				ImpactNormal;
			const float WalkableFloor = GetCharacterMovement()->GetWalkableFloorZ();
			bCanJump &= (FloorZ >= WalkableFloor || FMath::IsNearlyEqual(FloorZ, WalkableFloor));
		}
	}
	return bCanJump;
}

void ABSCharacterBase::RecalculateBaseEyeHeight()
{
	const float OldUnscaledHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float CrouchedHalfHeight = GetBSCharacterMovement()->GetCrouchedHalfHeight();
	const float FullCrouchDiff = OldUnscaledHalfHeight - CrouchedHalfHeight;

	const UCapsuleComponent* CharacterCapsule = GetCapsuleComponent();
	const float CurrentUnscaledHalfHeight = CharacterCapsule->GetUnscaledCapsuleHalfHeight();
	const float CurrentAlpha = 1.0f - (CurrentUnscaledHalfHeight - CrouchedHalfHeight) / FullCrouchDiff;

	BaseEyeHeight = FMath::Lerp(BaseEyeHeight, CrouchedEyeHeight, SimpleSpline(CurrentAlpha));
}

void ABSCharacterBase::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);
	if (!bPressedJump)
	{
		ResetJumpState();
	}

	if (GetCharacterMovement()->IsFalling())
	{
		// Record jump force start time for proxies. Allows us to expire the jump even if not continually ticking down a timer.
		if (bProxyIsJumpForceApplied)
		{
			ProxyJumpForceStartedTime = GetWorld()->GetTimeSeconds();
		}
	}
	else
	{
		JumpCurrentCount = 0;
		JumpKeyHoldTime = 0.0f;
		JumpForceTimeRemaining = 0.0f;
		bWasJumping = false;
	}

	K2_OnMovementModeChanged(PrevMovementMode, GetCharacterMovement()->MovementMode, PrevCustomMode,
		GetCharacterMovement()->CustomMovementMode);
	MovementModeChangedDelegate.Broadcast(this, PrevMovementMode, PrevCustomMode);
}

bool ABSCharacterBase::CanCrouch() const
{
	return !GetCharacterMovement()->bCheatFlying && Super::CanCrouch() && !GetBSCharacterMovement()->IsOnLadder();
}

void ABSCharacterBase::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// The character will move into the floor for some reason without this
	FVector Location = GetMesh()->GetRelativeLocation();
	CacheInitialMeshOffset(Location, GetMesh()->GetRelativeRotation());
	Location.Z += ScaledHalfHeightAdjust;
	GetMesh()->SetRelativeLocation(Location);
}

bool ABSCharacterBase::IsSprinting() const
{
	if (bWantsToWalk || bIsCrouched)
	{
		return false;
	}
	if (GetVelocity().IsNearlyZero(0.01))
	{
		return false;
	}
	return true;
}

void ABSCharacterBase::AddCharacterAbilities()
{
	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent.IsValid() || AbilitySystemComponent->
		bCharacterAbilitiesGiven)
	{
		return;
	}

	for (const UBSAbilitySet* AbilitySet : AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(GetBSAbilitySystemComponent(), &AbilitySet_GrantedHandles);
		}
	}
	AbilitySystemComponent->bCharacterAbilitiesGiven = true;
}

void ABSCharacterBase::AddCharacterInventoryItems()
{
	if (GetLocalRole() != ROLE_Authority || !GetInventoryManager() || !GetEquipmentManager())
	{
		UE_LOG(LogTemp, Display, TEXT("Something wrong with AddCharacterInventoryItems()"));
		return;
	}

	int32 Index = 0;
	for (const TSubclassOf<UBSInventoryItemDefinition> Definition : InitialInventoryItems)
	{
		if (UBSInventoryItemInstance* ItemInstance = GetInventoryManager()->AddItemInstance(Definition, 1))
		{
			GetInventoryManager()->AddItemToSlot(Index, ItemInstance);
			Index++;
		}
	}
	if (InitialInventoryItems.Num() > 0)
	{
		GetInventoryManager()->SetActiveSlotIndex(0);
	}
}

void ABSCharacterBase::RemoveCharacterAbilities()
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent.IsValid() || !AbilitySystemComponent->
		bCharacterAbilitiesGiven)
	{
		UE_LOG(LogTemp, Display, TEXT("Something wrong with RemoveCharacterAbilities()"));
		return;
	}

	AbilitySet_GrantedHandles.TakeFromAbilitySystem(GetBSAbilitySystemComponent());
	AbilitySystemComponent->bCharacterAbilitiesGiven = false;
}

void ABSCharacterBase::RemoveCharacterInventoryItems()
{
	if (GetLocalRole() != ROLE_Authority || !InventoryManagerComponent || !EquipmentManagerComponent)
	{
		UE_LOG(LogTemp, Display, TEXT("Something wrong with RemoveCharacterInventoryItems()"));
		return;
	}

	UBSInventoryManagerComponent* InventoryManager = GetInventoryManager();
	for (int32 Index = 0; Index < InventoryManager->GetSlots().Num(); Index++)
	{
		UBSInventoryItemInstance* Removed = InventoryManager->RemoveItemFromSlot(Index);
		if (Removed)
		{
			InventoryManager->RemoveItemInstance(Removed);
		}
	}
}

void ABSCharacterBase::BindLeftClick()
{
	if (const UBSInputConfig* LoadedConfig = InputConfig.Get())
	{
		UBSInputComponent* BSInputComponent = CastChecked<UBSInputComponent>(InputComponent);
		BSInputComponent->BindNativeAction(LoadedConfig, BSGameplayTags::Input_LeftClick, this,
			&ThisClass::Input_OnLeftClick, true);
	}
}

void ABSCharacterBase::UnbindLeftClick()
{
	UBSInputComponent* BSInputComponent = CastChecked<UBSInputComponent>(InputComponent);
	BSInputComponent->RemoveNativeActionBinding(BSGameplayTags::Input_LeftClick);
}

void ABSCharacterBase::Input_Move(const FInputActionValue& Value)
{
	if (Value.IsNonZero())
	{
		AddMovementInput(GetActorForwardVector(), Value[1]);
		AddMovementInput(GetActorRightVector(), Value[0]);
	}
}

void ABSCharacterBase::Input_Look(const FInputActionValue& Value)
{
	AddControllerPitchInput(Value[1] / Constants::SensitivityMultiplier * Sensitivity);
	AddControllerYawInput(Value[0] / Constants::SensitivityMultiplier * Sensitivity);
}

void ABSCharacterBase::Input_Crouch(const FInputActionValue& Value)
{
	if (bIsCrouched || GetCharacterMovement()->bWantsToCrouch)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABSCharacterBase::Input_WalkStart(const FInputActionValue& Value)
{
	bWantsToWalk = true;
}

void ABSCharacterBase::Input_WalkEnd(const FInputActionValue& Value)
{
	bWantsToWalk = false;
}

void ABSCharacterBase::Input_OnInteractStarted(const FInputActionValue& Value)
{
	if (OnInteractDelegate.IsBound())
	{
		OnInteractDelegate.Execute(0);
	}
}

void ABSCharacterBase::Input_OnInteractCompleted(const FInputActionValue& Value)
{
	if (OnInteractDelegate.IsBound())
	{
		OnInteractDelegate.Execute(1);
	}
}

void ABSCharacterBase::Input_OnShiftInteractStarted(const FInputActionValue& Value)
{
	if (OnShiftInteractDelegate.IsBound())
	{
		OnShiftInteractDelegate.Execute(0);
	}
}

void ABSCharacterBase::Input_OnShiftInteractCompleted(const FInputActionValue& Value)
{
	if (OnShiftInteractDelegate.IsBound())
	{
		OnShiftInteractDelegate.Execute(1);
	}
}

void ABSCharacterBase::Input_OnEquipmentSlot1Started(const FInputActionValue& Value)
{
	if (GetInventoryManager()->GetSlots().Num() > 1)
	{
		GetInventoryManager()->SetActiveSlotIndex(0);
	}
}

void ABSCharacterBase::Input_OnEquipmentSlot2Started(const FInputActionValue& Value)
{
	if (GetInventoryManager()->GetSlots().Num() > 1)
	{
		GetInventoryManager()->SetActiveSlotIndex(1);
	}
}

void ABSCharacterBase::Input_OnEquipmentSlot3Started(const FInputActionValue& Value)
{
	if (GetInventoryManager()->GetSlots().Num() > 1)
	{
		GetInventoryManager()->SetActiveSlotIndex(2);
	}
}

void ABSCharacterBase::Input_OnEquipmentSlotLastEquippedStarted(const FInputActionValue& Value)
{
	if (GetInventoryManager()->GetSlots().Num() > 1)
	{
		GetInventoryManager()->SetActiveSlotIndex(GetInventoryManager()->GetLastSlotIndex());
	}
}

void ABSCharacterBase::Input_OnPause(const FInputActionValue& Value)
{
	if (GetBSPlayerController())
	{
		GetBSPlayerController()->HandlePause();
	}
}

void ABSCharacterBase::Input_OnLeftClick(const FInputActionValue& Value)
{
	if (GetBSPlayerController())
	{
		GetBSPlayerController()->HandleLeftClick();
	}
}

void ABSCharacterBase::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	GetBSAbilitySystemComponent()->AbilityInputTagPressed(InputTag);
}

void ABSCharacterBase::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	GetBSAbilitySystemComponent()->AbilityInputTagReleased(InputTag);
}

void ABSCharacterBase::OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings)
{
	if (!GetAbilitySystemComponent()) return;
	/* Changing activation policy for FireGun ability based on automatic fire bool */
	if (TArray<FGameplayAbilitySpec*> Specs = GetBSAbilitySystemComponent()->GetAbilitySpecsFromGameplayTag(
		BSGameplayTags::Input_Fire); !Specs.IsEmpty())
	{
		if (GameSettings.bAutomaticFire)
		{
			if (UBSGameplayAbility* Ability = Cast<UBSGameplayAbility>(Specs[0]->Ability))
			{
				Ability->ActivationPolicy = EBSAbilityActivationPolicy::WhileInputActive;
				GetAbilitySystemComponent()->MarkAbilitySpecDirty(*Specs[0]);
			}
		}
		else
		{
			if (UBSGameplayAbility* Ability = Cast<UBSGameplayAbility>(Specs[0]->Ability))
			{
				Ability->ActivationPolicy = EBSAbilityActivationPolicy::OnInputTriggered;
				GetAbilitySystemComponent()->MarkAbilitySpecDirty(*Specs[0]);
			}
		}
	}
	SetActorHiddenInGame(!GameSettings.bShowCharacterMesh);
}

void ABSCharacterBase::OnPlayerSettingsChanged(const FPlayerSettings_User& UserSettings)
{
	Sensitivity = UserSettings.Sensitivity;
}
