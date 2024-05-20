// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.
// Credit to Project Borealis for almost all of this code

#include "Character/BSCharacterMovementComponent.h"
#include "Audio/BSMovementSounds.h"
#include "Character/BSCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "DeveloperSettings/BSAudioSettings.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/PhysicsSettings.h"

static TAutoConsoleVariable CVarShowPos(TEXT("cl.ShowPos"), 0, TEXT("Show position and movement information.\n"),
	ECVF_Default);

DECLARE_CYCLE_STAT(TEXT("Char StepUp"), STAT_CharStepUp, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char PhysFalling"), STAT_CharPhysFalling, STATGROUP_Character);

// MAGIC NUMBERS
constexpr float JumpVelocity = 266.7f;
const float MAX_STEP_SIDE_Z = 0.08f; // maximum z value for the normal on the vertical side of steps
const float VERTICAL_SLOPE_NORMAL_Z = 0.001f;
// Slope is vertical if Abs(Normal.Z) <= this threshold. Accounts for precision problems that sometimes angle

constexpr float DesiredGravity = -1143.0f;

UBSCharacterMovementComponent::UBSCharacterMovementComponent()
{
	// We have our own air movement handling, so we can allow for full air control through UE's logic
	AirControl = 1.0f;

	// Disable air control boost
	AirControlBoostMultiplier = 0.0f;
	AirControlBoostVelocityThreshold = 0.0f;

	// HL2 cl_(forward & side)speed = 450Hu
	MaxAcceleration = 857.25f;

	// Set the default walk speed
	MaxWalkSpeed = RunSpeed;

	// HL2 like friction (sv_friction)
	GroundFriction = 4.0f;
	BrakingFriction = 4.0f;
	bUseSeparateBrakingFriction = false;

	// No multiplier
	BrakingFrictionFactor = 1.0f;

	// Historical value for Source
	BrakingSubStepTime = 0.015f;

	// Avoid breaking up time step
	MaxSimulationTimeStep = 0.5f;
	MaxSimulationIterations = 1;

	// Braking deceleration (sv_stopspeed)
	FallingLateralFriction = 0.0f;
	BrakingDecelerationFalling = 0.0f;
	BrakingDecelerationFlying = 190.5f;
	BrakingDecelerationSwimming = 190.5f;
	BrakingDecelerationWalking = 190.5f;

	// HL2 step height
	MaxStepHeight = 34.29f;
	DefaultStepHeight = MaxStepHeight;

	// Jump z from HL2's 160Hu
	// 21Hu jump height
	// 510ms jump time
	JumpZVelocity = 304.8f;

	// Don't bounce off characters
	JumpOffJumpZFactor = 0.0f;

	// Default show pos to false
	bShowPos = false;

	// We aren't on a ladder at first
	bOnLadder = false;
	OffLadderTicks = LADDER_MOUNT_TIMEOUT;

	// Speed multiplier bounds
	SpeedMultMin = SprintSpeed * 1.7f;
	SpeedMultMax = SprintSpeed * 2.5f;

	// Start out braking
	bBrakingWindowElapsed = true;
	BrakingWindowTimeElapsed = 0.f;
	BrakingWindow = 15.f;

	// Crouching
	SetCrouchedHalfHeight(55.f);
	MaxWalkSpeedCrouched = RunSpeed * 0.33333333f;
	bCanWalkOffLedgesWhenCrouching = true;

	// Slope angle is 45.57 degrees
	SetWalkableFloorZ(0.7f);
	DefaultWalkableFloorZ = GetWalkableFloorZ();
	AxisSpeedLimit = 6667.5f;

	// Tune physics interactions
	StandingDownwardForceScale = 1.0f;

	// Reasonable values polled from NASA (https://msis.jsc.nasa.gov/sections/section04.htm#Figure%204.9.3-6)
	// and Standard Handbook of Machine Design
	InitialPushForceFactor = 100.0f;
	PushForceFactor = 500.0f;

	// Let's not do any weird stuff...Gordon isn't a trampoline
	RepulsionForce = 0.0f;
	MaxTouchForce = 0.0f;
	TouchForceFactor = 0.0f;

	// Just push all objects based on their impact point
	// it might be weird with a lot of dev objects due to scale, but
	// it's much more realistic.
	bPushForceUsingZOffset = false;
	PushForcePointZOffsetFactor = -0.66f;

	// Scale push force down if we are slow
	bScalePushForceToVelocity = true;

	// Don't push more if there's more mass
	bPushForceScaledToMass = false;
	bTouchForceScaledToMass = false;
	Mass = 85.0f; // player.mdl is 85kg

	// Don't smooth rotation at all
	bUseControllerDesiredRotation = false;

	// Flat base
	bUseFlatBaseForFloorChecks = true;

	// Agent props
	NavAgentProps.bCanCrouch = true;
	NavAgentProps.bCanJump = true;
	NavAgentProps.bCanFly = true;

	// Make sure gravity is correct for player movement
	GravityScale = DesiredGravity / UPhysicsSettings::Get()->DefaultGravityZ;

	// Make sure ramp movement in correct
	bMaintainHorizontalGroundVelocity = true;
}

void UBSCharacterMovementComponent::SetSprintSpeedMultiplier(float NewSpringSpeedMultiplier)
{
	SprintSpeedMultiplier = NewSpringSpeedMultiplier;
}

float UBSCharacterMovementComponent::GetMaxSpeed() const
{
	if (bCheatFlying)
	{
		return (BSCharacter->IsSprinting() ? SprintSpeed : WalkSpeed) * 1.5f;
	}
	float Speed;
	if (BSCharacter->IsSprinting())
	{
		if (IsCrouching() && bCrouchFrameTolerated)
		{
			Speed = MaxWalkSpeedCrouched * 1.7f;
		}
		else
		{
			Speed = SprintSpeed;
		}
	}
	else if (BSCharacter->DoesWantToWalk())
	{
		Speed = WalkSpeed;
	}
	else if (IsCrouching() && bCrouchFrameTolerated)
	{
		Speed = MaxWalkSpeedCrouched;
	}
	else
	{
		Speed = RunSpeed;
	}

	return Speed;
}

const FCharacterGroundInfo& UBSCharacterMovementComponent::GetGroundInfo()
{
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
	{
		return CachedGroundInfo;
	}

	if (MovementMode == MOVE_Walking)
	{
		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (UpdatedComponent
			? UpdatedComponent->GetCollisionObjectType()
			: ECC_Pawn);
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, TraceStart.Z - GroundTraceDistance - CapsuleHalfHeight);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(CharacterOwner);
		QueryParams.bReturnPhysicalMaterial = true;

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = GroundTraceDistance;

		if (MovementMode == MOVE_NavWalking)
		{
			CachedGroundInfo.GroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;
	return CachedGroundInfo;
}

float UBSCharacterMovementComponent::GetFrictionFromHit(const FHitResult& Hit) const
{
	float SurfaceFriction = 1.0f;
	if (Hit.PhysMaterial.IsValid())
	{
		SurfaceFriction = FMath::Min(1.0f, Hit.PhysMaterial->Friction * 1.25f);
	}
	return SurfaceFriction;
}

void UBSCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	BSCharacter = Cast<ABSCharacterBase>(GetCharacterOwner());
}

void UBSCharacterMovementComponent::OnRegister()
{
	Super::OnRegister();

	const bool bIsReplay = (GetWorld() && GetWorld()->IsPlayingReplay());
	if (!bIsReplay && GetNetMode() == NM_ListenServer)
	{
		NetworkSmoothingMode = ENetworkSmoothingMode::Linear;
	}
}

void UBSCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bHasDeferredMovementMode)
	{
		bHasDeferredMovementMode = false;
		SetMovementMode(DeferredMovementMode);
	}

	// Skip player movement when we're simulating physics (ie ragdoll)
	if (UpdatedComponent->IsSimulatingPhysics())
	{
		return;
	}

	if (bShowPos || CVarShowPos.GetValueOnGameThread() != 0)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Green,
			FString::Printf(TEXT("pos: %s"), *UpdatedComponent->GetComponentLocation().ToCompactString()));
		GEngine->AddOnScreenDebugMessage(2, 1.0f, FColor::Green,
			FString::Printf(TEXT("ang: %s"), *CharacterOwner->GetControlRotation().ToCompactString()));
		GEngine->AddOnScreenDebugMessage(3, 1.0f, FColor::Green, FString::Printf(TEXT("vel: %f"), Velocity.Size()));
	}

	if (RollAngle != 0 && RollSpeed != 0 && GetController())
	{
		FRotator ControlRotation = GetController()->GetControlRotation();
		ControlRotation.Roll = GetCameraRoll();
		GetController()->SetControlRotation(ControlRotation);
	}

	if (IsMovingOnGround())
	{
		if (!bBrakingWindowElapsed)
		{
			BrakingWindowTimeElapsed += DeltaTime * 1000;
		}

		if (BrakingWindowTimeElapsed >= BrakingWindow)
		{
			bBrakingWindowElapsed = true;
			BrakingWindowTimeElapsed = 0;
		}
	}
	else
	{
		bBrakingWindowElapsed = false; // don't brake in the air lol
		BrakingWindowTimeElapsed = 0;
		// make sure this is cleared so the window doesn't shrink on subsequent bhops until it expires.
	}

	bCrouchFrameTolerated = IsCrouching();
}

void UBSCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid,
	float BrakingDeceleration)
{
	// Do not update velocity when using root motion or when SimulatedProxy and not simulating root motion;
	// SimulatedProxy are repped their Velocity
	if (!HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME || (CharacterOwner && CharacterOwner->
		GetLocalRole() == ROLE_SimulatedProxy && !bWasSimulatingRootMotion))
	{
		return;
	}

	Friction = FMath::Max(0.0f, Friction);
	const float MaxAccel = GetMaxAcceleration();
	float MaxSpeed = GetMaxSpeed();

	if (bForceMaxAccel)
	{
		// Force acceleration at full speed.
		// In consideration order for direction: Acceleration, then Velocity, then Pawn's rotation.
		if (Acceleration.SizeSquared() > SMALL_NUMBER)
		{
			Acceleration = Acceleration.GetSafeNormal() * MaxAccel;
		}
		else
		{
			Acceleration = MaxAccel * (Velocity.SizeSquared() < SMALL_NUMBER
				? UpdatedComponent->GetForwardVector()
				: Velocity.GetSafeNormal());
		}

		AnalogInputModifier = 1.0f;
	}

	MaxSpeed = FMath::Max(MaxSpeed * AnalogInputModifier, GetMinAnalogSpeed());

	// Apply braking or deceleration
	const bool bZeroAcceleration = Acceleration.IsNearlyZero();
	const bool bIsGroundMove = IsMovingOnGround() && bBrakingWindowElapsed;

	// Apply friction
	if (bIsGroundMove)
	{
		const bool bVelocityOverMax = IsExceedingMaxSpeed(MaxSpeed);
		const FVector OldVelocity = Velocity;

		const float ActualBrakingFriction = (bUseSeparateBrakingFriction ? BrakingFriction : Friction) *
			LocalSurfaceFriction;

		// Apply quicker stopping when on ground
		if (bZeroAcceleration)
		{
			ApplyVelocityBraking(DeltaTime, GroundBrakingDeceleration, BrakingDeceleration);
		}
		else
		{
			ApplyVelocityBraking(DeltaTime, ActualBrakingFriction, BrakingDeceleration);
		}


		// Don't allow braking to lower us below max speed if we started above it.
		if (bVelocityOverMax && Velocity.SizeSquared() < FMath::Square(MaxSpeed) && FVector::DotProduct(Acceleration,
			OldVelocity) > 0.0f)
		{
			Velocity = OldVelocity.GetSafeNormal() * MaxSpeed;
		}
	}

	// Apply fluid friction
	if (bFluid)
	{
		Velocity = Velocity * (1.0f - FMath::Min(Friction * DeltaTime, 1.0f));
	}

	// Limit before
	Velocity.X = FMath::Clamp(Velocity.X, -AxisSpeedLimit, AxisSpeedLimit);
	Velocity.Y = FMath::Clamp(Velocity.Y, -AxisSpeedLimit, AxisSpeedLimit);

	// no clip
	if (bCheatFlying)
	{
		if (bZeroAcceleration)
		{
			Velocity = FVector(0.0f);
		}
		else
		{
			const auto LookVec = CharacterOwner->GetControlRotation().Vector();
			auto LookVec2D = CharacterOwner->GetActorForwardVector();
			LookVec2D.Z = 0.0f;
			const auto PerpendicularAccel = (LookVec2D | Acceleration) * LookVec2D;
			const auto TangentialAccel = Acceleration - PerpendicularAccel;
			const auto UnitAcceleration = Acceleration;
			const auto Dir = UnitAcceleration.CosineAngle2D(LookVec);
			const auto NoClipAccelClamp = BSCharacter->IsSprinting() ? 2.0f * MaxAcceleration : MaxAcceleration;
			Velocity = (Dir * LookVec * PerpendicularAccel.Size2D() + TangentialAccel).GetClampedToSize(
				NoClipAccelClamp, NoClipAccelClamp);
		}
	}
	// ladder movement
	else if (bOnLadder)
	{
	}
	// walk move
	else
	{
		// Apply input acceleration
		if (!bZeroAcceleration)
		{
			// Clamp acceleration to max speed
			Acceleration = Acceleration.GetClampedToMaxSize2D(MaxSpeed);
			// Find veer
			const FVector AccelDir = Acceleration.GetSafeNormal2D();
			const float Veer = Velocity.X * AccelDir.X + Velocity.Y * AccelDir.Y;
			// Get add speed with air speed cap
			const float AddSpeed = (bIsGroundMove ? Acceleration : Acceleration.GetClampedToMaxSize2D(AirSpeedCap)).
				Size2D() - Veer;
			if (AddSpeed > 0.0f)
			{
				// Apply acceleration
				const float AccelerationMultiplier = bIsGroundMove
					? GroundAccelerationMultiplier
					: AirAccelerationMultiplier;
				FVector CurrentAcceleration = Acceleration * AccelerationMultiplier * LocalSurfaceFriction * DeltaTime;
				CurrentAcceleration = CurrentAcceleration.GetClampedToMaxSize2D(AddSpeed);
				Velocity += CurrentAcceleration;
			}
		}
	}

	// Limit after
	Velocity.X = FMath::Clamp(Velocity.X, -AxisSpeedLimit, AxisSpeedLimit);
	Velocity.Y = FMath::Clamp(Velocity.Y, -AxisSpeedLimit, AxisSpeedLimit);

	const float SpeedSq = Velocity.SizeSquared2D();

	// Dynamic step height code for allowing sliding on a slope when at a high speed
	if (bOnLadder || SpeedSq <= MaxWalkSpeedCrouched * MaxWalkSpeedCrouched)
	{
		// If we're crouching or not sliding, just use max
		MaxStepHeight = DefaultStepHeight;
		SetWalkableFloorZ(DefaultWalkableFloorZ);
	}
	else
	{
		// Scale step/ramp height down the faster we go
		float Speed = FMath::Sqrt(SpeedSq);
		float SpeedScale = (Speed - SpeedMultMin) / (SpeedMultMax - SpeedMultMin);
		float SpeedMultiplier = FMath::Clamp(SpeedScale, 0.0f, 1.0f);
		SpeedMultiplier *= SpeedMultiplier;
		if (!IsFalling())
		{
			// If we're on ground, factor in friction.
			SpeedMultiplier = FMath::Max((1.0f - LocalSurfaceFriction) * SpeedMultiplier, 0.0f);
		}
		MaxStepHeight = FMath::Lerp(DefaultStepHeight, MinStepHeight, SpeedMultiplier);
		SetWalkableFloorZ(FMath::Lerp(DefaultWalkableFloorZ, 0.9848f, SpeedMultiplier));
	}
}

