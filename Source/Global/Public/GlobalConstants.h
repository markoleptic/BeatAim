﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

namespace Constants
{
	/** Min Value allowed for a BandFrequency channel */
	inline constexpr float MinValue_BandFrequency = 0;

	/** Max Value allowed for a BandFrequency channel */
	inline constexpr float MaxValue_BandFrequency = 22720;

	/** The length of the countdown timer */
	inline constexpr int32 CountdownTimerLength = 3;

	/** The value to divide the game sensitivity by to convert to Csgo sensitivity */
	inline constexpr float CsgoMultiplier = 3.18;

	/** The distance to trace the line */
	inline constexpr float TraceDistance = 999999;

	/** Max floor height for the MovablePlatform */
	inline const FVector MaxFloorHeight(-4000, 0, 500);

	/** Min floor height for the MovablePlatform */
	inline const FVector MinFloorHeight(-4000, 0, 0);

	/** Default Speed of the day to night transition */
	inline constexpr float DayNightCycleSpeed = 20;

	/** Length of time until video settings are reset */
	inline constexpr float VideoSettingsTimeoutLength = 10.f;

	/** Lighter background menu brush color */
	inline constexpr FLinearColor LightMenuBrushColor(0, 0, 0, 0.1);

	/** Darker background menu brush color */
	inline constexpr FLinearColor DarkMenuBrushColor(0, 0, 0, 0.2);

	/** The color used to change the GameModeButton color to when selected */
	inline constexpr FLinearColor BeatShotBlue(0.049707, 0.571125, 0.83077, 1.0);

#pragma region DefaultSettings

	/** The default Band Limit Thresholds for the AudioAnalyzer */
	inline const TArray DefaultBandLimits = {
		FVector2d(0, 44),
		FVector2d(45, 88),
		FVector2d(89, 177),
		FVector2d(178, 355),
		FVector2d(356, 710),
		FVector2d(711, 1420),
		FVector2d(1421, 2840),
		FVector2d(2841, 5680),
		FVector2d(5681, 11360),
		FVector2d(11361, 22720)};

	inline constexpr float DefaultSensitivity = 0.3f;

	inline constexpr float DefaultAlpha = 0.9f;
	inline constexpr float DefaultEpsilon = 0.9f;
	inline constexpr float DefaultGamma = 0.9f;

	inline constexpr int32 DefaultCombatTextFrequency = 5;
	const FLinearColor DefaultStartTargetColor = FLinearColor::White;
	const FLinearColor DefaultPeakTargetColor = FLinearColor::Green;
	const FLinearColor DefaultEndTargetColor = FLinearColor::Red;
	const FLinearColor DefaultTargetOutlineColor = FLinearColor::White;
	inline constexpr FLinearColor DefaultBeatGridInactiveTargetColor(83.f / 255.f, 0.f, 245.f / 255.f, 1.f);

	inline constexpr float DefaultGlobalVolume = 50.f;
	inline constexpr float DefaultMenuVolume = 50.f;
	inline constexpr float DefaultMusicVolume = 10.f;
	inline constexpr int32 DefaultFrameRateLimitMenu = 60.f;
	inline constexpr int32 DefaultFrameRateLimitGame = 0.f;
	
	inline constexpr float DefaultBandLimitThreshold = 2.1f;
	inline constexpr int32 DefaultNumBandChannels = 10;
	inline constexpr float DefaultTimeWindow = 0.02f;
	inline constexpr int32 DefaultHistorySize = 30;
	inline constexpr int32 DefaultMaxNumBandChannels = 32;

	inline constexpr int32 DefaultLineWidth = 4;
	inline constexpr int32 DefaultLineLength = 10;
	inline constexpr int32 DefaultInnerOffset = 6;
	inline constexpr FLinearColor DefaultCrossHairColor(63.f / 255.f, 199.f / 255.f, 235.f / 255.f, 1.f);
	inline constexpr float DefaultOutlineOpacity = 1.f;
	inline constexpr int32 DefaultOutlineWidth = 20;

#pragma endregion

#pragma region DefaultGameModes

	inline constexpr float DefaultPlayerDelay = 0.3f;
	inline constexpr float DefaultMinTargetScale = 0.8f;
	inline constexpr float DefaultMaxTargetScale = 2.f;
	inline constexpr float DefaultTargetSpawnCD = 0.35f;
	inline constexpr float DefaultTargetMaxLifeSpan = 1.5f;
	inline constexpr float DefaultSpawnBeatDelay = 0.3f;
	inline constexpr float DefaultMinDistanceBetweenTargets = 10.f;

