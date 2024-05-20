// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Equipment/BSKnife.h"
#include "BSGameInstance.h"
#include "SaveGames/SaveGamePlayerSettings.h"


ABSKnife::ABSKnife()
{
	PrimaryActorTick.bCanEverTick = false;

	Knife = CreateDefaultSubobject<UStaticMeshComponent>("Knife");
	SetRootComponent(Knife);
}

void ABSKnife::BeginPlay()
{
	Super::BeginPlay();

	OnPlayerSettingsChanged(LoadPlayerSettings().Game);

	UBSGameInstance* GI = Cast<UBSGameInstance>(GetGameInstance());
	GI->RegisterPlayerSettingsSubscriber<ABSKnife, FPlayerSettings_Game>(this, &ABSKnife::OnPlayerSettingsChanged);
}

void ABSKnife::OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings)
{
	SetShowKnifeMesh(GameSettings.bShowWeaponMesh);
}

void ABSKnife::SetShowKnifeMesh(const bool bShow)
{
	SetActorHiddenInGame(!bShow);
}
