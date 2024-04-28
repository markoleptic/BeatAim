﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "BSGameUserSettings.h"

#include "AudioModulationStatics.h"
#include "BSAudioSettings.h"
#include "BSSettingTypes.h"
#include "GlobalConstants.h"
#include "SaveGamePlayerSettings.h"
#include "SoundControlBusMix.h"
#include "VideoSettingEnumTagMap.h"
#include "DLSSLibrary.h"
#include "NISLibrary.h"
#include "StreamlineLibraryDLSSG.h"
#include "StreamlineLibraryReflex.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogBSGameUserSettings);

ENUM_RANGE_BY_FIRST_AND_LAST(UDLSSSupport, UDLSSSupport::Supported,
	UDLSSSupport::NotSupportedIncompatibleAPICaptureToolActive);

ENUM_RANGE_BY_FIRST_AND_LAST(UDLSSMode, UDLSSMode::Off, UDLSSMode::UltraPerformance);

ENUM_RANGE_BY_FIRST_AND_LAST(UNISMode, UNISMode::Off, UNISMode::Custom);

ENUM_RANGE_BY_FIRST_AND_LAST(UStreamlineReflexMode, UStreamlineReflexMode::Disabled,
	UStreamlineReflexMode::EnabledPlusBoost);

ENUM_RANGE_BY_FIRST_AND_LAST(UStreamlineDLSSGMode, UStreamlineDLSSGMode::Off, UStreamlineDLSSGMode::On);

ENUM_RANGE_BY_FIRST_AND_LAST(EWindowMode::Type, EWindowMode::Type::Fullscreen, EWindowMode::Type::WindowedFullscreen);


namespace
{
	/** Attempts to load a control bus from a soft object path. */
	USoundControlBus* TryLoadControlBus(const FSoftObjectPath& Path, TMap<FName, TObjectPtr<USoundControlBus>>& Map,
		const FName& Key)
	{
		if (UObject* ObjPath = Path.TryLoad())
		{
			if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
			{
				Map.Add(Key, SoundControlBus);
				return SoundControlBus;
			}
			else
			{
				ensureMsgf(SoundControlBus, TEXT("Control Bus reference missing from Audio Settings."));
			}
		}
		else
		{
			ensureMsgf(ObjPath, TEXT("Failed to load Control Bus."));
		}
		return nullptr;
	}

	/** Applies the value of DisplayGamma to the game engine. */
	void ApplyDisplayGamma(const float DisplayGamma)
	{
		if (GEngine)
		{
			GEngine->DisplayGamma = DisplayGamma;
		}
	}

	/** Applies the DLSS Mode and sets the screen percentage CVar. */
	bool ApplyDLSSMode(const UDLSSMode DLSSMode, const FIntPoint& ScreenRes,
		const bool bRestoreFullResWhenDisabled = true)
	{
		bool bShouldEnable = false;
		float ScreenPercentage = 100.f;

		if (UDLSSLibrary::IsDLSSSupported())
		{
			bool bIsSupported;
			float OptimalScreenPercentage;
			bool bIsFixedScreenPercentage;
			float MinScreenPercentage;
			float MaxScreenPercentage;
			float OptimalSharpness;

			UDLSSLibrary::GetDLSSModeInformation(DLSSMode, FVector2D(ScreenRes.X, ScreenRes.Y), bIsSupported,
				OptimalScreenPercentage, bIsFixedScreenPercentage, MinScreenPercentage, MaxScreenPercentage,
				OptimalSharpness);

			bIsSupported = bIsSupported || DLSSMode == UDLSSMode::Auto;
			const bool bIsDLAA = DLSSMode == UDLSSMode::DLAA;
			bShouldEnable = (DLSSMode != UDLSSMode::Off || bIsDLAA) && bIsSupported;
			const bool bValidScreenPercentage = OptimalScreenPercentage > 0.f && bIsSupported;

			// Enable/Disable DLSS
			UDLSSLibrary::EnableDLSS(bShouldEnable);

			// Set screen percentage to 100 if DLAA mode or invalid screen percentage
			ScreenPercentage = bIsDLAA || !bValidScreenPercentage ? 100.f : OptimalScreenPercentage;
		}

		if (bShouldEnable || bRestoreFullResWhenDisabled)
		{
			if (static IConsoleVariable* CVarScreenPercentage = IConsoleManager::Get().FindConsoleVariable(
				TEXT("r.ScreenPercentage")))
			{
				CVarScreenPercentage->Set(ScreenPercentage);
			}
		}

		return bShouldEnable;
	}
}

