// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SphereTarget.generated.h"

class UDefaultHealthComponent;
class UCapsuleComponent;
class UNiagaraSystem;
class UCurveFloat;
class UCurveLinearColor;

UCLASS()
class BEATSHOT_API ASphereTarget : public AActor
{
	GENERATED_BODY()

public:
	ASphereTarget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Target Properties", BlueprintReadOnly)
		UCapsuleComponent* CapsuleComp;

	UPROPERTY(VisibleAnywhere, Category = "Target Properties", BlueprintReadOnly)
		UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere, Category = "Target Properties", BlueprintReadOnly)
		FTimerHandle TimeSinceSpawn;

	UPROPERTY(EditAnywhere, Category = "Effects", BlueprintReadWrite)
		UNiagaraSystem* NS_Standard_Explosion;

	UFUNCTION(BlueprintCallable, Category = "Target Handling")
		void HandleDestruction();

	UPROPERTY(VisibleAnywhere, Category = "References", BlueprintReadOnly)
		class UDefaultGameInstance* GI;

	UPROPERTY(EditAnywhere, Category = "Materials", BlueprintReadWrite)
		UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, Category = "Materials", BlueprintReadWrite)
		UMaterialInstanceDynamic* MID_TargetColorChanger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
		UDefaultHealthComponent* HealthComp;

	UFUNCTION(BlueprintCallable, Category = "Target Properties")
		void SetMaxHealth(float NewMaxHealth);

	// base radius for sphere target
	const float BaseSphereRadius = 50.f;

	// soft white color for BeatGrid
	const FLinearColor OffWhite = { 0.75 , 0.75, 0.75, 1 };

	// Blueprint implementable events
	UFUNCTION(BlueprintImplementableEvent, Category = "Target Properties")
		void PlayColorGradient();

	UFUNCTION(BlueprintImplementableEvent, Category = "Target Properties")
		void ShowTargetExplosion();

	UFUNCTION(BlueprintImplementableEvent, Category = "Target Properties")
		void RemoveAndReappear();
};
