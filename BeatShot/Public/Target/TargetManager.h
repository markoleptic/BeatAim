// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TargetCommon.h"
#include "GameFramework/Actor.h"
#include "TargetManager.generated.h"

struct FCommonScoreInfo;
struct FBSConfig;
class UCompositeCurveTable;
class ATarget;
class UBoxComponent;
class UReinforcementLearningComponent;
class USpawnAreaManagerComponent;
struct FAccuracyData;
struct FPlayerSettings_Game;
struct FBS_TargetConfig;

DECLARE_LOG_CATEGORY_EXTERN(LogTargetManager, Log, All);

static constexpr int32 DefaultNumTargetsToActivate = 100;
static constexpr int32 DefaultMinToActivate_MinClamp = 1;
static constexpr int32 MaxToActivate_MinClamp = 1;

static const TArray AnyDirectionModeMultipliers = {
	FVector(0, -1, -1), FVector(0, 1, -1), FVector(0, -1, 1), FVector(0, 1, 1)
};

/** Class responsible for spawning and managing targets for all game modes. */
UCLASS()
class BEATSHOT_API ATargetManager : public AActor
{
	GENERATED_BODY()

	friend class UBSCheatManager;
	friend class ABeatShotGameModeFunctionalTest;
	friend class FTargetManagerTestBase;
	friend class FTargetManagerTestWithWorld;

public:
	ATargetManager();

	virtual void Tick(float DeltaTime) override;

	/** Delegate that is executed every time a target has been activated. */
	FOnTargetActivated OnTargetActivated;

	/** Delegate that is executed every time a target has been spawned, and passes a pointer to that target.
	 *  Used for AimBot */
	FOnTargetActivated_AimBot OnTargetActivated_AimBot;

	/** Used to notify DefaultCharacter when a BeatTrack target has changed directions. */
	FOnBeatTrackDirectionChanged OnBeatTrackDirectionChanged;

	/** Delegate that is broadcast when a target takes any type of damage. */
	FPostTargetDamageEvent PostTargetDamageEvent;

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	/** The current 2D spawn area. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	TObjectPtr<UBoxComponent> SpawnBox;

	/** The 2D spawn area representing the largest the SpawnBox will ever be. Visual only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	TObjectPtr<UBoxComponent> StaticExtentsBox;

	/** Three-dimensional spawn area that all targets must fit inside. Used to update boundary boxes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	TObjectPtr<UBoxComponent> SpawnVolume;

	/** All directional boxes act as a boundary for moving targets */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	TObjectPtr<UBoxComponent> TopBox;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	TObjectPtr<UBoxComponent> BottomBox;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	TObjectPtr<UBoxComponent> LeftBox;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	TObjectPtr<UBoxComponent> RightBox;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	TObjectPtr<UBoxComponent> ForwardBox;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Components")
	TObjectPtr<UBoxComponent> BackwardBox;

	/** Reinforcement learning agent component. */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Components")
	TObjectPtr<UReinforcementLearningComponent> RLComponent;

	/** Manages spawn points. */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Components")
	TObjectPtr<USpawnAreaManagerComponent> SpawnAreaManager;

	/** The target actor to spawn. */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Classes")
	TSubclassOf<ATarget> TargetToSpawn;

	/** Curves to look up values for Dynamic SpawnArea scaling. */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Tables")
	UCompositeCurveTable* CCT_SpawnArea;

	/** Curves to look up values for Dynamic target scaling. */
	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Tables")
	UCompositeCurveTable* CCT_TargetScale;

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Constants")
	FName CurveTableRowName_Cubic_ThresholdMet = FName("Cubic_ThresholdMet");

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Constants")
	FName CurveTableRowName_Linear_ThresholdMet = FName("Linear_ThresholdMet");

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Constants")
	FName CurveTableRowName_Linear_PreThreshold = FName("Linear_PreThreshold");

public:
	/** Initializes  */
	void Init(const TSharedPtr<FBSConfig>& InConfig, const FCommonScoreInfo& InCommonScoreInfo,
		const FPlayerSettings_Game& InPlayerSettings);

