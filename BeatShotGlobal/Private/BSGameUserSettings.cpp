// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "BSGameUserSettings.h"

#include "AudioModulationStatics.h"
#include "BSAudioSettings.h"
#include "GlobalConstants.h"
#include "SaveGamePlayerSettings.h"
#include "SoundControlBusMix.h"

namespace
{
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
		return nullptr;
	}

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
	SetBSSettingsToDefaults();
}

UBSGameUserSettings* UBSGameUserSettings::Get()
{
	return GEngine ? CastChecked<UBSGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

void UBSGameUserSettings::BeginDestroy()
{
	Super::BeginDestroy();
}

void UBSGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();
	SetBSSettingsToDefaults();
}

void UBSGameUserSettings::SetBSSettingsToDefaults()
{
	OverallVolume = Constants::DefaultGlobalVolume;
	MenuVolume = Constants::DefaultMenuVolume;
	MusicVolume = Constants::DefaultMusicVolume;
	SoundFXVolume = Constants::DefaultSoundFXVolume;
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
	bSoundControlBusMixLoaded = false;
}

void UBSGameUserSettings::InitDLSSSettings()
{
	// DLSS
	bool bDisableDLSS = true;
	bDLSSSupported = UDLSSLibrary::IsDLSSSupported();
	bNISSupported = UNISLibrary::IsNISSupported();

	if (bDLSSSupported)
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
	if (bNISSupported && UNISLibrary::IsNISModeSupported(NISMode) && DLSSEnabledMode == EDLSSEnabledMode::Off)
	{
		// TODO: Sometimes this causes an exception when called early?
		UNISLibrary::SetNISMode(NISMode);
		UNISLibrary::SetNISSharpness(NISSharpness);
	}
	else
	{
		// TODO: Sometimes this causes an exception when called early?
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

	bDLSSInitialized = true;
}

void UBSGameUserSettings::LoadSettings(const bool bForceReload)
{
	Super::LoadSettings(bForceReload);
}

void UBSGameUserSettings::ConfirmVideoMode()
{
	Super::ConfirmVideoMode();
}

float UBSGameUserSettings::GetEffectiveFrameRateLimit()
{
	return Super::GetEffectiveFrameRateLimit();
}

void UBSGameUserSettings::ResetToCurrentSettings()
{
	Super::ResetToCurrentSettings();
}

void UBSGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();

	LoadUserControlBusMix();
}

int32 UBSGameUserSettings::GetOverallScalabilityLevel() const
{
	return Super::GetOverallScalabilityLevel();
}

void UBSGameUserSettings::SetOverallScalabilityLevel(const int32 Value)
{
	Super::SetOverallScalabilityLevel(Value);
}

void UBSGameUserSettings::ApplyDisplayGamma()
{
	if (GEngine)
	{
		GEngine->DisplayGamma = DisplayGamma;
	}
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

						UAudioModulationStatics::ActivateBusMix(World, ControlBusMix);

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

						UAudioModulationStatics::UpdateMix(World, ControlBusMix, ControlBusMixStageArray);

						bSoundControlBusMixLoaded = true;
					}
					else
					{
						ensureMsgf(SoundControlBusMix,
							TEXT("User Settings Control Bus Mix reference missing from Lyra Audio Settings."));
					}
				}
			}
		}
	}
}

bool UBSGameUserSettings::IsDLSSSupported()
{
	if (!bDLSSInitialized)
	{
		InitDLSSSettings();
	}
	return bDLSSSupported;
}

