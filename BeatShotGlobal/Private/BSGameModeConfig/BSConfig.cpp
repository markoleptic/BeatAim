// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "BSGameModeConfig/BSConfig.h"
#include "JsonObjectConverter.h"


FBSConfig::FBSConfig()
{
	DefiningConfig = FBS_DefiningConfig();
	AIConfig = FBS_AIConfig();
	AudioConfig = FBS_AudioConfig();
	GridConfig = FBS_GridConfig();
	TargetConfig = FBS_TargetConfig();
	DynamicSpawnAreaScaling = FBS_Dynamic_SpawnArea();
	DynamicTargetScaling = FBS_Dynamic();
}

FBSConfig::FBSConfig(const EBaseGameMode InBaseGameMode, const EGameModeDifficulty InDifficulty)
{
	DefiningConfig = GetConfigForPreset(InBaseGameMode, InDifficulty);
	AIConfig = FBS_AIConfig();
	AudioConfig = FBS_AudioConfig();
	GridConfig = FBS_GridConfig();
	TargetConfig = FBS_TargetConfig();
}

FBS_DefiningConfig FBSConfig::GetConfigForPreset(const EBaseGameMode InBaseGameMode,
	const EGameModeDifficulty InDifficulty)
{
	FBS_DefiningConfig Config;
	Config.BaseGameMode = InBaseGameMode;
	Config.Difficulty = InDifficulty;
	Config.GameModeType = EGameModeType::Preset;
	Config.CustomGameModeName = "";
	return Config;
}

void FBSConfig::OnCreate()
{
	// Override the player delay to zero if using Capture
	if (AudioConfig.AudioFormat == EAudioFormat::Capture || AudioConfig.AudioFormat == EAudioFormat::Loopback)
	{
		AudioConfig.PlayerDelay = 0.f;
		TargetConfig.SpawnBeatDelay = 0.f;
	}
	AudioConfig.PlayerDelay = TargetConfig.SpawnBeatDelay;

	// Set the Reinforcement Learning Mode
	if (IsCompatibleWithReinforcementLearning())
	{
		AIConfig.ReinforcementLearningMode = AIConfig.bEnableReinforcementLearning
			? EReinforcementLearningMode::ActiveAgent
			: EReinforcementLearningMode::Training;
	}
	else
	{
		AIConfig.ReinforcementLearningMode = EReinforcementLearningMode::None;
	}
}

void FBSConfig::OnCreate_Custom()
{
	OnCreate();
	// Override GameModeType to always be Custom
	DefiningConfig.GameModeType = EGameModeType::Custom;
	// Override Difficulty to always be None
	DefiningConfig.Difficulty = EGameModeDifficulty::None;
}

bool FBSConfig::IsCompatibleWithReinforcementLearning() const
{
	if (TargetConfig.TargetDamageType == ETargetDamageType::Tracking)
	{
		return false;
	}
	return true;
}

void FBSConfig::InitColors(const bool bUseSeparateOutlineColor, const FLinearColor& Inactive,
	const FLinearColor& Outline, const FLinearColor& Start, const FLinearColor& Peak, const FLinearColor& End,
	const FLinearColor& TrackingDam, const FLinearColor& NotTrackingDam)
{
	TargetConfig.bUseSeparateOutlineColor = bUseSeparateOutlineColor;
	TargetConfig.OutlineColor = Outline;

	if (TargetConfig.TargetSpawningPolicy == ETargetSpawningPolicy::UpfrontOnly)
	{
		if (TargetConfig.TargetDamageType == ETargetDamageType::Tracking)
		{
			TargetConfig.OnSpawnColor = NotTrackingDam;
		}
		else
		{
			TargetConfig.OnSpawnColor = Inactive;
		}
	}
	else if (TargetConfig.TargetSpawningPolicy == ETargetSpawningPolicy::RuntimeOnly)
	{
		if (TargetConfig.TargetDamageType == ETargetDamageType::Tracking)
		{
			TargetConfig.OnSpawnColor = NotTrackingDam;
		}
		else if (TargetConfig.bAllowSpawnWithoutActivation)
		{
			TargetConfig.OnSpawnColor = Inactive;
		}
		else
		{
			TargetConfig.OnSpawnColor = Start;
		}
	}

	TargetConfig.InactiveTargetColor = Inactive;
	TargetConfig.StartColor = Start;
	TargetConfig.PeakColor = Peak;
	TargetConfig.EndColor = End;
	TargetConfig.TakingTrackingDamageColor = TrackingDam;
	TargetConfig.NotTakingTrackingDamageColor = NotTrackingDam;
}

FString FBSConfig::ToString() const
{
	const TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject<FBSConfig>(*this, 0,
		CPF_Transient | CPF_SkipSerialization, nullptr);
	if (JsonObject.IsValid())
	{
		FString JsonString;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
		return JsonString;
	}
	return FString();
}

FString FBSConfig::EncodeToString() const
{
	const FString JsonString = ToString();
	if (!JsonString.IsEmpty())
	{
		return FBase64::Encode(JsonString);
	}
	return FString();
}

bool FBSConfig::FromString(const FString& JsonString, FBSConfig& OutConfig, FText* OutFailReason)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		return FJsonObjectConverter::JsonObjectToUStruct<FBSConfig>(JsonObject.ToSharedRef(), &OutConfig, 0,
			CPF_Transient | CPF_SkipSerialization, false, OutFailReason);
	}
	return false;
}

bool FBSConfig::DecodeFromString(const FString& EncodedString, FBSConfig& OutConfig, FText* OutFailReason)
{
	TArray<uint8> DecodedBytes;
	if (FBase64::Decode(EncodedString, DecodedBytes))
	{
		const FString JsonString(DecodedBytes.Num(), UTF8_TO_TCHAR(DecodedBytes.GetData()));

		// Deserialize the JSON string to the struct
		return FromString(JsonString, OutConfig, OutFailReason);
	}
	return false;
}