UBSGameUserSettings::UBSGameUserSettings()
{
	SetToBSDefaults();
	VideoSettingEnumMap = GetDefault<UVideoSettingEnumTagMap>();
}

UBSGameUserSettings* UBSGameUserSettings::Get()
{
	return GEngine ? CastChecked<UBSGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

void UBSGameUserSettings::InitIfNecessary()
{
	LoadDLSSSettings();
	LoadUserControlBusMix();

	if (GEngine)
	{
		if (const UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			FOnAudioOutputDevicesObtained OnAudioOutputDevicesObtained;
			OnAudioOutputDevicesObtained.BindDynamic(this, &UBSGameUserSettings::HandleAudioOutputDevicesObtained);
			UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(World, OnAudioOutputDevicesObtained);

			FOnMainAudioOutputDeviceObtained OnMainAudioOutputDeviceObtained;
			OnMainAudioOutputDeviceObtained.
				BindDynamic(this, &UBSGameUserSettings::HandleMainAudioOutputDeviceObtained);
			UAudioMixerBlueprintLibrary::GetCurrentAudioOutputDeviceName(World, OnMainAudioOutputDeviceObtained);
		}
	}
}

void UBSGameUserSettings::SetToBSDefaults()
{
	OverallVolume = Constants::DefaultGlobalVolume;
	MenuVolume = Constants::DefaultMenuVolume;
	MusicVolume = Constants::DefaultMusicVolume;
	SoundFXVolume = Constants::DefaultSoundFXVolume;
	Brightness = Constants::DefaultBrightness;
	DisplayGamma = Constants::DefaultDisplayGamma;
	bShowFPSCounter = false;
	DLSSSharpness = 0.0;
	NISSharpness = 0.0;
	FrameRateLimitMenu = Constants::DefaultFrameRateLimitMenu;
	FrameRateLimitGame = Constants::DefaultFrameRateLimitGame;
	FrameRateLimitBackground = Constants::DefaultFrameRateLimitBackground;
	DLSSEnabledMode = EDLSSEnabledMode::On;
	NISEnabledMode = ENISEnabledMode::Off;
	FrameGenerationEnabledMode = UStreamlineDLSSGMode::On;
	DLSSMode = UDLSSMode::Auto;
	NISMode = UNISMode::Off;
	StreamlineReflexMode = UStreamlineReflexMode::Enabled;
	AntiAliasingMethod = AAM_TSR;
	// bSoundControlBusMixLoaded = false;
	// VideoSettingEnumMap = nullptr;
	bDLSSInitialized = false;
}

bool UBSGameUserSettings::IsBSVersionValid() const
{
	return Version == Constants::BSGameUserSettingsVersion;
}

void UBSGameUserSettings::UpdateBSVersion()
{
	BSVersion = Constants::BSGameUserSettingsVersion;
}

void UBSGameUserSettings::LoadDLSSSettings()
{
	if (!FModuleManager::Get().IsModuleLoaded(FName("DLSS")))
	{
		UE_LOG(LogBSGameUserSettings, Warning, TEXT("DLSS not loaded"));
		return;
	}
	if (!FModuleManager::Get().IsModuleLoaded(FName("DLSSBlueprint")))
	{
		UE_LOG(LogBSGameUserSettings, Warning, TEXT("DLSSBlueprint not loaded"));
		return;
	}
	if (!FModuleManager::Get().IsModuleLoaded(FName("NISBlueprint")))
	{
		UE_LOG(LogBSGameUserSettings, Warning, TEXT("NISBlueprint not loaded"));
		return;
	}
	if (!FModuleManager::Get().IsModuleLoaded(FName("StreamlineCore")))
	{
		UE_LOG(LogBSGameUserSettings, Warning, TEXT("StreamlineCore not loaded"));
		return;
	}
	if (!FModuleManager::Get().IsModuleLoaded(FName("StreamlineCore")))
	{
		UE_LOG(LogBSGameUserSettings, Warning, TEXT("StreamlineCore not loaded"));
		return;
	}
	if (!FModuleManager::Get().IsModuleLoaded(FName("StreamlineBlueprint")))
	{
		UE_LOG(LogBSGameUserSettings, Warning, TEXT("StreamlineBlueprint not loaded"));
		return;
	}

	bool bDisableDLSS = true;

	// DLSS
	if (UDLSSLibrary::IsDLSSSupported())
	{
		UDLSSMode LocalDLSSMode = DLSSMode;

		// Fallback to auto if mode is invalid for whatever reason
		if (!UDLSSLibrary::IsDLSSModeSupported(LocalDLSSMode) && LocalDLSSMode != UDLSSMode::Auto)
		{
			LocalDLSSMode = UDLSSMode::Auto;
		}

		if (UDLSSLibrary::IsDLSSModeSupported(LocalDLSSMode) || LocalDLSSMode == UDLSSMode::Auto)
		{
			// Try to apply the DLSS Mode
			if (ApplyDLSSMode(LocalDLSSMode, GetScreenResolution()))
			{
				DLSSEnabledMode = EDLSSEnabledMode::On;
				DLSSMode = LocalDLSSMode;
				bDisableDLSS = false;
			}
		}
	}

	if (bDisableDLSS)
	{
		DLSSEnabledMode = EDLSSEnabledMode::Off;
		DLSSMode = UDLSSMode::Off;
	}

	// Frame Generation
	if (UStreamlineLibraryDLSSG::IsDLSSGSupported() && UStreamlineLibraryDLSSG::IsDLSSGModeSupported(
		FrameGenerationEnabledMode))
	{
		UStreamlineLibraryDLSSG::SetDLSSGMode(FrameGenerationEnabledMode);
	}
	else
	{
		UStreamlineLibraryDLSSG::SetDLSSGMode(UStreamlineDLSSGMode::Off);
		FrameGenerationEnabledMode = UStreamlineDLSSGMode::Off;
	}

	// NIS
	if (UNISLibrary::IsNISSupported() && UNISLibrary::IsNISModeSupported(NISMode) && DLSSEnabledMode ==
		EDLSSEnabledMode::Off)
	{
		UNISLibrary::SetNISMode(NISMode);
		UNISLibrary::SetNISSharpness(NISSharpness);
	}
	else
	{
		UNISLibrary::SetNISMode(UNISMode::Off);
		NISMode = UNISMode::Off;
	}

	// Reflex
	if (UStreamlineLibraryReflex::IsReflexSupported())
	{
		UStreamlineLibraryReflex::SetReflexMode(StreamlineReflexMode);
	}
	else
	{
		UStreamlineLibraryReflex::SetReflexMode(UStreamlineReflexMode::Disabled);
		StreamlineReflexMode = UStreamlineReflexMode::Disabled;
	}

	UE_LOG(LogBSGameUserSettings, Warning, TEXT("DLSS Initialized"));
	bDLSSInitialized = true;
}

void UBSGameUserSettings::LoadUserControlBusMix()
{
	if (GEngine)
	{
		if (const UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			if (const UBSAudioSettings* BSAudioSettings = GetDefault<UBSAudioSettings>())
			{
				ControlBusMap.Empty();

				USoundControlBus* OverallControlBus = TryLoadControlBus(BSAudioSettings->OverallVolumeControlBus,
					ControlBusMap, TEXT("Overall"));
				USoundControlBus* MenuControlBus = TryLoadControlBus(BSAudioSettings->MenuVolumeControlBus,
					ControlBusMap, TEXT("Menu"));
				USoundControlBus* MusicControlBus = TryLoadControlBus(BSAudioSettings->MusicVolumeControlBus,
					ControlBusMap, TEXT("Music"));
				USoundControlBus* SoundFXControlBus = TryLoadControlBus(BSAudioSettings->SoundFXVolumeControlBus,
					ControlBusMap, TEXT("SoundFX"));

				if (UObject* ObjPath = BSAudioSettings->UserSettingsControlBusMix.TryLoad())
				{
					if (USoundControlBusMix* SoundControlBusMix = Cast<USoundControlBusMix>(ObjPath))
					{
						ControlBusMix = SoundControlBusMix;

						UAudioModulationStatics::ActivateBusMix(World, SoundControlBusMix);

						const FSoundControlBusMixStage OverallControlBusMixStage =
							UAudioModulationStatics::CreateBusMixStage(World, OverallControlBus, OverallVolume / 100.0);
						const FSoundControlBusMixStage MenuControlBusMixStage =
							UAudioModulationStatics::CreateBusMixStage(World, MenuControlBus, MenuVolume / 100.0);
						const FSoundControlBusMixStage MusicControlBusMixStage =
							UAudioModulationStatics::CreateBusMixStage(World, MusicControlBus, MusicVolume / 100.0);
						const FSoundControlBusMixStage SoundFXControlBusMixStage =
							UAudioModulationStatics::CreateBusMixStage(World, SoundFXControlBus, SoundFXVolume / 100.0);

						TArray<FSoundControlBusMixStage> ControlBusMixStageArray;
						ControlBusMixStageArray.Add(OverallControlBusMixStage);
						ControlBusMixStageArray.Add(MenuControlBusMixStage);
						ControlBusMixStageArray.Add(MusicControlBusMixStage);
						ControlBusMixStageArray.Add(SoundFXControlBusMixStage);

						// TODO Fix UpdateMix not working

						UAudioModulationStatics::UpdateMix(World, SoundControlBusMix, ControlBusMixStageArray);

						bSoundControlBusMixLoaded = true;
					}
					else
					{
						UE_LOG(LogBSGameUserSettings, Warning, TEXT("User Settings Control Bus Mix reference missing"));
					}
				}
				else
				{
					ensureMsgf(ObjPath, TEXT("Failed to load Control Bus Mix."));
				}
			}
		}
	}
}

void UBSGameUserSettings::SetVolumeForControlBus(const FName& ControlBusKey, const float InVolume)
{
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	TObjectPtr<USoundControlBus> ControlBus = ControlBusMap.FindRef(ControlBusKey);

	if (GEngine && ControlBus && bSoundControlBusMixLoaded)
	{
		if (const UWorld* AudioWorld = GEngine->GetCurrentPlayWorld())
		{
			ensureMsgf(ControlBusMix, TEXT("Control Bus Mix failed to load."));

			// Create and set the Control Bus Mix Stage Parameters
			FSoundControlBusMixStage UpdatedControlBusMixStage;
			UpdatedControlBusMixStage.Bus = ControlBus;
			UpdatedControlBusMixStage.Value.TargetValue = InVolume / 100.0;
			UpdatedControlBusMixStage.Value.AttackTime = 0.01f;
			UpdatedControlBusMixStage.Value.ReleaseTime = 0.01f;

			// Add the Control Bus Mix Stage to an Array as the UpdateMix function requires
			TArray<FSoundControlBusMixStage> UpdatedMixStageArray;
			UpdatedMixStageArray.Add(UpdatedControlBusMixStage);

			// Modify the matching bus Mix Stage parameters on the User Control Bus Mix
			UAudioModulationStatics::UpdateMix(AudioWorld, ControlBusMix, UpdatedMixStageArray);
		}
	}
}

void UBSGameUserSettings::SetToDefaults()
{
	// Don't reset to default if only GameUserSettings is different version
	if (IsVersionValid())
	{
		SetToBSDefaults();
	}
	Super::SetToDefaults();
	UE_LOG(LogBSGameUserSettings, Warning, TEXT("Set to Defaults"));
}

void UBSGameUserSettings::LoadSettings(const bool bForceReload)
{
	Super::LoadSettings(bForceReload);
	LoadDLSSSettings();
	LoadUserControlBusMix();

	if (GEngine)
	{
		if (const UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			FOnAudioOutputDevicesObtained OnAudioOutputDevicesObtained;
			OnAudioOutputDevicesObtained.BindDynamic(this, &UBSGameUserSettings::HandleAudioOutputDevicesObtained);
			UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(World, OnAudioOutputDevicesObtained);

			FOnMainAudioOutputDeviceObtained OnMainAudioOutputDeviceObtained;
			OnMainAudioOutputDeviceObtained.
				BindDynamic(this, &UBSGameUserSettings::HandleMainAudioOutputDeviceObtained);
			UAudioMixerBlueprintLibrary::GetCurrentAudioOutputDeviceName(World, OnMainAudioOutputDeviceObtained);
		}
	}

	UE_LOG(LogBSGameUserSettings, Warning, TEXT("Load Settings"));
}

void UBSGameUserSettings::ValidateSettings()
{
	const bool bGameUserSettingsValid = IsVersionValid();
	const bool bBSGameUserSettingsValid = IsBSVersionValid();

	// If GameUserSettings is invalid, ini file will be deleted and reloaded
	if (bGameUserSettingsValid && !bBSGameUserSettingsValid)
	{
		// First try loading the settings, if they haven't been loaded before.
		LoadSettings(true);

		// If it still an old version, delete the user settings file and reload defaults.
		if (!IsVersionValid())
		{
			SetToBSDefaults();
			IFileManager::Get().Delete(*GGameUserSettingsIni);
			LoadSettings(true);
		}
		UpdateBSVersion();
	}

	DisplayGamma = FMath::Clamp(DisplayGamma, Constants::MinValue_DisplayGamma, Constants::MaxValue_DisplayGamma);
	Brightness = FMath::Clamp(Brightness, Constants::MinValue_Brightness, Constants::MaxValue_Brightness);

	Super::ValidateSettings();
	UE_LOG(LogBSGameUserSettings, Warning, TEXT("Validate Settings"));
}

void UBSGameUserSettings::ResetToCurrentSettings()
{
	Super::ResetToCurrentSettings();
}

void UBSGameUserSettings::ApplySettings(bool bForceReload)
{
	UE_LOG(LogBSGameUserSettings, Warning, TEXT("ApplySettings"));
	Super::ApplySettings(bForceReload);
	SetAntiAliasingMethod(GetAntiAliasingMethod());
	SetBrightness(GetBrightness());
	SetDisplayGamma(GetDisplayGamma());
	OnSettingsChanged.Broadcast(this);
	// TODO: Apply more settings
}

TMap<FString, uint8> UBSGameUserSettings::GetSupportedNvidiaSettingModes(
	const ENvidiaSettingType NvidiaSettingType) const
{
	TMap<FString, uint8> Out;
	switch (NvidiaSettingType)
	{
	case ENvidiaSettingType::DLSSEnabledMode:
		{
			TArray<EDLSSEnabledMode> Modes = UDLSSLibrary::IsDLSSSupported()
				? TArray{EDLSSEnabledMode::On, EDLSSEnabledMode::Off}
				: TArray{EDLSSEnabledMode::Off};
			Out = VideoSettingEnumMap->GetNvidiaSettingModes(Modes);
		}
		break;
	case ENvidiaSettingType::FrameGenerationEnabledMode:
		{
			TArray<UStreamlineDLSSGMode> Modes = UStreamlineLibraryDLSSG::GetSupportedDLSSGModes();
			Out = VideoSettingEnumMap->GetNvidiaSettingModes(Modes);
		}
		break;
	case ENvidiaSettingType::DLSSMode:
		{
			TArray<UDLSSMode> Modes = UDLSSLibrary::GetSupportedDLSSModes();
			Modes.AddUnique(UDLSSMode::Auto);
			Modes.AddUnique(UDLSSMode::Off);
			Out = VideoSettingEnumMap->GetNvidiaSettingModes(Modes);
		}
		break;
	case ENvidiaSettingType::NISEnabledMode:
		{
			TArray<ENISEnabledMode> Modes = UNISLibrary::IsNISSupported()
				? TArray{ENISEnabledMode::On, ENISEnabledMode::Off}
				: TArray{ENISEnabledMode::Off};
			Out = VideoSettingEnumMap->GetNvidiaSettingModes(Modes);
		}
		break;
	case ENvidiaSettingType::NISMode:
		{
			TArray<UNISMode> Modes = UNISLibrary::GetSupportedNISModes();
			Modes.Remove(UNISMode::Custom);
			Out = VideoSettingEnumMap->GetNvidiaSettingModes(Modes);
		}
		break;
	case ENvidiaSettingType::StreamlineReflexMode:
		{
			TArray<UStreamlineReflexMode> Modes = TArray{UStreamlineReflexMode::Disabled};
			if (UStreamlineLibraryReflex::IsReflexSupported())
			{
				Modes.Add(UStreamlineReflexMode::Enabled);
				Modes.Add(UStreamlineReflexMode::EnabledPlusBoost);
			}
			Out = VideoSettingEnumMap->GetNvidiaSettingModes(Modes);
		}
		break;
	}
	return Out;
}

float UBSGameUserSettings::GetPostProcessBiasFromBrightness() const
{
	return FMath::GetMappedRangeValueClamped(FVector2D(Constants::MinValue_Brightness, Constants::MaxValue_Brightness),
		FVector2D(Constants::MinValue_ExposureCompensation, Constants::MaxValue_ExposureCompensation), Brightness);
}

void UBSGameUserSettings::SetLoadingScreenMixActivationState(const bool bEnable)
{
	if (!LoadingScreenControlBusMix)
	{
		const UBSAudioSettings* AudioSettings = GetDefault<UBSAudioSettings>();
		if (UObject* ObjPath = AudioSettings->LoadingScreenControlBusMix.TryLoad())
		{
			if (USoundControlBusMix* SoundControlBusMix = Cast<USoundControlBusMix>(ObjPath))
			{
				LoadingScreenControlBusMix = SoundControlBusMix;
			}
		}
	}
	if (!LoadingScreenControlBusMix)
	{
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		if (bEnable)
		{
			const UBSAudioSettings* AudioSettings = GetDefault<UBSAudioSettings>();
			if (UObject* ObjPath = AudioSettings->LoadingScreenSound.TryLoad())
			{
				if (USoundBase* SoundBase = Cast<USoundBase>(ObjPath))
				{
					LoadingScreenAudioComponent = UGameplayStatics::CreateSound2D(World, SoundBase);
					LoadingScreenAudioComponent->Play();
				}
			}
			UAudioModulationStatics::ActivateBusMix(World, LoadingScreenControlBusMix);

			UE_LOG(LogTemp, Warning, TEXT("Movie Playback Started; Activating LoadingScreenMix"));
		}
		else
		{
			UAudioModulationStatics::DeactivateBusMix(World, LoadingScreenControlBusMix);
			LoadingScreenAudioComponent = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("Movie Playback Ended; Deactivating LoadingScreenMix"));
		}
	}
}