	/** Resets all state and destroys all actors. Calls clear on Components who also manage state. */
	void Clear();

protected:
	/** Initializes the Composite Curve Tables. */
	void Init_Tables() const;

public:
	/** Called from BSGameMode */
	void SetShouldSpawn(const bool bShouldSpawn);

	/** Called when a player moves the CrossHair off a target. Updates target colors if they are vulnerable
	 *  to tracking damage. */
	UFUNCTION()
	void OnPlayerStopTrackingTarget();

	/** Called from GameMode when it's an appropriate time to spawn or activate a target. This is the main loop
	 *  that drives this class. */
	void OnAudioAnalyzerBeat();

protected:
	/** Generic spawn function that all game modes use to spawn a target. Initializes the target, binds to its
	 *  delegates, sets the InSpawnArea's Guid, and adds the target to ManagedTargets. */
	virtual ATarget* SpawnTarget(const FTargetSpawnParams& Params);

	/** Adds a Target to the ManagedTargets array, and updates the associated SpawnArea IsManaged flag. */
	void AddToManagedTargets(ATarget* SpawnTarget, const int32 SpawnAreaIndex);

	/** Returns whether the target was activated. Executes any Target Activation Responses
	 *  and calls ActivateTarget on InTarget. */
	bool ActivateTarget(ATarget* InTarget) const;

	/** Calls DeactivateTarget and executes any deactivation responses to the target being deactivated.
	 *  Possible to reactivate the target if conditions permit. */
	virtual void DeactivateTarget(ATarget* InTarget, const bool bExpired, const bool bOutOfHealth) const;

	/** Returns true if the target should be deactivated based on TargetDeactivationConditions. */
	bool ShouldDeactivateTarget(const bool bExpired, const float CurrentHealth,
		const float DeactivationThreshold) const;

	/** Returns true if the target should be destroyed based on TargetDestructionConditions. */
	bool ShouldDestroyTarget(const bool bExpired, const bool bOutOfHealth) const;

	/** Spawns targets at the beginning of a game mode based on the TargetDistributionPolicy. */
	int32 HandleUpfrontSpawning();

	/** Tries to spawn target(s) if there are fewer targets in ManagedTargets than the game mode allows at one time.
	 *  Can be limited by number of targets to activate if the game mode doesn't allow spawning without activation. */
	int32 HandleRuntimeSpawning();

	/** Activate target(s) if there are any ManagedTargets that are not activated and the game modes settings permit. */
	int32 HandleTargetActivation() const;

	/** Returns the number of targets that are allowed to be spawned at once, at runtime. */
	int32 GetNumberOfTargetsToSpawn() const;

	/** Returns the number of targets that are allowed to be activated at once. Will only return values >= 0. */
	int32 GetNumberOfTargetsToActivate(const int32 MaxAvailable, const int32 NumActivated) const;

	/** Returns the scale for next target. */
	FVector FindNextSpawnedTargetScale() const;

	/** Calls GetTargetSpawnParams on the SpawnAreaManager. Updates spawn volume. */
	TSet<FTargetSpawnParams> GetTargetSpawnParams(const int32 NumToSpawn) const;

	/** Finds the correct damage type for the next target spawn. */
	ETargetDamageType FindNextTargetDamageType();

	/** The expiration or destruction of any target is bound to this function, which handles firing delegates,
	 *  target flags, target removal. */
	UFUNCTION()
	void OnTargetDamageEvent(FTargetDamageEvent& Event);

	/** Updates CurrentStreak, based on if the target expired or not. */
	void UpdateCurrentStreak(const FTargetDamageEvent& Event);

	/** Updates DynamicLookUpValue_TargetScale and DynamicLookUpValue_SpawnAreaScale,
	 *  based on if the target expired or not. */
	void UpdateDynamicLookUpValues(const FTargetDamageEvent& Event);