	inline const FVector BoxBounds_Wide_SingleBeat(0.f, 3200.f, 1000.f);
	inline const FVector BoxBounds_Narrow_SingleBeat(0.f, 1600.f, 500.f);
	inline const FVector BoxBounds_Dynamic_SingleBeat(0.f, 2000.f, 800.f);

	inline const FVector BoxBounds_Wide_MultiBeat(0.f, 3200.f, 1000.f);
	inline const FVector BoxBounds_Narrow_MultiBeat(0.f, 1600.f, 500.f);
	inline const FVector BoxBounds_Dynamic_MultiBeat(0.f, 2000.f, 800.f);

	/* BeatTrack is the only case where PlayerDelay is different from TargetSpawnCD */
	inline constexpr float PlayerDelay_BeatTrack = 0.f;
	inline constexpr float TargetMaxLifeSpan_BeatTrack = 0.f;
	
	// Normal Difficulty

	inline constexpr int32 NumHorizontalBeatGridTargets_Normal = 5;
	inline constexpr int32 NumVerticalBeatGridTargets_Normal = 5;
	inline constexpr int32 NumTargetsAtOnceBeatGrid_Normal = -1;
	inline const FVector2D BeatGridSpacing_Normal(75, 50);
	inline constexpr float MinTrackingSpeed_Normal = 400.f;
	inline constexpr float MaxTrackingSpeed_Normal = 500.f;
	
	inline constexpr float PlayerDelay_SingleBeat_Normal = 0.3f;
	inline constexpr float TargetSpawnCD_SingleBeat_Normal = PlayerDelay_SingleBeat_Normal;
	inline constexpr float TargetMaxLifeSpan_SingleBeat_Normal = 0.8f;
	inline constexpr float MinTargetScale_SingleBeat_Normal = 0.75f;
	inline constexpr float MaxTargetScale_SingleBeat_Normal = 2.f;

	inline constexpr float PlayerDelay_MultiBeat_Normal = 0.35f;
	inline constexpr float TargetSpawnCD_MultiBeat_Normal = PlayerDelay_MultiBeat_Normal;
	inline constexpr float TargetMaxLifeSpan_MultiBeat_Normal = 1.f;
	inline constexpr float MinTargetScale_MultiBeat_Normal = 0.75f;
	inline constexpr float MaxTargetScale_MultiBeat_Normal = 2.f;

	inline constexpr float PlayerDelay_BeatGrid_Normal = 0.30f;
	inline constexpr float TargetSpawnCD_BeatGrid_Normal = PlayerDelay_BeatGrid_Normal;
	inline constexpr float TargetMaxLifeSpan_BeatGrid_Normal = 1.f;
	inline constexpr float MinTargetScale_BeatGrid_Normal = 0.80f;
	inline constexpr float MaxTargetScale_BeatGrid_Normal = 0.80f;
	
	inline constexpr float TargetSpawnCD_BeatTrack_Normal = 0.75f;
	inline constexpr float MinTargetScale_BeatTrack_Normal = 1.3f;
	inline constexpr float MaxTargetScale_BeatTrack_Normal = 1.3f;
	
	// Hard Difficulty
	
	inline constexpr int32 NumHorizontalBeatGridTargets_Hard = 8;
	inline constexpr int32 NumVerticalBeatGridTargets_Hard = 8;
	inline constexpr int32 NumTargetsAtOnceBeatGrid_Hard = -1;
	inline const FVector2D BeatGridSpacing_Hard(75, 50);
	inline constexpr float MinTrackingSpeed_Hard = 500.f;
	inline constexpr float MaxTrackingSpeed_Hard = 600.f;
	
	inline constexpr float PlayerDelay_SingleBeat_Hard = 0.25f;
	inline constexpr float TargetSpawnCD_SingleBeat_Hard = PlayerDelay_SingleBeat_Hard;
	inline constexpr float TargetMaxLifeSpan_SingleBeat_Hard = 0.65f;
	inline constexpr float MinTargetScale_SingleBeat_Hard = 0.6f;
	inline constexpr float MaxTargetScale_SingleBeat_Hard = 1.5f;

	inline constexpr float PlayerDelay_MultiBeat_Hard = 0.3f;
	inline constexpr float TargetSpawnCD_MultiBeat_Hard = PlayerDelay_MultiBeat_Hard;
	inline constexpr float TargetMaxLifeSpan_MultiBeat_Hard = 0.75f;
	inline constexpr float MinTargetScale_MultiBeat_Hard = 0.6f;
	inline constexpr float MaxTargetScale_MultiBeat_Hard = 1.5f;

