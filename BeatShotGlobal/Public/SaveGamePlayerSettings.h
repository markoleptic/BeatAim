// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GlobalConstants.h"
#include "InputCoreTypes.h"
#include "GameFramework/SaveGame.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "SaveGamePlayerSettings.generated.h"

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
		CombatTextFrequency = Constants::DefaultCombatTextFrequency;
		StartTargetColor = Constants::DefaultStartTargetColor;
		PeakTargetColor = Constants::DefaultPeakTargetColor;
		EndTargetColor = Constants::DefaultEndTargetColor;
		bUseSeparateOutlineColor = false;
		TargetOutlineColor = Constants::DefaultTargetOutlineColor;
		InactiveTargetColor = Constants::DefaultInactiveTargetColor;
		TakingTrackingDamageColor = Constants::DefaultTakingTrackingDamageColor;
		NotTakingTrackingDamageColor = Constants::DefaultNotTakingTrackingDamageColor;
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
		CombatTextFrequency = Constants::DefaultCombatTextFrequency;
		StartTargetColor = Constants::DefaultStartTargetColor;
		PeakTargetColor = Constants::DefaultPeakTargetColor;
		EndTargetColor = Constants::DefaultEndTargetColor;
		bUseSeparateOutlineColor = false;
		TargetOutlineColor = Constants::DefaultTargetOutlineColor;
		InactiveTargetColor = Constants::DefaultInactiveTargetColor;
		TakingTrackingDamageColor = Constants::DefaultTakingTrackingDamageColor;
		NotTakingTrackingDamageColor = Constants::DefaultNotTakingTrackingDamageColor;
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
		Sensitivity = Constants::DefaultSensitivity;
		UserID = FString();
		RefreshCookie = FString();
		bNightModeUnlocked = false;
		bHasRanBenchmark = false;
		Keybindings = TMap<FName, FKey>();
	}

	/** Returns an array of Keybindings for use with UEnhancedInputUserSettings. Empties the Keybindings map */
	TArray<FMapPlayerKeyArgs> GetLegacyKeybindings()
	{
		TArray<FMapPlayerKeyArgs> Out;
		for (const TPair<FName, FKey>& Keybinding : Keybindings)
		{
			FGameplayTagContainer Failure;
			FMapPlayerKeyArgs Args;
			Args.NewKey = Keybinding.Value;

			FString StringKey = Keybinding.Key.ToString();
			if (StringKey.Len() > 2 && StringKey.EndsWith("_2"))
			{
				StringKey = StringKey.LeftChop(2);
				FString Last;
				Last.AppendChar(StringKey[StringKey.Len() - 1]);
				if (!Last.IsNumeric())
				{
					Args.MappingName = FName(StringKey);
					Args.Slot = EPlayerMappableKeySlot::Second;
				}
			}
			else
			{
				Args.MappingName = Keybinding.Key;
				Args.Slot = EPlayerMappableKeySlot::First;
			}
			Out.Add(Args);
		}
		Keybindings.Empty();
		return Out;
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
		LineWidth = Constants::DefaultLineWidth;
		LineLength = Constants::DefaultLineLength;
		InnerOffset = Constants::DefaultInnerOffset;
		CrossHairColor = Constants::DefaultCrossHairColor;

		OutlineColor = Constants::DefaultCrossHairOutlineColor;
		OutlineSize = Constants::DefaultOutlineSize;

		bShowCrossHairDot = false;
		CrossHairDotColor = Constants::DefaultCrossHairColor;
		CrossHairDotSize = Constants::DefaultCrossHairDotSize;
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

	FPlayerSettings_AudioAnalyzer()
	{
		BandLimits = Constants::DefaultBandLimits;
		BandLimitsThreshold = TArray<float>();
		BandLimitsThreshold.Init(Constants::DefaultBandLimitThreshold, Constants::DefaultNumBandChannels);
		NumBandChannels = Constants::DefaultNumBandChannels;
		TimeWindow = Constants::DefaultTimeWindow;
		HistorySize = Constants::DefaultHistorySize;
		MaxNumBandChannels = Constants::DefaultMaxNumBandChannels;
		LastSelectedInputAudioDevice = "";
	}

	/** Resets all settings to default, but keeps audio device information */
	void ResetToDefault()
	{
		BandLimits = Constants::DefaultBandLimits;
		BandLimitsThreshold = TArray<float>();
		BandLimitsThreshold.Init(Constants::DefaultBandLimitThreshold, Constants::DefaultNumBandChannels);
		NumBandChannels = Constants::DefaultNumBandChannels;
		TimeWindow = Constants::DefaultTimeWindow;
		HistorySize = Constants::DefaultHistorySize;
		MaxNumBandChannels = Constants::DefaultMaxNumBandChannels;
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
	FPlayerSettings_CrossHair CrossHair;

	UPROPERTY(BlueprintReadOnly)
	FPlayerSettings_AudioAnalyzer AudioAnalyzer;

	FPlayerSettings()
	{
		User = FPlayerSettings_User();
		Game = FPlayerSettings_Game();
		CrossHair = FPlayerSettings_CrossHair();
		AudioAnalyzer = FPlayerSettings_AudioAnalyzer();
	}

	void ResetGameSettings()
	{
		Game.ResetToDefault();
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

private:
	UPROPERTY()
	FPlayerSettings PlayerSettings;
};
