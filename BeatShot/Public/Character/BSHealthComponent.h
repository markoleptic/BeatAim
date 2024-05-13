// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/Globals/BSAttributeSetBase.h"
#include "BSHealthComponent.generated.h"

class UBSAbilitySystemComponent;
class UBSAttributeSetBase;

/** Base HealthComponent for this game */
UCLASS(ClassGroup=(Custom), meta= (BlueprintSpawnableComponent))
class BEATSHOT_API UBSHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties */
	UBSHealthComponent();

protected:
	/** Called when the game starts */
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<UBSAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const UBSAttributeSetBase> AttributeSetBase;

	/** Callback function for when the owner's AttributeSetBase OnHealthChanged delegate is broadcast */
	void OnHealthAttributeChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec,
		float EffectMagnitude, float OldValue, float NewValue);

	/** Callback function for when the owner's AttributeSetBase OnDamageTaken delegate is broadcast */
	void OnDamageTaken(const FDamageEventData& DamageEvent);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** Initialize the component using an ability system component */
	UFUNCTION(BlueprintCallable, Category = "BeatShot|Health")
	void InitializeWithAbilitySystem(UBSAbilitySystemComponent* InASC);

	/** Broadcasts when the health attribute changes */
	FBSAttributeEvent OnHealthChangedDelegate;

	/** Broadcast any time the owner's AttributeSetBase takes damage */
	FBSDamageEvent OnDamageTakenDelegate;
};
