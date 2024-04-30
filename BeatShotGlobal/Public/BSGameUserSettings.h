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
UCLASS()
class BEATSHOTGLOBAL_API UBSGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UBSGameUserSettings();

	/** @return the game local machine settings (resolution, windowing mode, scalability settings, etc...) */
	static UBSGameUserSettings* Get();

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

	void UpdateEffectiveFrameRateLimit();

public:
	virtual void BeginDestroy() override;
	virtual void SetToDefaults() override;
	virtual void LoadSettings(bool bForceReload) override;
	virtual void ValidateSettings() override;
	virtual void ResetToCurrentSettings() override;
	virtual void ApplySettings(bool bForceReload) override;
	virtual void SaveSettings() override;
	virtual float GetEffectiveFrameRateLimit() override;

	/** Queries the supported setting type and creates a map based on the results.
	 *  @param NvidiaSettingType the Nvidia setting type to query for
	 *  @return a map that maps each supported setting's string representation to its int equivalent enum value
	 */
	TMap<FString, uint8> GetSupportedNvidiaSettingModes(ENvidiaSettingType NvidiaSettingType) const;

	float GetPostProcessBiasFromBrightness() const;

	void SetLoadingScreenMixActivationState(bool bEnable);

	void SetInMenu(bool bIsInMenu);

	UFUNCTION()
	TArray<FString> GetAvailableAudioDeviceNames() const;
	UFUNCTION()
	FString GetAudioOutputDeviceId() const;
	UFUNCTION()
	float GetOverallVolume() const;
	UFUNCTION()
	float GetMenuVolume() const;
	UFUNCTION()
	float GetMusicVolume() const;
	UFUNCTION()
	float GetSoundFXVolume() const;

	UFUNCTION()
	uint8 GetAntiAliasingMethod() const;
	UFUNCTION()
	float GetBrightness() const;
	UFUNCTION()
	float GetDisplayGamma() const;
	UFUNCTION()
	int32 GetFrameRateLimitGame() const;
	UFUNCTION()
	int32 GetFrameRateLimitMenu() const;
	UFUNCTION()
	int32 GetFrameRateLimitBackground() const;
	UFUNCTION()
	bool GetShowFPSCounter() const;

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
	float GetDLSSSharpness() const;
	UFUNCTION()
	float GetNISSharpness() const;
	UFUNCTION()
	static bool IsDLSSEnabled();
	UFUNCTION()
	bool IsNISEnabled() const;

	UFUNCTION()
	void SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId);
	UFUNCTION()
	void SetOverallVolume(float InVolume);
	UFUNCTION()
	void SetMenuVolume(float InVolume);
	UFUNCTION()
	void SetMusicVolume(float InVolume);
	UFUNCTION()
	void SetSoundFXVolume(float InVolume);

	UFUNCTION()
	void SetAntiAliasingMethod(uint8 InAntiAliasingMethod);
	UFUNCTION()
	void SetBrightness(float InBrightness);
	UFUNCTION()
	void SetDisplayGamma(float InGamma);
	UFUNCTION()
	void SetFrameRateLimitMenu(int32 InFrameRateLimitMenu);
	UFUNCTION()
	void SetFrameRateLimitGame(int32 InFrameRateLimitGame);
	UFUNCTION()
	void SetFrameRateLimitBackground(int32 InFrameRateLimitBackground);
	UFUNCTION()
	void SetResolutionScaleChecked(float InResolutionScale);
	UFUNCTION()
	void SetShowFPSCounter(bool InShowFPSCounter);

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
	void SetDLSSSharpness(float InDLSSSharpness);
	UFUNCTION()
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

	/** The Loading Screen Control Bus Mix. */
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBusMix> LoadingScreenControlBusMix = nullptr;

	UPROPERTY(Transient)
	UAudioComponent* LoadingScreenAudioComponent = nullptr;

	/** Whether the ControlBusMix (UserControlBusMix) has been loaded. */
	UPROPERTY(Transient)
	bool bSoundControlBusMixLoaded;

	/** Whether InitDLSSSettings has been successfully called. */
	UPROPERTY(Transient)
	bool bDLSSInitialized;

	/** Whether to use FrameRateLimitMenu as the effective frame rate. */
	bool bInMenu;

	/** Audio Device names. */
	UPROPERTY(Transient)
	TArray<FString> AudioDeviceNames;

	/** Enum to string map storage for Nvidia settings. */
	UPROPERTY(Transient)
	const UVideoSettingEnumTagMap* VideoSettingEnumMap;

	/** Delegate bound to FSlateApplication OnApplicationActivationStateChanged.  */
	FDelegateHandle OnApplicationActivationStateChangedHandle;
};