TArray<FString> UBSGameUserSettings::GetAvailableAudioDeviceNames() const
{
	return AudioDeviceNames;
}

FString UBSGameUserSettings::GetAudioOutputDeviceId() const
{
	return AudioOutputDeviceId;
}

float UBSGameUserSettings::GetOverallVolume() const
{
	return OverallVolume;
}

float UBSGameUserSettings::GetMenuVolume() const
{
	return MenuVolume;
}

float UBSGameUserSettings::GetMusicVolume() const
{
	return MusicVolume;
}

float UBSGameUserSettings::GetSoundFXVolume() const
{
	return SoundFXVolume;
}

uint8 UBSGameUserSettings::GetAntiAliasingMethod() const
{
	return AntiAliasingMethod.GetIntValue();
}

float UBSGameUserSettings::GetBrightness() const
{
	return Brightness;
}

float UBSGameUserSettings::GetDisplayGamma() const
{
	return DisplayGamma;
}

int32 UBSGameUserSettings::GetFrameRateLimitMenu() const
{
	return FrameRateLimitMenu;
}

int32 UBSGameUserSettings::GetFrameRateLimitGame() const
{
	return FrameRateLimitGame;
}

int32 UBSGameUserSettings::GetFrameRateLimitBackground() const
{
	return FrameRateLimitBackground;
}

