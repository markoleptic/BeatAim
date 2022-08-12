// Copyright Epic Games, Inc. All Rights Reserved.

#include "BeatAimGameModeBase.h"
#include "SphereTarget.h"
#include "TargetSpawner.h"
#include "DefaultCharacter.h"
#include "DefaultGameInstance.h"
#include "PlayerHUD.h"
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void ABeatAimGameModeBase::ActorDied(AActor* DeadActor)
{
	if (ASphereTarget* DestroyedTarget = Cast<ASphereTarget>(DeadActor))
	{
		DestroyedTarget->HandleDestruction();
	}
	if (ASpiderShotSelector* DestroyedSelector = Cast<ASpiderShotSelector>(DeadActor))
	{
		HandleGameStart(DestroyedSelector);
	}
}

void ABeatAimGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	// Store reference of GameModeBase in Game Instance
	GI = Cast<UDefaultGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		GI->RegisterGameModeBase(this);
	}

	//Default class specific values
	GameModeSelected = false;
}

void ABeatAimGameModeBase::ShowPlayerHUD()
{
	GI->DefaultCharacterRef->LoadGame();
	GI->DefaultCharacterRef->ShowPlayerHUD(true);
}

void ABeatAimGameModeBase::StartSpiderShot()
{
	if (GameModeSelected == true)
	{
		GI->TargetSpawnerRef->SetShouldSpawn(true);
		GetWorldTimerManager().SetTimer(SpiderShotGameLength, this, &ABeatAimGameModeBase::EndSpiderShot, 60.f, false);
		if (SpiderShotGameLength.IsValid())
		{
			GI->TargetSpawnerRef->SpawnActor();
		}
	}
}

void ABeatAimGameModeBase::EndSpiderShot()
{
	if (GI->GetScore() >  GI->GetHighScore())
	{
		GI->UpdateHighScore(GI->GetScore());
	}
	GI->DefaultCharacterRef->SaveGame();
	GameModeSelected = false;
	GI->TargetSpawnerRef->SetShouldSpawn(false);
	SpiderShotGameLength.Invalidate();
	if (GI->SphereTargetRef)
	{
		GI->SphereTargetRef->HandleDestruction();
	}
	GI->DefaultCharacterRef->ShowPlayerHUD(false);
}

/* Called by DefaultCharacter to update shots fired.
 * Called by Projectile to update targets hit.
 * Called by TargetSpawner to update targets spawned.
 */
void ABeatAimGameModeBase::UpdatePlayerStats(bool ShotFired, bool TargetHit, bool TargetSpawned)
{
	if (ShotFired == true)
	{
		GI->UpdateShotsFired();
		GI->DefaultCharacterRef->PlayerHUD->SetShotsFired(GI->GetShotsFired());
	}
	if (TargetHit == true)
	{
		GI->UpdateTargetsHit();
		GI->DefaultCharacterRef->PlayerHUD->SetTargetsHit(GI->GetTargetsHit());
	}
	if (TargetSpawned == true)
	{
		GI->UpdateTargetsSpawned();
		GI->DefaultCharacterRef->PlayerHUD->SetTargetsSpawned(GI->GetTargetsSpawned());
	}
	GI->DefaultCharacterRef->PlayerHUD->SetAccuracy(GI->GetTargetsHit(), GI->GetShotsFired());
	GI->DefaultCharacterRef->PlayerHUD->SetTargetBar(GI->GetTargetsHit(), GI->GetShotsFired());
}

void ABeatAimGameModeBase::ResetPlayerStats()
{
	GI->UpdateShotsFired(true);
	GI->UpdateTargetsHit(true);
	GI->UpdateTargetsSpawned(true);
	GI->UpdateScore(0,true);

	GI->DefaultCharacterRef->PlayerHUD->SetShotsFired(GI->GetShotsFired());
	GI->DefaultCharacterRef->PlayerHUD->SetTargetsHit(GI->GetTargetsHit());
	GI->DefaultCharacterRef->PlayerHUD->SetTargetsSpawned(GI->GetTargetsSpawned());

	GI->DefaultCharacterRef->PlayerHUD->SetAccuracy(GI->GetTargetsHit(), GI->GetShotsFired());
	GI->DefaultCharacterRef->PlayerHUD->SetTargetBar(GI->GetTargetsHit(), GI->GetShotsFired());

	GI->DefaultCharacterRef->PlayerHUD->SetCurrentScore(GI->GetScore());
}