	/** Removes a target from ManagedTargets. */
	void RemoveFromManagedTargets(const FGuid GuidToRemove);

	/** Static function that returns the static location to place the SpawnBox. */
	static FVector GenerateStaticLocation(const FBSConfig* InCfg);

	/** Static function that returns the ending extents for a game mode. If dynamic, this is the extents that the
	 *  curves lerp towards from StartExtents. */
	static FVector GenerateStaticExtents(const FBSConfig* InCfg);

	/** Static function that returns the absolute minimum and maximum corners of the spawn volume, based on the
	 *  Static Extents and SpawnBox origin. */
	static FExtrema GenerateStaticExtrema(const FBSConfig* InCfg, const FVector& InOrigin,
		const FVector& InStaticExtents);

	/** Static function that returns the location to place the SpawnVolume based on the Factor if dynamic or the
	 *  StaticExtents otherwise. X and Y will be the same as the SpawnBox's location. */
	static FVector GenerateSpawnVolumeLocation(const FBSConfig* InCfg, const FVector& InOrigin,
		const FVector& InSpawnVolumeExtents);

	/** Static function that returns the extents to apply to the SpawnVolume based on the Factor if dynamic or the
	 *  StaticExtents otherwise. X and Y will be the same as the SpawnBox's extents. */
	static FVector GenerateSpawnVolumeExtents(const FBSConfig* InCfg, const FVector& InSpawnBoxExtents,
		const FVector& InStaticExtents, const float Factor = 1.f);

	/** Static function that returns the max extrema the SpawnVolume will ever be. Not used currently. */
	static FExtrema GenerateMaxSpawnVolumeExtrema(const FBSConfig* InCfg, const FVector& InOrigin,
		const FVector& InStaticExtents);

	/** Returns SpawnVolume's Location. */
	FVector GetSpawnVolumeLocation() const;

	/** Returns SpawnVolume's BoxExtents. */
	FVector GetSpawnVolumeExtents() const;

	/** Returns a FExtrema struct containing both the min extrema and max extrema of the SpawnVolume. */
	FExtrema GetSpawnVolumeExtrema() const;

	/** Returns SpawnBox's origin. X will always be 3700. */
	FVector GetSpawnBoxOrigin() const;

	/** Returns SpawnBox's current BoxExtents. X will always be 0. */
	FVector GetSpawnBoxExtents() const;

	/** Returns the current min and max extrema for the SpawnBox. X will always be 0. */
	FExtrema GetSpawnBoxExtrema() const;

	/** Sets the SpawnBox's BoxExtents based on the current value of DynamicScaleFactor. */
	void UpdateSpawnBoxExtents(const float Factor) const;

	/** Updates the SpawnVolume and all directional boxes to match the current SpawnBox. */
	virtual void UpdateSpawnVolume(const float Factor) const;

	/** Calls GetNewTargetDirection and sets the new direction of the target. Also bound to targets'
	 *  OnDeactivationResponse_ChangeDirection delegate, which it calls with parameter 2. \n\n
	 *  Spawn = 0, Activation  = 1, Deactivation = 2. */
	void ChangeTargetDirection(ATarget* InTarget, const uint8 InSpawnActivationDeactivation) const;

	/** Returns a new unit vector direction for a target. */
	FVector GetNewTargetDirection(const FVector& LocationBeforeChange, const bool bLastDirectionChangeHorizontal) const;

	static TArray<FVector> GetAnyDirectionMultipliers(const FVector& LocationBeforeChange, const FVector& Origin);

	/** Updates the total amount of damage that can be done if a tracking target is damageable. */
	void UpdateTotalPossibleDamage();

	/** Returns true if a target exists that is vulnerable to tracking damage. */
	bool TrackingTargetIsDamageable() const;

	/** Evaluates the specified curve at InTime. */
	float GetCurveTableValue(const bool bIsSpawnArea, const int32 InTime) const;