bool UBSGameUserSettings::GetShowFPSCounter() const
{
	return bShowFPSCounter;
}

uint8 UBSGameUserSettings::GetDLSSEnabledMode() const
{
	return static_cast<uint8>(DLSSEnabledMode);
}

uint8 UBSGameUserSettings::GetNISEnabledMode() const
{
	return static_cast<uint8>(NISEnabledMode);
}

uint8 UBSGameUserSettings::GetFrameGenerationEnabledMode() const
{
	return static_cast<uint8>(FrameGenerationEnabledMode);
}

uint8 UBSGameUserSettings::GetDLSSMode() const
{
	return static_cast<uint8>(DLSSMode);
}

uint8 UBSGameUserSettings::GetNISMode() const
{
	return static_cast<uint8>(NISMode);
}

uint8 UBSGameUserSettings::GetStreamlineReflexMode() const
{
	return static_cast<uint8>(StreamlineReflexMode);
}

float UBSGameUserSettings::GetDLSSSharpness() const
{
	return DLSSSharpness;
}

float UBSGameUserSettings::GetNISSharpness() const
{
	return NISSharpness;
}

bool UBSGameUserSettings::IsDLSSEnabled()
{
	return UDLSSLibrary::IsDLSSEnabled();
}

bool UBSGameUserSettings::IsNISEnabled() const
{
	return NISEnabledMode == ENISEnabledMode::On;
}

