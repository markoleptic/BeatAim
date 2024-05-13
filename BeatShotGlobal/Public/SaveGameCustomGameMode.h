// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSGameModeDataAsset.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameCustomGameMode.generated.h"

UCLASS()
class BEATSHOTGLOBAL_API USaveGameCustomGameMode : public USaveGame
{
	GENERATED_BODY()

public:
	USaveGameCustomGameMode();

	virtual void Serialize(FStructuredArchive::FRecord Record) override;

	/** Returns a copy of CustomGameModes */
	TArray<FBSConfig> GetCustomGameModes() const;

	/** Returns true if found custom game mode and copied to OutConfig */
	bool FindCustomGameMode(const FString& GameModeName, FBSConfig& OutConfig) const;

	/** Adds a new entry to CustomGameModes or overwrites one if found with matching FBS_DefiningConfig */
	void SaveCustomGameMode(const FBSConfig& InCustomGameMode);

	/** Removes all instances InCustomGameModeToRemove from the CustomGameModes array, returning the number of removed elements */
	int32 RemoveCustomGameMode(const FBSConfig& InCustomGameMode);

	/** Empties the CustomGameModes array, returning the number of removed elements */
	int32 RemoveAll();

	/** Returns true if there is a CustomGameMode matching the GameModeName */
	bool IsCustomGameMode(const FString& GameModeName) const;

	/** Returns the version of the SaveGame */
	int32 GetVersion() const { return Version; }

	/** Returns the last loaded version of the SaveGame */
	int32 GetLastLoadedVersion() const { return LastLoadedVersion; }

	/** Upgrades all Custom Game Modes in CustomGameModes to the latest Version */
	void UpgradeCustomGameModes();

	/** Upgrades an FBSConfig struct to Version 1 from Version 0 */
	static void UpgradeCustomGameModeToVersion1(FBSConfig& InConfig);

private:
	UPROPERTY()
	TArray<FBSConfig> CustomGameModes;

	UPROPERTY()
	int32 Version = 0;

	UPROPERTY(Transient)
	int32 LastLoadedVersion = -1;
};
