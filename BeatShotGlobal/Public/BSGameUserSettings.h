// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DLSSLibrary.h"
#include "NISLibrary.h"
#include "StreamlineLibraryDLSSG.h"
#include "StreamlineLibraryReflex.h"
#include "GameFramework/GameUserSettings.h"
#include "BSGameUserSettings.generated.h"

enum class ENISEnabledMode : uint8;
enum class EDLSSEnabledMode : uint8;
class USoundControlBusMix;
class USoundControlBus;
/** Video and Sound Settings that are saved to GameUserSettings.ini */
UCLASS(config=GameUserSettings, configdonotcheckdefaults)
class BEATSHOTGLOBAL_API UBSGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UBSGameUserSettings();

	static UBSGameUserSettings* Get();

	//~UObject interface
	virtual void BeginDestroy() override;
	//~End of UObject interface

	//~UGameUserSettings interface
	virtual void SetToDefaults() override;
	virtual void LoadSettings(bool bForceReload) override;
	virtual void ConfirmVideoMode() override;
	virtual float GetEffectiveFrameRateLimit() override;
	virtual void ResetToCurrentSettings() override;
	virtual void ApplyNonResolutionSettings() override;
	virtual int32 GetOverallScalabilityLevel() const override;
	virtual void SetOverallScalabilityLevel(int32 Value) override;
	//~End of UGameUserSettings interface

	/** Initializes DLSS and NIS settings. */
	void InitDLSSSettings();

	/** Initializes Audio Control Buses and UserMix. */
	void LoadUserControlBusMix();

	bool IsDLSSSupported();

	bool IsNISSupported();

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
	EDLSSEnabledMode GetDLSSEnabledMode() const;
	UFUNCTION()
	ENISEnabledMode GetNISEnabledMode() const;
	UFUNCTION()
	UStreamlineDLSSGMode GetFrameGenerationEnabledMode() const;
	UFUNCTION()
	UDLSSMode GetDLSSMode() const;
	UFUNCTION()
	UNISMode GetNISMode() const;
	UFUNCTION()
	UStreamlineReflexMode GetStreamlineReflexMode() const;
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
	TEnumAsByte<EAntiAliasingMethod> GetAntiAliasingMethod() const;

	UFUNCTION()
	void SetShowFPSCounter(bool InShowFPSCounter);
	UFUNCTION()
	void SetFrameRateLimitMenu(int32 InFrameRateLimitMenu);
	UFUNCTION()
	void SetFrameRateLimitGame(int32 InFrameRateLimitGame);
	UFUNCTION()
	void SetFrameRateLimitBackground(int32 InFrameRateLimitBackground);
	UFUNCTION()
	void SetDLSSEnabledMode(EDLSSEnabledMode InDLSSEnabledMode);
	UFUNCTION()
	void SetNISEnabledMode(ENISEnabledMode InNISEnabledMode);
	UFUNCTION()
	void SetFrameGenerationEnabledMode(UStreamlineDLSSGMode InFrameGenerationEnabledMode);
	/** Sets display settings using console variables according to the DLSSMode. */
	UFUNCTION()
	void SetDLSSMode(UDLSSMode InDLSSMode);
	UFUNCTION()
	void SetNISMode(UNISMode InNISMode);
	UFUNCTION()
	void SetStreamlineReflexMode(UStreamlineReflexMode InStreamlineReflexMode);
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
	void SetAntiAliasingMethod(TEnumAsByte<EAntiAliasingMethod> InAntiAliasingMethod);
	UFUNCTION()
	void SetResolutionScaleChecked(float InResolutionScale);

private:
	void SetBSSettingsToDefaults();

	void ApplyDisplayGamma();

	void SetVolumeForControlBus(const FName& ControlBusKey, float InVolume);

	UPROPERTY(Config)
	FString AudioOutputDeviceId;

	// GlobalVolume, which also affects Menu and Music volume
	UPROPERTY(Config)
	float OverallVolume;

	// Volume of the Main Menu Music
	UPROPERTY(Config)
	float MenuVolume;

	// Volume of the AudioAnalyzer Tracker
	UPROPERTY(Config)
	float MusicVolume;

	// Volume of sound effects
	UPROPERTY(Config)
	float SoundFXVolume;

	UPROPERTY(Config)
	bool bShowFPSCounter;

	// Brightness?
	UPROPERTY(Config)
	float DisplayGamma = 2.2;

	UPROPERTY(Config)
	float DLSSSharpness;

	UPROPERTY(Config)
	float NISSharpness;

	UPROPERTY(Config)
	int32 FrameRateLimitMenu;

	UPROPERTY(Config)
	int32 FrameRateLimitGame;

	UPROPERTY(Config)
	int32 FrameRateLimitBackground;

	// DLSS On/Off
	UPROPERTY(Config)
	EDLSSEnabledMode DLSSEnabledMode;

	// NIS On/Off
	UPROPERTY(Config)
	ENISEnabledMode NISEnabledMode;

	// Frame Generation
	UPROPERTY(Config)
	UStreamlineDLSSGMode FrameGenerationEnabledMode;

	// Super Resolution Mode
	UPROPERTY(Config)
	UDLSSMode DLSSMode;

	// NIS Mode
	UPROPERTY(Config)
	UNISMode NISMode;

	// Reflex Mode
	UPROPERTY(Config)
	UStreamlineReflexMode StreamlineReflexMode;

	UPROPERTY(Config)
	TEnumAsByte<EAntiAliasingMethod> AntiAliasingMethod;

	// Sound class to control bus map
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<USoundControlBus>> ControlBusMap;

	UPROPERTY(Transient)
	TObjectPtr<USoundControlBusMix> ControlBusMix = nullptr;

	UPROPERTY(Transient)
	bool bSoundControlBusMixLoaded;

	UPROPERTY(Transient)
	bool bDLSSInitialized;

	UPROPERTY(Transient)
	bool bDLSSSupported;

	UPROPERTY(Transient)
	bool bNISSupported;
};
