// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BSPlayerSettingsInterface.generated.h"

struct FPlayerSettings;
struct FPlayerSettings_Game;
struct FPlayerSettings_AudioAnalyzer;
struct FPlayerSettings_User;
struct FPlayerSettings_CrossHair;
class USaveGamePlayerSettings;

/** Broadcast when game specific settings are changed and saved. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSettingsChanged_Game, const FPlayerSettings_Game& GameSettings);

/** Broadcast when Audio Analyzer specific settings are changed and saved. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSettingsChanged_AudioAnalyzer,
	const FPlayerSettings_AudioAnalyzer& AudioAnalyzerSettings);

/** Broadcast when User specific settings are changed and saved. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSettingsChanged_User, const FPlayerSettings_User& UserSettings);

/** Broadcast when CrossHair specific settings are changed and saved. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSettingsChanged_CrossHair,
	const FPlayerSettings_CrossHair& CrossHairSettings);

UINTERFACE()
class UBSPlayerSettingsInterface : public UInterface
{
	GENERATED_BODY()
};

/** Gives implementing classes delegates and functions to use for loading and saving player settings. */
class BEATSHOTGLOBAL_API IBSPlayerSettingsInterface
{
	GENERATED_BODY()

public:
	/** @return all player settings loaded from slot. */
	static FPlayerSettings LoadPlayerSettings();

	/** Saves specific sub-setting struct, preserving all other settings.
	 *  @param InSettingsStruct settings to save
	 */
	void SavePlayerSettings(const FPlayerSettings_AudioAnalyzer& InSettingsStruct);

	/** Saves specific sub-setting struct, preserving all other settings.
	 *  @param InSettingsStruct settings to save
	 */
	void SavePlayerSettings(const FPlayerSettings_CrossHair& InSettingsStruct);

	/** Saves specific sub-setting struct, preserving all other settings.
	 *  @param InSettingsStruct settings to save
	 */
	void SavePlayerSettings(const FPlayerSettings_Game& InSettingsStruct);

	/** Saves specific sub-setting struct, preserving all other settings.
	 *  @param InSettingsStruct settings to save
	 */
	void SavePlayerSettings(const FPlayerSettings_User& InSettingsStruct);

protected:
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_AudioAnalyzer& NewAudioAnalyzerSettings)
	{
	}

	virtual void OnPlayerSettingsChanged(const FPlayerSettings_CrossHair& NewCrossHairSettings)
	{
	}

	virtual void OnPlayerSettingsChanged(const FPlayerSettings_Game& NewGameSettings)
	{
	}

	virtual void OnPlayerSettingsChanged(const FPlayerSettings_User& NewUserSettings)
	{
	}

	/** The delegate that is broadcast when this class saves Audio Analyzer settings. */
	FOnPlayerSettingsChanged_AudioAnalyzer OnPlayerSettingsChangedDelegate_AudioAnalyzer;

	/** The delegate that is broadcast when this class saves CrossHair settings. */
	FOnPlayerSettingsChanged_CrossHair OnPlayerSettingsChangedDelegate_CrossHair;

	/** The delegate that is broadcast when this class saves Game settings. */
	FOnPlayerSettingsChanged_Game OnPlayerSettingsChangedDelegate_Game;

	/** The delegate that is broadcast when this class saves User settings. */
	FOnPlayerSettingsChanged_User OnPlayerSettingsChangedDelegate_User;
};
