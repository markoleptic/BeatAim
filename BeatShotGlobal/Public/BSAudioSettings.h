// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "GameplayTagContainer.h"
#include "BSAudioSettings.generated.h"

class UObject;
class USoundEffectSubmixPreset;
class USoundSubmix;
enum EPhysicalSurface : int;

USTRUCT()
struct FBSSubmixEffectChainMap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (AllowedClasses = "/Script/Engine.SoundSubmix"))
	TSoftObjectPtr<USoundSubmix> Submix = nullptr;

	UPROPERTY(EditAnywhere, meta = (AllowedClasses = "/Script/Engine.SoundEffectSubmixPreset"))
	TArray<TSoftObjectPtr<USoundEffectSubmixPreset>> SubmixEffectChain;
};

/** Audio settings that are editable within the Unreal Editor. */
UCLASS(config = Game, defaultconfig, MinimalAPI, meta = (DisplayName = "BeatShot Audio"))
class UBSAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Maps a physical surface to the Gameplay Tag representation of the surface. */
	UPROPERTY(config, EditAnywhere, Category = PhysicalSurfaces)
	TMap<TEnumAsByte<EPhysicalSurface>, FGameplayTag> SurfaceTypeToGameplayTagMap;

	/** The Default Base Control Bus Mix */
	UPROPERTY(config, EditAnywhere, Category = "MixSettings",
		meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBusMix"))
	FSoftObjectPath DefaultControlBusMix;

	/** The Loading Screen Control Bus Mix - Called during loading screens to cover background audio events */
	UPROPERTY(config, EditAnywhere, Category = "MixSettings",
		meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBusMix"))
	FSoftObjectPath LoadingScreenControlBusMix;

	/** The Default Base Control Bus Mix */
	UPROPERTY(config, EditAnywhere, Category = "UserMixSettings",
		meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBusMix"))
	FSoftObjectPath UserSettingsControlBusMix;

	/** Control Bus assigned to the Overall sound volume setting */
	UPROPERTY(config, EditAnywhere, Category = "UserMixSettings",
		meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
	FSoftObjectPath OverallVolumeControlBus;

	/** Control Bus assigned to the Menu sound volume setting */
	UPROPERTY(config, EditAnywhere, Category = "UserMixSettings",
		meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
	FSoftObjectPath MenuVolumeControlBus;

	/** Control Bus assigned to the Music sound volume setting */
	UPROPERTY(config, EditAnywhere, Category = "UserMixSettings",
		meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
	FSoftObjectPath MusicVolumeControlBus;

	/** Control Bus assigned to the SoundFX sound volume setting */
	UPROPERTY(config, EditAnywhere, Category = "UserMixSettings",
		meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
	FSoftObjectPath SoundFXVolumeControlBus;

	/** Submix Processing Chains to achieve high dynamic range audio output */
	UPROPERTY(config, EditAnywhere, Category = "EffectSettings")
	TArray<FBSSubmixEffectChainMap> HDRAudioSubmixEffectChain;

	/** Submix Processing Chains to achieve low dynamic range audio output */
	UPROPERTY(config, EditAnywhere, Category = "EffectSettings")
	TArray<FBSSubmixEffectChainMap> LDRAudioSubmixEffectChain;
};