void UBSGameUserSettings::SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId)
{
	AudioOutputDeviceId = InAudioOutputDeviceId;
	OnAudioOutputDeviceChanged.Broadcast(AudioOutputDeviceId);
}

void UBSGameUserSettings::SetOverallVolume(const float InVolume)
{
	OverallVolume = InVolume;
	SetVolumeForControlBus(TEXT("Overall"), OverallVolume);
}

void UBSGameUserSettings::SetMenuVolume(const float InVolume)
{
	MenuVolume = InVolume;
	SetVolumeForControlBus(TEXT("Menu"), MenuVolume);
}

void UBSGameUserSettings::SetMusicVolume(const float InVolume)
{
	MusicVolume = InVolume;
	SetVolumeForControlBus(TEXT("Music"), MusicVolume);
}

void UBSGameUserSettings::SetSoundFXVolume(const float InVolume)
{
	SoundFXVolume = InVolume;
	SetVolumeForControlBus(TEXT("SoundFX"), SoundFXVolume);
}

void UBSGameUserSettings::SetAntiAliasingMethod(const uint8 InAntiAliasingMethod)
{
	AntiAliasingMethod = TEnumAsByte<EAntiAliasingMethod>(InAntiAliasingMethod);
	if (IConsoleVariable* CVarAntiAliasingMethod = IConsoleManager::Get().FindConsoleVariable(
		TEXT("r.AntiAliasingMethod")))
	{
		CVarAntiAliasingMethod->Set(InAntiAliasingMethod, ECVF_SetByGameOverride);
	}
	if (GConfig)
	{
		const FString Value = FString::FromInt(InAntiAliasingMethod);
		GConfig->SetString(TEXT("/Script/Engine.RendererSettings"), TEXT("r.AntiAliasingMethod"), *Value, GEngineIni);
		GConfig->Flush(false, GEngineIni);
	}
}

