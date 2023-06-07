// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Character/BSHealthComponent.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/BSAbilitySystemGlobals.h"
#include "Target/SphereTarget.h"
#include "BeatShot/BSGameplayTags.h"
#include "AbilitySystem/AttributeSets/BSAttributeSetBase.h"
#include "GameFramework/Actor.h"

UBSHealthComponent::UBSHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	AbilitySystemComponent = nullptr;
	AttributeSetBase = nullptr;
	TotalPossibleDamage = 0.f;
}

void UBSHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	TotalPossibleDamage = 0.f;
}

void UBSHealthComponent::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	AActor* Instigator = nullptr;
	if (const FGameplayEffectSpec* Spec = ChangeData.GEModData ? &ChangeData.GEModData->EffectSpec : UBSAbilitySystemGlobals::GetTestGlobals().GetCurrentAppliedGE())
	{
		const FGameplayEffectContextHandle& EffectContext = Spec->GetEffectContext();
		Instigator = EffectContext.GetOriginalInstigator();
	}
	OnHealthChanged.Broadcast(Instigator, ChangeData.OldValue, ChangeData.NewValue, TotalPossibleDamage);
}

void UBSHealthComponent::HandleMaxHealthChanged(const FOnAttributeChangeData& ChangeData)
{
}

void UBSHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
	OnOutOfHealth.Broadcast();
}

void UBSHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UBSHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ShouldUpdateTotalPossibleDamage)
	{
		TotalPossibleDamage++;
	}
}

void UBSHealthComponent::InitializeWithAbilitySystem(UBSAbilitySystemComponent* InASC, const FGameplayTagContainer& GameplayTagContainer)
{
	const AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("BSHealthComponent: Health component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("BSHealthComponent: Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	AttributeSetBase = AbilitySystemComponent->GetSet<UBSAttributeSetBase>();
	if (!AttributeSetBase)
	{
		UE_LOG(LogTemp, Error, TEXT("BSHealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// Register to listen for attribute changes.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBSAttributeSetBase::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	AttributeSetBase->OnHealthReachZero.AddUObject(this, &ThisClass::HandleOutOfHealth);
}

void UBSHealthComponent::SetShouldUpdateTotalPossibleDamage(const bool bShouldUpdate, const FGameplayTagContainer& TagContainer)
{
	if (TagContainer.HasTagExact(FBSGameplayTags::Get().Target_State_Tracking))
	{
		ShouldUpdateTotalPossibleDamage = bShouldUpdate;
		GetWorld()->GetTimerManager().SetTimer(TotalPossibleDamageUpdate, this, &UBSHealthComponent::OnSecondPassedCallback, 1.f, true);
	}
}

void UBSHealthComponent::OnSecondPassedCallback() const
{
	OnSecondPassedTotalPossibleDamage.Broadcast(TotalPossibleDamage);
}
