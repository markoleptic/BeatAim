// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "DefaultGameInstance.h"
#include "GameModeActorBase.h"
#include "SphereTarget.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	GI = Cast<UDefaultGameInstance>(UGameplayStatics::GetGameInstance(this));
	Health = MaxHealth;
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::DamageTaken);
	ShouldUpdateTotalPossibleDamage = false;
	TotalPossibleDamage = 0.f;
	if (Cast<ASphereTarget>(GetOwner()) && GI->GameModeActorStruct.IsBeatTrackMode)
	{
		ShouldUpdateTotalPossibleDamage = true;
	}
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (ShouldUpdateTotalPossibleDamage)
	{
		TotalPossibleDamage++;
		GI->GameModeActorBaseRef->UpdateTrackingScore(0.f, TotalPossibleDamage);
	}
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth)
{
	MaxHealth = NewMaxHealth;
	Health = MaxHealth;
}

void UHealthComponent::DamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* Instigator, AActor* DamageCauser)
{
	if (Damage <= 0.f) return;

	Health -= Damage;

	if (ASphereTarget* DamagedTarget = Cast<ASphereTarget>(DamagedActor))
	{
		if (Health <= 0.f)
		{
			DamagedTarget->HandleDestruction();
		}
		else if (Health > 101 && GI->GameModeActorStruct.IsBeatTrackMode == true)
		{
			GI->GameModeActorBaseRef->UpdateTrackingScore(Damage, TotalPossibleDamage);
		}
		else if (Health > 101 && GI->GameModeActorStruct.IsBeatGridMode == true)
		{
			GI->GameModeActorBaseRef->UpdateTargetsHit();
			DamagedTarget->HandleDestruction();
		}
	}
}

