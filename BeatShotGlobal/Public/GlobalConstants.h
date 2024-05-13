// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

namespace Constants
{
	inline constexpr int32 CustomGameModeVersion = 1;
	inline constexpr int32 BSGameUserSettingsVersion = 1;

	/** Min Value allowed for a BandFrequency channel */
	inline constexpr float MinValue_BandFrequency = 0.f;

	/** Max Value allowed for a BandFrequency channel */
	inline constexpr float MaxValue_BandFrequency = 22720.f;

	/** Min Value allowed for a BandFrequency channel threshold */
	inline constexpr float MinValue_BandFrequencyThreshold = 0.1f;

	/** Max Value allowed for a BandFrequency channel threshold */
	inline constexpr float MaxValue_BandFrequencyThreshold = 5.0f;

	/** Snap size for a BandFrequency channel threshold */
	inline constexpr float SnapSize_BandFrequencyThreshold = 0.1f;

	/** The length of the countdown timer */
	inline constexpr int32 CountdownTimerLength = 3;

	/** The value to divide the game sensitivity by to convert to Csgo sensitivity */
	inline constexpr float CsgoMultiplier = 3.18f;

	/** The value to multiply raw input values by for looking */
	inline constexpr float SensitivityMultiplier = 14.2789148024750118991f;

	/** The distance to trace the line */
	inline constexpr float TraceDistance = 999999;

	/** Max floor height for the MovablePlatform */
	inline const FVector MaxFloorHeight(-4000, 0, 500);

	/** Min floor height for the MovablePlatform */
	inline const FVector MinFloorHeight(-4000, 0, 0);

	/** Default Speed of the day to night transition */
	inline constexpr float DayNightCycleSpeed = 20.f;

	/** Length of time until video settings are reset */
	inline constexpr float VideoSettingsTimeoutLength = 10.f;

	/** Lighter background menu brush color */
	inline constexpr FLinearColor LightMenuBrushColor(0, 0, 0, 0.1);

	/** Darker background menu brush color */
	inline constexpr FLinearColor DarkMenuBrushColor(0, 0, 0, 0.2);

	/** The color used to change the GameModeButton color to when selected */
	inline constexpr FLinearColor BeatShotBlue(0.049707, 0.571125, 0.83077, 1.0);
	inline constexpr FLinearColor DefaultTakingTrackingDamageColor(0.049707, 0.571125, 0.83077, 1.0);
	inline constexpr FLinearColor DefaultNotTakingTrackingDamageColor(1.f, 0.25f, 0.f, 1.0);

	/** The time threshold required to receive full score for hitting a target */
	inline constexpr float PerfectScoreTimeThreshold = 0.1f;

	/** The min time required to receive credit for playing a game mode */
	inline constexpr float MinStatRequirement_Duration_NumGamesPlayed = 60.f;

	inline constexpr float DirBoxPadding = 5.f;

	inline constexpr int32 DefaultNumberOfQTableRows = 25;

	inline constexpr int32 DefaultNumberOfQTableColumns = 25;

	inline constexpr int32 DefaultQTableSize = 625;

	inline constexpr int32 DefaultNumberOfAccuracyDataRows = 5;

	inline constexpr int32 DefaultNumberOfAccuracyDataColumns = 5;

	inline constexpr int32 DefaultSpawnAreaDimension = 50;

#pragma region DefaultSettings

	/** The default Band Limit Thresholds for the AudioAnalyzer */
	inline const TArray DefaultBandLimits = {
		FVector2d(0, 44), FVector2d(45, 88), FVector2d(89, 177), FVector2d(178, 355), FVector2d(356, 710),
		FVector2d(711, 1420), FVector2d(1421, 2840), FVector2d(2841, 5680), FVector2d(5681, 11360),
		FVector2d(11361, 22720)
	};

	inline constexpr float DefaultSensitivity = 0.3f;

	inline constexpr float DefaultAlpha = 0.9f;
	inline constexpr float DefaultEpsilon = 0.9f;
	inline constexpr float DefaultGamma = 0.9f;

	inline constexpr int32 DefaultCombatTextFrequency = 5;
	const FLinearColor DefaultStartTargetColor = FLinearColor::White;
	const FLinearColor DefaultPeakTargetColor = FLinearColor::Green;
	const FLinearColor DefaultEndTargetColor = FLinearColor::Red;
	const FLinearColor DefaultTargetOutlineColor = FLinearColor::White;
	inline constexpr FLinearColor DefaultInactiveTargetColor(83.f / 255.f, 0.f, 245.f / 255.f, 1.f);

