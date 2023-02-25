// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "DefaultCharacter.h"
#include "SaveGamePlayerSettings.h"
#include "DefaultGameInstance.h"
#include "DefaultPlayerController.h"
#include "SphereTarget.h"
#include "Gun_AK47.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Kismet/KismetMathLibrary.h"

ADefaultCharacter::ADefaultCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(55.f, DefaultCapsuleHalfHeight);
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	SpringArmComponent->SetupAttachment(RootComponent);

	ShotDirection = CreateDefaultSubobject<UArrowComponent>("ShotDirection");
	ShotDirection->SetupAttachment(SpringArmComponent);

	CameraRecoilComp = CreateDefaultSubobject<USceneComponent>("CameraRecoilComp");
	CameraRecoilComp->SetupAttachment(ShotDirection);

	Camera = CreateDefaultSubobject<UCameraComponent>("First Person Camera");
	Camera->SetupAttachment(CameraRecoilComp);
	Camera->bUsePawnControlRotation = false;
	Camera->SetFieldOfView(103);
	Camera->PostProcessSettings.MotionBlurAmount = 0;
	Camera->PostProcessSettings.bOverride_MotionBlurMax = 0;

	HandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Character Mesh");
	HandsMesh->SetOnlyOwnerSee(true);
	HandsMesh->SetupAttachment(Camera);
	HandsMesh->bCastDynamicShadow = false;
	HandsMesh->CastShadow = false;
	HandsMesh->SetRelativeRotation(FRotator(1.53f, -15.20f, 8.32f));
	HandsMesh->SetRelativeLocation(FVector(-17.69f, -10.50f, -149.11f));

	GunActorComp = CreateDefaultSubobject<UChildActorComponent>("GunActorComp");
	GunActorComp->SetChildActorClass(GunClass);
	GunActorComp->SetupAttachment(HandsMesh, "GripPoint");
	GunActorComp->CreateChildActor();
}

void ADefaultCharacter::BeginPlay()
{
	Super::BeginPlay();

	Gun = Cast<AGun_AK47>(GunActorComp->GetChildActor());
	OnUserSettingsChange(LoadPlayerSettings());
	ADefaultPlayerController* PlayerController = GetController<ADefaultPlayerController>();
	if (IsLocallyControlled())
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
	Cast<UDefaultGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))->OnPlayerSettingsChange.AddUniqueDynamic(this, &ADefaultCharacter::OnUserSettingsChange);
	
	FOnTimelineFloat OnTimelineFloat;
	OnTimelineFloat.BindUFunction(this, FName("OnTimelineTick_AimBot"));
	FOnTimelineEvent OnTimelineEvent;
	OnTimelineEvent.BindUFunction(this, FName("OnTimelineCompleted_AimBot"));
	AimBotTimeline.SetTimelineFinishedFunc(OnTimelineEvent);
	AimBotTimeline.AddInterpFloat(Curve_AimBotRotationSpeed, OnTimelineFloat);
	bEnabled_AimBot = false;
}

void ADefaultCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	/* Make sure that we have a valid PlayerController */
	if (const ADefaultPlayerController* PlayerController = Cast<ADefaultPlayerController>(GetController()))
	{
		/* Get the Enhanced Input Local Player Subsystem from the Local Player related to our Player Controller */
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			/* PawnClientRestart can run more than once in an Actor's lifetime, so start by clearing out any leftover mappings */
			Subsystem->ClearAllMappings();
			/* Add each mapping context, along with their priority values. Higher values out-prioritize lower values */
			Subsystem->AddMappingContext(BaseMappingContext, BaseMappingPriority);
		}
	}
}

void ADefaultCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/* Sets the new Target Half Height based on whether the player is crouching or standing */
	const float TargetHalfHeight = (MovementState == EMovementType::Crouching
		                                ? CrouchedCapsuleHalfHeight
		                                : DefaultCapsuleHalfHeight);
	/* Interpolates between the current height and the target height */
	const float NewHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetHalfHeight,
	                                             DeltaTime, CrouchSpeed);
	/* Sets the half height of the capsule component to the new interpolated half height */
	GetCapsuleComponent()->SetCapsuleHalfHeight(NewHalfHeight);

	if (!bEnabled_AimBot)
	{
		return;
	}
	AimBotTimeline.TickTimeline(DeltaTime);
}

void ADefaultCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (JumpAction)
		{
			// Jumping
			PlayerEnhancedInputComponent->
				BindAction(JumpAction, ETriggerEvent::Started, this, &ADefaultCharacter::Jump);
		}
		if (SprintAction)
		{
			// Sprinting
			PlayerEnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this,
			                                         &ADefaultCharacter::StartWalk);
			PlayerEnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this,
			                                         &ADefaultCharacter::StopWalk);
		}
		if (MovementAction)
		{
			// Move forward/back + left/right inputs
			PlayerEnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this,
			                                         &ADefaultCharacter::Move);
		}
		if (LookAction)
		{
			// Look up/down + left/right
			PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
			                                         &ADefaultCharacter::Look);
		}
		if (CrouchAction)
		{
			// Crouching
			PlayerEnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this,
			                                         &ADefaultCharacter::StartCrouch);
			PlayerEnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this,
			                                         &ADefaultCharacter::ReleaseCrouch);
		}
		if (FiringAction)
		{
			// Firing
			PlayerEnhancedInputComponent->BindAction(FiringAction, ETriggerEvent::Started, this,
			                                         &ADefaultCharacter::StartFire);
			PlayerEnhancedInputComponent->BindAction(FiringAction, ETriggerEvent::Completed, this,
			                                         &ADefaultCharacter::StopFire);
		}
		if (InteractAction)
		{
			PlayerEnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ADefaultCharacter::OnInteractStarted);
			PlayerEnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ADefaultCharacter::OnInteractCompleted);
		}
		if (ShiftInteractAction)
		{
			PlayerEnhancedInputComponent->BindAction(ShiftInteractAction, ETriggerEvent::Started, this, &ADefaultCharacter::OnShiftInteractStarted);
			PlayerEnhancedInputComponent->BindAction(ShiftInteractAction, ETriggerEvent::Completed, this, &ADefaultCharacter::OnShiftInteractCompleted);
		}
	}
}

void ADefaultCharacter::Destroyed()
{
	Super::Destroyed();
}

void ADefaultCharacter::OnUserSettingsChange(const FPlayerSettings& PlayerSettings)
{
	Sensitivity = PlayerSettings.Sensitivity;
	if (Gun)
	{
		if (Gun->bAutomaticFire != PlayerSettings.Game.bAutomaticFire ||
			Gun->bShouldRecoil != PlayerSettings.Game.bShouldRecoil)
		{
			Gun->StopFire();
			Camera->SetRelativeRotation(FRotator(0, 0, 0));
			CameraRecoilComp->SetRelativeRotation(FRotator(0, 0, 0));
			Gun->bShouldRecoil = PlayerSettings.Game.bShouldRecoil;
			Gun->bAutomaticFire = PlayerSettings.Game.bAutomaticFire;
		}
		if (Gun->bShowBulletDecals != PlayerSettings.Game.bShowBulletDecals)
		{
			Gun->bShowBulletDecals = PlayerSettings.Game.bShowBulletDecals;
		}
		Gun->PlayerSettings = PlayerSettings;
	}
}

void ADefaultCharacter::StartFire() const
{
	Gun->StartFire();
}

void ADefaultCharacter::StopFire() const
{
	Gun->StopFire();
}

void ADefaultCharacter::Move(const FInputActionValue& Value)
{
	// Moving the player
	if (Value.GetMagnitude() != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value[1]);
		AddMovementInput(GetActorRightVector(), Value[0]);
	}
}

void ADefaultCharacter::Look(const FInputActionValue& Value)
{
	AddControllerPitchInput(Value[1] / 14.2789148024750118991f * Sensitivity);
	AddControllerYawInput(Value[0] / 14.2789148024750118991f * Sensitivity);
}

void ADefaultCharacter::ReleaseCrouch()
{
	bHoldingCrouch = false;
	if (MovementState == EMovementType::Walking)
	{
		return;
	}
	UpdateMovementValues(EMovementType::Sprinting);
}

void ADefaultCharacter::StartCrouch()
{
	bHoldingCrouch = true;
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		UpdateMovementValues(EMovementType::Crouching);
	}
}