void UBSCharacterMovementComponent::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	if (Velocity.IsNearlyZero(0.1f) || !HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	const float FrictionFactor = FMath::Max(0.f, BrakingFrictionFactor);
	Friction = FMath::Max(0.f, Friction * FrictionFactor);
	BrakingDeceleration = FMath::Max(0.f, BrakingDeceleration);
	const bool bZeroFriction = (Friction == 0.f);
	const bool bZeroBraking = (BrakingDeceleration == 0.f);

	if (bZeroFriction && bZeroBraking)
	{
		return;
	}

	const FVector OldVel = Velocity;

	// subdivide braking to get reasonably consistent results at lower frame rates
	// (important for packet loss situations w/ networking)
	float RemainingTime = DeltaTime;
	const float MaxTimeStep = FMath::Clamp(BrakingSubStepTime, 1.0f / 75.0f, 1.0f / 20.0f);

	// Decelerate to brake to a stop
	const FVector RevAccel = (bZeroBraking ? FVector::ZeroVector : (-BrakingDeceleration * Velocity.GetSafeNormal()));
	while (RemainingTime >= MIN_TICK_TIME)
	{
		// Zero friction uses constant deceleration, so no need for iteration.
		const float dt = ((RemainingTime > MaxTimeStep && !bZeroFriction)
			? FMath::Min(MaxTimeStep, RemainingTime * 0.5f)
			: RemainingTime);
		RemainingTime -= dt;

		// apply friction and braking
		Velocity = Velocity + ((-Friction) * Velocity + RevAccel) * dt;

		// Don't reverse direction
		if ((Velocity | OldVel) <= 0.f)
		{
			Velocity = FVector::ZeroVector;
			return;
		}
	}

	// Clamp to zero if nearly zero, or if below min threshold and braking.
	const float VSizeSq = Velocity.SizeSquared();
	if (VSizeSq <= UE_KINDA_SMALL_NUMBER || (!bZeroBraking && VSizeSq <= FMath::Square(BRAKE_TO_STOP_VELOCITY)))
	{
		Velocity = FVector::ZeroVector;
	}
}

void UBSCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	SCOPE_CYCLE_COUNTER(STAT_CharPhysFalling);
	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(CharPhysFalling);

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	FVector FallAcceleration = GetFallingLateralAcceleration(deltaTime);
	FallAcceleration.Z = 0.f;
	const bool bHasLimitedAirControl = ShouldLimitAirControl(deltaTime, FallAcceleration);

	float remainingTime = deltaTime;
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
	{
		Iterations++;
		float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
		bJustTeleported = false;

		const FVector OldVelocityWithRootMotion = Velocity;

		RestorePreAdditiveRootMotionVelocity();

		const FVector OldVelocity = Velocity;

		// Apply input
		const float MaxDecel = GetMaxBrakingDeceleration();
		if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			// Compute Velocity
			{
				// Acceleration = FallAcceleration for CalcVelocity(), but we restore it after using it.
				TGuardValue<FVector> RestoreAcceleration(Acceleration, FallAcceleration);
				Velocity.Z = 0.f;
				CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
				Velocity.Z = OldVelocity.Z;
			}
		}

		// Compute current gravity
		const FVector Gravity(0.f, 0.f, GetGravityZ());
		float GravityTime = timeTick;

		// If jump is providing force, gravity may be affected.
		bool bEndingJumpForce = false;
		if (CharacterOwner->JumpForceTimeRemaining > 0.0f)
		{
			// Consume some of the force time. Only the remaining time (if any) is affected by gravity when
			// bApplyGravityWhileJumping=false.
			const float JumpForceTime = FMath::Min(CharacterOwner->JumpForceTimeRemaining, timeTick);
			GravityTime = bApplyGravityWhileJumping ? timeTick : FMath::Max(0.0f, timeTick - JumpForceTime);

			// Update Character state
			CharacterOwner->JumpForceTimeRemaining -= JumpForceTime;
			if (CharacterOwner->JumpForceTimeRemaining <= 0.0f)
			{
				CharacterOwner->ResetJumpState();
				bEndingJumpForce = true;
			}
		}

		// Apply gravity
		Velocity = NewFallVelocity(Velocity, Gravity, GravityTime);

		// See if we need to sub-step to exactly reach the apex. This is important for avoiding "cutting off the top"
		// of the trajectory as framerate varies.
		if (OldVelocity.Z > 0.f && Velocity.Z <= 0.f && NumJumpApexAttempts < MaxJumpApexAttemptsPerSimulation)
		{
			const FVector DerivedAccel = (Velocity - OldVelocity) / timeTick;
			if (!FMath::IsNearlyZero(DerivedAccel.Z))
			{
				const float TimeToApex = -OldVelocity.Z / DerivedAccel.Z;

				// The time-to-apex calculation should be precise, and we want to avoid adding a substep when we
				// are basically already at the apex from the previous iteration's work.
				const float ApexTimeMinimum = 0.0001f;
				if (TimeToApex >= ApexTimeMinimum && TimeToApex < timeTick)
				{
					const FVector ApexVelocity = OldVelocity + DerivedAccel * TimeToApex;
					Velocity = ApexVelocity;
					Velocity.Z = 0.f; // Should be nearly zero anyway, but this makes apex notifications consistent.

					// We only want to move the amount of time it takes to reach the apex, and refund the unused
					// time for next iteration.
					remainingTime += (timeTick - TimeToApex);
					timeTick = TimeToApex;
					Iterations--;
					NumJumpApexAttempts++;
				}
			}
		}

		// UE_LOG(LogCharacterMovement, Log, TEXT("dt=(%.6f) OldLocation=(%s) OldVelocity=(%s)
		// OldVelocityWithRootMotion=(%s) NewVelocity=(%s)"), timeTick,
		// *(UpdatedComponent->GetComponentLocation()).ToString(), *OldVelocity.ToString(),
		// *OldVelocityWithRootMotion.ToString(), *Velocity.ToString());
		ApplyRootMotionToVelocity(timeTick);

		if (bNotifyApex && (Velocity.Z < 0.f))
		{
			// Just passed jump apex since now going down
			bNotifyApex = false;
			NotifyJumpApex();
		}

		// Compute change in position (using midpoint integration method).
		FVector Adjusted = 0.5f * (OldVelocityWithRootMotion + Velocity) * timeTick;

		// Special handling if ending the jump force where we didn't apply gravity during the jump.
		if (bEndingJumpForce && !bApplyGravityWhileJumping)
		{
			// We had a portion of the time at constant speed then a portion with acceleration due to gravity.
			// Account for that here with a more correct change in position.
			const float NonGravityTime = FMath::Max(0.f, timeTick - GravityTime);
			Adjusted = (OldVelocityWithRootMotion * NonGravityTime) + (0.5f * (OldVelocityWithRootMotion + Velocity) *
				GravityTime);
		}

		// Move
		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Adjusted, PawnRotation, true, Hit);

		if (!HasValidData())
		{
			return;
		}

		float LastMoveTimeSlice = timeTick;
		float subTimeTickRemaining = timeTick * (1.f - Hit.Time);

		if (IsSwimming()) //just entered water
		{
			remainingTime += subTimeTickRemaining;
			StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
			return;
		}
		if (Hit.bBlockingHit)
		{
			if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
			{
				remainingTime += subTimeTickRemaining;
				ProcessLanded(Hit, remainingTime, Iterations);
				return;
			}
			// Compute impact deflection based on final velocity, not integration step.
			// This allows us to compute a new velocity from the deflected vector, and ensures the full gravity effect is included in the slide result.
			// UNDONE: NOPE NOPE NOPE, that's not how positional integration steps work!!!
			//Adjusted = Velocity * timeTick;

			// See if we can convert a normally invalid landing spot (based on the hit result) to a usable one.
			if (!Hit.bStartPenetrating && ShouldCheckForValidLandingSpot(timeTick, Adjusted, Hit))
			{
				const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
				FFindFloorResult FloorResult;
				FindFloor(PawnLocation, FloorResult, false);
				if (FloorResult.IsWalkableFloor() && IsValidLandingSpot(PawnLocation, FloorResult.HitResult))
				{
					remainingTime += subTimeTickRemaining;
					ProcessLanded(FloorResult.HitResult, remainingTime, Iterations);
					return;
				}
			}

			HandleImpact(Hit, LastMoveTimeSlice, Adjusted);

			// If we've changed physics mode, abort.
			if (!HasValidData() || !IsFalling())
			{
				return;
			}

			// Limit air control based on what we hit.
			// We moved to the impact point using air control, but may want to deflect from there based on a limited air control acceleration.
			FVector VelocityNoAirControl = OldVelocity;
			FVector AirControlAccel = Acceleration;
			if (bHasLimitedAirControl)
			{
				// Compute VelocityNoAirControl
				{
					// Find velocity *without* acceleration.
					TGuardValue<FVector> RestoreAcceleration(Acceleration, FVector::ZeroVector);
					TGuardValue<FVector> RestoreVelocity(Velocity, OldVelocity);
					Velocity.Z = 0.f;
					CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
					VelocityNoAirControl = FVector(Velocity.X, Velocity.Y, OldVelocity.Z);
					VelocityNoAirControl = NewFallVelocity(VelocityNoAirControl, Gravity, GravityTime);
				}

				constexpr bool bCheckLandingSpot = false; // we already checked above.
				AirControlAccel = (Velocity - VelocityNoAirControl) / timeTick;
				const FVector AirControlDeltaV = LimitAirControl(LastMoveTimeSlice, AirControlAccel, Hit,
					bCheckLandingSpot) * LastMoveTimeSlice;
				Adjusted = (VelocityNoAirControl + AirControlDeltaV) * LastMoveTimeSlice;
			}

			const FVector OldHitNormal = Hit.Normal;
			const FVector OldHitImpactNormal = Hit.ImpactNormal;
			FVector Delta = ComputeSlideVector(Adjusted, 1.f - Hit.Time, OldHitNormal, Hit);
			// TODO: Maybe there's a better way of integrating this?
			FVector DeltaStep = ComputeSlideVector(Velocity * timeTick, 1.f - Hit.Time, OldHitNormal, Hit);

			// Compute velocity after deflection (only gravity component for RootMotion)
			if (subTimeTickRemaining > KINDA_SMALL_NUMBER && !bJustTeleported)
			{
				const FVector NewVelocity = (DeltaStep / subTimeTickRemaining);
				Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate()
					? FVector(Velocity.X, Velocity.Y, NewVelocity.Z)
					: NewVelocity;
			}

			if (subTimeTickRemaining > KINDA_SMALL_NUMBER && (Delta | Adjusted) > 0.f)
			{
				// Move in deflected direction.
				SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);

				if (Hit.bBlockingHit)
				{
					// hit second wall
					LastMoveTimeSlice = subTimeTickRemaining;
					subTimeTickRemaining = subTimeTickRemaining * (1.f - Hit.Time);

					if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
					{
						remainingTime += subTimeTickRemaining;
						ProcessLanded(Hit, remainingTime, Iterations);
						return;
					}

					HandleImpact(Hit, LastMoveTimeSlice, Delta);

					// If we've changed physics mode, abort.
					if (!HasValidData() || !IsFalling())
					{
						return;
					}

					// Act as if there was no air control on the last move when computing new deflection.
					if (bHasLimitedAirControl && Hit.Normal.Z > VERTICAL_SLOPE_NORMAL_Z)
					{
						const FVector LastMoveNoAirControl = VelocityNoAirControl * LastMoveTimeSlice;
						Delta = ComputeSlideVector(LastMoveNoAirControl, 1.f, OldHitNormal, Hit);
					}

					TwoWallAdjust(Delta, Hit, OldHitNormal);

					// Limit air control, but allow a slide along the second wall.
					if (bHasLimitedAirControl)
					{
						constexpr bool bCheckLandingSpot = false; // we already checked above.
						const FVector AirControlDeltaV = LimitAirControl(subTimeTickRemaining, AirControlAccel, Hit,
							bCheckLandingSpot) * subTimeTickRemaining;

						// Only allow if not back in to first wall
						if (FVector::DotProduct(AirControlDeltaV, OldHitNormal) > 0.f)
						{
							Delta += (AirControlDeltaV * subTimeTickRemaining);
						}
					}

					// Compute velocity after deflection (only gravity component for RootMotion)
					if (subTimeTickRemaining > KINDA_SMALL_NUMBER && !bJustTeleported)
					{
						const FVector NewVelocity = (Delta / subTimeTickRemaining);
						Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate()
							? FVector(Velocity.X, Velocity.Y, NewVelocity.Z)
							: NewVelocity;
					}

					// bDitch=true means that pawn is straddling two slopes, neither of which he can stand on
					bool bDitch = ((OldHitImpactNormal.Z > 0.f) && (Hit.ImpactNormal.Z > 0.f) && (FMath::Abs(Delta.Z) <=
						KINDA_SMALL_NUMBER) && ((Hit.ImpactNormal | OldHitImpactNormal) < 0.f));
					SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
					if (Hit.Time == 0.f)
					{
						// if we are stuck then try to side step
						FVector SideDelta = (OldHitNormal + Hit.ImpactNormal).GetSafeNormal2D();
						if (SideDelta.IsNearlyZero())
						{
							SideDelta = FVector(OldHitNormal.Y, -OldHitNormal.X, 0).GetSafeNormal();
						}
						SafeMoveUpdatedComponent(SideDelta, PawnRotation, true, Hit);
					}

					if (bDitch || IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit) || Hit.Time == 0.f)
					{
						remainingTime = 0.f;
						ProcessLanded(Hit, remainingTime, Iterations);
						return;
					}
					if (GetPerchRadiusThreshold() > 0.f && Hit.Time == 1.f && OldHitImpactNormal.Z >=
						GetWalkableFloorZ())
					{
						// We might be in a virtual 'ditch' within our perch radius. This is rare.
						const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
						const float ZMovedDist = FMath::Abs(PawnLocation.Z - OldLocation.Z);
						const float MovedDist2DSq = (PawnLocation - OldLocation).SizeSquared2D();
						if (ZMovedDist <= 0.2f * timeTick && MovedDist2DSq <= 4.f * timeTick)
						{
							Velocity.X += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
							Velocity.Y += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
							Velocity.Z = FMath::Max<float>(JumpZVelocity * 0.25f, 1.f);
							Delta = Velocity * timeTick;
							SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
						}
					}
				}
			}
		}

		if (Velocity.SizeSquared2D() <= KINDA_SMALL_NUMBER * 10.f)
		{
			Velocity.X = 0.f;
			Velocity.Y = 0.f;
		}
	}
}