	inline constexpr float DefaultGlobalVolume = 50.f;
	inline constexpr float DefaultMenuVolume = 50.f;
	inline constexpr float DefaultMusicVolume = 10.f;
	inline constexpr float DefaultSoundFXVolume = 25.f;
	inline constexpr int32 DefaultFrameRateLimitMenu = 60.f;
	inline constexpr int32 DefaultFrameRateLimitGame = 0.f;
	inline constexpr int32 DefaultFrameRateLimitBackground = 30.f;
	inline constexpr float DefaultBrightness = 50.f;
	inline constexpr float DefaultDisplayGamma = 2.2f;

	inline constexpr float DefaultBandLimitThreshold = 2.1f;
	inline constexpr int32 DefaultNumBandChannels = 10;
	inline constexpr float DefaultTimeWindow = 0.02f;
	inline constexpr int32 DefaultHistorySize = 30;
	inline constexpr int32 DefaultMaxNumBandChannels = 32;

	inline constexpr int32 DefaultLineWidth = 4;
	inline constexpr int32 DefaultLineLength = 10;
	inline constexpr int32 DefaultInnerOffset = 6;
	inline constexpr FLinearColor DefaultCrossHairColor(63.f / 255.f, 199.f / 255.f, 235.f / 255.f, 1.f);
	inline constexpr FLinearColor DefaultCrossHairOutlineColor(0.f, 0.f, 0.f, 1.f);
	inline constexpr int32 DefaultOutlineSize = 2;
	inline constexpr int32 DefaultCrossHairDotSize = 4;

	inline constexpr float MaxValue_TimeWindow = 0.01f;
	inline constexpr float MinValue_TimeWindow = 1.00f;
	inline constexpr float SnapSize_TimeWindow = 0.01f;

	inline constexpr float MaxValue_FrameRateLimit = 1000.f;
	inline constexpr float MinValue_FrameRateLimit = 0.f;
	inline constexpr int32 SnapSize_FrameRateLimit = 1;

	inline constexpr float MinValue_Volume = 0.f;
	inline constexpr float MaxValue_Volume = 100.f;
	inline constexpr int32 SnapSize_Volume = 1;

	inline constexpr float MinValue_DLSSSharpness = 0.f;
	inline constexpr float MaxValue_DLSSSharpness = 1.f;
	inline constexpr float SnapSize_DLSSSharpness = 0.01f;

	inline constexpr float MinValue_NISSharpness = 0.f;
	inline constexpr float MaxValue_NISSharpness = 1.f;
	inline constexpr float SnapSize_NISSharpness = 0.01f;

	inline constexpr float MaxValue_ResolutionScale = 100;
	inline constexpr float SnapSize_ResolutionScale = 0.001;

#pragma endregion

#pragma region DefaultGameModes

	inline constexpr float DefaultPlayerDelay = 0.3f;
	inline constexpr float DefaultSpawnBeatDelay = 0.3f;
	inline constexpr float DefaultMinTargetScale = 0.8f;
	inline constexpr float DefaultMaxTargetScale = 2.f;
	inline constexpr float DefaultTargetSpawnCD = 0.35f;
	inline constexpr float DefaultTargetMaxLifeSpan = 1.5f;
	inline constexpr float DefaultMinDistanceBetweenTargets = 10.f;
	inline constexpr int32 DefaultNumCharges = INDEX_NONE;
	inline constexpr float DefaultChargeScaleMultiplier = 1.f;

	inline const FVector BoxBounds_Wide_SingleBeat(0.f, 3200.f, 1000.f);
	inline const FVector BoxBounds_Narrow_SingleBeat(0.f, 1600.f, 500.f);
	inline const FVector BoxBounds_Dynamic_SingleBeat(0.f, 2000.f, 800.f);

	inline const FVector BoxBounds_Wide_MultiBeat(0.f, 3200.f, 1000.f);
	inline const FVector BoxBounds_Narrow_MultiBeat(0.f, 1600.f, 500.f);
	inline const FVector BoxBounds_Dynamic_MultiBeat(0.f, 2000.f, 800.f);

	/* BeatTrack is the only case where PlayerDelay is different from TargetSpawnCD */
	inline constexpr float SpawnBeatDelay_BeatTrack = 0.f;
	inline constexpr float TargetMaxLifeSpan_BeatTrack = 0.f;

