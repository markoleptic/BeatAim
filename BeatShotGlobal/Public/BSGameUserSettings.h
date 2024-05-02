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

/** Video and Sound Settings that are saved to GameUserSettings.ini. */
UCLASS()
class BEATSHOTGLOBAL_API UBSGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UBSGameUserSettings();

	/** @return the game local machine settings (resolution, windowing mode, scalability settings, etc...) */
	static UBSGameUserSettings* Get();

	/** Performs initialization of DLSS settings and the user control box mix. Binds to Slate application delegates. */
	void Initialize(const UWorld* World);

private:
	/** Resets all BeatShot Video and Sound settings to default values. */
	void SetToBSDefaults();

	/** @return whether the version is equal to the current version */
	bool IsBSVersionValid() const;

	/** Updates the version to the current version. */
	void UpdateBSVersion();

	/** Initializes and validates DLSS and NIS settings. */
	void LoadDLSSSettings();

	/** Initializes Audio Control Buses and UserMix. */
	void LoadUserControlBusMix(const UWorld* World);

	/** Sets the volume for the specified control bus.
	 *  @param ControlBusKey the name of the control bus
	 *  @param InVolume the value of the volume, a value between 0 and 100.0
	 */
	void SetVolumeForControlBus(const FName& ControlBusKey, float InVolume);

	/** Updates the effective frame rate limit. */
	void UpdateEffectiveFrameRateLimit();

	/** Updates and applies all Nvidia settings based on the DLSSEnabledMode. */
	void UpdateNvidiaSettings();

public:
	virtual void BeginDestroy() override;
	virtual void SetToDefaults() override;
	virtual void LoadSettings(bool bForceReload) override;
	virtual void ValidateSettings() override;
	virtual void ResetToCurrentSettings() override;
	virtual void ApplyNonResolutionSettings() override;
	virtual void ApplySettings(bool bForceReload) override;
	virtual void SaveSettings() override;
	virtual float GetEffectiveFrameRateLimit() override;

	/** Queries the supported setting type and creates a map based on the results.
	 *  @param NvidiaSettingType the Nvidia setting type to query for
	 *  @return a map that maps each supported setting's string representation to its int equivalent enum value
	 */
	TMap<FString, uint8> GetSupportedNvidiaSettingModes(ENvidiaSettingType NvidiaSettingType) const;

	/** @return an interpolated post process bias based on Brightness. */
	float GetPostProcessBiasFromBrightness() const;

	/** Updates the effective frame rate limit based on if in a menu screen.
	 *  @param bIsInMenu whether the local player is in a menu screen or not
	 */
	void SetInMenu(bool bIsInMenu);

	TArray<FString> GetAvailableAudioDeviceNames() const;
	FString GetAudioOutputDeviceId() const;
	float GetOverallVolume() const;
	float GetMenuVolume() const;
	float GetMusicVolume() const;
	float GetSoundFXVolume() const;
	uint8 GetAntiAliasingMethod() const;
	float GetBrightness() const;
	float GetDisplayGamma() const;
	int32 GetFrameRateLimitGame() const;
	int32 GetFrameRateLimitMenu() const;
	int32 GetFrameRateLimitBackground() const;
	bool GetShowFPSCounter() const;

	uint8 GetDLSSEnabledMode() const;
	uint8 GetNISEnabledMode() const;
	uint8 GetFrameGenerationEnabledMode() const;
	uint8 GetDLSSMode() const;
	uint8 GetNISMode() const;
	uint8 GetStreamlineReflexMode() const;

	float GetDLSSSharpness() const;
	float GetNISSharpness() const;
	static bool IsDLSSEnabled();
	bool IsNISEnabled() const;

	/** Sets the value of AudioOutputDeviceId and broadcasts it to the local player to change.\n
	 *  Changes not automatically saved. */
	void SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId);

	/** Sets the value of OverallVolume and the corresponding control bus.\n
	 *  Changes not automatically saved. */
	void SetOverallVolume(float InVolume);

	/** Sets the value of MenuVolume and the corresponding control bus.\n
	 *  Changes not automatically saved. */
	void SetMenuVolume(float InVolume);

	/** Sets the value of MusicVolume and the corresponding control bus.\n
	 *  Changes not automatically saved. */
	void SetMusicVolume(float InVolume);

	/** Sets the value of SoundFXVolume and the corresponding control bus.\n
     *  Changes not automatically saved. */
	void SetSoundFXVolume(float InVolume);

	/** Sets the value of AntiAliasingMethod.\n
	 *  Changes not applied until calling ApplySettings or ApplyNonResolutionSettings.\n
	 *  Changes not automatically saved. */
	void SetAntiAliasingMethod(uint8 InAntiAliasingMethod);

	/** Sets the value of Brightness.\n
	 *  Changes not applied until calling ApplySettings or SaveSettings.\n
	 *  Changes not automatically saved. */
	void SetBrightness(float InBrightness);

	/** Sets the value of DisplayGamma and applies it.\n
	 *  Changes not automatically saved. */
	void SetDisplayGamma(float InGamma);

	/** Sets the value of FrameRateLimitMenu.\n
	 *  Changes not automatically saved. */
	void SetFrameRateLimitMenu(int32 InFrameRateLimitMenu);

	/** Sets the value of FrameRateLimitGame.\n
	 *  Changes not automatically saved. */
	void SetFrameRateLimitGame(int32 InFrameRateLimitGame);

	/** Sets the value of FrameRateLimitBackground.\n
	 *  Changes not automatically saved. */
	void SetFrameRateLimitBackground(int32 InFrameRateLimitBackground);

	/** Sets the resolution scale if DLSS and NIS are disabled.\n
	 * 	Changes not applied until calling ApplySettings or ApplyResolutionSettings.\n
	 *  Changes not automatically saved. */
	void SetResolutionScaleChecked(float InResolutionScale);

	/** Sets the value of bShowFPSCounter. \n
	 * 	Changes not applied until calling ApplySettings or SaveSettings.\n
	 *  Changes not automatically saved. */
	void SetShowFPSCounter(bool InShowFPSCounter);

	/** Sets the value of DLSSEnabledMode. Disables NIS if it is currently enabled. Updates all Nvidia settings
	 *  based on the value of DLSSEnabledMode. \n
	 *  Changes not automatically saved. */
	void SetDLSSEnabledMode(uint8 InDLSSEnabledMode);

	/** Sets the value of NISEnabledMode. Disables DLSS if it is currently enabled. Updates and applies all Nvidia
	 *  settings based on the value of DLSSEnabledMode. \n
	 *  Changes not automatically saved. */
	void SetNISEnabledMode(uint8 InNISEnabledMode);

	/** Sets the value of FrameGenerationEnabledMode and applies it.\n
	 *  Changes not automatically saved. */
	void SetFrameGenerationEnabledMode(uint8 InFrameGenerationEnabledMode);

	/** Sets the value of DLSSMode and applies it, if NIS is disabled. Updates and applies all Nvidia settings
	 *  based on the value of DLSSEnabledMode.\n
	 *  Changes not automatically saved. */
	void SetDLSSMode(uint8 InDLSSMode);

	/** Sets the value of NIS and applies it, if DLSSMode is off.\n
	 *  Changes not automatically saved. */
	void SetNISMode(uint8 InNISMode);

	/** Sets the value of StreamlineReflexMode and applies it.\n
	 *  Changes not automatically saved. */
	void SetStreamlineReflexMode(uint8 InStreamlineReflexMode);

	/** Sets the value of DLSSSharpness and applies it if DLSS is enabled and NIS is disabled.\n
	 *  Changes not automatically saved. */
	void SetDLSSSharpness(float InDLSSSharpness);

	/** Sets the value of NISharpness and applies it if DLSS is disabled and NIS is enabled.\n
     *  Changes not automatically saved. */
	void SetNISSharpness(float InNISSharpness);

	/** Broadcast when the Audio Output Device has been changed for the local player. */
	TMulticastDelegate<void(const FString&)> OnAudioOutputDeviceChanged;

	/** Broadcast when the settings are changed. */
	static TMulticastDelegate<void(const UBSGameUserSettings*)> OnSettingsChanged;

