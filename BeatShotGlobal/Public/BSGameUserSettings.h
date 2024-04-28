// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AudioMixerBlueprintLibrary.h"
#include "GameFramework/GameUserSettings.h"
#include "BSGameUserSettings.generated.h"

class UVideoSettingEnumTagMap;
enum class ENvidiaSettingType : uint8;
enum class UStreamlineReflexMode : uint8;
enum class UNISMode : uint8;
enum class UDLSSMode : uint8;
enum class UStreamlineDLSSGMode : uint8;
enum class ENISEnabledMode : uint8;
enum class EDLSSEnabledMode : uint8;
class USoundControlBusMix;
class USoundControlBus;

DECLARE_LOG_CATEGORY_EXTERN(LogBSGameUserSettings, Log, All);

/** Video and Sound Settings that are saved to GameUserSettings.ini */
UCLASS(config=GameUserSettings, configdonotcheckdefaults)
class BEATSHOTGLOBAL_API UBSGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UBSGameUserSettings();

	/** @return the game local machine settings (resolution, windowing mode, scalability settings, etc...) */
	static UBSGameUserSettings* Get();

	void InitIfNecessary();

private:
	/** Resets all BeatShot Video and Sound settings to default values. */
	void SetToBSDefaults();

	/** @return whether the version is equal to the current version */
	bool IsBSVersionValid() const;

	/** Updates the version to the current version. */
	void UpdateBSVersion();

	/** Initializes DLSS and NIS settings. */
	void LoadDLSSSettings();

	/** Initializes Audio Control Buses and UserMix. */
	void LoadUserControlBusMix();

	/** Sets the volume for the specified control bus.
	 *  @param ControlBusKey the name of the control bus
	 *  @param InVolume the value of the volume, a value between 0 and 100.0
	 */
	void SetVolumeForControlBus(const FName& ControlBusKey, float InVolume);

public:
	//~UGameUserSettings interface
	virtual void SetToDefaults() override;
	virtual void LoadSettings(bool bForceReload) override;
	virtual void ValidateSettings() override;
	virtual void ResetToCurrentSettings() override;
	virtual void ApplySettings(bool bForceReload) override;
	//~End of UGameUserSettings interface

	/** Queries the supported setting type and creates a map based on the results.
	 *  @param NvidiaSettingType the Nvidia setting type to query for
	 *  @return a map that maps each supported setting's string representation to its int equivalent enum value
	 */
	TMap<FString, uint8> GetSupportedNvidiaSettingModes(ENvidiaSettingType NvidiaSettingType) const;

	UFUNCTION()
	TArray<FString> GetAvailableAudioDeviceNames() const;
	UFUNCTION()
	bool GetShowFPSCounter() const;
	UFUNCTION()
	float GetDisplayGamma() const;
	UFUNCTION()
	float GetDLSSSharpness() const;
	UFUNCTION()
	float GetNISSharpness() const;
	UFUNCTION()
	int32 GetFrameRateLimitMenu() const;
	UFUNCTION()
	int32 GetFrameRateLimitGame() const;
	UFUNCTION()
	int32 GetFrameRateLimitBackground() const;
	UFUNCTION()
	uint8 GetDLSSEnabledMode() const;
	UFUNCTION()
	uint8 GetNISEnabledMode() const;
	UFUNCTION()
	uint8 GetFrameGenerationEnabledMode() const;
	UFUNCTION()
	uint8 GetDLSSMode() const;
	UFUNCTION()
	uint8 GetNISMode() const;
	UFUNCTION()
	uint8 GetStreamlineReflexMode() const;
	UFUNCTION()
	float GetOverallVolume() const;
	UFUNCTION()
	float GetMenuVolume() const;
	UFUNCTION()
	float GetMusicVolume() const;
	UFUNCTION()
	float GetSoundFXVolume() const;
	UFUNCTION()
	FString GetAudioOutputDeviceId() const;
	UFUNCTION()
	uint8 GetAntiAliasingMethod() const;
	UFUNCTION()
	static bool IsDLSSEnabled();
	UFUNCTION()
	bool IsNISEnabled() const;

	UFUNCTION()
	void SetShowFPSCounter(bool InShowFPSCounter);
	UFUNCTION()
	void SetFrameRateLimitMenu(int32 InFrameRateLimitMenu);
	UFUNCTION()
	void SetFrameRateLimitGame(int32 InFrameRateLimitGame);
	UFUNCTION()
	void SetFrameRateLimitBackground(int32 InFrameRateLimitBackground);
	UFUNCTION()
	void SetDLSSEnabledMode(uint8 InDLSSEnabledMode);
	UFUNCTION()
	void SetNISEnabledMode(uint8 InNISEnabledMode);
	UFUNCTION()
	void SetFrameGenerationEnabledMode(uint8 InFrameGenerationEnabledMode);
	UFUNCTION()
	void SetDLSSMode(uint8 InDLSSMode);
	UFUNCTION()
	void SetNISMode(uint8 InNISMode);
	UFUNCTION()
	void SetStreamlineReflexMode(uint8 InStreamlineReflexMode);
	UFUNCTION()
	void SetDisplayGamma(float InGamma);
	UFUNCTION()
	void SetOverallVolume(float InVolume);
	UFUNCTION()
	void SetMenuVolume(float InVolume);
	UFUNCTION()
	void SetMusicVolume(float InVolume);
	UFUNCTION()
	void SetSoundFXVolume(float InVolume);
	UFUNCTION()
	void SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId);
	UFUNCTION()
	void SetDLSSSharpness(float InDLSSSharpness);
	UFUNCTION()
	void SetNISSharpness(float InNISSharpness);
	UFUNCTION()
	void SetAntiAliasingMethod(uint8 InAntiAliasingMethod);
	UFUNCTION()
	void SetResolutionScaleChecked(float InResolutionScale);

	TMulticastDelegate<void(const FString&)> OnAudioOutputDeviceChanged;