bool UBSCharacterMovementComponent::ShouldLimitAirControl(float DeltaTime, const FVector& FallAcceleration) const
{
	//return Super::ShouldLimitAirControl(DeltaTime, FallAcceleration);
	return false;
}

FVector UBSCharacterMovementComponent::NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity,
	float DeltaTime) const
{
	//return Super::NewFallVelocity(InitialVelocity, Gravity, DeltaTime);
	FVector FallVel = Super::NewFallVelocity(InitialVelocity, Gravity, DeltaTime);
	FallVel.Z = FMath::Clamp(FallVel.Z, -AxisSpeedLimit, AxisSpeedLimit);
	return FallVel;
}

void UBSCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
	Velocity.Z = FMath::Clamp(Velocity.Z, -AxisSpeedLimit, AxisSpeedLimit);
	UpdateCrouching(DeltaSeconds);
}

void UBSCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
	Velocity.Z = FMath::Clamp(Velocity.Z, -AxisSpeedLimit, AxisSpeedLimit);
	UpdateSurfaceFriction();
	UpdateCrouching(DeltaSeconds, true);
}

void UBSCharacterMovementComponent::UpdateSurfaceFriction(bool bIsSliding)
{
	if (!IsFalling() && CurrentFloor.IsWalkableFloor())
	{
		const FCharacterGroundInfo GroundInfo = GetGroundInfo();
		LocalSurfaceFriction = GetFrictionFromHit(GroundInfo.GroundHitResult);
	}
	else
	{
		const bool bPlayerControlsMovedVertically = bOnLadder || Velocity.Z > JumpVelocity || Velocity.Z <= 0.0f ||
			bCheatFlying;
		if (bPlayerControlsMovedVertically)
		{
			LocalSurfaceFriction = 1.0f;
		}
		else if (bIsSliding)
		{
			LocalSurfaceFriction = 0.25f;
		}
	}
}

void UBSCharacterMovementComponent::UpdateCrouching(float DeltaTime, bool bOnlyUncrouch)
{
	if (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}

	// Crouch transition but not in no-clip
	if (bIsInCrouchTransition && !bCheatFlying)
	{
		// If the player wants to uncrouch, or we have to uncrouch after movement
		if ((!bOnlyUncrouch && !bWantsToCrouch) || (bOnlyUncrouch && !CanCrouchInCurrentState()))
		{
			{
				if (IsWalking())
				{
					// Normal uncrouch
					DoUnCrouchResize(UnCrouchTime, DeltaTime);
				}
				else
				{
					// Uncrouch jump
					DoUnCrouchResize(UnCrouchJumpTime, DeltaTime);
				}
			}
		}
		else if (!bOnlyUncrouch)
		{
			if (bOnLadder) // if on a ladder, cancel this because bWantsToCrouch should be false
			{
				bIsInCrouchTransition = false;
				CurrentCrouchProgress = 0.f;
			}
			else
			{
				if (IsWalking())
				{
					DoCrouchResize(CrouchTime, DeltaTime);
				}
				else
				{
					DoCrouchResize(CrouchJumpTime, DeltaTime);
				}
			}
		}
	}
}

void UBSCharacterMovementComponent::Crouch(bool bClientSimulation)
{
	// TODO: replicate to the client simulation that we are in a crouch transition so they can do the resize too.
	if (bClientSimulation)
	{
		Super::Crouch(true);
		return;
	}
	bIsInCrouchTransition = true;
}

void UBSCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	// TODO: replicate to the client simulation that we are in a crouch transition so they can do the resize too.
	if (bClientSimulation)
	{
		Super::UnCrouch(true);
		return;
	}
	bIsInCrouchTransition = true;
}

