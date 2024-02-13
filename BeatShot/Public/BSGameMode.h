// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpRequestInterface.h"
#include "SaveGamePlayerSettings.h"
#include "SaveLoadInterface.h"
#include "AbilitySystem/Globals/BSAbilitySet.h"
#include "GameFramework/GameMode.h"
#include "Target/Target.h"
#include "BSGameMode.generated.h"

class ABSGun;
struct FBSGrantedAbilitySet;
class UBSAbilitySet;
class AVisualizerManager;
class AFloatingTextActor;
class ATargetManager;
class ATarget;
class ABSPlayerController;
class UAudioAnalyzerManager;

DECLARE_LOG_CATEGORY_EXTERN(LogAudioData, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAAManagerSecondPassed, const float PlaybackTime);
DECLARE_DELEGATE(FOnStreakThresholdPassed)

/** GameMode used for the Range level. A BSCharacter is spawned and possessed for each player controller. The game mode
 *  is responsible for starting and ending a BeatShot default or custom game mode, spawning and destroying the
 *  TargetManager and VisualizerManager, and monitoring the AudioAnalyzer on tick. It also manages abilities used for
 *  specific BeatShot game modes and saves scores locally before asking Game Instance to save to database */
UCLASS()
class BEATSHOT_API ABSGameMode : public AGameMode, public ISaveLoadInterface, public IHttpRequestInterface
{
	GENERATED_BODY()

	ABSGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void PostLoad() override;
	virtual void Logout(AController* Exiting) override;
	ACharacter* SpawnPlayer(APlayerController* PlayerController);

	UPROPERTY()
	TArray<ABSPlayerController*> Controllers;

protected:
	/* The TargetManager class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Spawnable Classes")
	TSubclassOf<ATargetManager> TargetManagerClass;

	/** The VisualizerManager class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Spawnable Classes")
	TSubclassOf<AVisualizerManager> VisualizerManagerClass;

	/** The VisualizerManager class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Spawnable Classes")
	TSubclassOf<ACharacter> CharacterClass;

	/* The spawned TargetManager */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Spawned Actors")
	TObjectPtr<ATargetManager> TargetManager;

	/** The Visualizer Manager */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Spawned Actors")
	TObjectPtr<AVisualizerManager> VisualizerManager;

	/** The spawned AATracker object */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Spawned Objects")
	UAudioAnalyzerManager* AATracker;

	/** The spawned AAPlayer object */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Spawned Objects")
	UAudioAnalyzerManager* AAPlayer;

	/** The ability set that contains the TrackGun ability for tracking damage type game modes */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Abilities")
	UBSAbilitySet* TrackGunAbilitySet;

	/** Granted data about the TrackGun ability */
	FBSGrantedAbilitySet TrackGunAbilityGrantedHandles;

public:
	/** Returns pointer to TargetManager */
	ATargetManager* GetTargetManager() const { return TargetManager; }
	
	/** Entry point into starting game. Spawn TargetManager, Visualizers 
	 *  If a valid path is found, calls InitializeAudioManagers, otherwise calls ShowSongPathErrorMessage */
	void InitializeGameMode(const TSharedPtr<FBSConfig> InConfig);

	/** Allows TargetManager to start spawning targets, starts timers, shows PlayerHUD, CrossHair, and hides the
	 *  countdown widget. Called from Countdown widget when the countdown has completed */
	void StartGameMode();
	
	/** Destroys all actors involved in a game mode and saves scores if applicable */
	void EndGameMode(const bool bSaveScores, const ETransitionState TransitionState);

	/** Called from Countdown widget when user clicks to start game mode. If player delay is > 0.05, the function
	 *  begins playback for AATracker and sets a timer of length player delay to then begin playback of AAPlayer.
	 *  If player delay isn't long enough to justify two separate players, only AATracker begins playback. */
	void StartAAManagerPlayback();

	/** Called from PlayerController when the game is paused */
	void PauseAAManager(bool ShouldPause);

	/** Binds to the gun's OnShotFired delegate */
	void RegisterGun(ABSGun* InGun);

	virtual void OnPlayerSettingsChanged_Game(const FPlayerSettings_Game& GameSettings) override;
	virtual void OnPlayerSettingsChanged_AudioAnalyzer(const FPlayerSettings_AudioAnalyzer& AudioAnalyzerSettings) override;
	virtual void OnPlayerSettingsChanged_VideoAndSound(const FPlayerSettings_VideoAndSound& VideoAndSoundSettings) override;

	/** Delegate that is executed every second to update the progress into song on PlayerHUD.
	 *  PlayerHUD binds to it, while DefaultGameMode (this) executes it */
	FOnAAManagerSecondPassed OnSecondPassed;

	/** Called if the streak threshold is passed and user has not unlocked night mode */
	FOnStreakThresholdPassed OnStreakThresholdPassed;

private:
	/** Starts all DefaultGameMode timers */
	void StartGameModeTimers();

	/** Binds all delegates associated with DefaultGameMode */
	void BindGameModeDelegates();

	/** Function to tell TargetManager to spawn a new target */
	void SpawnNewTarget(bool bNewTargetState);