	// Normal Difficulty

	inline constexpr int32 NumHorizontalBeatGridTargets_Normal = 5;
	inline constexpr int32 NumVerticalBeatGridTargets_Normal = 5;
	inline constexpr int32 NumTargetsAtOnceBeatGrid_Normal = -1;
	inline const FVector2D BeatGridSpacing_Normal(75, 50);

	inline constexpr float SpawnBeatDelay_SingleBeat_Normal = 0.3f;
	inline constexpr float TargetSpawnCD_SingleBeat_Normal = SpawnBeatDelay_SingleBeat_Normal;
	inline constexpr float TargetMaxLifeSpan_SingleBeat_Normal = 0.8f;
	inline constexpr float MinTargetScale_SingleBeat_Normal = 0.75f;
	inline constexpr float MaxTargetScale_SingleBeat_Normal = 2.f;

	inline constexpr float SpawnBeatDelay_MultiBeat_Normal = 0.35f;
	inline constexpr float TargetSpawnCD_MultiBeat_Normal = SpawnBeatDelay_MultiBeat_Normal;
	inline constexpr float TargetMaxLifeSpan_MultiBeat_Normal = 1.f;
	inline constexpr float MinTargetScale_MultiBeat_Normal = 0.75f;
	inline constexpr float MaxTargetScale_MultiBeat_Normal = 2.f;

	inline constexpr float SpawnBeatDelay_BeatGrid_Normal = 0.30f;
	inline constexpr float TargetSpawnCD_BeatGrid_Normal = SpawnBeatDelay_BeatGrid_Normal;
	inline constexpr float TargetMaxLifeSpan_BeatGrid_Normal = 1.f;
	inline constexpr float MinTargetScale_BeatGrid_Normal = 0.80f;
	inline constexpr float MaxTargetScale_BeatGrid_Normal = 0.80f;

	inline constexpr float TargetSpawnCD_BeatTrack_Normal = 0.75f;
	inline constexpr float MinTargetScale_BeatTrack_Normal = 1.3f;
	inline constexpr float MaxTargetScale_BeatTrack_Normal = 1.3f;
	inline constexpr float MinTrackingSpeed_BeatTrack_Normal = 400.f;
	inline constexpr float MaxTrackingSpeed_BeatTrack_Normal = 500.f;

	inline constexpr float TargetSpawnCD_ChargedBeatTrack_Normal = 0.4f;
	inline constexpr float MinTargetScale_ChargedBeatTrack_Normal = 1.3f;
	inline constexpr float MaxTargetScale_ChargedBeatTrack_Normal = 1.3f;
	inline constexpr float TargetMaxLifeSpan_ChargedBeatTrack_Normal = 0.8f;
	inline constexpr int32 NumCharges_ChargedBeatTrack_Normal = 2;
	inline constexpr float ChargeScaleMultiplier_ChargedBeatTrack_Normal = 0.50f;
	inline constexpr float MinTrackingSpeed_ChargedBeatTrack_Normal = 400.f;
	inline constexpr float MaxTrackingSpeed_ChargedBeatTrack_Normal = 500.f;

	// Hard Difficulty

	inline constexpr int32 NumHorizontalBeatGridTargets_Hard = 8;
	inline constexpr int32 NumVerticalBeatGridTargets_Hard = 8;
	inline constexpr int32 NumTargetsAtOnceBeatGrid_Hard = -1;
	inline const FVector2D BeatGridSpacing_Hard(75, 50);

	inline constexpr float SpawnBeatDelay_SingleBeat_Hard = 0.25f;
	inline constexpr float TargetSpawnCD_SingleBeat_Hard = SpawnBeatDelay_SingleBeat_Hard;
	inline constexpr float TargetMaxLifeSpan_SingleBeat_Hard = 0.65f;
	inline constexpr float MinTargetScale_SingleBeat_Hard = 0.6f;
	inline constexpr float MaxTargetScale_SingleBeat_Hard = 1.5f;

	inline constexpr float SpawnBeatDelay_MultiBeat_Hard = 0.3f;
	inline constexpr float TargetSpawnCD_MultiBeat_Hard = SpawnBeatDelay_MultiBeat_Hard;
	inline constexpr float TargetMaxLifeSpan_MultiBeat_Hard = 0.75f;
	inline constexpr float MinTargetScale_MultiBeat_Hard = 0.6f;
	inline constexpr float MaxTargetScale_MultiBeat_Hard = 1.5f;

