// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "AbilitySystem/BSAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/BSGameplayAbility.h"
#include "Animation/BSAnimInstance.h"
#include "BeatShot/BSGameplayTags.h"
#include "Character/BSCharacterBase.h"


void UBSAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	if (UBSAnimInstance* AnimInstance = Cast<UBSAnimInstance>(ActorInfo->GetAnimInstance()))
	{
		AnimInstance->InitializeWithAbilitySystem(this);
	}
}

void UBSAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc,
	bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		UBSGameplayAbility* AbilityCDO = CastChecked<UBSGameplayAbility>(AbilitySpec.Ability);

		if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			// Cancel all the spawned instances, not the CDO.
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			for (UGameplayAbility* AbilityInstance : Instances)
			{
				UBSGameplayAbility* BSAbilityInstance = CastChecked<UBSGameplayAbility>(AbilityInstance);

				if (ShouldCancelFunc(BSAbilityInstance, AbilitySpec.Handle))
				{
					if (BSAbilityInstance->CanBeCanceled())
					{
						BSAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(),
							BSAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
					}
					else
					{
						UE_LOG(LogTemp, Error,
							TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."),
							*BSAbilityInstance->GetName());
					}
				}
			}
		}
		else
		{
			// Cancel the non-instanced ability CDO.
			if (ShouldCancelFunc(AbilityCDO, AbilitySpec.Handle))
			{
				// Non-instanced abilities can always be canceled.
				check(AbilityCDO->CanBeCanceled());
				AbilityCDO->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(),
					bReplicateCancelAbility);
			}
		}
	}
}

void UBSAbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](const UBSGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)
	{
		const EBSAbilityActivationPolicy ActivationPolicy = Ability->GetActivationPolicy();
		return ((ActivationPolicy == EBSAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy ==
			EBSAbilityActivationPolicy::WhileInputActive));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

bool UBSAbilitySystemComponent::IsActivationGroupBlocked(EBSAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case EBSAbilityActivationGroup::Independent:
		// Independent abilities are never blocked.
		bBlocked = false;
		break;

	case EBSAbilityActivationGroup::Exclusive_Replaceable:
	case EBSAbilityActivationGroup::Exclusive_Blocking:
		// Exclusive abilities can activate if nothing is blocking.
		bBlocked = (ActivationGroupCounts[static_cast<uint8>(EBSAbilityActivationGroup::Exclusive_Blocking)] > 0);
		break;

	default: checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), static_cast<uint8>(Group));
		break;
	}

	return bBlocked;
}

void UBSAbilitySystemComponent::AddAbilityToActivationGroup(EBSAbilityActivationGroup Group,
	UBSGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[static_cast<uint8>(Group)] < INT32_MAX);

	ActivationGroupCounts[static_cast<uint8>(Group)]++;

	constexpr bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case EBSAbilityActivationGroup::Independent:
		// Independent abilities do not cancel any other abilities.
		break;

	case EBSAbilityActivationGroup::Exclusive_Replaceable:
	case EBSAbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(EBSAbilityActivationGroup::Exclusive_Replaceable, Ability,
			bReplicateCancelAbility);
		break;

	default: checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"),
			static_cast<uint8>(Group));
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[static_cast<uint8>(
		EBSAbilityActivationGroup::Exclusive_Replaceable)] + ActivationGroupCounts[static_cast<uint8>(
		EBSAbilityActivationGroup::Exclusive_Blocking)];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogTemp, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void UBSAbilitySystemComponent::RemoveAbilityFromActivationGroup(EBSAbilityActivationGroup Group,
	UBSGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[static_cast<uint8>(Group)] > 0);

	ActivationGroupCounts[static_cast<uint8>(Group)]--;
}

void UBSAbilitySystemComponent::CancelActivationGroupAbilities(EBSAbilityActivationGroup Group,
	UBSGameplayAbility* IgnoreAbility, bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreAbility](const UBSGameplayAbility* Ability,
		FGameplayAbilitySpecHandle Handle)
	{
		return ((Ability->GetActivationGroup() == Group) && (Ability != IgnoreAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UBSAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle,
	FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(
		FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}

TArray<FGameplayAbilitySpec*> UBSAbilitySystemComponent::GetAbilitySpecsFromGameplayTag(
	const FGameplayTag& InputTag) const
{
	FGameplayTagContainer Container;
	TArray<FGameplayAbilitySpec*> Activatable;
	Container.AddTag(InputTag);
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(Container, Activatable);
	if (!Activatable.IsEmpty())
	{
		return Activatable;
	}
	return TArray<FGameplayAbilitySpec*>();
}

void UBSAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);
	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle,
			Spec.ActivationInfo.GetActivationPredictionKey());
	}
}

void UBSAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle,
			Spec.ActivationInfo.GetActivationPredictionKey());
	}
}

void UBSAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(BSGameplayTags::Ability_InputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	// Process all abilities that activate when the input is held.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UBSGameplayAbility* BSAbilityCDO = CastChecked<UBSGameplayAbility>(AbilitySpec->Ability);

				if (BSAbilityCDO->GetActivationPolicy() == EBSAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	// Process all abilities that had their input pressed this frame.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;
				if (AbilitySpec->IsActive())
				{
					if (CastChecked<UBSGameplayAbility>(AbilitySpec->Ability)->GetActivationPolicy() ==
						EBSAbilityActivationPolicy::SpammableTriggered)
					{
						// Even though ability is active, activating it will cancel and reactivate
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
					else
					{
						// Ability is active and not spammable so pass along the input event.
						AbilitySpecInputPressed(*AbilitySpec);
					}
				}
				else if (const UBSGameplayAbility* BSAbilityCDO = CastChecked<UBSGameplayAbility>(AbilitySpec->Ability);
					BSAbilityCDO->GetActivationPolicy() == EBSAbilityActivationPolicy::OnInputTriggered || BSAbilityCDO
					->GetActivationPolicy() == EBSAbilityActivationPolicy::SpammableTriggered)
				{
					// Ability is not active and but should be since input has been triggered
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send a input event to the ability because of the press.
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	// Process all abilities that had their input released this frame.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	// Clear the cached ability handles.
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UBSAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void UBSAbilitySystemComponent::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.IsValid())
	{
		for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UBSAbilitySystemComponent::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}