	/** Does all of the AudioAnalyzer initialization, called during InitializeGameMode */
	bool InitializeAudioManagers();

	/** Retrieves all AudioAnalyzer data on tick */
	void OnTick_AudioAnalyzers(const float DeltaSeconds);

	/** play AAPlayer, used as callback function to set delay from AATracker */
	UFUNCTION()
	void PlayAAPlayer() const;

	/** Change volume of given AAManager, or if none provided change Player/Tracker volume */
	void SetAAManagerVolume(float GlobalVolume, float MusicVolume, UAudioAnalyzerManager* AAManager = nullptr) const;

	void OnAAManagerError()
	{
		bShouldTick = false;
		UE_LOG(LogTemp, Warning, TEXT("Init Player Error"));
	}
	
	void GoToMainMenu();
	
	/** Loads matching player scores into CurrentPlayerScore and calculates the MaxScorePerTarget */
	void LoadMatchingPlayerScores();

	/** Saves player scores to slot and calls SaveScoresToDatabase() if bShouldSavePlayerScores is true and
	 *  GetCompletedPlayerScores() returns a valid score object. Otherwise Broadcasts OnPostScoresResponse with "None" */
	void HandleScoreSaving(const bool bExternalSaveScores);
	
	/** Returns the current player scores, checking for NaNs and updating the time */
	void GetCompletedPlayerScores(FPlayerScore& InScore);

	/** Function bound to TargetManager's PostTargetDamageEvent delegate */
	void OnPostTargetDamageEvent(const FTargetDamageEvent& Event);

	/** Function bound to DefaultGameMode's OnTargetActivated delegate to keep track of number of targets spawned.
	 *  Executed by TargetManager */
	UFUNCTION()
	void UpdateTargetsSpawned(const ETargetDamageType& DamageType);

	/** Function bound to Gun_AK47's FOnShotFired delegate to keep track of number of targets spawned.
	 *  Executed by Gun_AK47 */
	UFUNCTION()
	void UpdateShotsFired(ABSPlayerController* Controller);
	
	/** Called by UpdatePlayerScores to update the streak */
	void UpdateStreak(ABSPlayerController* Controller, FPlayerScore& InScore, int32 Streak, const FTransform& Transform);

	/** Not currently used */
	void UpdateTimeOffset(const float TimeOffset, const FTransform& Transform);

	/** Callback function for OnSecondPassedTimer, executes OnSecondPassed */
	UFUNCTION()
	void OnSecondPassedCallback() const;

	static float FloatDivide(const float Num, const float Denom);

	/** Returns the score based on the time the target was alive for */
	float GetScoreFromTimeAlive(const float InTimeAlive) const;

	/** Returns the length of time away from a perfect shot, with negative values indicating an early shot and positive
	 *  values indicating a late shot, based on the time the target was alive for */
	float GetHitTimingError(const float InTimeAlive) const;

	/** Returns the length of time away from a perfect shot, based on the time the target was alive for */
	float GetAbsHitTimingError(const float InTimeAlive) const;
	
	/** Returns a normalized time offset between 0 and 1, where 0.5 is perfect (~no time offset), based on the time the
	 *  target was alive for */
	float GetNormalizedHitTimingError(const float InTimeAlive) const;

	/** Honestly idk what this does, but it was used in the AudioAnalyzer example so I'm sticking with it >.> */
	bool bLastTargetOnSet;
	
	/** Whether or not to run tick functions */
	bool bShouldTick;

	const FActorSpawnParameters SpawnParameters;

	/** The game mode defining properties */
	TSharedPtr<FBSConfig> BSConfig;

	/** The time elapsed since last target spawn */
	float Elapsed;

	/** Max score per target based on total amount of targets that could spawn */
	float MaxScorePerTarget;

	/** The time played for the current game mode, used to update Steam achievements */
	float TimePlayedGameMode = 0.f;

	/** The "live" player score objects, which start fresh and import high score from SavedPlayerScores */
	TMap<ABSPlayerController*, FPlayerScore> CurrentPlayerScores;
	
	/* Locally stored AASettings since they must be accessed frequently in OnTick() */
	UPROPERTY()
	FPlayerSettings_AudioAnalyzer AASettings;
	
	/** Timer that spans the length of the song */
	UPROPERTY()
	FTimerHandle GameModeLengthTimer;

	FTimerDelegate GameModeLengthTimerDelegate;

	/** A timer used to set the difference in start times between AATracker and AAPlayer */
	UPROPERTY()
	FTimerHandle PlayerDelayTimer;

	/** A timer used to track every passing second of the song */
	UPROPERTY()
	FTimerHandle OnSecondPassedTimer;

	UPROPERTY()
	FTimerHandle GoToMainMenuTimer;

	/** The threshold to activate night mode if not yet unlocked */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|General")
	int32 StreakThreshold = 50;
	
	TArray<bool> Beats;
	TArray<float> SpectrumValues;
	TArray<float> SpectrumVariance;
	TArray<int32> BpmCurrent;
	TArray<int32> BpmTotal;
};