void ADefaultCharacter::StartWalk()
{
	bHoldingWalk = true;
	UpdateMovementValues(EMovementType::Walking);
}

void ADefaultCharacter::StopWalk()
{
	if (MovementState == EMovementType::Walking)
	{
		UpdateMovementValues(EMovementType::Sprinting);
	}
	bHoldingWalk = false;
}

void ADefaultCharacter::OnInteractStarted(const FInputActionInstance& Instance)
{
	if (!OnInteractDelegate.ExecuteIfBound(0))
	{
		UE_LOG(LogTemp, Display, TEXT("OnInteractDelegate not bound."));
	}
}

void ADefaultCharacter::OnInteractCompleted(const FInputActionInstance& Instance)
{
	if (!OnInteractDelegate.ExecuteIfBound(1))
	{
		UE_LOG(LogTemp, Display, TEXT("OnInteractDelegate not bound."));
	}
}

void ADefaultCharacter::OnShiftInteractStarted(const FInputActionInstance& Instance)
{
	if (!OnShiftInteractDelegate.ExecuteIfBound(0))
	{
		UE_LOG(LogTemp, Display, TEXT("OnInteractDelegate not bound."));
	}
}

void ADefaultCharacter::OnShiftInteractCompleted(const FInputActionInstance& Instance)
{
	if (!OnShiftInteractDelegate.ExecuteIfBound(1))
	{
		UE_LOG(LogTemp, Display, TEXT("OnInteractDelegate not bound."));
	}
}

void ADefaultCharacter::UpdateMovementValues(const EMovementType NewMovementType)
{
	// Clearing sprinting and crouching flags
	bIsWalking = false;
	bIsCrouching = false;

	// Updating the movement state
	MovementState = NewMovementType;

	//GunActorComp->SetCanFire(MovementDataMap[MovementState].bCanFire);
	GetCharacterMovement()->MaxAcceleration = MovementDataMap[MovementState].MaxAcceleration;
	GetCharacterMovement()->BrakingDecelerationWalking = MovementDataMap[MovementState].BreakingDecelerationWalking;
	GetCharacterMovement()->GroundFriction = MovementDataMap[MovementState].GroundFriction;
	GetCharacterMovement()->MaxWalkSpeed = MovementDataMap[MovementState].MaxWalkSpeed;


	// Updating sprinting and crouching flags
	if (MovementState == EMovementType::Crouching)
	{
		bIsCrouching = true;
	}
	if (MovementState == EMovementType::Walking)
	{
		bIsWalking = true;
	}
}

void ADefaultCharacter::PassTrackingTargetToGun(ASphereTarget* TrackingTarget)
{
	Gun->TrackingTarget = TrackingTarget;
}

void ADefaultCharacter::OnTargetSpawned_AimBot(ASphereTarget* SpawnedTarget)
{
	ActiveTargets_AimBot.Enqueue(SpawnedTarget->GetActorLocation());
	if (!AimBotTimeline.IsPlaying())
	{
		DestroyNextTarget_AimBot();
	}
}

void ADefaultCharacter::OnTimelineTick_AimBot(const float Alpha)
{
	GetController()->SetControlRotation(UKismetMathLibrary::RLerp(StartRotation_AimBot, TargetRotation_AimBot, Alpha, true));
}

void ADefaultCharacter::OnTimelineCompleted_AimBot()
{
	ActiveTargets_AimBot.Pop();
	Gun->Fire_AimBot();
	if (!AimBotTimeline.IsPlaying())
	{
		DestroyNextTarget_AimBot();
	}
}

void ADefaultCharacter::DestroyNextTarget_AimBot()
{
	if (!ActiveTargets_AimBot.IsEmpty())
	{
		FVector TargetLocation;
		if (ActiveTargets_AimBot.Peek(TargetLocation))
		{
			StartRotation_AimBot = GetController()->GetControlRotation();
			StartRotation_AimBot.Normalize();
			FVector Loc;
			FRotator Rot;
			GetController()->GetActorEyesViewPoint(Loc, Rot);
			TargetRotation_AimBot = UKismetMathLibrary::FindLookAtRotation(Loc, TargetLocation);
			AimBotTimeline.SetPlayRate(TimelinePlaybackRate_AimBot);
			AimBotTimeline.PlayFromStart();
		}
	}
}