	inline constexpr float SpawnBeatDelay_BeatGrid_Hard = 0.25f;
	inline constexpr float TargetSpawnCD_BeatGrid_Hard = SpawnBeatDelay_BeatGrid_Hard;
	inline constexpr float TargetMaxLifeSpan_BeatGrid_Hard = 0.8f;
	inline constexpr float MinTargetScale_BeatGrid_Hard = 0.65f;
	inline constexpr float MaxTargetScale_BeatGrid_Hard = 0.65f;

	inline constexpr float TargetSpawnCD_BeatTrack_Hard = 0.6f;
	inline constexpr float MinTargetScale_BeatTrack_Hard = 1.f;
	inline constexpr float MaxTargetScale_BeatTrack_Hard = 1.f;
	inline constexpr float MinTrackingSpeed_BeatTrack_Hard = 500.f;
	inline constexpr float MaxTrackingSpeed_BeatTrack_Hard = 600.f;

	inline constexpr float TargetSpawnCD_ChargedBeatTrack_Hard = 0.35f;
	inline constexpr float MinTargetScale_ChargedBeatTrack_Hard = 1.f;
	inline constexpr float MaxTargetScale_ChargedBeatTrack_Hard = 1.f;
	inline constexpr float TargetMaxLifeSpan_ChargedBeatTrack_Hard = 0.7f;
	inline constexpr int32 NumCharges_ChargedBeatTrack_Hard = 3;
	inline constexpr float ChargeScaleMultiplier_ChargedBeatTrack_Hard = 0.70f;
	inline constexpr float MinTrackingSpeed_ChargedBeatTrack_Hard = 500.f;
	inline constexpr float MaxTrackingSpeed_ChargedBeatTrack_Hard = 600.f;

	// Death Difficulty

	inline constexpr int32 NumHorizontalBeatGridTargets_Death = 15;
	inline constexpr int32 NumVerticalBeatGridTargets_Death = 10;
	inline constexpr int32 NumTargetsAtOnceBeatGrid_Death = -1;
	inline const FVector2D BeatGridSpacing_Death(75, 50);

	inline constexpr float SpawnBeatDelay_SingleBeat_Death = 0.2f;
	inline constexpr float TargetSpawnCD_SingleBeat_Death = SpawnBeatDelay_SingleBeat_Death;
	inline constexpr float TargetMaxLifeSpan_SingleBeat_Death = 0.45f;
	inline constexpr float MinTargetScale_SingleBeat_Death = 0.4f;
	inline constexpr float MaxTargetScale_SingleBeat_Death = 1.5f;

	inline constexpr float SpawnBeatDelay_MultiBeat_Death = 0.25f;
	inline constexpr float TargetSpawnCD_MultiBeat_Death = SpawnBeatDelay_MultiBeat_Death;
	inline constexpr float TargetMaxLifeSpan_MultiBeat_Death = 0.5f;
	inline constexpr float MinTargetScale_MultiBeat_Death = 0.4f;
	inline constexpr float MaxTargetScale_MultiBeat_Death = 1.25f;

	inline constexpr float SpawnBeatDelay_BeatGrid_Death = 0.25f;
	inline constexpr float TargetSpawnCD_BeatGrid_Death = SpawnBeatDelay_BeatGrid_Death;
	inline constexpr float TargetMaxLifeSpan_BeatGrid_Death = 0.65f;
	inline constexpr float MinTargetScale_BeatGrid_Death = 0.5f;
	inline constexpr float MaxTargetScale_BeatGrid_Death = 0.5f;

	inline constexpr float TargetSpawnCD_BeatTrack_Death = 0.45f;
	inline constexpr float MinTargetScale_BeatTrack_Death = 0.75;
	inline constexpr float MaxTargetScale_BeatTrack_Death = 0.75;
	inline constexpr float MinTrackingSpeed_BeatTrack_Death = 600.f;
	inline constexpr float MaxTrackingSpeed_BeatTrack_Death = 700.f;