void UBSGameUserSettings::SetBrightness(const float InBrightness)
{
	Brightness = FMath::Max(Constants::MinValue_Brightness, InBrightness);
	Brightness = FMath::Min(Constants::MaxValue_Brightness, Brightness);
	// TODO: Broadcast setting changed delegate
}

void UBSGameUserSettings::SetDisplayGamma(const float InGamma)
{
	DisplayGamma = FMath::Max(Constants::MinValue_DisplayGamma, InGamma);
	DisplayGamma = FMath::Min(Constants::MaxValue_DisplayGamma, DisplayGamma);
	ApplyDisplayGamma(DisplayGamma);
}

void UBSGameUserSettings::SetFrameRateLimitMenu(const int32 InFrameRateLimitMenu)
{
	FrameRateLimitMenu = InFrameRateLimitMenu;
}

void UBSGameUserSettings::SetFrameRateLimitGame(const int32 InFrameRateLimitGame)
{
	FrameRateLimitGame = InFrameRateLimitGame;
}

void UBSGameUserSettings::SetFrameRateLimitBackground(const int32 InFrameRateLimitBackground)
{
	FrameRateLimitBackground = InFrameRateLimitBackground;
}

void UBSGameUserSettings::SetResolutionScaleChecked(const float InResolutionScale)
{
	if (!UDLSSLibrary::IsDLSSEnabled() && NISEnabledMode != ENISEnabledMode::On)
	{
		SetResolutionScaleValueEx(InResolutionScale * 100.f);
	}
}