bool UBSGameUserSettings::IsNISSupported()
{
	if (!bDLSSInitialized)
	{
		InitDLSSSettings();
	}
	return bNISSupported;
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

float UBSGameUserSettings::GetDLSSSharpness() const
{
	return DLSSSharpness;
}

float UBSGameUserSettings::GetNISSharpness() const
{
	return NISSharpness;
}

bool UBSGameUserSettings::GetShowFPSCounter() const
{
	return bShowFPSCounter;
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

EDLSSEnabledMode UBSGameUserSettings::GetDLSSEnabledMode() const
{
	return DLSSEnabledMode;
}

ENISEnabledMode UBSGameUserSettings::GetNISEnabledMode() const
{
	return NISEnabledMode;
}

UStreamlineDLSSGMode UBSGameUserSettings::GetFrameGenerationEnabledMode() const
{
	return FrameGenerationEnabledMode;
}

UDLSSMode UBSGameUserSettings::GetDLSSMode() const
{
	return DLSSMode;
}

UNISMode UBSGameUserSettings::GetNISMode() const
{
	return NISMode;
}

UStreamlineReflexMode UBSGameUserSettings::GetStreamlineReflexMode() const
{
	return StreamlineReflexMode;
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

FString UBSGameUserSettings::GetAudioOutputDeviceId() const
{
	return AudioOutputDeviceId;
}

TEnumAsByte<EAntiAliasingMethod> UBSGameUserSettings::GetAntiAliasingMethod() const
{
	return AntiAliasingMethod;
}

float UBSGameUserSettings::GetDisplayGamma() const
{
	return DisplayGamma;
}

void UBSGameUserSettings::SetShowFPSCounter(const bool InShowFPSCounter)
{
	bShowFPSCounter = InShowFPSCounter;
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

void UBSGameUserSettings::SetDLSSEnabledMode(const EDLSSEnabledMode InDLSSEnabledMode)
{
	if (!bDLSSInitialized)
	{
		InitDLSSSettings();
	}

	DLSSEnabledMode = InDLSSEnabledMode;
}

void UBSGameUserSettings::SetNISEnabledMode(const ENISEnabledMode InNISEnabledMode)
{
	if (!bDLSSInitialized)
	{
		InitDLSSSettings();
	}

	if (UNISLibrary::IsNISSupported() && DLSSEnabledMode == EDLSSEnabledMode::Off)
	{
		NISEnabledMode = InNISEnabledMode;
	}
	else
	{
		NISEnabledMode = ENISEnabledMode::Off;
	}
}

void UBSGameUserSettings::SetFrameGenerationEnabledMode(const UStreamlineDLSSGMode InFrameGenerationEnabledMode)
{
	if (!bDLSSInitialized)
	{
		InitDLSSSettings();
	}
	if (DLSSMode != UDLSSMode::Off && UStreamlineLibraryDLSSG::IsDLSSGSupported() &&
		UStreamlineLibraryDLSSG::IsDLSSGModeSupported(InFrameGenerationEnabledMode))
	{
		UStreamlineLibraryDLSSG::SetDLSSGMode(InFrameGenerationEnabledMode);
		FrameGenerationEnabledMode = InFrameGenerationEnabledMode;
	}
	else
	{
		FrameGenerationEnabledMode = UStreamlineDLSSGMode::Off;
	}
}

void UBSGameUserSettings::SetDLSSMode(const UDLSSMode InDLSSMode)
{
	if (!bDLSSInitialized)
	{
		InitDLSSSettings();
	}

	UDLSSMode LocalDLSSMode = InDLSSMode;
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

	SetFrameGenerationEnabledMode(LocalFrameGenerationEnabledMode);
	SetNISEnabledMode(LocalNISEnabledMode);
	SetNISMode(LocalNISMode);
	SetStreamlineReflexMode(LocalReflexMode);
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
}

void UBSGameUserSettings::SetNISMode(const UNISMode InNISMode)
{
	if (!bDLSSInitialized)
	{
		InitDLSSSettings();
	}

	if (UNISLibrary::IsNISSupported() && DLSSEnabledMode == EDLSSEnabledMode::Off && UNISLibrary::IsNISModeSupported(
		InNISMode))
	{
		UNISLibrary::SetNISMode(InNISMode);
		NISMode = InNISMode;
	}
	else
	{
		NISMode = UNISMode::Off;
	}
}

void UBSGameUserSettings::SetStreamlineReflexMode(const UStreamlineReflexMode InStreamlineReflexMode)
{
	if (!bDLSSInitialized)
	{
		InitDLSSSettings();
	}

	if (UStreamlineLibraryReflex::IsReflexSupported())
	{
		UStreamlineLibraryReflex::SetReflexMode(InStreamlineReflexMode);
		StreamlineReflexMode = InStreamlineReflexMode;
	}
	else
	{
		StreamlineReflexMode = UStreamlineReflexMode::Disabled;
	}
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

void UBSGameUserSettings::SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId)
{
	AudioOutputDeviceId = InAudioOutputDeviceId;
}

void UBSGameUserSettings::SetDisplayGamma(const float InGamma)
{
	// TODO: Fix this or revert to using old brightness
	DisplayGamma = FMath::GetMappedRangeValueClamped(FVector2d(0.0, 100.0), FVector2d(0.0, 7.0), InGamma);
	ApplyDisplayGamma();
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

void UBSGameUserSettings::SetAntiAliasingMethod(const TEnumAsByte<EAntiAliasingMethod> InAntiAliasingMethod)
{
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

void UBSGameUserSettings::SetResolutionScaleChecked(const float InResolutionScale)
{
	if (!UDLSSLibrary::IsDLSSEnabled() && NISEnabledMode != ENISEnabledMode::On)
	{
		SetResolutionScaleValueEx(InResolutionScale * 100.f);
	}
}