	inline constexpr float TargetSpawnCD_ChargedBeatTrack_Death = 0.25f;
	inline constexpr float MinTargetScale_ChargedBeatTrack_Death = 0.75;
	inline constexpr float MaxTargetScale_ChargedBeatTrack_Death = 0.75;
	inline constexpr float TargetMaxLifeSpan_ChargedBeatTrack_Death = 0.5f;
	inline constexpr int32 NumCharges_ChargedBeatTrack_Death = 4;
	inline constexpr float ChargeScaleMultiplier_ChargedBeatTrack_Death = 0.70f;
	inline constexpr float MinTrackingSpeed_ChargedBeatTrack_Death = 600.f;
	inline constexpr float MaxTrackingSpeed_ChargedBeatTrack_Death = 700.f;


#pragma endregion

#pragma region Moon

	/** Offset of the moon from world origin */
	inline const FVector MoonMeshOffset(250000, 0, 250000);

	/** Relative scale of the glow mesh compared to MoonMesh */
	inline const FVector MoonGlowMeshScale(2.f);

	/** Scale of the MoonMesh */
	inline const FVector MoonMeshScale(400, 400, 400);

	/** Scale of the Moon Directional Light Component */
	inline const FVector MoonLightScale(0.0025, 0.0025, 0.0025);

	/** Radius of the Moon's SphereComponent, originally used to rotate the moon around the world, but no longer used  */
	inline constexpr float MoonOrbitRadius = 400000;

#pragma endregion

#pragma region Target

	/** The default location to spawn the SpawnBox */
	inline const FVector DefaultTargetManagerLocation(3700.f, 0.f, 0.f);

	/** The default BoxBounds multiplied by two for the user interface. The real box bounds are 1600 x 500, which creates a box with size 3200 x 1000 */
	inline const FVector DefaultSpawnBoxBounds(0.f, 3200.f, 1000.f);

	/** How much to shrink the target scale to during BeatGrid after successful hit */
	inline constexpr float MinShrinkTargetScale = 0.2f;

	/** Base sphere diameter at 1.0 scale */
	constexpr float SphereTargetDiameter = 100.f;

	/** Base sphere radius at 1.0 scale */
	constexpr float SphereTargetRadius = 50.f;

	/** Default distance between floor and bottom of the SpawnBox */
	inline constexpr float DistanceFromFloor = 110.f;

	/** Distance between floor and HeadshotHeight */
	inline constexpr float HeadshotHeight = 189.5f;

	inline constexpr float BaseTargetHealth = 100.f;

	inline constexpr float BaseTrackingTargetHealth = 1000000.f;

#pragma endregion

#pragma region MinMaxSnapSize

	inline constexpr float MinValue_PlayerDelay = 0.f;
	inline constexpr float MaxValue_PlayerDelay = 0.5f;
	inline constexpr float SnapSize_PlayerDelay = 0.01f;

	inline constexpr float MinValue_Lifespan = 0.25f;
	inline constexpr float MaxValue_Lifespan = 10.f;
	inline constexpr float SnapSize_Lifespan = 0.01f;

	inline constexpr float MinValue_TargetSpawnCD = 0.05f;
	inline constexpr float MaxValue_TargetSpawnCD = 2.f;
	inline constexpr float SnapSize_TargetSpawnCD = 0.01f;

	inline constexpr float MinValue_MinTargetDistance = 0.f;
	inline constexpr float MaxValue_MinTargetDistance = 600.f;
	inline constexpr float SnapSize_MinTargetDistance = 5.f;

	inline constexpr float MinValue_HorizontalSpread = 100.f;
	inline constexpr float MaxValue_HorizontalSpread = 3200.f;
	inline constexpr float SnapSize_HorizontalSpread = 100.f;

	inline constexpr float MinValue_VerticalSpread = 100.f;
	inline constexpr float MaxValue_VerticalSpread = 1000.f;
	inline constexpr float SnapSize_VerticalSpread = 100.f;
	inline constexpr float HeadshotHeight_VerticalSpread = 50.f;

	inline constexpr float MinValue_ForwardSpread = 0.f;
	inline constexpr float MaxValue_ForwardSpread = 2000.f;
	inline constexpr float SnapSize_ForwardSpread = 50.f;

	inline const FVector MinValue_DynamicSpread(0.f, 100.f, 100.f);
	inline const FVector MaxValue_DynamicSpread(2000.f, 3200.f, 1000.f);
	inline const FVector SnapSize_DynamicSpread(50.f, 100.f, 100.f);

	inline constexpr float MinValue_TargetScale = 0.25f;
	inline constexpr float MaxValue_TargetScale = 2.f;
	inline constexpr float SnapSize_TargetScale = 0.01f;

