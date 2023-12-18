// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DLSSLibrary.h"
#include "GlobalConstants.h"
#include "NISLibrary.h"
#include "InputCoreTypes.h"
#include "StreamlineLibraryDLSSG.h"
#include "StreamlineLibraryReflex.h"
#include "GameFramework/SaveGame.h"
#include "SaveGamePlayerSettings.generated.h"

using namespace Constants;

/** Nvidia DLSS Enabled Mode */
UENUM(BlueprintType)
enum class EDLSSEnabledMode : uint8
{
	Off UMETA(DisplayName = "Off"),
	On UMETA(DisplayName = "On"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EDLSSEnabledMode, EDLSSEnabledMode::Off, EDLSSEnabledMode::On);


/** Nvidia NIS Enabled Mode */
UENUM(BlueprintType)
enum class ENISEnabledMode : uint8
{
	Off UMETA(DisplayName = "Off"),
	On UMETA(DisplayName = "On"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ENISEnabledMode, ENISEnabledMode::Off, ENISEnabledMode::On);

/** Game settings */
USTRUCT(BlueprintType)
struct FPlayerSettings_Game
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FLinearColor StartTargetColor;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor PeakTargetColor;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor EndTargetColor;

	UPROPERTY(BlueprintReadOnly)
	bool bUseSeparateOutlineColor;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor TargetOutlineColor;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor InactiveTargetColor;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor TakingTrackingDamageColor;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor NotTakingTrackingDamageColor;

	UPROPERTY(BlueprintReadOnly)
	bool bShowStreakCombatText;

	UPROPERTY(BlueprintReadOnly)
	int32 CombatTextFrequency;

	UPROPERTY(BlueprintReadWrite)
	bool bShouldRecoil;

	UPROPERTY(BlueprintReadWrite)
	bool bAutomaticFire;

	UPROPERTY(BlueprintReadWrite)
	bool bShowBulletDecals;

	UPROPERTY(BlueprintReadWrite)
	bool bShowBulletTracers;

	UPROPERTY(BlueprintReadWrite)
	bool bShowMuzzleFlash;

	UPROPERTY(BlueprintReadWrite)
	bool bShowCharacterMesh;

	UPROPERTY(BlueprintReadWrite)
	bool bShowWeaponMesh;

	UPROPERTY(BlueprintReadWrite)
	bool bShowHitTimingWidget;

	/* Wall Menu settings */

	UPROPERTY(BlueprintReadWrite)
	bool bNightModeSelected;

	UPROPERTY(BlueprintReadWrite)
	bool bShowLightVisualizers;

	UPROPERTY(BlueprintReadWrite)
	bool bShow_LVFrontBeam;

	UPROPERTY(BlueprintReadWrite)
	bool bShow_LVLeftBeam;

	UPROPERTY(BlueprintReadWrite)
	bool bShow_LVRightBeam;

	UPROPERTY(BlueprintReadWrite)
	bool bShow_LVTopBeam;

	UPROPERTY(BlueprintReadWrite)
	bool bShow_LVLeftCube;

	UPROPERTY(BlueprintReadWrite)
	bool bShow_LVRightCube;

	FPlayerSettings_Game()
	{
		bShowStreakCombatText = true;
		CombatTextFrequency = DefaultCombatTextFrequency;
		StartTargetColor = DefaultStartTargetColor;
		PeakTargetColor = DefaultPeakTargetColor;
		EndTargetColor = DefaultEndTargetColor;
		bUseSeparateOutlineColor = false;
		TargetOutlineColor = DefaultTargetOutlineColor;
		InactiveTargetColor = DefaultInactiveTargetColor;
		TakingTrackingDamageColor = DefaultTakingTrackingDamageColor;
		NotTakingTrackingDamageColor = DefaultNotTakingTrackingDamageColor;
		bShouldRecoil = false;
		bAutomaticFire = true;
		bShowBulletDecals = true;
		bShowBulletTracers = true;
		bShowMuzzleFlash = true;
		bShowCharacterMesh = true;
		bShowWeaponMesh = true;
		bShowHitTimingWidget = true;
		bNightModeSelected = false;
		bShowLightVisualizers = false;
		bShow_LVFrontBeam = false;
		bShow_LVLeftBeam = false;
		bShow_LVRightBeam = false;
		bShow_LVTopBeam = false;
		bShow_LVLeftCube = false;
		bShow_LVRightCube = false;
	}