	inline constexpr float PlayerDelay_BeatGrid_Hard = 0.25f;
	inline constexpr float TargetSpawnCD_BeatGrid_Hard = PlayerDelay_BeatGrid_Hard;
	inline constexpr float TargetMaxLifeSpan_BeatGrid_Hard = 0.8f;
	inline constexpr float MinTargetScale_BeatGrid_Hard = 0.65f;
	inline constexpr float MaxTargetScale_BeatGrid_Hard = 0.65f;
	
	inline constexpr float TargetSpawnCD_BeatTrack_Hard = 0.6f;
	inline constexpr float MinTargetScale_BeatTrack_Hard = 1.f;
	inline constexpr float MaxTargetScale_BeatTrack_Hard = 1.f;
	
	// Death Difficulty
	
	inline constexpr int32 NumHorizontalBeatGridTargets_Death = 15;
	inline constexpr int32 NumVerticalBeatGridTargets_Death = 10;
	inline constexpr int32 NumTargetsAtOnceBeatGrid_Death = -1;
	inline const FVector2D BeatGridSpacing_Death(75, 50);
	inline constexpr float MinTrackingSpeed_Death = 600.f;
	inline constexpr float MaxTrackingSpeed_Death = 700.f;
	
	inline constexpr float PlayerDelay_SingleBeat_Death = 0.2f;
	inline constexpr float TargetSpawnCD_SingleBeat_Death = PlayerDelay_SingleBeat_Death;
	inline constexpr float TargetMaxLifeSpan_SingleBeat_Death = 0.45f;
	inline constexpr float MinTargetScale_SingleBeat_Death = 0.4f;
	inline constexpr float MaxTargetScale_SingleBeat_Death= 1.5f;

	inline constexpr float PlayerDelay_MultiBeat_Death = 0.25f;
	inline constexpr float TargetSpawnCD_MultiBeat_Death = PlayerDelay_MultiBeat_Death;
	inline constexpr float TargetMaxLifeSpan_MultiBeat_Death = 0.5f;
	inline constexpr float MinTargetScale_MultiBeat_Death = 0.4f;
	inline constexpr float MaxTargetScale_MultiBeat_Death = 1.25f;

	inline constexpr float PlayerDelay_BeatGrid_Death = 0.25f;
	inline constexpr float TargetSpawnCD_BeatGrid_Death = PlayerDelay_BeatGrid_Death;
	inline constexpr float TargetMaxLifeSpan_BeatGrid_Death = 0.65f;
	inline constexpr float MinTargetScale_BeatGrid_Death = 0.5f;
	inline constexpr float MaxTargetScale_BeatGrid_Death = 0.5f;
	
	inline constexpr float TargetSpawnCD_BeatTrack_Death = 0.45f;
	inline constexpr float MinTargetScale_BeatTrack_Death = 0.75;
	inline constexpr float MaxTargetScale_BeatTrack_Death = 0.75;

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
	inline const FVector DefaultTargetSpawnerLocation(3700.f, 0.f, 160.f);
	
	/** The default BoxBounds multiplied by two for the user interface. The real box bounds are 1600 x 500, which creates a box with size 3200 x 1000 */
	inline const FVector DefaultSpawnBoxBounds(0.f, 3200.f, 1000.f);

	/** How much to shrink the target scale to during BeatGrid after successful hit */
	inline constexpr float MinShrinkTargetScale = 0.1f;

	/** Base sphere diameter at 1.0 scale */
	constexpr float SphereTargetDiameter = 100.f;

	/** Base sphere radius at 1.0 scale */
	constexpr float SphereTargetRadius = 50.f;

	/** Default distance between floor and bottom of the SpawnBox */
	inline constexpr float DistanceFromFloor = 110.f;

	/** Distance between floor and HeadshotHeight */
	inline constexpr float HeadshotHeight = 160.f;
	
#pragma endregion

#pragma region MinMaxSnapSize
	
	inline constexpr float MinValue_PlayerDelay = 0;
	inline constexpr float MaxValue_PlayerDelay = 0.5;
	inline constexpr float SnapSize_PlayerDelay = 0.01;

	inline constexpr float MinValue_Lifespan = 0.25;
	inline constexpr float MaxValue_Lifespan = 2;
	inline constexpr float SnapSize_Lifespan = 0.01;

	inline constexpr float MinValue_TargetSpawnCD = 0.05;
	inline constexpr float MaxValue_TargetSpawnCD = 2;
	inline constexpr float SnapSize_TargetSpawnCD = 0.01;

	inline constexpr float MinValue_MinTargetDistance = 0;
	inline constexpr float MaxValue_MinTargetDistance = 600;
	inline constexpr float SnapSize_MinTargetDistance = 5;