	inline constexpr float MinValue_TargetSpeed = 0.f;
	inline constexpr float MaxValue_TargetSpeed = 1000.f;
	inline constexpr float SnapSize_TargetSpeed = 10.f;

	inline constexpr float MinValue_FloorDistance = 110.f;
	inline constexpr float MaxValue_FloorDistance = 1000.f;
	inline constexpr float SnapSize_FloorDistance = 10.f;

	inline constexpr float MinValue_ConsecutiveChargeScaleMultiplier = 0.25f;
	inline constexpr float MaxValue_ConsecutiveChargeScaleMultiplier = 1.f;
	inline constexpr float SnapSize_ConsecutiveChargeScaleMultiplier = 0.01f;

	inline constexpr int32 MinValue_MaxNumRecentTargets = 0;
	inline constexpr int32 MaxValue_MaxNumRecentTargets = 50;
	inline constexpr int32 SnapSize_MaxNumRecentTargets = 1;

	inline constexpr float MinValue_RecentTargetTimeLength = 0.f;
	inline constexpr float MaxValue_RecentTargetTimeLength = 30.f;
	inline constexpr float SnapSize_RecentTargetTimeLength = 0.05f;

	inline constexpr int32 MinValue_NumUpfrontTargetsToSpawn = 0;
	inline constexpr int32 MaxValue_NumUpfrontTargetsToSpawn = 100;
	inline constexpr int32 SnapSize_NumUpfrontTargetsToSpawn = 1;

	inline constexpr int32 MinValue_SpecificHealthLost = 0;
	inline constexpr int32 MaxValue_SpecificHealthLost = 1000;
	inline constexpr int32 SnapSize_SpecificHealthLost = 10;

	inline constexpr int32 MinValue_ExpirationHealthPenalty = 0;
	inline constexpr int32 MaxValue_ExpirationHealthPenalty = 1000;
	inline constexpr int32 SnapSize_ExpirationHealthPenalty = 10;

	inline constexpr int32 MinValue_MaxHealth = 10;
	inline constexpr int32 MaxValue_MaxHealth = 1000;
	inline constexpr int32 SnapSize_MaxHealth = 10;

	inline constexpr int32 MinValue_NumRuntimeTargetsToSpawn = -1;
	inline constexpr int32 MaxValue_NumRuntimeTargetsToSpawn = 100;
	inline constexpr int32 SnapSize_NumRuntimeTargetsToSpawn = 1;

	inline constexpr int32 MinValue_MaxNumActivatedTargetsAtOnce = -1;
	inline constexpr int32 MaxValue_MaxNumActivatedTargetsAtOnce = 50;
	inline constexpr int32 SnapSize_MaxNumActivatedTargetsAtOnce = 1;

	inline constexpr int32 MinValue_MaxNumTargetsAtOnce = -1;
	inline constexpr int32 MaxValue_MaxNumTargetsAtOnce = 100;
	inline constexpr int32 SnapSize_MaxNumTargetsAtOnce = 1;

	inline constexpr int32 MinValue_MinNumTargetsToActivateAtOnce = -1;
	inline constexpr int32 MaxValue_MinNumTargetsToActivateAtOnce = 25;
	inline constexpr int32 SnapSize_MinNumTargetsToActivateAtOnce = 1;

	inline constexpr int32 MinValue_MaxNumTargetsToActivateAtOnce = -1;
	inline constexpr int32 MaxValue_MaxNumTargetsToActivateAtOnce = 25;
	inline constexpr int32 SnapSize_MaxNumTargetsToActivateAtOnce = 1;

	// AI

	inline constexpr float MinValue_Alpha = 0.1;
	inline constexpr float MaxValue_Alpha = 1;
	inline constexpr float SnapSize_Alpha = 0.01;

	inline constexpr float MinValue_Epsilon = 0;
	inline constexpr float MaxValue_Epsilon = 1;
	inline constexpr float SnapSize_Epsilon = 0.01;

	inline constexpr float MinValue_Gamma = 0.1;
	inline constexpr float MaxValue_Gamma = 1;
	inline constexpr float SnapSize_Gamma = 0.01;

	// BeatGrid

	inline constexpr float MinValue_HorizontalGridSpacing = 10;
	inline constexpr float MaxValue_HorizontalGridSpacing = 3200;
	inline constexpr float SnapSize_HorizontalGridSpacing = 10;

