// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


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

DEFINE_LOG_CATEGORY(LogBSGameUserSettings);

ENUM_RANGE_BY_FIRST_AND_LAST(UDLSSSupport, UDLSSSupport::Supported,
	UDLSSSupport::NotSupportedIncompatibleAPICaptureToolActive);

ENUM_RANGE_BY_FIRST_AND_LAST(UDLSSMode, UDLSSMode::Off, UDLSSMode::UltraPerformance);

ENUM_RANGE_BY_FIRST_AND_LAST(UNISMode, UNISMode::Off, UNISMode::Custom);

ENUM_RANGE_BY_FIRST_AND_LAST(UStreamlineReflexMode, UStreamlineReflexMode::Disabled,
	UStreamlineReflexMode::EnabledPlusBoost);

ENUM_RANGE_BY_FIRST_AND_LAST(UStreamlineDLSSGMode, UStreamlineDLSSGMode::Off, UStreamlineDLSSGMode::On);

ENUM_RANGE_BY_FIRST_AND_LAST(EWindowMode::Type, EWindowMode::Type::Fullscreen, EWindowMode::Type::WindowedFullscreen);

TMulticastDelegate<void(const UBSGameUserSettings*)> UBSGameUserSettings::OnSettingsChanged = TMulticastDelegate<void(
	const UBSGameUserSettings*)>();

namespace
{
	/** Applies the value of AntiAliasingMethod to the global GEngine ini. */
	void ApplyAntiAliasingMethod(const TEnumAsByte<EAntiAliasingMethod> AntiAliasingMethod)
	{
		if (IConsoleVariable* CVarAntiAliasingMethod = IConsoleManager::Get().FindConsoleVariable(
			TEXT("r.AntiAliasingMethod")))
		{
			CVarAntiAliasingMethod->Set(AntiAliasingMethod, ECVF_SetByGameOverride);
		}
		if (GConfig)
		{
			const FString Value = FString::FromInt(AntiAliasingMethod);
			GConfig->SetString(TEXT("/Script/Engine.RendererSettings"), TEXT("r.AntiAliasingMethod"), *Value,
				GEngineIni);
			GConfig->Flush(false, GEngineIni);
		}
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
}

UBSGameUserSettings::UBSGameUserSettings() : bInMenu(false)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (FSlateApplication::IsInitialized())
		{
			OnApplicationActivationStateChangedHandle = FSlateApplication::Get().OnApplicationActivationStateChanged().
				AddUObject(this, &ThisClass::HandleApplicationActivationStateChanged);
		}
		VideoSettingEnumMap = GetDefault<UVideoSettingEnumTagMap>();
	}
	SetToBSDefaults();
}

