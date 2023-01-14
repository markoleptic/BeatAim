// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveGameCustomGameMode.h"
#include "SaveLoadInterface.h"
#include "Engine/GameInstance.h"
#include "DefaultGameInstance.generated.h"

class AGameModeBase;
class ASphereTarget;
class AGameModeActorBase;
class ADefaultPlayerController;

UCLASS()
class BEATSHOT_API UDefaultGameInstance : public UGameInstance, public ISaveLoadInterface
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void StartGameMode(const bool bShowOpenFileDialog) const;
	UFUNCTION()
	void HandleGameModeTransition(const FGameModeTransitionState& GameModeTransitionState);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameModeActorStruct GameModeActorStruct;
	bool bLastSavedShowOpenFileDialog;
};