void UBSCharacterMovementComponent::DoCrouchResize(float TargetTime, float DeltaTime, bool bClientSimulation)
{
	if (!HasValidData() || (!bClientSimulation && !CanCrouchInCurrentState()))
	{
		bIsInCrouchTransition = false;
		return;
	}

	// See if collision is already at desired size.
	UCapsuleComponent* CharacterCapsule = CharacterOwner->GetCapsuleComponent();
	if (FMath::IsNearlyEqual(CharacterCapsule->GetUnscaledCapsuleHalfHeight(), GetCrouchedHalfHeight()))
	{
		if (!bClientSimulation)
		{
			CharacterOwner->bIsCrouched = true;
		}
		CharacterOwner->OnStartCrouch(0.0f, 0.0f);
		bIsInCrouchTransition = false;
		CurrentCrouchProgress = 1.f;
		return;
	}

	const ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	if (bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		// restore collision size before crouching
		CharacterCapsule->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
			DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
		bShrinkProxyCapsule = true;
	}

	// Change collision size to crouching dimensions
	const float ComponentScale = CharacterCapsule->GetShapeScale();
	const float OldUnscaledHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterCapsule->GetUnscaledCapsuleRadius();
	const float FullCrouchDiff = OldUnscaledHalfHeight - GetCrouchedHalfHeight();
	const float CurrentUnscaledHalfHeight = CharacterCapsule->GetUnscaledCapsuleHalfHeight();
	// Determine the crouching progress
	const bool bInstantCrouch = FMath::IsNearlyZero(TargetTime);
	const float CurrentAlpha = 1.0f - (CurrentUnscaledHalfHeight - GetCrouchedHalfHeight()) / FullCrouchDiff;
	// Determine how much we are progressing this tick
	float TargetAlphaDiff = 1.0f;
	float TargetAlpha = 1.0f;
	if (!bInstantCrouch)
	{
		TargetAlphaDiff = DeltaTime / CrouchTime;
		TargetAlpha = CurrentAlpha + TargetAlphaDiff;
	}
	if (TargetAlpha >= 1.0f || FMath::IsNearlyEqual(TargetAlpha, 1.0f))
	{
		TargetAlpha = 1.0f;
		TargetAlphaDiff = TargetAlpha - CurrentAlpha;
		bIsInCrouchTransition = false;
		CharacterOwner->bIsCrouched = true;
	}

	CurrentCrouchProgress = TargetAlpha;

	// Determine the target height for this tick
	const float TargetCrouchedHalfHeight = OldUnscaledHalfHeight - FullCrouchDiff * TargetAlpha;
	// Height is not allowed to be smaller than radius.
	const float ClampedCrouchedHalfHeight = FMath::Max3(0.0f, OldUnscaledRadius, TargetCrouchedHalfHeight);
	CharacterCapsule->SetCapsuleSize(OldUnscaledRadius, ClampedCrouchedHalfHeight);
	const float HalfHeightAdjust = FullCrouchDiff * TargetAlphaDiff;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	if (!bClientSimulation)
	{
		if (bCrouchMaintainsBaseLocation)
		{
			// Intentionally not using MoveUpdatedComponent, where a horizontal
			// plane constraint would prevent the base of the capsule from
			// staying at the same spot.
			UpdatedComponent->MoveComponent(FVector(0.0f, 0.0f, -ScaledHalfHeightAdjust),
				UpdatedComponent->GetComponentQuat(), true, nullptr, MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
		}
		else
		{
			UpdatedComponent->MoveComponent(FVector(0.0f, 0.0f, ScaledHalfHeightAdjust),
				UpdatedComponent->GetComponentQuat(), true, nullptr, MOVECOMP_NoFlags, ETeleportType::None);
		}
	}

	bForceNextFloorCheck = true;

	const float MeshAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() -
		ClampedCrouchedHalfHeight;
	AdjustProxyCapsuleSize();
	CharacterOwner->OnStartCrouch(MeshAdjust, MeshAdjust * ComponentScale);

	// Don't smooth this change in mesh position
	if ((bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) &&
		CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset -= FVector(0.0f, 0.0f, ScaledHalfHeightAdjust);
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

void UBSCharacterMovementComponent::DoUnCrouchResize(float TargetTime, float DeltaTime, bool bClientSimulation)
{
	// UE4-COPY: void UCharacterMovementComponent::UnCrouch(bool bClientSimulation)

	if (!HasValidData())
	{
		bIsInCrouchTransition = false;
		return;
	}

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	UCapsuleComponent* CharacterCapsule = CharacterOwner->GetCapsuleComponent();

	// See if collision is already at desired size.
	if (FMath::IsNearlyEqual(CharacterCapsule->GetUnscaledCapsuleHalfHeight(),
		DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()))
	{
		if (!bClientSimulation)
		{
			CharacterOwner->bIsCrouched = false;
		}
		CharacterOwner->OnEndCrouch(0.0f, 0.0f);
		bCrouchFrameTolerated = false;
		bIsInCrouchTransition = false;
		CurrentCrouchProgress = 0.f;
		return;
	}

	const float CurrentCrouchedHalfHeight = CharacterCapsule->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterCapsule->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterCapsule->GetUnscaledCapsuleHalfHeight();
	const float UnCrouchedHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float FullCrouchDiff = UnCrouchedHeight - GetCrouchedHalfHeight();
	// Determine the crouching progress
	const bool InstantCrouch = FMath::IsNearlyZero(TargetTime);
	float CurrentAlpha = 1.0f - (UnCrouchedHeight - OldUnscaledHalfHeight) / FullCrouchDiff;
	float TargetAlphaDiff = 1.0f;
	float TargetAlpha = 1.0f;
	const UWorld* MyWorld = GetWorld();
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
	if (!InstantCrouch)
	{
		TargetAlphaDiff = DeltaTime / TargetTime;
		TargetAlpha = CurrentAlpha + TargetAlphaDiff;
		// Don't partial uncrouch in tight places (like vents)
		if (bCrouchMaintainsBaseLocation)
		{
			// Try to stay in place and see if the larger capsule fits. We use a
			// slightly taller capsule to avoid penetration.
			constexpr float SweepInflation = KINDA_SMALL_NUMBER * 10.0f;
			FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
			FCollisionResponseParams ResponseParam;
			InitCollisionParams(CapsuleParams, ResponseParam);

			// Check how much we have left to go (with some wiggle room to still allow for partial uncrouches in some areas)
			const float HalfHeightAdjust = ComponentScale * (UnCrouchedHeight - OldUnscaledHalfHeight) *
				GroundUnCrouchCheckFactor;

			// Compensate for the difference between current capsule size and standing size
			// Shrink by negative amount, so actually grow it.
			const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom,
				-SweepInflation - HalfHeightAdjust);
			const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
			FVector StandingLocation = PawnLocation + FVector(0.0f, 0.0f,
				StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight);
			bool bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity,
				CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
			if (bEncroached)
			{
				// We're blocked from doing a full uncrouch, so don't attempt for now
				return;
			}
		}
	}
	if (TargetAlpha >= 1.0f || FMath::IsNearlyEqual(TargetAlpha, 1.0f))
	{
		TargetAlpha = 1.0f;
		TargetAlphaDiff = TargetAlpha - CurrentAlpha;
		bIsInCrouchTransition = false;
	}

	CurrentCrouchProgress = 1 - TargetAlpha;

	const float HalfHeightAdjust = FullCrouchDiff * TargetAlphaDiff;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	// Grow to un-crouched size.
	check(CharacterCapsule);

	if (!bClientSimulation)
	{
		// Try to stay in place and see if the larger capsule fits. We use a
		// slightly taller capsule to avoid penetration.
		constexpr float SweepInflation = KINDA_SMALL_NUMBER * 10.0f;
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);

		// Compensate for the difference between current capsule size and
		// standing size
		// Shrink by negative amount, so actually grow it.
		const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom,
			-SweepInflation - ScaledHalfHeightAdjust);
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
		bool bEncroached = true;

		if (!bCrouchMaintainsBaseLocation)
		{
			// Expand in place
			bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel,
				StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				// Try adjusting capsule position to see if we can avoid
				// encroachment.
				if (ScaledHalfHeightAdjust > 0.0f)
				{
					// Shrink to a short capsule, sweep down to base to find
					// where that would hit something, and then try to stand up
					// from there.
					float PawnRadius, PawnHalfHeight;
					CharacterCapsule->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
					const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
					const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
					// const FVector Down = FVector(0.0f, 0.0f, -TraceDist);

					FHitResult Hit(1.0f);
					const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom,
						ShrinkHalfHeight);
					// const bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, FQuat::Identity, CollisionChannel,
					// ShortCapsuleShape, CapsuleParams);

					if (!Hit.bStartPenetrating)
					{
						// Compute where the base of the sweep ended up, and see
						// if we can stand there
						const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
						const FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y,
							PawnLocation.Z - DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight + SweepInflation +
							MIN_FLOOR_DIST / 2.0f);
						bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, FQuat::Identity, CollisionChannel,
							StandingCapsuleShape, CapsuleParams, ResponseParam);
						if (!bEncroached)
						{
							// Intentionally not using MoveUpdatedComponent,
							// where a horizontal plane constraint would prevent
							// the base of the capsule from staying at the same
							// spot.
							UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(),
								false, nullptr, MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
						}
					}
				}
			}
		}
		else
		{
			// Expand while keeping base location the same.
			FVector StandingLocation = PawnLocation + FVector(0.0f, 0.0f,
				StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight);
			bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel,
				StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				if (IsMovingOnGround())
				{
					// Something might be just barely overhead, try moving down
					// closer to the floor to avoid it.
					constexpr float MinFloorDist = KINDA_SMALL_NUMBER * 10.0f;
					if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
					{
						StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
						bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity,
							CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
					}
				}
			}

			if (!bEncroached)
			{
				// Commit the change in location.
				UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(),
					false, nullptr, MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
				bForceNextFloorCheck = true;
			}
		}

		// If still encroached then abort.
		if (bEncroached)
		{
			return;
		}

		CharacterOwner->bIsCrouched = false;
	}
	else
	{
		bShrinkProxyCapsule = true;
	}

	const float NewHalfHeight = OldUnscaledHalfHeight + HalfHeightAdjust;

	// Now call SetCapsuleSize() to cause touch/un-touch events and actually grow the capsule
	CharacterCapsule->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), NewHalfHeight,
		true);

	// OnEndCrouch takes the change from the Default size, not the current one
	const float MeshAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - NewHalfHeight;

	AdjustProxyCapsuleSize();

	CharacterOwner->OnEndCrouch(MeshAdjust, MeshAdjust * ComponentScale);
	bCrouchFrameTolerated = false;

	// Don't smooth this change in mesh position
	if ((bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) &&
		CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset += FVector(0.0f, 0.0f, MeshAdjust);
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

bool UBSCharacterMovementComponent::MoveUpdatedComponentImpl(const FVector& Delta, const FQuat& NewRotation,
	bool bSweep, FHitResult* OutHit, ETeleportType Teleport)
{
	FVector NewDelta = Delta;
	if (bSweep && Teleport == ETeleportType::None && Delta != FVector::ZeroVector && IsFalling() && Delta.Z > 0.0f)
	{
		const float HorizontalMovement = Delta.SizeSquared2D();
		if (HorizontalMovement > KINDA_SMALL_NUMBER)
		{
			float PawnRadius, PawnHalfHeight;
			CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
			FVector LineTraceStart = UpdatedComponent->GetComponentLocation();
			// Shrink our base height so we don't intersect any current floor, and find where we would end up if we moved
			LineTraceStart.Z += -PawnHalfHeight + MAX_FLOOR_DIST + Delta.Z;
			// Inflate our search radius so we can anticipate new surfaces
			FVector DeltaDir = Delta.GetSafeNormal2D() * (PawnRadius + SWEEP_EDGE_REJECT_DISTANCE);
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CapsuleHemisphereTrace), false, CharacterOwner);
			FCollisionResponseParams ResponseParam;
			InitCollisionParams(QueryParams, ResponseParam);
			const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
			FHitResult Hit(1.f);
			const bool bBlockingHit = GetWorld()->LineTraceSingleByChannel(Hit, LineTraceStart,
				LineTraceStart + DeltaDir, CollisionChannel, QueryParams, ResponseParam);
			if (bBlockingHit && FMath::Abs(Hit.ImpactNormal.Z) <= VERTICAL_SLOPE_NORMAL_Z)
			{
				// DrawDebugLine(GetWorld(), LineTraceStart, LineTraceStart + DeltaDir, FColor::Red, false, 10.0f, 0, 0.5f);
				// UE_LOG(LogTemp, Log, TEXT("%f"), Hit.ImpactNormal.Z);
				//  Blocked horizontally by box
				NewDelta = Super::ComputeSlideVector(Delta, 1.0f, Hit.ImpactNormal, Hit);
			}
		}
	}

	return Super::MoveUpdatedComponentImpl(NewDelta, NewRotation, bSweep, OutHit, Teleport);
}

