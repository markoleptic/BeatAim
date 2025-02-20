// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerScoreInterface.h"
#include "BSPlayerSettingsInterface.h"
#include "HttpRequestInterface.h"
#include "MetasoundGeneratorHandle.h"
#include "Engine/GameInstance.h"
#include "BSGameInstance.generated.h"

class UTooltipWidget;
struct FBSConfig;
struct FGameModeTransitionState;
struct FMetaSoundOutput;
class ULoadingScreenWidgetStyle;
class USoundControlBus;
class USoundControlBusMix;
class ABSPlayerController;
class SLoadingScreenWidget;
class ATimeOfDayManager;
class USteamManager;

/** Base GameInstance for this game. */
UCLASS()
class BEATSHOT_API UBSGameInstance : public UGameInstance, public IBSPlayerSettingsInterface,
                                     public IHttpRequestInterface, public IBSPlayerScoreInterface
{
	GENERATED_BODY()

	virtual void Init() override;

#if WITH_EDITOR
	/** Called as soon as the game mode is spawned, prior to creating the local players. */
	virtual FGameInstancePIEResult PostCreateGameModeForPIE(const FGameInstancePIEParameters& Params,
		AGameModeBase* GameMode) override;
	/** Called to actually start the game when doing Play/Simulate In Editor. */
	virtual FGameInstancePIEResult StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer,
		const FGameInstancePIEParameters& Params) override;
#else
	void OnPreLoadMapWithContext(const FWorldContext& InWorldContext, const FString& MapName);
#endif WITH_EDITOR

	/** Called after a level has been loaded. */
	void OnPostLoadMapWithWorld(UWorld* World);

	/** Called after the loading screen widget has completed its fade out. */
	void OnLoadingScreenFadeOutComplete();

	/** Prepares the loading screen widget for the MoviePlayer. */
	void PrepareLoadingScreen();

	/** Called when the game instance is started either normally or through PIE. */
	virtual void OnStart() override;

	virtual void Shutdown() override;

public:
	/** Handles game mode transitions initiated by Main Menu, Pause Menu, or Post Game Menu. */
	void HandleGameModeTransition(const FGameModeTransitionState& NewGameModeTransitionState);

	/** Calls EndGameMode if the current game mode is BSGameMode. */
	void EndBSGameMode(const FGameModeTransitionState& NewGameModeTransitionState) const;

	/** Returns shared pointer to defining game mode options configuration. */
	TSharedPtr<FBSConfig> GetBSConfig() const { return BSConfig; }

	/** Returns the object that interfaces with Steam API. */
	TObjectPtr<USteamManager> GetSteamManager() const { return SteamManager; }

	/** Returns actor that manages the time of day in the Range level. */
	TObjectPtr<ATimeOfDayManager> GetTimeOfDayManager() const { return TimeOfDayManager; }

	/** Returns the name of the MainMenuLevel. */
	FName GetMainMenuLevelName() const { return MainMenuLevelName; }

	/** Returns the name of the RangeLevelName. */
	FName GetRangeLevelName() const { return RangeLevelName; }

	/** Sets the actor that manages the time of day in the Range level. */
	void SetTimeOfDayManager(const TObjectPtr<ATimeOfDayManager>& InManager) { TimeOfDayManager = InManager; }

	/** Handles saving scores to database, called by BSGameMode. */
	void SavePlayerScoresToDatabase(ABSPlayerController* PlayerController, bool bWasValidToSave,
		bool bQuitToDesktopAfterSave) const;

	/** Sets the loading screen state to fading out and updates the loading screen audio component state. */
	void RemoveLoadingScreen();

	/** A function pair that can be called externally, executes OnSteamOverlayIsActive(). */
	void OnSteamOverlayIsOn();
	void OnSteamOverlayIsOff();

	/** Binds OnPlayerSettingsChanged delegates to the function in this class with the matching signature. When the
	 *  delegate is broadcast, this class will also broadcast its matching delegate to all objects subscribed. */
	template <typename... T>
	void RegisterPlayerSettingsUpdaters(TMulticastDelegate<void(const T& SettingType)>&... Delegates);

	/** Binds a function to an OnPlayerSettingsChanged delegate with the matching signature. The function will be
	 *  called any time an updater broadcasts their delegate. */
	template <typename UserClass, typename StructType>
	void RegisterPlayerSettingsSubscriber(UserClass* InUserObject,
		typename TMemFunPtrType<false, UserClass, void (const StructType&)>::Type InFunc);

