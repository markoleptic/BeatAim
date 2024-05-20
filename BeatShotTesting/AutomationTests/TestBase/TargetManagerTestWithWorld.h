// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TargetManagerTestBase.h"
#include "BSGameModeConfig/BSGameModeDataAsset.h"
#include "Misc/AutomationTest.h"

/** Base class for unit tests dealing with the TargetManager, with a world. */
class FTargetManagerTestWithWorld : public FTargetManagerTestBase
{
	using Super = ::FTargetManagerTestBase;

public:
	FTargetManagerTestWithWorld(const FString& InName, const bool bInComplexTask): Super(InName, bInComplexTask),
		World(nullptr), InitialFrameCounter(0), TargetSpawnParamsExecutionTime(0.0f), TargetManager(nullptr),
		GameModeDataAsset(nullptr)
	{
	}

	virtual ~FTargetManagerTestWithWorld() override
	{
		CleanUpWorld();
	}

	virtual bool Init() override;

	void CleanUpWorld();

protected:
	/** Ticks the World and increments GFrameCounter. */
	void TickWorld(float Time);

	/** The World created in Init. */
	UWorld* World;

	/** GFrameCounter prior to any increments. */
	uint64 InitialFrameCounter;

	/** Total time spent executing the GetSpawnableSpawnAreas function. */
	double TargetSpawnParamsExecutionTime;

	/** Increments TimeSpentInSpawnableSpawnAreas. */
	void UpdateExecutionTime(const double Time);

	/** Initializes the Target Manager. */
	virtual bool InitTargetManager();

	/** Initializes the Target Manager. */
	virtual bool InitGameModeDataAsset(const FString& InPath) const;

	/** Returns the SpawnAreaManager. */
	TObjectPtr<USpawnAreaManagerComponent> GetSpawnAreaManager() const;

	/** Returns the ManagedTargets map. */
	TMap<FGuid, ATarget*> GetManagedTargets() const;

	/** Pointer to the Target Manager. */
	ATargetManager* TargetManager;

	/** Shared pointer to the Game mode config. Should be initialized in actual tests. */
	TSharedPtr<FBSConfig> BSConfig;

	/** Pointer to Game Mode Data Asset containing read-only game mode config data.
	 *  Mutable so that it can be set during GetTests. */
	mutable UBSGameModeDataAsset* GameModeDataAsset;

	const FTransform TargetManagerTransform = FTransform(FRotator(), Constants::DefaultTargetManagerLocation,
		FVector(1.f));

	/** Map to pull game mode configs from during tests. */
	mutable TMap<FString, FBSConfig> TestMap;
};