private:
	/** Callback function for when audio devices have been obtained. */
	UFUNCTION()
	void HandleAudioOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices);

	/** Callback function for when the main audio device has been obtained. */
	UFUNCTION()
	void HandleMainAudioOutputDeviceObtained(const FString& CurrentDevice);

	/** Callback function for when the slate application activation state is changed. */
	UFUNCTION()
	void HandleApplicationActivationStateChanged(bool bIsActive);

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

	/** Brightness in (Game only). */
	UPROPERTY(Config)
	float Brightness;

	/** Display Gamma (Game and UI). */
	UPROPERTY(Config)
	float DisplayGamma;

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

	/** The User Control Bus Mix. */
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBusMix> ControlBusMix = nullptr;

	/** Whether the ControlBusMix (UserControlBusMix) has been loaded. */
	UPROPERTY(Transient)
	bool bSoundControlBusMixLoaded;

	/** Whether InitDLSSSettings has been successfully called. */
	UPROPERTY(Transient)
	bool bDLSSInitialized;

	/** Whether to use FrameRateLimitMenu as the effective frame rate. */
	UPROPERTY(Transient)
	bool bInMenu;

	/** Audio Device names obtained from AudioMixerBlueprintLibrary. */
	UPROPERTY(Transient)
	TArray<FString> AudioDeviceNames;

	/** Enum to string map storage for Nvidia settings. */
	UPROPERTY(Transient)
	const UVideoSettingEnumTagMap* VideoSettingEnumMap;

	/** Delegate bound to FSlateApplication OnApplicationActivationStateChanged. */
	FDelegateHandle OnApplicationActivationStateChangedHandle;
};