private:
	/** Callback function for when audio devices have been obtained. */
	UFUNCTION()
	void HandleAudioOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices);

	/** Callback function for when the main audio device has been obtained. */
	UFUNCTION()
	void HandleMainAudioOutputDeviceObtained(const FString& CurrentDevice);

	/** Applies the value of DisplayGamma to the game engine. */
	void ApplyDisplayGamma() const;

	/** Current BSGameUserSettings version. */
	UPROPERTY(Config)
	uint32 BSVersion;

	/** Output audio device. */
	UPROPERTY(Config)
	FString AudioOutputDeviceId;

	/** Global or overall volume. */
	UPROPERTY(Config)
	float OverallVolume;

	/** Volume of the Main Menu Music. */
	UPROPERTY(Config)
	float MenuVolume;

	/** Volume of the AudioAnalyzer Tracker. */
	UPROPERTY(Config)
	float MusicVolume;

	/** Volume of sound effects. */
	UPROPERTY(Config)
	float SoundFXVolume;

	/** Whether to show the FPS counter at the top left. */
	UPROPERTY(Config)
	bool bShowFPSCounter;

	/** Display Gamma (Brightness?). */
	UPROPERTY(Config)
	float DisplayGamma = 2.2;

	/** Nvidia DLSS Sharpness. */
	UPROPERTY(Config)
	float DLSSSharpness;

	/** Nvidia NIS Sharpness. */
	UPROPERTY(Config)
	float NISSharpness;

	/** Frame rate limit while in a UI menu. */
	UPROPERTY(Config)
	int32 FrameRateLimitMenu;

	/** Frame rate limit while in game (not in UI menu). */
	UPROPERTY(Config)
	int32 FrameRateLimitGame;

	/** Frame rate limit while application is in the background. */
	UPROPERTY(Config)
	int32 FrameRateLimitBackground;

	/** Nvidia DLSS On/Off. */
	UPROPERTY(Config)
	EDLSSEnabledMode DLSSEnabledMode;

	/** Nvidia NIS On/Off. */
	UPROPERTY(Config)
	ENISEnabledMode NISEnabledMode;

	/** Nvidia Streamline Frame Generation Mode. */
	UPROPERTY(Config)
	UStreamlineDLSSGMode FrameGenerationEnabledMode;

	/** Nvidia DLSS Mode or Super Resolution Mode. */
	UPROPERTY(Config)
	UDLSSMode DLSSMode;

	/** Nvidia NIS Mode. */
	UPROPERTY(Config)
	UNISMode NISMode;

	/** Nvidia Streamline Reflex Mode. */
	UPROPERTY(Config)
	UStreamlineReflexMode StreamlineReflexMode;

	/** Anti Aliasing Method. */
	UPROPERTY(Config)
	TEnumAsByte<EAntiAliasingMethod> AntiAliasingMethod;

	/** Maps each sound control bus to a unique string. */
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<USoundControlBus>> ControlBusMap;

	/** The UserControlBusMix. */
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBusMix> ControlBusMix = nullptr;

	/** Whether the ControlBusMix (UserControlBusMix) has been loaded. */
	UPROPERTY(Transient)
	bool bSoundControlBusMixLoaded;

	/** Whether InitDLSSSettings has been successfully called. */
	UPROPERTY(Transient)
	bool bDLSSInitialized;

	/** Audio Device names. */
	UPROPERTY(Transient)
	TArray<FString> AudioDeviceNames;

	/** Enum to string map storage for Nvidia settings. */
	UPROPERTY(Transient)
	const UVideoSettingEnumTagMap* VideoSettingEnumMap;
};
