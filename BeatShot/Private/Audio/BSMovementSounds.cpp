// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Audio/BSMovementSounds.h"
#include "BSAudioSettings.h"

void UBSMovementSounds::PostLoad()
{
	UObject::PostLoad();

	if (!FootstepSounds.IsEmpty()) return;

	const UBSAudioSettings* AudioSettings = GetDefault<UBSAudioSettings>();
	if (AudioSettings)
	{
		for (const auto& Pair : AudioSettings->SurfaceTypeToGameplayTagMap)
		{
			FootstepSounds.Add(FBSFootstepSoundKey(FGameplayTag(), Pair.Value), {});
		}
	}
}

void UBSMovementSounds::GetFootstepSounds(const FGameplayTag Effect, const FGameplayTagContainer& Context,
	TArray<TObjectPtr<USoundBase>>& Sounds) const
{
	if (Effect.IsValid() && Context.IsValid())
	{
		for (const auto& FootstepSound : FootstepSounds)
		{
			if (Effect.MatchesTagExact(FootstepSound.Key.EffectTag) && Context.HasAllExact(FootstepSound.Key.Context) &&
				Context.IsEmpty() == FootstepSound.Key.Context.IsEmpty())
			{
				Sounds.Append(FootstepSound.Value.Sounds);
			}
		}
	}
}
