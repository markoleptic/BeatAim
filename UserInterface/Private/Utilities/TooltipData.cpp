// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/TooltipData.h"
#include "Utilities/TooltipIcon.h"
#include "Utilities/BSWidgetInterface.h"

int32 FTooltipData::GId = 0;

FTooltipData::FTooltipData(): TooltipIconType(ETooltipIconType::Default), bAllowTextWrap(false), Id(GId++)
{
}

FTooltipData::FTooltipData(UTooltipIcon* InTooltipIcon): TooltipIconType(ETooltipIconType::Default),
                                                         TooltipIcon(InTooltipIcon), bAllowTextWrap(false), Id(GId++)
{
}

FText FTooltipData::GetTooltipText() const
{
	return TooltipText;
}

bool FTooltipData::GetAllowTextWrap() const
{
	return bAllowTextWrap;
}

/*void FTooltipData::UpdateDynamicTooltipText(const FDynamicTooltipState& InUpdateData)
{
	if (!bIsDynamic)
	{
		return;
	}
	if (InUpdateData.Actual > InUpdateData.MaxAllowed && (!FMath::IsNearlyEqual(LastActual, InUpdateData.Actual) || !
		FMath::IsNearlyEqual(LastMaxAllowed, InUpdateData.MaxAllowed)))
	{
		if (InUpdateData.MaxAllowed < DynamicTooltipData.MinAllowed)
		{
			TooltipText = DynamicTooltipData.FallbackText;
		}
		else
		{
			float NewValue;
			if (DynamicTooltipData.Precision == 0)
			{
				NewValue = roundf(InUpdateData.MaxAllowed);
			}
			else
			{
				const float Multiplier = FMath::Pow(10.f, DynamicTooltipData.Precision);
				NewValue = roundf(InUpdateData.MaxAllowed * Multiplier) / Multiplier;
			}
			TooltipText = FText::FromString(
				DynamicTooltipData.TryChangeString + FString::SanitizeFloat(NewValue, 0) + ".");
		}
		bIsDirty = true;
	}

	LastActual = InUpdateData.Actual;
	LastMaxAllowed = InUpdateData.MaxAllowed;

	if (InUpdateData.bOverride)
	{
		SetShouldShowTooltipImage(false);
	}
	else
	{
		SetShouldShowTooltipImage(InUpdateData.Actual > InUpdateData.MaxAllowed);
	}
}*/


void Func(const FText& StringTableText, const FTextFormat& StringTableTextFormat)
{
	FNumberFormattingOptions Options = FNumberFormattingOptions();

	TArray<FString> ArgNames;
	StringTableTextFormat.GetFormatArgumentNames(ArgNames);
	FFormatNamedArguments Args;

	FText FormattedText = FText::Format(StringTableTextFormat, Args);
}
