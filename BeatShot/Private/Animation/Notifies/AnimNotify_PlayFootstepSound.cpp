// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Animation/Notifies/AnimNotify_PlayFootstepSound.h"
#include "Audio/BSMovementSoundInterface.h"

UAnimNotify_PlayFootstepSound::UAnimNotify_PlayFootstepSound(): bAttached(1), bPerformTrace(0)
{
}

FString UAnimNotify_PlayFootstepSound::GetNotifyName_Implementation() const
{
	// If the Effect Tag is valid, pass the string name to the notify name
	if (Effect.IsValid())
	{
		return Effect.ToString();
	}

	return Super::GetNotifyName_Implementation();
}

void UAnimNotify_PlayFootstepSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		// Make sure both MeshComp and Owning Actor is valid
		if (AActor* OwningActor = MeshComp->GetOwner())
		{
			// Prepare Trace Data
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;

			if (TraceProperties.bIgnoreActor)
			{
				QueryParams.AddIgnoredActor(OwningActor);
			}

			QueryParams.bReturnPhysicalMaterial = true;

			if (bPerformTrace)
			{
				// If trace is needed, set up Start Location to Attached
				FVector TraceStart = bAttached
					? MeshComp->GetSocketLocation(SocketName)
					: MeshComp->GetComponentLocation();

				if (UWorld* World = OwningActor->GetWorld())
				{
					World->LineTraceSingleByChannel(HitResult, TraceStart,
						(TraceStart + TraceProperties.EndTraceLocationOffset), TraceProperties.TraceChannel,
						QueryParams, FCollisionResponseParams::DefaultResponseParam);
				}
			}

			// Prepare Contexts in advance
			FGameplayTagContainer Context;

			// Set up Array of Objects that implement the Context Effects Interface
			TArray<UObject*> ImplementingObjects;

			// Determine if the Owning Actor is one of the Objects that implements the Context Effects Interface
			if (OwningActor->Implements<UBSMovementSoundInterface>())
			{
				// If so, add it to the Array
				ImplementingObjects.Add(OwningActor);
			}

			for (const auto Component : OwningActor->GetComponents())
			{
				if (Component && Component->Implements<UBSMovementSoundInterface>())
				{
					ImplementingObjects.Add(Component);
				}
			}

			// Cycle through all objects implementing the Context Effect Interface
			for (UObject* ImplementingObject : ImplementingObjects)
			{
				// If the object is still valid, Execute the AnimMotionEffect Event on it, passing in relevant data
				IBSMovementSoundInterface::Execute_PlayMovementSound(ImplementingObject,
					bAttached ? SocketName : FName("None"), Effect, MeshComp, LocationOffset, RotationOffset, Animation,
					HitResult, Context, AudioProperties.VolumeMultiplier, AudioProperties.PitchMultiplier);
			}
		}
	}
}

#if WITH_EDITOR
void UAnimNotify_PlayFootstepSound::SetParameters(const FGameplayTag EffectIn, const FVector LocationOffsetIn,
	const FRotator RotationOffsetIn, const FBSAnimNotifySoundSettings AudioPropertiesIn, const bool bAttachedIn,
	const FName SocketNameIn, const bool bPerformTraceIn, const FBSAnimNotifyTraceSettings TracePropertiesIn)
{
	Effect = EffectIn;
	LocationOffset = LocationOffsetIn;
	RotationOffset = RotationOffsetIn;
	AudioProperties.PitchMultiplier = AudioPropertiesIn.PitchMultiplier;
	AudioProperties.VolumeMultiplier = AudioPropertiesIn.VolumeMultiplier;
	bAttached = bAttachedIn;
	SocketName = SocketNameIn;
	bPerformTrace = bPerformTraceIn;
	TraceProperties.EndTraceLocationOffset = TracePropertiesIn.EndTraceLocationOffset;
	TraceProperties.TraceChannel = TracePropertiesIn.TraceChannel;
	TraceProperties.bIgnoreActor = TracePropertiesIn.bIgnoreActor;
}
#endif