	inline constexpr float MinValue_VerticalGridSpacing = 10;
	inline constexpr float MaxValue_VerticalGridSpacing = 1000;
	inline constexpr float SnapSize_VerticalGridSpacing = 10;

	inline constexpr int32 MinValue_NumHorizontalGridTargets = 2;
	inline constexpr int32 MaxValue_NumHorizontalGridTargets = 35;
	inline constexpr int32 SnapSize_NumHorizontalGridTargets = 1;

	inline constexpr int32 MinValue_NumVerticalGridTargets = 2;
	inline constexpr int32 MaxValue_NumVerticalGridTargets = 15;
	inline constexpr int32 SnapSize_NumVerticalGridTargets = 1;

	/** The min Exposure Compensation for the Post Process Volume */
	inline constexpr float MinValue_ExposureCompensation = -3.f;

	/** The max Exposure Compensation for the Post Process Volume */
	inline constexpr float MaxValue_ExposureCompensation = 5.f;

	inline constexpr float MinValue_Brightness = 0.f;
	inline constexpr float MaxValue_Brightness = 100.f;
	inline constexpr float SnapSize_Brightness = 1.f;

	inline constexpr float MinValue_DisplayGamma = 0.1f;
	inline constexpr float MaxValue_DisplayGamma = 4.3f;
	inline constexpr float SnapSize_DisplayGamma = 0.01f;

	inline constexpr float MinValue_DynamicStartThreshold = 0.f;
	inline constexpr float MaxValue_DynamicStartThreshold = 100.f;
	inline constexpr float SnapSize_DynamicStartThreshold = 1.f;

	inline constexpr float MinValue_DynamicEndThreshold = 0.f;
	inline constexpr float MaxValue_DynamicEndThreshold = 150.f;
	inline constexpr float SnapSize_DynamicEndThreshold = 1.f;

	inline constexpr float MinValue_DynamicDecrementAmount = 1.f;
	inline constexpr float MaxValue_DynamicDecrementAmount = 150.f;
	inline constexpr float SnapSize_DynamicDecrementAmount = 1.f;

#pragma endregion

#pragma region LightVisualizers

	/** Location of the StaticCubeVisualizer to the left of the TargetManager */
	inline const FVector LeftStaticCubeVisualizerLocation(4000, -1935, 210);

	/** Location of the StaticCubeVisualizer to the right of the TargetManager */
	inline const FVector RightStaticCubeVisualizerLocation(4000, 1935, 210);

	/** Rotation for both StaticCubeVisualizers */
	inline const FRotator DefaultStaticCubeVisualizerRotation(0, 90, 90);

	/** How far to space the cubes for the StaticCubeVisualizer */
	inline const FVector DefaultCubeVisualizerOffset(0, -120, 0);

	/** Min scale to apply to the height of a single Cube in a StaticCubeVisualizer */
	inline constexpr float DefaultMinCubeVisualizerHeightScale = 1;

	/** Max scale to apply to the height of a single Cube in a StaticCubeVisualizer */
	inline constexpr float DefaultMaxCubeVisualizerHeightScale = 4;

	// Beam Visualizers

	/** Location of the BeamVisualizer in middle of room */
	inline const FVector DefaultMiddleRoomBeamVisualizerLocation(0, 1920, 1340);

	/** Rotation of the BeamVisualizer in middle of room */
	inline const FRotator DefaultMiddleRoomBeamRotation(0, 0, 0);

	/** Default spacing between BeamVisualizers in middle of room */
	inline const FVector DefaultMiddleRoomBeamLightOffset(0, 100, 0);

	inline constexpr int32 DefaultNumVisualizerLightsToSpawn = 10;

	/** Relative offset of the Spotlight light component from the Spotlight head */
	inline const FRotator DefaultBeamLightRotation(0, 0, 0);

	/** Normal offset to apply to the location of the BeamEndLight */
	inline constexpr float DefaultBeamEndLightOffset = 5;

	/** The max value of the Spotlight intensity */
	inline constexpr float DefaultMaxSpotlightIntensity = 16000000;

	/** The max value of the BeamEndLight intensity */
	inline constexpr float DefaultMaxBeamEndLightIntensity = 80000;

	/** The default value for the inner cone angle of the Spotlight */
	inline constexpr float DefaultSimpleBeamLightInnerConeAngle = 0.5f;