UBSGameUserSettings* UBSGameUserSettings::Get()
{
	return GEngine ? CastChecked<UBSGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

void UBSGameUserSettings::Initialize(const UWorld* World)
{
	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
	{
		LoadDLSSSettings();
		LoadUserControlBusMix(World);
		FOnAudioOutputDevicesObtained OnAudioOutputDevicesObtained;
		OnAudioOutputDevicesObtained.BindDynamic(this, &UBSGameUserSettings::HandleAudioOutputDevicesObtained);
		UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(World, OnAudioOutputDevicesObtained);

		FOnMainAudioOutputDeviceObtained OnMainAudioOutputDeviceObtained;
		OnMainAudioOutputDeviceObtained.BindDynamic(this, &UBSGameUserSettings::HandleMainAudioOutputDeviceObtained);
		UAudioMixerBlueprintLibrary::GetCurrentAudioOutputDeviceName(World, OnMainAudioOutputDeviceObtained);
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
	// bInMenu = false;
	bDLSSInitialized = false;
}

bool UBSGameUserSettings::IsBSVersionValid() const
{
	return BSVersion == Constants::BSGameUserSettingsVersion;
}

void UBSGameUserSettings::UpdateBSVersion()
{
	const int32 LastVersion = BSVersion;
	BSVersion = Constants::BSGameUserSettingsVersion;
	UE_LOG(LogBSGameUserSettings, Display, TEXT("Updated UBSGameUserSettings from %d to %d"), LastVersion, BSVersion);
}

void UBSGameUserSettings::LoadDLSSSettings()
{
	if (!FModuleManager::Get().IsModuleLoaded(FName("DLSS")))
	{
		UE_LOG(LogBSGameUserSettings, Warning, TEXT("DLSS not loaded"));
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

void UBSGameUserSettings::LoadUserControlBusMix(const UWorld* World)
{
	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
	{
		const UBSAudioSettings* BSAudioSettings = GetDefault<UBSAudioSettings>();

		ControlBusMap.Empty();

		USoundControlBus* OverallControlBus = TryLoadControlBus(BSAudioSettings->OverallVolumeControlBus, ControlBusMap,
			TEXT("Overall"));
		USoundControlBus* MenuControlBus = TryLoadControlBus(BSAudioSettings->MenuVolumeControlBus, ControlBusMap,
			TEXT("Menu"));
		USoundControlBus* MusicControlBus = TryLoadControlBus(BSAudioSettings->MusicVolumeControlBus, ControlBusMap,
			TEXT("Music"));
		USoundControlBus* SoundFXControlBus = TryLoadControlBus(BSAudioSettings->SoundFXVolumeControlBus, ControlBusMap,
			TEXT("SoundFX"));

		if (UObject* ObjPath = BSAudioSettings->UserSettingsControlBusMix.TryLoad())
		{
			if (USoundControlBusMix* SoundControlBusMix = Cast<USoundControlBusMix>(ObjPath))
			{
				ControlBusMix = SoundControlBusMix;
				UAudioModulationStatics::ActivateBusMix(World, SoundControlBusMix);

				const FSoundControlBusMixStage OverallControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(
					World, OverallControlBus, OverallVolume / 100.0);
				const FSoundControlBusMixStage MenuControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(
					World, MenuControlBus, MenuVolume / 100.0);
				const FSoundControlBusMixStage MusicControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(
					World, MusicControlBus, MusicVolume / 100.0);
				const FSoundControlBusMixStage SoundFXControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(
					World, SoundFXControlBus, SoundFXVolume / 100.0);

				TArray<FSoundControlBusMixStage> ControlBusMixStageArray;
				ControlBusMixStageArray.Add(OverallControlBusMixStage);
				ControlBusMixStageArray.Add(MenuControlBusMixStage);
				ControlBusMixStageArray.Add(MusicControlBusMixStage);
				ControlBusMixStageArray.Add(SoundFXControlBusMixStage);

				UAudioModulationStatics::UpdateMix(World, SoundControlBusMix, ControlBusMixStageArray);

				bSoundControlBusMixLoaded = true;
			}
			else
			{
				UE_LOG(LogBSGameUserSettings, Warning, TEXT("User Settings Control Bus Mix reference missing."));
			}
		}
		else
		{
			ensureMsgf(ObjPath, TEXT("Failed to load Control Bus Mix."));
		}
	}
	else
	{
		ensureMsgf(World, TEXT("Failed to load current player world"));
	}
}

void UBSGameUserSettings::SetVolumeForControlBus(const FName& ControlBusKey, const float InVolume) const
{
	TObjectPtr<USoundControlBus> ControlBus = ControlBusMap.FindRef(ControlBusKey);

	if (GEngine && ControlBus && bSoundControlBusMixLoaded)
	{
		if (const UWorld* AudioWorld = GEngine->GetCurrentPlayWorld())
		{
			if (ensureMsgf(ControlBusMix, TEXT("Control Bus Mix failed to load.")))
			{
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
}

void UBSGameUserSettings::UpdateEffectiveFrameRateLimit()
{
	if (!IsRunningDedicatedServer())
	{
		SetFrameRateLimitCVar(GetEffectiveFrameRateLimit());
	}
}

void UBSGameUserSettings::ValidateNvidiaSettings()
{
	// DLSSEnabledMode and DLSSMode
	if (DLSSEnabledMode == EDLSSEnabledMode::On)
	{
		if (UDLSSLibrary::IsDLSSSupported())
		{
			if (DLSSMode == UDLSSMode::Off)
			{
				DLSSMode = UDLSSMode::Auto;
			}
			if (!UDLSSLibrary::IsDLSSModeSupported(DLSSMode) && DLSSMode != UDLSSMode::Auto)
			{
				DLSSMode = UDLSSMode::Off;
			}
			if (UStreamlineLibraryReflex::IsReflexSupported())
			{
				StreamlineReflexMode = UStreamlineReflexMode::Enabled;
			}
			if (UStreamlineLibraryDLSSG::IsDLSSGSupported() && UStreamlineLibraryDLSSG::IsDLSSGModeSupported(
				FrameGenerationEnabledMode))
			{
				FrameGenerationEnabledMode = UStreamlineDLSSGMode::On;
			}
		}
		else
		{
			DLSSEnabledMode = EDLSSEnabledMode::Off;
			DLSSMode = UDLSSMode::Off;
		}
	}
	else
	{
		DLSSMode = UDLSSMode::Off;
		FrameGenerationEnabledMode = UStreamlineDLSSGMode::Off;
	}

	// NISEnabledMode and NISMode
	if (NISEnabledMode == ENISEnabledMode::On)
	{
		if (!UNISLibrary::IsNISModeSupported(NISMode))
		{
			NISMode = UNISMode::Off;
		}
	}
	else
	{
		NISMode = UNISMode::Off;
	}

	// Frame Generation
	if (!UStreamlineLibraryDLSSG::IsDLSSGSupported() || !UStreamlineLibraryDLSSG::IsDLSSGModeSupported(
		FrameGenerationEnabledMode))
	{
		FrameGenerationEnabledMode = UStreamlineDLSSGMode::Off;
	}

	// Reflex
	if (!UStreamlineLibraryReflex::IsReflexSupported())
	{
		StreamlineReflexMode = UStreamlineReflexMode::Disabled;
	}

	if (DLSSEnabledMode == EDLSSEnabledMode::On && NISEnabledMode == ENISEnabledMode::On)
	{
		NISEnabledMode = ENISEnabledMode::Off;
		NISMode = UNISMode::Off;
	}
}

void UBSGameUserSettings::ApplyNvidiaSettings()
{
	if (!bDLSSInitialized)
	{
		return;
	}

	ApplyDLSSMode(DLSSMode, GetScreenResolution());

	if (UNISLibrary::IsNISSupported() && UNISLibrary::IsNISModeSupported(NISMode))
	{
		UNISLibrary::SetNISMode(NISMode);
	}
	if (UStreamlineLibraryDLSSG::IsDLSSGSupported() && UStreamlineLibraryDLSSG::IsDLSSGModeSupported(
		FrameGenerationEnabledMode))
	{
		UStreamlineLibraryDLSSG::SetDLSSGMode(FrameGenerationEnabledMode);
	}
	if (UStreamlineLibraryReflex::IsReflexSupported())
	{
		UStreamlineLibraryReflex::SetReflexMode(StreamlineReflexMode);
	}
	if (DLSSEnabledMode == EDLSSEnabledMode::On && NISEnabledMode == ENISEnabledMode::Off)
	{
		UDLSSLibrary::SetDLSSSharpness(DLSSSharpness);
	}
	if (DLSSEnabledMode == EDLSSEnabledMode::Off && NISEnabledMode == ENISEnabledMode::On)
	{
		UNISLibrary::SetNISSharpness(NISSharpness);
	}
	if (DLSSEnabledMode == EDLSSEnabledMode::On)
	{
		if (IsVSyncEnabled())
		{
			SetVSyncEnabled(false);
			static auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.VSync"));
			CVar->Set(IsVSyncEnabled(), ECVF_SetByGameSetting);
		}
	}
	if (DLSSEnabledMode == EDLSSEnabledMode::On || NISEnabledMode == ENISEnabledMode::On)
	{
		if (ScalabilityQuality.ResolutionQuality != 100.f)
		{
			SetResolutionScaleValueEx(100.f);
		}
	}
}

void UBSGameUserSettings::BeginDestroy()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnApplicationActivationStateChanged().
		                         Remove(OnApplicationActivationStateChangedHandle);
	}
	Super::BeginDestroy();
}

void UBSGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();
	SetVSyncEnabled(false);
	SetToBSDefaults();
}

void UBSGameUserSettings::LoadSettings(const bool bForceReload)
{
	Super::LoadSettings(bForceReload);
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

	ValidateNvidiaSettings();

	Super::ValidateSettings();

	UE_LOG(LogBSGameUserSettings, Warning, TEXT("Validate Settings"));
}

void UBSGameUserSettings::ResetToCurrentSettings()
{
	Super::ResetToCurrentSettings();
}

void UBSGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();
	ApplyAntiAliasingMethod(AntiAliasingMethod);
	ApplyDisplayGamma(DisplayGamma);
	ApplyNvidiaSettings();
}

void UBSGameUserSettings::ApplySettings(bool bForceReload)
{
	Super::ApplySettings(bForceReload);
}

void UBSGameUserSettings::SaveSettings()
{
	Super::SaveSettings();
	OnSettingsChanged.Broadcast(this);
}

float UBSGameUserSettings::GetEffectiveFrameRateLimit()
{
	if (FSlateApplication::IsInitialized() && !FSlateApplication::Get().IsActive())
	{
		return FrameRateLimitBackground;
	}

	if (bInMenu)
	{
		return FrameRateLimitMenu;
	}

	return FrameRateLimitGame;
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

void UBSGameUserSettings::SetInMenu(const bool bIsInMenu)
{
	bInMenu = bIsInMenu;
	UpdateEffectiveFrameRateLimit();
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

bool UBSGameUserSettings::IsDLSSEnabled() const
{
	return DLSSEnabledMode == EDLSSEnabledMode::On;
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
}

void UBSGameUserSettings::SetBrightness(const float InBrightness)
{
	Brightness = FMath::Clamp(InBrightness, Constants::MinValue_Brightness, Constants::MaxValue_Brightness);
}

void UBSGameUserSettings::SetDisplayGamma(const float InGamma)
{
	DisplayGamma = FMath::Clamp(InGamma, Constants::MinValue_DisplayGamma, Constants::MaxValue_DisplayGamma);
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
	if (DLSSEnabledMode != EDLSSEnabledMode::On && NISEnabledMode != ENISEnabledMode::On)
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
	if (NISEnabledMode == ENISEnabledMode::On)
	{
		NISEnabledMode = ENISEnabledMode::Off;
	}
}

void UBSGameUserSettings::SetNISEnabledMode(const uint8 InNISEnabledMode)
{
	NISEnabledMode = static_cast<ENISEnabledMode>(InNISEnabledMode);
	if (DLSSEnabledMode == EDLSSEnabledMode::On)
	{
		DLSSEnabledMode = EDLSSEnabledMode::Off;
	}
}

void UBSGameUserSettings::SetFrameGenerationEnabledMode(const uint8 InFrameGenerationEnabledMode)
{
	FrameGenerationEnabledMode = static_cast<UStreamlineDLSSGMode>(InFrameGenerationEnabledMode);
}

void UBSGameUserSettings::SetDLSSMode(const uint8 InDLSSMode)
{
	DLSSMode = static_cast<UDLSSMode>(InDLSSMode);
}

void UBSGameUserSettings::SetNISMode(const uint8 InNISMode)
{
	NISMode = static_cast<UNISMode>(InNISMode);
}

void UBSGameUserSettings::SetStreamlineReflexMode(const uint8 InStreamlineReflexMode)
{
	StreamlineReflexMode = static_cast<UStreamlineReflexMode>(InStreamlineReflexMode);
}

void UBSGameUserSettings::SetDLSSSharpness(const float InDLSSSharpness)
{
	DLSSSharpness = InDLSSSharpness;
}

void UBSGameUserSettings::SetNISSharpness(const float InNISSharpness)
{
	NISSharpness = InNISSharpness;
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

void UBSGameUserSettings::HandleApplicationActivationStateChanged(bool bIsActive)
{
	UpdateEffectiveFrameRateLimit();
}