	/** Resets all game settings not on the wall menu */
	void ResetToDefault()
	{
		bShowStreakCombatText = true;
		CombatTextFrequency = DefaultCombatTextFrequency;
		StartTargetColor = DefaultStartTargetColor;
		PeakTargetColor = DefaultPeakTargetColor;
		EndTargetColor = DefaultEndTargetColor;
		bUseSeparateOutlineColor = false;
		TargetOutlineColor = DefaultTargetOutlineColor;
		InactiveTargetColor = DefaultInactiveTargetColor;
		TakingTrackingDamageColor = DefaultTakingTrackingDamageColor;
		NotTakingTrackingDamageColor = DefaultNotTakingTrackingDamageColor;
		bShouldRecoil = false;
		bAutomaticFire = true;
		bShowBulletDecals = true;
		bShowBulletTracers = true;
		bShowMuzzleFlash = true;
		bShowCharacterMesh = true;
		bShowWeaponMesh = true;
		bShowHitTimingWidget = true;
	}
};

/** Video and sound settings */
USTRUCT(BlueprintType)
struct FPlayerSettings_VideoAndSound
{
	GENERATED_USTRUCT_BODY()

	// GlobalVolume, which also affects Menu and Music volume
	UPROPERTY(BlueprintReadOnly)
	float GlobalVolume;

	// Volume of the Main Menu Music
	UPROPERTY(BlueprintReadOnly)
	float MenuVolume;

	// Volume of the AudioAnalyzer Tracker
	UPROPERTY(BlueprintReadOnly)
	float MusicVolume;

	UPROPERTY(BlueprintReadOnly)
	int32 FrameRateLimitMenu;

	UPROPERTY(BlueprintReadOnly)
	int32 FrameRateLimitGame;

	UPROPERTY(BlueprintReadOnly)
	bool bShowFPSCounter;

	// DLSS On/Off
	UPROPERTY(BlueprintReadOnly)
	EDLSSEnabledMode DLSSEnabledMode;

	// NIS On/Off
	UPROPERTY(BlueprintReadOnly)
	ENISEnabledMode NISEnabledMode;

	// Frame Generation
	UPROPERTY(BlueprintReadOnly)
	UStreamlineDLSSGMode FrameGenerationEnabledMode;

	// Super Resolution Mode
	UPROPERTY(BlueprintReadOnly)
	UDLSSMode DLSSMode;

	// NIS Mode
	UPROPERTY(BlueprintReadOnly)
	UNISMode NISMode;

	// Reflex Mode
	UPROPERTY(BlueprintReadOnly)
	UStreamlineReflexMode StreamlineReflexMode;

	UPROPERTY(BlueprintReadOnly)
	float DLSSSharpness;

	UPROPERTY(BlueprintReadOnly)
	float NISSharpness;

	UPROPERTY(BlueprintReadOnly)
	float Brightness;

	FPlayerSettings_VideoAndSound()
	{
		GlobalVolume = DefaultGlobalVolume;
		MenuVolume = DefaultMenuVolume;
		MusicVolume = DefaultMusicVolume;
		FrameRateLimitMenu = DefaultFrameRateLimitMenu;
		FrameRateLimitGame = DefaultFrameRateLimitGame;
		bShowFPSCounter = false;
		DLSSEnabledMode = EDLSSEnabledMode::On;
		FrameGenerationEnabledMode = UStreamlineDLSSGMode::On;
		DLSSMode = UDLSSMode::Auto;
		DLSSSharpness = 0.f;
		NISEnabledMode = ENISEnabledMode::Off;
		NISMode = UNISMode::Off;
		NISSharpness = 0.f;
		StreamlineReflexMode = UStreamlineReflexMode::Enabled;
		Brightness = 50.f;
	}