void UBSGameUserSettings::SetShowFPSCounter(const bool InShowFPSCounter)
{
	bShowFPSCounter = InShowFPSCounter;
}

void UBSGameUserSettings::SetDLSSEnabledMode(const uint8 InDLSSEnabledMode)
{
	DLSSEnabledMode = static_cast<EDLSSEnabledMode>(InDLSSEnabledMode);
}

void UBSGameUserSettings::SetNISEnabledMode(const uint8 InNISEnabledMode)
{
	if (UNISLibrary::IsNISSupported() && DLSSEnabledMode == EDLSSEnabledMode::Off)
	{
		NISEnabledMode = static_cast<ENISEnabledMode>(InNISEnabledMode);
	}
	else
	{
		NISEnabledMode = ENISEnabledMode::Off;
	}
}

void UBSGameUserSettings::SetFrameGenerationEnabledMode(const uint8 InFrameGenerationEnabledMode)
{
	const auto Mode = static_cast<UStreamlineDLSSGMode>(InFrameGenerationEnabledMode);
	if (DLSSMode != UDLSSMode::Off && UStreamlineLibraryDLSSG::IsDLSSGSupported() &&
		UStreamlineLibraryDLSSG::IsDLSSGModeSupported(Mode))
	{
		UStreamlineLibraryDLSSG::SetDLSSGMode(Mode);
		FrameGenerationEnabledMode = Mode;
	}
	else
	{
		FrameGenerationEnabledMode = UStreamlineDLSSGMode::Off;
	}
}

