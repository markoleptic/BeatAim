// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "TargetManagerTestWithWorld.h"
#include "Target/TargetManager.h"
#include "PackageTools.h"

using namespace TargetManagerTestHelpers;

bool FTargetManagerTestWithWorld::Init()
{
	World = UWorld::CreateWorld(EWorldType::Game, false);
	UPackage* Package = World->GetPackage();
	Package->SetFlags(RF_Transient | RF_Public);
	Package->AddToRoot();

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	InitialFrameCounter = GFrameCounter;
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	return InitTargetManager();
}

void FTargetManagerTestWithWorld::CleanUpWorld()
{
	if (TargetManager)
	{
		if (AActor* Actor = Cast<AActor>(TargetManager))
		{
			Actor->RemoveFromRoot();
			Actor->Destroy();
		}
	}
	if (BSConfig.IsValid())
	{
		BSConfig.Reset();
	}
	TargetManager = nullptr;
	BSConfig = nullptr;
	GameModeDataAsset = nullptr;

	if (World)
	{
		UPackage* Package = World->GetPackage();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		World->MarkAsGarbage();
		GFrameCounter = InitialFrameCounter;
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		if (Package)
		{
			Package->RemoveFromRoot();
			TArray<UPackage*> PackagesToUnload;
			PackagesToUnload.Add(Package);
			UPackageTools::UnloadPackages(PackagesToUnload);
		}
	}
}

bool FTargetManagerTestWithWorld::InitTargetManager()
{
	UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, TargetManagerAssetPath);
	if (!LoadedObject)
	{
		AddError("Failed to load Target Manager");
		return false;
	}
	const UBlueprint* BlueprintClass = Cast<UBlueprint>(LoadedObject);
	const TSubclassOf<UObject> GeneratedClass = BlueprintClass->GeneratedClass;
	TargetManager = World->SpawnActor<ATargetManager>(GeneratedClass, TargetManagerTransform, FActorSpawnParameters());
	if (!TargetManager)
	{
		AddError("Failed to spawn Target Manager");
		return false;
	}

	TargetManager->AddToRoot();
	TargetManager->DispatchBeginPlay();
	TargetManager->ExecutionTimeDelegate.BindRaw(this, &FTargetManagerTestWithWorld::UpdateExecutionTime);

	return true;
}

bool FTargetManagerTestWithWorld::InitGameModeDataAsset(const FString& InPath) const
{
	if (GameModeDataAsset) return true;

	UObject* LoadedObject = StaticLoadObject(UBSGameModeDataAsset::StaticClass(), nullptr, *InPath);
	if (!LoadedObject)
	{
		return false;
	}

	GameModeDataAsset = Cast<UBSGameModeDataAsset>(LoadedObject);
	if (!GameModeDataAsset)
	{
		return false;
	}

	return true;
}

TObjectPtr<USpawnAreaManagerComponent> FTargetManagerTestWithWorld::GetSpawnAreaManager() const
{
	if (TargetManager)
	{
		return TargetManager->SpawnAreaManager;
	}
	return TObjectPtr<USpawnAreaManagerComponent>(nullptr);
}

TMap<FGuid, ATarget*> FTargetManagerTestWithWorld::GetManagedTargets() const
{
	if (TargetManager)
	{
		return TargetManager->ManagedTargets;
	}
	return TMap<FGuid, ATarget*>();
}

void FTargetManagerTestWithWorld::TickWorld(float Time)
{
	constexpr float Step = 0.1f;
	while (Time > 0.f)
	{
		World->Tick(ELevelTick::LEVELTICK_All, FMath::Min(Time, Step));
		Time -= Step;
		GFrameCounter++;
	}
}

void FTargetManagerTestWithWorld::UpdateExecutionTime(const double Time)
{
	TargetSpawnParamsExecutionTime += Time;
}
