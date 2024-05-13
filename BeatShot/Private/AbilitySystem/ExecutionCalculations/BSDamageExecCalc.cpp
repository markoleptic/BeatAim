// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "AbilitySystem/ExecutionCalculations/BSDamageExecCalc.h"
#include "BeatShot/BSGameplayTags.h"
#include "AbilitySystem/Globals/BSAttributeSetBase.h"

// Declare the attributes to capture and define how we want to capture them from the Source and Target.
struct BSDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition HitDamageDef;
	FGameplayEffectAttributeCaptureDefinition TrackingDamageDef;
	FGameplayEffectAttributeCaptureDefinition SelfDamageDef;

	BSDamageStatics()
	{
		HitDamageDef = FGameplayEffectAttributeCaptureDefinition(UBSAttributeSetBase::GetHitDamageAttribute(),
			EGameplayEffectAttributeCaptureSource::Source, true);
		TrackingDamageDef = FGameplayEffectAttributeCaptureDefinition(UBSAttributeSetBase::GetTrackingDamageAttribute(),
			EGameplayEffectAttributeCaptureSource::Source, true);
		SelfDamageDef = FGameplayEffectAttributeCaptureDefinition(UBSAttributeSetBase::GetSelfDamageAttribute(),
			EGameplayEffectAttributeCaptureSource::Source, true);

		// Capture the Target's Health. Don't snapshot.
		//HealthDef = FGameplayEffectAttributeCaptureDefinition(UBSAttributeSetBase::GetHealthAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
	}
};

static const BSDamageStatics& DamageStatics()
{
	static BSDamageStatics DStatics;
	return DStatics;
}

UBSDamageExecCalc::UBSDamageExecCalc()
{
	RelevantAttributesToCapture.Add(DamageStatics().HitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().TrackingDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().SelfDamageDef);
}

void UBSDamageExecCalc::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// Gather the tags from the source and target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float HitDamage = 0.0f;
	if (!TargetTags->HasTag(BSGameplayTags::Target_State_Immune_HitDamage))
	{
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().HitDamageDef, EvaluationParameters,
			HitDamage);
	}

	float TrackingDamage = 0.0f;
	if (!TargetTags->HasTag(BSGameplayTags::Target_State_Immune_TrackingDamage))
	{
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().TrackingDamageDef,
			EvaluationParameters, TrackingDamage);
	}

	float SelfDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().SelfDamageDef, EvaluationParameters,
		SelfDamage);

	if (HitDamage > 0.0f)
	{
		// Set the Target's Incoming Hit Damage meta attribute, this gets turned into - health on the target
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UBSAttributeSetBase::GetIncomingHitDamageAttribute(), EGameplayModOp::Additive, HitDamage));
	}

	if (TrackingDamage > 0.0f)
	{
		// Set the Target's Incoming Tracking Damage meta attribute, this gets turned into - health on the target
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UBSAttributeSetBase::GetIncomingTrackingDamageAttribute(), EGameplayModOp::Additive, TrackingDamage));
	}

	if (TrackingDamage <= 0.0f && HitDamage <= 0.0f && SelfDamage > 0.0f)
	{
		// Set the Target's Self Damage meta attribute, this gets turned into - health on the target
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UBSAttributeSetBase::GetIncomingSelfDamageAttribute(), EGameplayModOp::Additive, SelfDamage));
	}
	// OutExecutionOutput.AddOutputModifier(
	//		FGameplayModifierEvaluatedData(
	//			UBSAttributeSetBase::GetHealthAttribute(),
	//			EGameplayModOp::Additive,
	//			-IncomingTotalDamage));
}
