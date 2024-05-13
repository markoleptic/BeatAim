// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BSGameModeInterface.generated.h"

class UBSGameModeDataAsset;
class USaveGameCustomGameMode;
enum class EBaseGameMode : uint8;
enum class EGameModeDifficulty : uint8;
struct FBSConfig;

UINTERFACE()
class UBSGameModeInterface : public UInterface
{
	GENERATED_BODY()
};

/** Interface to allow classes in this game to query, load, and save Custom Game Modes. */
class BEATSHOTGLOBAL_API IBSGameModeInterface
{
	GENERATED_BODY()

public:
	/** Returns all Custom Game Modes. */
	static TArray<FBSConfig> LoadCustomGameModes();

	/** Returns true if the Custom Game Mode was found and copied to OutConfig. */
	static bool FindCustomGameMode(const FString& CustomGameModeName, FBSConfig& OutConfig);

	/** Saves a custom game mode to slot, checking if any matching exist and overriding if they do. */
	static void SaveCustomGameMode(const FBSConfig& ConfigToSave);

	/** Removes a custom game mode from SaveGameCustomGameMode and saves to slot. Also removes the FCommonScoreInfo
	 *  from SaveGamePlayerScore. */
	static int32 RemoveCustomGameMode(const FBSConfig& ConfigToRemove);

	/** Removes all custom game modes and saves to slot. Also removes the FCommonScoreInfos for all custom game modes
	 *  from SaveGamePlayerScore. */
	static int32 RemoveAllCustomGameModes();

	/** Returns whether or not the GameModeName is already a custom game mode name. */
	static bool IsCustomGameMode(const FString& GameModeName);

	/** Returns whether or not the CustomGameMode is Custom and identical to the config. */
	static bool DoesCustomGameModeMatchConfig(const FString& CustomGameModeName, const FBSConfig& InConfig);

	/** Attempts to initialize a given config using the serialized json string. Returns true on success. */
	static bool ImportCustomGameMode(const FString& InSerializedJsonString, FBSConfig& OutConfig,
		FText& OutFailureReason);

	/** Creates a serialized Json object export string from an FBSConfig .*/
	static FString ExportCustomGameMode(const FBSConfig& InConfig);

	/** Returns true if found Config corresponding to the input GameModeName string and difficulty,
	 *  and copies to OutConfig. */
	static bool FindPresetGameMode(const FString& GameModeName, const EGameModeDifficulty& Difficulty,
		const UBSGameModeDataAsset* PresetGameModeDataAsset, FBSConfig& OutConfig);

	/** Returns true if found Config corresponding to the input BaseGameMode and difficulty,
	 *  and copies to OutConfig. */
	static bool FindPresetGameMode(const EBaseGameMode& BaseGameMode, const EGameModeDifficulty& Difficulty,
		const UBSGameModeDataAsset* PresetGameModeDataAsset, FBSConfig& OutConfig);

	/** Returns whether or not the GameModeName is part of the game's default game modes. */
	static bool IsPresetGameMode(const FString& GameModeName);
};