	/** Returns the maximum target radius for a game mode. */
	static float GetMaxTargetRadius(const FBS_TargetConfig& InTargetCfg);

	/** Returns the maximum target diameter for a game mode. */
	static float GetMaxTargetDiameter(const FBS_TargetConfig& InTargetCfg);

	/** Destroys all targets in the ManagedTargets map. */
	void DestroyTargets();

	/** Returns a map containing the locations of all moving targets in ManagedTargets. */
	void GetMovingTargetLocations(FMovingTargetLocations& MovingTargetLocations) const;

public:
	/** Function called from BSGameMode any time a player changes settings.
	 *  Propagates to all targets currently active. */
	void UpdatePlayerSettings(const FPlayerSettings_Game& InPlayerSettings);

	/** Called from DefaultGameMode, returns the player accuracy matrix. */
	FAccuracyData GetLocationAccuracy() const;

	/** Saves the QTable inside InCommonScoreInfo. */
	void UpdateCommonScoreInfoQTable(FCommonScoreInfo& InCommonScoreInfo) const;

protected:
	/** Random number stream to keep randomization in sync between HandleRuntimeSpawning and HandleTargetActivation. */
	FRandomStream RandomNumToActivateStream;

	/** Initialized at start of game mode by DefaultGameMode. */
	TSharedPtr<FBSConfig> BSConfig;

	/** whether the TargetManager is allowed to spawn a target at a given time. */
	bool ShouldSpawn;

	/** The type of damage that the last target was vulnerable to. */
	ETargetDamageType LastTargetDamageType;

	/** The scale to apply to the next/current target. */
	FVector CurrentTargetScale;

	/** The min and max extrema, set during initialization. This value can be different from current
	 *  BoxBounds extrema if DynamicSpreadType. */
	FExtrema StaticExtrema;

	/** The static extents for the spawn box. This is half the value that was passed in with the FBSConfig. */
	FVector StaticExtents;

	/** Consecutively destroyed targets. */
	int32 CurrentStreak;

	/** Dimensions of each and every Spawn Area. */
	FIntVector3 SpawnAreaDimensions;

	/** The time to use when looking up values from CCT_TargetScale. Incremented by for each consecutive target hit,
	 *  decremented by setting value. */
	int32 DynamicLookUpValue_TargetScale;

	/** The time to use when looking up values from CCT_SpawnArea. Incremented by for each consecutive target hit,
	 *  decremented by setting value. */
	int32 DynamicLookUpValue_SpawnAreaScale;

	/** A map of spawned Targets that are being actively managed by this class. This is the only place where
	 *  references to spawned targets are stored. */
	UPROPERTY()
	TMap<FGuid, ATarget*> ManagedTargets;

	/** The total amount of ticks while at least one tracking target was damageable. */
	double TotalPossibleDamage;

	/** whether the last activated target direction change was horizontal. */
	mutable bool bLastActivatedTargetDirectionHorizontal;

	/** whether the last spawned target direction change was horizontal. */
	mutable bool bLastSpawnedTargetDirectionHorizontal;

	/** Spawn parameters for spawning targets. */
	FActorSpawnParameters TargetSpawnInfo;

#if !UE_BUILD_SHIPPING

	/** Clears all debug persistent lines and draws debug boxes and/or lines based on debug bool variables. Calls
	 *	same function on Spawn Area Manager. */
	void DrawDebug() const;

	/** Delegate used to record execution time taken for SpawnAreaManager's GetSpawnableSpawnAreas function. */
	TDelegate<void(const double)> ExecutionTimeDelegate;

	/** The most recent sectors for a target that received a new direction when the moving target direction mode is
	 *  set to any. */
	mutable FAnyMovingTargetDirectionModeDebug LastAnyTargetDirectionModeSectors;

	/** Toggles showing debug boxes for choosing a random target location when the moving target direction mode is
	 *  set to any. */
public:
	bool bShowDebug_AnyMovingTargetDirectionMode;

#endif
};