	float GetPostProcessBiasFromBrightness() const
	{
		return FMath::GetMappedRangeValueClamped(FVector2D(MinValue_Brightness, MaxValue_Brightness),
			FVector2D(MinValue_ExposureCompensation, MaxValue_ExposureCompensation), Brightness);
	}

	static float GetBrightnessFromPostProcessBias(const float InBias)
	{
		return FMath::GetMappedRangeValueClamped(
			FVector2D(MinValue_ExposureCompensation, MaxValue_ExposureCompensation),
			FVector2D(MinValue_Brightness, MaxValue_Brightness), InBias);
	}
};

/** User settings */
USTRUCT(BlueprintType)
struct FPlayerSettings_User
{
	GENERATED_USTRUCT_BODY()

	/** Sensitivity of DefaultCharacter */
	UPROPERTY(BlueprintReadOnly)
	float Sensitivity;

	UPROPERTY(BlueprintReadOnly)
	FString UserID;

	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly)
	FString RefreshCookie;

	UPROPERTY(BlueprintReadOnly)
	bool bNightModeUnlocked;

	UPROPERTY(BlueprintReadOnly)
	bool bHasRanBenchmark;

	UPROPERTY(BlueprintReadOnly)
	TMap<FName, FKey> Keybindings;

	FPlayerSettings_User()
	{
		Sensitivity = DefaultSensitivity;
		UserID = FString();
		RefreshCookie = FString();
		bNightModeUnlocked = false;
		bHasRanBenchmark = false;
		Keybindings = TMap<FName, FKey>();
	}
};

/** CrossHair settings */
USTRUCT(BlueprintType)
struct FPlayerSettings_CrossHair
{
	GENERATED_USTRUCT_BODY()

	// Lines

	UPROPERTY(BlueprintReadOnly)
	int32 LineWidth;

	UPROPERTY(BlueprintReadOnly)
	int32 LineLength;

	UPROPERTY(BlueprintReadOnly)
	int32 InnerOffset;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor CrossHairColor;

	// Outline

	UPROPERTY(BlueprintReadOnly)
	FLinearColor OutlineColor;

	UPROPERTY(BlueprintReadOnly)
	int32 OutlineSize;

	// Dot

	UPROPERTY(BlueprintReadOnly)
	bool bShowCrossHairDot;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor CrossHairDotColor;

	UPROPERTY(BlueprintReadOnly)
	int32 CrossHairDotSize;

	FPlayerSettings_CrossHair()
	{
		LineWidth = DefaultLineWidth;
		LineLength = DefaultLineLength;
		InnerOffset = DefaultInnerOffset;
		CrossHairColor = DefaultCrossHairColor;

		OutlineColor = DefaultCrossHairOutlineColor;
		OutlineSize = DefaultOutlineSize;

		bShowCrossHairDot = false;
		CrossHairDotColor = DefaultCrossHairColor;
		CrossHairDotSize = DefaultCrossHairDotSize;
	}
};

/** Audio Analyzer specific settings */
USTRUCT(BlueprintType)
struct FPlayerSettings_AudioAnalyzer
{
	GENERATED_BODY()

	/** Number of channels to break Tracker Sound frequencies into */
	UPROPERTY(BlueprintReadOnly)
	int NumBandChannels;

	/** Array to store Threshold values for each active band channel */
	UPROPERTY(BlueprintReadOnly)
	TArray<float> BandLimitsThreshold;

	/** Array to store band frequency channels */
	UPROPERTY(BlueprintReadOnly)
	TArray<FVector2D> BandLimits;

	/** Time window to take frequency sample */
	UPROPERTY(BlueprintReadOnly)
	float TimeWindow;