protected:
	void SetBSConfig(const FBSConfig& InConfig);

	void InitializeAudioComponent(const UWorld* World);

	void SetLoadingScreenAudioComponentState(float FadeTarget, float FadeDuration);

	/** Pauses the game and shows the Pause Screen if the overlay is active. */
	void OnSteamOverlayIsActive(bool bIsOverlayActive) const;

	virtual void OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings) override;
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_AudioAnalyzer& AudioAnalyzerSettings) override;
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_User& UserSettings) override;
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_CrossHair& CrossHairSettings) override;

	UFUNCTION()
	void HandleFadeCompleted(FName OutputName, const FMetaSoundOutput& Output);

	UFUNCTION()
	void HandlePlaybackTimeChanged(FName OutputName, const FMetaSoundOutput& Output);

	/** Object that interfaces with Steam API. */
	UPROPERTY()
	TObjectPtr<USteamManager> SteamManager;

	/** Actor that manages the time of day in the Range level. */
	UPROPERTY()
	TObjectPtr<ATimeOfDayManager> TimeOfDayManager;

	/** Shared pointer to loading screen slate widget. */
	TSharedPtr<SLoadingScreenWidget> LoadingScreenWidget;

	/** The defining game mode options that are populated from a menu widget, and accessed by the GameMode. */
	TSharedPtr<FBSConfig> BSConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Sound")
	USoundBase* LoadingScreenSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Levels")
	FName MainMenuLevelName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Levels")
	FName RangeLevelName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|LoadingScreen")
	TObjectPtr<USlateWidgetStyleAsset> LoadingScreenStyle;

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Classes")
	TSubclassOf<UTooltipWidget> TooltipClass;

	/** Whether the Steam Overlay is active. */
	bool IsSteamOverlayActive = false;

	/** Whether the loading screen is the initial one, which changes how the loading screen is rendered. */
	bool bIsInitialLoadingScreen = true;

	/** The Loading Screen Control Bus Mix. */
	UPROPERTY()
	TObjectPtr<USoundControlBusMix> LoadingScreenControlBusMix = nullptr;

	UPROPERTY()
	TObjectPtr<UAudioComponent> LoadingScreenAudioComponent = nullptr;

	float LastPlaybackPosition = -1.f;
	FOnMetasoundOutputValueChanged OnFadeCompleted;
	FOnMetasoundOutputValueChanged PlaybackTime;
};

template <typename... T>
void UBSGameInstance::RegisterPlayerSettingsUpdaters(TMulticastDelegate<void(const T& SettingType)>&... Delegates)
{
	(Delegates.AddUObject(this, &UBSGameInstance::OnPlayerSettingsChanged), ...);
}

template <typename UserClass, typename StructType>
void UBSGameInstance::RegisterPlayerSettingsSubscriber(UserClass* InUserObject,
	typename TMemFunPtrType<false, UserClass, void (const StructType&)>::Type InFunc)
{
	if constexpr (std::is_same_v<StructType, FPlayerSettings_Game>)
	{
		OnPlayerSettingsChangedDelegate_Game.AddUObject(InUserObject, InFunc);
	}
	else if constexpr (std::is_same_v<StructType, FPlayerSettings_User>)
	{
		OnPlayerSettingsChangedDelegate_User.AddUObject(InUserObject, InFunc);
	}
	else if constexpr (std::is_same_v<StructType, FPlayerSettings_CrossHair>)
	{
		OnPlayerSettingsChangedDelegate_CrossHair.AddUObject(InUserObject, InFunc);
	}
	else if constexpr (std::is_same_v<StructType, FPlayerSettings_AudioAnalyzer>)
	{
		OnPlayerSettingsChangedDelegate_AudioAnalyzer.AddUObject(InUserObject, InFunc);
	}
}