bool UBSCharacterMovementComponent::CanAttemptJump() const
{
	bool bCanAttemptJump = IsJumpAllowed();
	if (IsMovingOnGround())
	{
		const float FloorZ = FVector(0.0f, 0.0f, 1.0f) | CurrentFloor.HitResult.ImpactNormal;
		const float WalkableFloor = GetWalkableFloorZ();
		bCanAttemptJump &= (FloorZ >= WalkableFloor) || FMath::IsNearlyEqual(FloorZ, WalkableFloor);
	}
	else if (!IsFalling())
	{
		bCanAttemptJump &= bOnLadder;
	}
	return bCanAttemptJump;
}

bool UBSCharacterMovementComponent::DoJump(bool bClientSimulation)
{
	//return Super::DoJump(bClientSimulation);

	// UE-COPY: UCharacterMovementComponent::DoJump(bool bReplayingMoves)

	if (!bCheatFlying && CharacterOwner && CharacterOwner->CanJump())
	{
		// Don't jump if we can't move up/down.
		if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
		{
			if (Velocity.Z <= 0.0f)
			{
				Velocity.Z = JumpZVelocity;
			}
			else
			{
				Velocity.Z += JumpZVelocity;
			}
			SetMovementMode(MOVE_Falling);
			return true;
		}
	}

	return false;
}

FVector UBSCharacterMovementComponent::HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta,
	const float Time, const FVector& Normal, const FHitResult& Hit) const
{
	//return Super::HandleSlopeBoosting(SlideResult, Delta, Time, Normal, Hit);
	if (bOnLadder || bCheatFlying)
	{
		return Super::HandleSlopeBoosting(SlideResult, Delta, Time, Normal, Hit);
	}
	const float WallAngle = FMath::Abs(Hit.ImpactNormal.Z);
	FVector ImpactNormal;
	// If too extreme, use the more stable hit normal
	if (WallAngle <= VERTICAL_SLOPE_NORMAL_Z || WallAngle == 1.0f)
	{
		ImpactNormal = Normal;
	}
	else
	{
		ImpactNormal = Hit.ImpactNormal;
	}
	if (bConstrainToPlane)
	{
		ImpactNormal = ConstrainNormalToPlane(ImpactNormal);
	}
	const float BounceCoefficient = 1.0f + BounceMultiplier * (1.0f - LocalSurfaceFriction);
	return (Delta - BounceCoefficient * Delta.ProjectOnToNormal(ImpactNormal)) * Time;
}