	/** History size of frequency sample */
	UPROPERTY(BlueprintReadOnly)
	int HistorySize;

	/** Max number of band channels allowed */
	int32 MaxNumBandChannels;

	UPROPERTY(BlueprintReadOnly)
	FString LastSelectedInputAudioDevice;

	UPROPERTY(BlueprintReadOnly)
	FString LastSelectedOutputAudioDevice;

	FPlayerSettings_AudioAnalyzer()
	{
		BandLimits = DefaultBandLimits;
		BandLimitsThreshold = TArray<float>();
		BandLimitsThreshold.Init(DefaultBandLimitThreshold, DefaultNumBandChannels);
		NumBandChannels = DefaultNumBandChannels;
		TimeWindow = DefaultTimeWindow;
		HistorySize = DefaultHistorySize;
		MaxNumBandChannels = DefaultMaxNumBandChannels;
		LastSelectedInputAudioDevice = "";
		LastSelectedOutputAudioDevice = "";
	}

	/** Resets all settings to default, but keeps audio device information */
	void ResetToDefault()
	{
		BandLimits = DefaultBandLimits;
		BandLimitsThreshold = TArray<float>();
		BandLimitsThreshold.Init(DefaultBandLimitThreshold, DefaultNumBandChannels);
		NumBandChannels = DefaultNumBandChannels;
		TimeWindow = DefaultTimeWindow;
		HistorySize = DefaultHistorySize;
		MaxNumBandChannels = DefaultMaxNumBandChannels;
	}
};

/** Wrapper holding all player settings sub-structs */
USTRUCT(BlueprintType)
struct FPlayerSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FPlayerSettings_User User;

	UPROPERTY(BlueprintReadWrite)
	FPlayerSettings_Game Game;

	UPROPERTY(BlueprintReadOnly)
	FPlayerSettings_VideoAndSound VideoAndSound;

	UPROPERTY(BlueprintReadOnly)
	FPlayerSettings_CrossHair CrossHair;

	UPROPERTY(BlueprintReadOnly)
	FPlayerSettings_AudioAnalyzer AudioAnalyzer;

	FPlayerSettings()
	{
		User = FPlayerSettings_User();
		Game = FPlayerSettings_Game();
		VideoAndSound = FPlayerSettings_VideoAndSound();
		CrossHair = FPlayerSettings_CrossHair();
		AudioAnalyzer = FPlayerSettings_AudioAnalyzer();
	}

	void ResetGameSettings()
	{
		Game.ResetToDefault();
	}

	void ResetVideoAndSoundSettings()
	{
		VideoAndSound = FPlayerSettings_VideoAndSound();
	}

	void ResetCrossHair()
	{
		CrossHair = FPlayerSettings_CrossHair();
	}

	void ResetAudioAnalyzer()
	{
		AudioAnalyzer.ResetToDefault();
	}
};

UCLASS()
class BEATSHOTGLOBAL_API USaveGamePlayerSettings : public USaveGame
{
	GENERATED_BODY()

public:
	/** Returns a copy of PlayerSettings */
	FPlayerSettings GetPlayerSettings() const;

	/** Saves Game specific settings, preserving all other settings */
	void SavePlayerSettings(const FPlayerSettings_Game& InGameSettings);

	/** Saves Audio Analyzer specific settings, preserving all other settings */
	void SavePlayerSettings(const FPlayerSettings_AudioAnalyzer& InAudioAnalyzerSettings);

	/** Saves User specific settings, preserving all other settings */
	void SavePlayerSettings(const FPlayerSettings_User& InUserSettings);

	/** Saves CrossHair specific settings, preserving all other settings */
	void SavePlayerSettings(const FPlayerSettings_CrossHair& InCrossHairSettings);

	/** Saves VideoAndSound settings, preserving all other settings */
	void SavePlayerSettings(const FPlayerSettings_VideoAndSound& InVideoAndSoundSettings);

private:
	UPROPERTY()
	FPlayerSettings PlayerSettings;
};