	/** The default value for the inner cone angle of the Spotlight */
	inline constexpr float DefaultSimpleBeamLightOuterConeAngle = 1.5f;

	/** The default value for the inner cone angle of the Spotlight */
	inline constexpr float DefaultSimpleBeamLightBeamWidth = 0.17f;

	/** The default value for the inner cone angle of the Spotlight */
	inline constexpr float DefaultSimpleBeamLightBeamLength = 10.f;

	/** Relative offset of the Spotlight limb from the SpotlightBase */
	inline const FVector DefaultSpotlightLimbOffset(18, 0, 0);

	/** Relative offset of the Spotlight head from the Spotlight limb */
	inline const FVector DefaultSpotlightHeadOffset(40, 0, 0);

	/** Relative offset of the Spotlight light component from the Spotlight head */
	inline const FVector DefaultSpotlightOffset(22, 0, 0);

	/** Default Spectrum of colors to use with BeamVisualizer */
	inline const TArray DefaultBeamLightColors = {
		FLinearColor(255 / 255.f, 0 / 255.f, 0 / 255.f), FLinearColor(255 / 255.f, 127 / 255.f, 0 / 255.f),
		FLinearColor(255 / 255.f, 255 / 255.f, 0 / 255.f), FLinearColor(127 / 255.f, 255 / 255.f, 0 / 255.f),
		FLinearColor(0 / 255.f, 255 / 255.f, 0 / 255.f), FLinearColor(0 / 255.f, 255 / 255.f, 127 / 255.f),
		FLinearColor(0 / 255.f, 255 / 255.f, 255 / 255.f), FLinearColor(0 / 255.f, 127 / 255.f, 255 / 255.f),
		FLinearColor(0 / 255.f, 0 / 255.f, 255 / 255.f), FLinearColor(127 / 255.f, 0 / 255.f, 255 / 255.f)
	};

	/** Default lifetimes for the BeamVisualizer lights */
	inline const TArray DefaultBeamLightLifetimes = {2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f,};

	/** Value to divide the Average spectrum value by, and then subtract from the current spectrum value */
	inline constexpr float CurrentSpectrumValueDecrementDivide = 120.f;

	/** Value to subtract from the current cube spectrum value */
	inline constexpr float CurrentCubeSpectrumValueDecrement = 0.005f;

	/** Value to divide the Average spectrum value by, and then subtract from the max spectrum value */
	inline constexpr float MaxSpectrumValueDecrementDivide = 500.f;

#pragma endregion

#pragma region URLs

	/** base url */
	const FString OriginURL = "https://beatshot.gg";

	/** endpoint, safe to use as standalone URL */
	const FString Endpoint_PatchNotes = OriginURL + "/patchnotes";
	/** endpoint, safe to use as standalone URL */
	const FString Endpoint_Login = OriginURL + "/api/login";
	/** endpoint, safe to use as standalone URL */
	const FString Endpoint_Refresh = OriginURL + "/api/refresh";
	/** endpoint, safe to use as standalone URL */
	const FString Endpoint_SendFeedback = OriginURL + "/api/sendfeedback";

	/** endpoint, safe to use as standalone URL WITH an AuthTicket appended */
	const FString Endpoint_AuthenticateUserTicket = OriginURL + "/login/steam/authenticate/";
	/** endpoint, safe to use as standalone URL WITH an AuthTicket appended */
	const FString Endpoint_AuthenticateUserTicketNoRedirect = OriginURL + "/login/steam/authenticate/noredirect/";

	/** Segment only containing origin+profile+/ , NOT standalone URL*/
	const FString Segment_Profile = OriginURL + "/profile/";
	/** Segment only containing origin+api+profile+/ NOT standalone URL, needs a suffix to be used, like (Segment_ApiProfile + userID + Segment_SaveScores) */
	const FString Segment_ApiProfile = OriginURL + "/api/profile/";
	/** Segment, NOT standalone URL, needs a prefix to be used */
	const FString Segment_SaveScores = "/savescores";
	/** Segment, NOT standalone URL, needs a prefix to be used, like (Segment_Profile+userID+Segment_DefaultModes) */
	const FString Segment_DefaultModes = "/stats/defaultmodes";
	/** Segment, NOT standalone URL, needs a prefix to be used, like (Segment_Profile+userID+Segment_CustomModes) */
	const FString Segment_CustomModes = "/stats/custommodes";
#pragma endregion
}