	inline constexpr float MinValue_HorizontalSpread = 200;
	inline constexpr float MaxValue_HorizontalSpread = 3200;
	inline constexpr float SnapSize_HorizontalSpread = 100;
	
	inline constexpr float MinValue_VerticalSpread = 200;
	inline constexpr float MaxValue_VerticalSpread = 1000;
	inline constexpr float SnapSize_VerticalSpread = 100;
	
	inline constexpr float MinValue_ForwardSpread = 100;
	inline constexpr float MaxValue_ForwardSpread = 5000;
	
	inline constexpr float MinValue_TargetScale = 0.1;
	inline constexpr float MaxValue_TargetScale = 2;
	inline constexpr float SnapSize_TargetScale = 0.01;

	inline constexpr float MinValue_TargetSpeed = 300;
	inline constexpr float MaxValue_TargetSpeed = 1000;
	inline constexpr float SnapSize_TargetSpeed = 10;

	inline constexpr float MinValue_FloorDistance = 110;
	inline constexpr float MaxValue_FloorDistance = 1000;
	inline constexpr float SnapSize_FloorDistance = 10;
	
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
	
	inline constexpr float MinValue_BeatGridHorizontalSpacing = 10;
	inline constexpr float MaxValue_BeatGridHorizontalSpacing = 3200;
	inline constexpr float SnapSize_BeatGridHorizontalSpacing = 10;

	inline constexpr float MinValue_BeatGridVerticalSpacing = 10;
	inline constexpr float MaxValue_BeatGridVerticalSpacing = 1000;
	inline constexpr float SnapSize_BeatGridVerticalSpacing = 10;
	
	inline constexpr int32 MinValue_NumBeatGridHorizontalTargets = 2;
	inline constexpr int32 MaxValue_NumBeatGridHorizontalTargets = 35;
	inline constexpr int32 SnapSize_NumBeatGridHorizontalTargets = 1;

	inline constexpr int32 MinValue_NumBeatGridVerticalTargets = 2;
	inline constexpr int32 MaxValue_NumBeatGridVerticalTargets = 15;
	inline constexpr int32 SnapSize_NumBeatGridVerticalTargets = 1;

#pragma endregion

#pragma region LightVisualizers
	
	/** Location of the StaticCubeVisualizer to the left of the TargetSpawner */
	inline const FVector LeftStaticCubeVisualizerLocation(4000, -1935, 210);

	/** Location of the StaticCubeVisualizer to the right of the TargetSpawner */
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
	inline const FVector DefaultMiddleRoomBeamVisualizerLocation(0, 1920, 1320);

	/** Rotation of the BeamVisualizer in middle of room */
	inline const FRotator DefaultMiddleRoomBeamRotation(0, 0, 0);

	/** Location of the first spawned Light Beam Visualizer in middle of room */
	inline const FVector DefaultMiddleRoomInitialBeamLightLocation(0, 0, 1340);

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
	inline const FVector DefaultSpotlightLimbOffset(0, 0, -18);

	/** Relative offset of the Spotlight head from the Spotlight limb */
	inline const FVector DefaultSpotlightHeadOffset(0, 0, -41);

	/** Relative rotation of the Spotlight head from the Spotlight limb */
	inline const FRotator DefaultSpotlightHeadRotation(-90, 0, 0);

	/** Relative offset of the Spotlight light component from the Spotlight head */
	inline const FVector DefaultSpotlightOffset(22, 0, 0);

	/** Default Spectrum of colors to use with BeamVisualizer */
	inline const TArray DefaultBeamLightColors = {
		FLinearColor(255 / 255.f, 0 / 255.f, 0 / 255.f),
		FLinearColor(255 / 255.f, 127 / 255.f, 0 / 255.f),
		FLinearColor(255 / 255.f, 255 / 255.f, 0 / 255.f),
		FLinearColor(127 / 255.f, 255 / 255.f, 0 / 255.f),
		FLinearColor(0 / 255.f, 255 / 255.f, 0 / 255.f),
		FLinearColor(0 / 255.f, 255 / 255.f, 127 / 255.f),
		FLinearColor(0 / 255.f, 255 / 255.f, 255 / 255.f),
		FLinearColor(0 / 255.f, 127 / 255.f, 255 / 255.f),
		FLinearColor(0 / 255.f, 0 / 255.f, 255 / 255.f),
		FLinearColor(127 / 255.f, 0 / 255.f, 255 / 255.f)
	};

	/** Default lifetimes for the BeamVisualizer lights */
	inline const TArray DefaultBeamLightLifetimes = {
		2.f,
		2.f,
		2.f,
		2.f,
		2.f,
		2.f,
		2.f,
		2.f,
		2.f,
		2.f,
	};
	
#pragma endregion
	
}
