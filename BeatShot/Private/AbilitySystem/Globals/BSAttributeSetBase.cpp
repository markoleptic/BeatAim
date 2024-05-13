// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.
// Credit to Dan Kestranek.

#include "AbilitySystem/Globals/BSAttributeSetBase.h"
#include "GameplayEffectExtension.h"
#include "BeatShot/BSGameplayTags.h"
#include "Net/UnrealNetwork.h"

UBSAttributeSetBase::UBSAttributeSetBase()
{
	MaxHealthBeforeAttributeChange = 0.f;
	HealthBeforeAttributeChange = 0.f;
	bOutOfHealth = false;
	Health = 100.f;
	MaxHealth = 100.f;

	HitDamage = 100.f;
	TrackingDamage = 1.f;
	SelfDamage = 0.f;

	IncomingHitDamage = 0.f;
	IncomingTrackingDamage = 0.f;
	IncomingTotalDamage = 0.f;
	IncomingSelfDamage = 0.f;
}

void UBSAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// This is called whenever attributes change, so for max health we want to scale the current totals to match
	Super::PreAttributeChange(Attribute, NewValue);

	// If a Max value changes, adjust current to keep Current % of Current to Max
	if (Attribute == GetMaxHealthAttribute())
	// GetMaxHealthAttribute comes from the Macros defined at the top of the header
	{
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
	}
}

void UBSAttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

bool UBSAttributeSetBase::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	// Save the current health
	HealthBeforeAttributeChange = GetHealth();
	MaxHealthBeforeAttributeChange = GetMaxHealth();

	return true;
}

void UBSAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();

	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	FGameplayTagContainer Container;
	Data.EffectSpec.GetAllAssetTags(Container);

	ETargetDamageType DamageType = ETargetDamageType::None;

	// Convert into -Health and then clamp
	if (Data.EvaluatedData.Attribute == GetIncomingHitDamageAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth() - GetIncomingHitDamage(), MinPossibleHealth, GetMaxHealth()));
		SetIncomingHitDamage(0.0f);
		DamageType = ETargetDamageType::Hit;
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingTrackingDamageAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth() - GetIncomingTrackingDamage(), MinPossibleHealth, GetMaxHealth()));
		SetIncomingTrackingDamage(0.0f);
		DamageType = ETargetDamageType::Tracking;
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingTotalDamageAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth() - GetIncomingTotalDamage(), MinPossibleHealth, GetMaxHealth()));
		SetIncomingTotalDamage(0.0f);
		DamageType = ETargetDamageType::Combined;
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingSelfDamageAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth() - GetIncomingSelfDamage(), MinPossibleHealth, GetMaxHealth()));
		SetIncomingSelfDamage(0.0f);
		DamageType = ETargetDamageType::Self;
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), MinPossibleHealth, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		// Notify any requested max health changes
		OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			MaxHealthBeforeAttributeChange, GetMaxHealth());
	}

	// If health has actually changed activate callbacks. Also check that it isn't resetting health, in which case we
	// don't want the target to be notified
	if (GetHealth() != HealthBeforeAttributeChange && !Container.HasTag(BSGameplayTags::Target_ResetHealth))
	{
		if (DamageType != ETargetDamageType::None)
		{
			const FDamageEventData DamageEvent(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
				HealthBeforeAttributeChange, GetHealth(), DamageType);
			OnDamageTaken.Broadcast(DamageEvent);
		}
		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			HealthBeforeAttributeChange, GetHealth());
	}

	if (GetHealth() <= 0.0f && !bOutOfHealth)
	{
		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			HealthBeforeAttributeChange, GetHealth());
	}

	// Check health again in case an event above changed it.
	bOutOfHealth = GetHealth() <= 0.0f;
}

void UBSAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBSAttributeSetBase, Health, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UBSAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UBSAttributeSetBase, HitDamage, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UBSAttributeSetBase, TrackingDamage, COND_None, REPNOTIFY_OnChanged);
}

void UBSAttributeSetBase::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute,
	const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f)
			? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue
			: NewMaxValue;

		// Clamp the max to be less than Max - Current since it gets added
		NewDelta = FMath::Clamp(NewDelta, 0.f, 10000000 - CurrentValue);

		AbilityComp->ApplyModToAttribute(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void UBSAttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBSAttributeSetBase, Health, OldHealth);
}

void UBSAttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBSAttributeSetBase, MaxHealth, OldMaxHealth);
}

void UBSAttributeSetBase::OnRep_HitDamage(const FGameplayAttributeData& OldHitDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBSAttributeSetBase, HitDamage, OldHitDamage);
}

void UBSAttributeSetBase::OnRep_TrackingDamage(const FGameplayAttributeData& OldTrackingDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBSAttributeSetBase, TrackingDamage, OldTrackingDamage);
}
