// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveGamePlayerSettings.h"
#include "SaveLoadInterface.h"
#include "GameFramework/Actor.h"
#include "Visualizer.generated.h"

class UAudioAnalyzerManager;
UCLASS()
class BEATSHOT_API AVisualizer : public AActor, public ISaveLoadInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVisualizer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
	UAudioAnalyzerManager* AAManager;

	UFUNCTION()
	void OnAAPlayerLoaded(UAudioAnalyzerManager* Manager);

	UFUNCTION(BlueprintImplementableEvent)
	void InitializeCubes(int32 NewNumBandChannels);

	UFUNCTION()
	void UpdateAASettings(const FAASettingsStruct AASettingsStruct);

	UPROPERTY(BlueprintReadOnly)
	bool bAAManagerInitialized;

	UPROPERTY(BlueprintReadOnly)
	FAASettingsStruct AASettings;
};


