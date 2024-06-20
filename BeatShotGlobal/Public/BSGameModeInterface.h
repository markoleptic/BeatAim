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

UENUM()
enum class ECustomGameModeImportResult : uint8
{
	Success,
	InvalidImportString,
	DefaultGameMode,
	EmptyCustomGameModeName,
	Existing
};

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
	/** @return all saved Custom Game Modes. */
	static TArray<FBSConfig> LoadCustomGameModes();

	/** Attempts to find a saved custom game mode.
	 *  @param CustomGameModeName to search for
	 *  @param OutConfig the Custom Game Mode if found, or a default if not
	 *  @return true if found
	 */
	static bool FindCustomGameMode(const FString& CustomGameModeName, FBSConfig& OutConfig);

	/** Saves a custom game mode to slot.
	 *  @param ConfigToSave to configuration to save, overriding any if matching exist
	 */
	static void SaveCustomGameMode(const FBSConfig& ConfigToSave);

	/** Removes a custom game modeand saves to slot. Also removes the FCommonScoreInfo.
	 *  @param ConfigToRemove Configuration to search for
	 */
	static int32 RemoveCustomGameMode(const FBSConfig& ConfigToRemove);

	/** Removes all custom game modes and saves to slot. Also removes the FCommonScoreInfos for all custom game modes
	 *  from SaveGamePlayerScore.
	 *  @return number of custom game modes removed
	 */
	static int32 RemoveAllCustomGameModes();

	/**
	 *  @param GameModeName game mode name to search for
	 *  @return whether the GameModeName is already a custom game mode name
	 */
	static bool IsCustomGameMode(const FString& GameModeName);

	/**
	 *  @param CustomGameModeName game mode name to search for
	 *  @param InConfig game mode configuration to compare
	 *  @return Returns whether the CustomGameMode is Custom and identical to the config
	 */
	static bool DoesCustomGameModeMatchConfig(const FString& CustomGameModeName, const FBSConfig& InConfig);

	/** Attempts to initialize a given config using the serialized json string. Returns true on success.
	 *  @param InSerializedJsonString serialized json string
	 *  @param OutConfig game mode configuration created from json string if successful
	 *  @param OutDecodeFailureReason contains reason for decode failure if decode failure
	 *  @return the import result as an enum
	 */
	static ECustomGameModeImportResult ImportCustomGameMode(const FString& InSerializedJsonString, FBSConfig& OutConfig,
		FText& OutDecodeFailureReason);

	/** Creates a serialized Json object export string from an FBSConfig
	 *  @param InConfig game mode configuration to create serialized json string from
	 */
	static FString ExportCustomGameMode(const FBSConfig& InConfig);

	/** Finds a preset game mode.
	 *  @param GameModeName preset game mode name to search for
	 *  @param Difficulty difficulty to search for
	 *  @param OutConfig game mode configuration found from the default game mode and difficulty
	 *  @param PresetGameModeDataAsset data asset containing preset game modes
	 *  @return true if found Config corresponding to the input GameModeName string and difficulty
	 */
	static bool FindPresetGameMode(const FString& GameModeName, const EGameModeDifficulty& Difficulty,
		const UBSGameModeDataAsset* PresetGameModeDataAsset, FBSConfig& OutConfig);

	/** Finds a preset game mode.
	 *  @param BaseGameMode preset game mode name to search for
	 *  @param Difficulty difficulty to search for
	 *  @param OutConfig game mode configuration found from the default game mode and difficulty
	 *  @param PresetGameModeDataAsset data asset containing preset game modes
	 *  @return true if found Config corresponding to the input GameModeName string and difficulty
	 */
	static bool FindPresetGameMode(const EBaseGameMode& BaseGameMode, const EGameModeDifficulty& Difficulty,
		const UBSGameModeDataAsset* PresetGameModeDataAsset, FBSConfig& OutConfig);

	/** 
     *  @param GameModeName preset game mode name to search for
	 *  @return whether the GameModeName is part of the default game modes
	 */
	static bool IsPresetGameMode(const FString& GameModeName);
};