bool UBSCharacterMovementComponent::ShouldCatchAir(const FFindFloorResult& OldFloor, const FFindFloorResult& NewFloor)
{
	// Get surface friction
	const float OldSurfaceFriction = GetFrictionFromHit(OldFloor.HitResult);

	// As we get faster, make our speed multiplier smaller (so it scales with smaller friction)
	const float SpeedMult = SpeedMultMax / Velocity.Size2D();
	const bool bSliding = OldSurfaceFriction * SpeedMult < 0.5f;

	// See if we got less steep or are continuing at the same slope
	const float ZDiff = NewFloor.HitResult.ImpactNormal.Z - OldFloor.HitResult.ImpactNormal.Z;
	const bool bGainingRamp = ZDiff >= 0.0f;

	// Velocity is always horizontal. Therefore, if we are moving up a ramp, we get >90 deg angle with the normal
	// This results in a negative cos. This also checks if our old floor was ramped at all, because a flat floor wouldn't pass this check.
	const float Slope = Velocity | OldFloor.HitResult.ImpactNormal;
	const bool bWasGoingUpRamp = Slope < 0.0f;

	// Finally, we want to also handle the case of strafing off of a ramp, so check if they're strafing.
	const float StrafeMovement = FMath::Abs(GetLastInputVector() | GetOwner()->GetActorRightVector());
	const bool bStrafingOffRamp = StrafeMovement > 0.0f;

	// So, our only relevant conditions are when we are going up a ramp or strafing off of it.
	const bool bMovingForCatchAir = bWasGoingUpRamp || bStrafingOffRamp;

	if (bSliding && bGainingRamp && bMovingForCatchAir)
	{
		return true;
	}
	return Super::ShouldCatchAir(OldFloor, NewFloor);
}

bool UBSCharacterMovementComponent::IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const
{
	//return Super::IsValidLandingSpot(CapsuleLocation, Hit);

	if (!Hit.bBlockingHit)
	{
		return false;
	}
	// Skip some checks if penetrating. Penetration will be handled by the FindFloor call (using a smaller capsule)
	if (!Hit.bStartPenetrating)
	{
		// Reject unwalkable floor normals.
		if (!IsWalkable(Hit))
		{
			return false;
		}

		float PawnRadius, PawnHalfHeight;
		CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);

		// Reject hits that are above our lower hemisphere (can happen when sliding down a vertical surface).
		if (bUseFlatBaseForFloorChecks)
		{
			// Reject hits that are above our box
			const float LowerHemisphereZ = Hit.Location.Z - PawnHalfHeight + MAX_FLOOR_DIST;
			if ((Hit.ImpactNormal.Z < GetWalkableFloorZ() || Hit.ImpactNormal.Z == 1.0f) && Hit.ImpactPoint.Z >
				LowerHemisphereZ)
			{
				return false;
			}
		}
		else
		{
			// Reject hits that are above our lower hemisphere (can happen when sliding down a vertical surface).
			const float LowerHemisphereZ = Hit.Location.Z - PawnHalfHeight + PawnRadius;
			if (Hit.ImpactPoint.Z >= LowerHemisphereZ)
			{
				return false;
			}
		}

		// Reject hits that are barely on the cusp of the radius of the capsule
		if (!IsWithinEdgeTolerance(Hit.Location, Hit.ImpactPoint, PawnRadius))
		{
			return false;
		}
	}
	else
	{
		// Penetrating
		if (Hit.Normal.Z < KINDA_SMALL_NUMBER)
		{
			// Normal is nearly horizontal or downward, that's a penetration adjustment next to a vertical or overhanging wall. Don't pop to the floor.
			return false;
		}
	}
	FFindFloorResult FloorResult;
	FindFloor(CapsuleLocation, FloorResult, false, &Hit);
	if (!FloorResult.IsWalkableFloor())
	{
		return false;
	}
	// Slope bug fix
	// If moving up a slope...
	if (Hit.Normal.Z < 1.0f && (Velocity | Hit.Normal) < 0.0f)
	{
		// Let's calculate how we are gonna deflect off the surface
		FVector DeflectionVector = Velocity;
		// a step of gravity
		DeflectionVector.Z += 0.5f * GetGravityZ() * GetWorld()->GetDeltaSeconds();
		DeflectionVector = ComputeSlideVector(DeflectionVector, 1.0f, Hit.Normal, Hit);

		// going up too fast to land
		if (DeflectionVector.Z > JumpVelocity)
		{
			return false;
		}
	}
	return true;
}

bool UBSCharacterMovementComponent::ShouldCheckForValidLandingSpot(float DeltaTime, const FVector& Delta,
	const FHitResult& Hit) const
{
	// TODO: check for flat base valid landing spots? at the moment this check is too generous for the capsule hemisphere
	return !bUseFlatBaseForFloorChecks && Super::ShouldCheckForValidLandingSpot(DeltaTime, Delta, Hit);
}

void UBSCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	// Reset step side if we are changing modes
	StepSide = false;

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

float UBSCharacterMovementComponent::GetCameraRoll()
{
	if (RollSpeed == 0.0f || RollAngle == 0.0f)
	{
		return 0.0f;
	}
	float Side = Velocity | FRotationMatrix(GetCharacterOwner()->GetControlRotation()).GetScaledAxis(EAxis::Y);
	const float Sign = FMath::Sign(Side);
	Side = FMath::Abs(Side);
	if (Side < RollSpeed)
	{
		Side = Side * RollAngle / RollSpeed;
	}
	else
	{
		Side = RollAngle;
	}
	return Side * Sign;
}

void UBSCharacterMovementComponent::SetNoClip(bool bNoClip)
{
	// We need to defer movement in case we set this outside of main game thread loop, since character movement resets movement back in tick.
	if (bNoClip)
	{
		SetMovementMode(MOVE_Flying);
		DeferredMovementMode = MOVE_Flying;
		bCheatFlying = true;
		GetCharacterOwner()->SetActorEnableCollision(false);
	}
	else
	{
		SetMovementMode(MOVE_Walking);
		DeferredMovementMode = MOVE_Walking;
		bCheatFlying = false;
		GetCharacterOwner()->SetActorEnableCollision(true);
	}
	bHasDeferredMovementMode = true;
}

void UBSCharacterMovementComponent::ToggleNoClip()
{
	SetNoClip(!bCheatFlying);
}

void UBSCharacterMovementComponent::PlayMovementSound_Implementation(const FName Bone, const FGameplayTag MotionEffect,
	USceneComponent* StaticMeshComponent, const FVector LocationOffset, const FRotator RotationOffset,
	const UAnimSequenceBase* AnimationSequence, const FHitResult HitResult, FGameplayTagContainer Context,
	float AudioVolume, float AudioPitch)
{
	if (HitResult.PhysMaterial.IsValid())
	{
		TEnumAsByte<EPhysicalSurface> SurfaceType = HitResult.PhysMaterial->SurfaceType;
		if (const UBSAudioSettings* AudioSettings = GetDefault<UBSAudioSettings>())
		{
			if (const FGameplayTag* SurfaceContextPtr = AudioSettings->SurfaceTypeToGameplayTagMap.Find(SurfaceType))
			{
				FGameplayTag SurfaceContext = *SurfaceContextPtr;
				Context.AddTag(SurfaceContext);
			}
		}
		if (MovementSounds)
		{
			TArray<TObjectPtr<USoundBase>> Sounds;
			MovementSounds->GetFootstepSounds(MotionEffect, Context, Sounds);
			for (const TObjectPtr<USoundBase>& Sound : Sounds)
			{
				UGameplayStatics::SpawnSoundAttached(Sound.Get(), StaticMeshComponent, Bone, LocationOffset,
					RotationOffset, EAttachLocation::KeepRelativeOffset, false, AudioVolume, AudioPitch, 0.0f, nullptr,
					nullptr, true);
			}
		}
	}
}