void UBSGameUserSettings::SetDLSSMode(const uint8 InDLSSMode)
{
	auto LocalDLSSMode = static_cast<UDLSSMode>(InDLSSMode);
	EDLSSEnabledMode LocalDLSSEnabledMode = DLSSEnabledMode;
	UStreamlineDLSSGMode LocalFrameGenerationEnabledMode = FrameGenerationEnabledMode;
	ENISEnabledMode LocalNISEnabledMode = NISEnabledMode;
	UNISMode LocalNISMode = NISMode;
	UStreamlineReflexMode LocalReflexMode = StreamlineReflexMode;

	if (LocalDLSSEnabledMode == EDLSSEnabledMode::On)
	{
		// Force disable NIS
		LocalNISEnabledMode = ENISEnabledMode::Off;
		// Force NIS off
		LocalNISMode = UNISMode::Off;
		// Force Reflex enabled
		LocalReflexMode = UStreamlineReflexMode::Enabled;
		// Enable Frame Generation if supported
		if (LocalFrameGenerationEnabledMode == UStreamlineDLSSGMode::Off && UStreamlineLibraryDLSSG::IsDLSSGSupported()
			&& UStreamlineLibraryDLSSG::IsDLSSGModeSupported(UStreamlineDLSSGMode::On))
		{
			LocalFrameGenerationEnabledMode = UStreamlineDLSSGMode::On;
		}
		// Enable Super Resolution if supported
		if (LocalDLSSMode == UDLSSMode::Off && UDLSSLibrary::IsDLSSSupported())
		{
			LocalDLSSMode = UDLSSMode::Auto;
		}
	}
	else
	{
		// Force disable Frame Generation
		LocalFrameGenerationEnabledMode = UStreamlineDLSSGMode::Off;
		// Force disable Super Resolution
		LocalDLSSMode = UDLSSMode::Off;
	}

	if (ApplyDLSSMode(LocalDLSSMode, GetScreenResolution()))
	{
		DLSSMode = LocalDLSSMode;
	}
	else
	{
		DLSSMode = UDLSSMode::Off;
	}

	SetFrameGenerationEnabledMode(static_cast<uint8>(LocalFrameGenerationEnabledMode));
	SetNISEnabledMode(static_cast<uint8>(LocalNISEnabledMode));
	SetNISMode(static_cast<uint8>(LocalNISMode));
	SetStreamlineReflexMode(static_cast<uint8>(LocalReflexMode));
	SetDLSSSharpness(DLSSSharpness);
	SetNISSharpness(NISSharpness);

	// V-Sync
	if (DLSSEnabledMode == EDLSSEnabledMode::On)
	{
		if (IsVSyncEnabled())
		{
			SetVSyncEnabled(false);
			// TODO: Apply Resolution Settings or non resolution settings?
		}
	}

	if (DLSSEnabledMode == EDLSSEnabledMode::On || NISEnabledMode == ENISEnabledMode::On)
	{
		SetResolutionScaleValueEx(100.f);
	}

	// TODO: Maybe apply resolution settings?
}

void UBSGameUserSettings::SetNISMode(const uint8 InNISMode)
{
	const auto Mode = static_cast<UNISMode>(InNISMode);
	if (UNISLibrary::IsNISSupported() && DLSSEnabledMode == EDLSSEnabledMode::Off && UNISLibrary::IsNISModeSupported(
		Mode))
	{
		UNISLibrary::SetNISMode(Mode);
		NISMode = Mode;
	}
	else
	{
		NISMode = UNISMode::Off;
	}
}

void UBSGameUserSettings::SetStreamlineReflexMode(const uint8 InStreamlineReflexMode)
{
	const auto Mode = static_cast<UStreamlineReflexMode>(InStreamlineReflexMode);
	if (UStreamlineLibraryReflex::IsReflexSupported())
	{
		UStreamlineLibraryReflex::SetReflexMode(Mode);
		StreamlineReflexMode = Mode;
	}
	else
	{
		StreamlineReflexMode = UStreamlineReflexMode::Disabled;
	}
}

void UBSGameUserSettings::SetDLSSSharpness(const float InDLSSSharpness)
{
	DLSSSharpness = InDLSSSharpness;
	if (UDLSSLibrary::IsDLSSEnabled())
	{
		UDLSSLibrary::SetDLSSSharpness(DLSSSharpness);
	}
}

void UBSGameUserSettings::SetNISSharpness(const float InNISSharpness)
{
	NISSharpness = InNISSharpness;
	if (!UDLSSLibrary::IsDLSSEnabled() && NISEnabledMode == ENISEnabledMode::On)
	{
		UNISLibrary::SetNISSharpness(NISSharpness);
	}
}

void UBSGameUserSettings::HandleAudioOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices)
{
	AudioDeviceNames.Empty();
	for (auto& Device : AvailableDevices)
	{
		AudioDeviceNames.Add(Device.Name);
	}
}

void UBSGameUserSettings::HandleMainAudioOutputDeviceObtained(const FString& CurrentDevice)
{
	AudioOutputDeviceId = CurrentDevice;
}
