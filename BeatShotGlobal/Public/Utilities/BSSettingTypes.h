// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Nvidia DLSS Enabled Mode. */
UENUM(BlueprintType)
enum class EDLSSEnabledMode : uint8
{
	Off UMETA(DisplayName = "Off"),
	On UMETA(DisplayName = "On"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EDLSSEnabledMode, EDLSSEnabledMode::Off, EDLSSEnabledMode::On);

/** Nvidia NIS Enabled Mode. */
UENUM(BlueprintType)
enum class ENISEnabledMode : uint8
{
	Off UMETA(DisplayName = "Off"),
	On UMETA(DisplayName = "On"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ENISEnabledMode, ENISEnabledMode::Off, ENISEnabledMode::On);

/** Scalability Quality Video Settings. */
UENUM()
enum class EVideoSettingType : uint8
{
	AntiAliasing UMETA(DisplayName="AntiAliasing"),
	GlobalIllumination UMETA(DisplayName="GlobalIllumination"),
	PostProcessing UMETA(DisplayName="PostProcessing"),
	Reflection UMETA(DisplayName="Reflection"),
	Shadow UMETA(DisplayName="Shadow"),
	Shading UMETA(DisplayName="Shading"),
	Texture UMETA(DisplayName="Texture"),
	ViewDistance UMETA(DisplayName="ViewDistance"),
	VisualEffect UMETA(DisplayName="VisualEffect")};

ENUM_RANGE_BY_FIRST_AND_LAST(EVideoSettingType, EVideoSettingType::AntiAliasing, EVideoSettingType::VisualEffect);

/** Scalability Quality Video Settings. */
UENUM()
enum class ENvidiaSettingType : uint8
{
	DLSSEnabledMode UMETA(DisplayName="DLSSEnabledMode"),
	FrameGenerationEnabledMode UMETA(DisplayName="FrameGenerationEnabledMode"),
	DLSSMode UMETA(DisplayName="DLSSMode"),
	NISEnabledMode UMETA(DisplayName="NISEnabledMode"),
	NISMode UMETA(DisplayName="NISMode"),
	StreamlineReflexMode UMETA(DisplayName="StreamlineReflexMode"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ENvidiaSettingType, ENvidiaSettingType::DLSSEnabledMode,
	ENvidiaSettingType::StreamlineReflexMode);
