// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/TooltipData.h"
#include "Utilities/TooltipIcon.h"

int32 FTooltipData::GId = 0;

FTooltipData::FTooltipData(): Id(GId++), bAllowTextWrap(false)
{
	NumberFormattingOptions.SetAlwaysSign(false).SetRoundingMode(HalfFromZero).SetMaximumFractionalDigits(2);
}

FText FTooltipData::GetTooltipText() const
{
	return TooltipText;
}

bool FTooltipData::GetAllowTextWrap() const
{
	return bAllowTextWrap;
}

TObjectPtr<UTooltipIcon> FTooltipData::GetTooltipIcon() const
{
	return TooltipIcon;
}

void FTooltipData::SetTooltipIcon(const TObjectPtr<UTooltipIcon>& InTooltipIcon)
{
	TooltipIcon = InTooltipIcon;
}

void FTooltipData::SetTooltipText(const FText& InTooltipText)
{
	TooltipText = InTooltipText;
}

void FTooltipData::SetAllowTextWrap(const bool InbAllowTextWrap)
{
	bAllowTextWrap = InbAllowTextWrap;
}

void FTooltipData::CreateFormattedText(const FText& InText)
{
	FormattedText = FTextFormat(InText);
}

void FTooltipData::SetFormattedTooltipText(const TArray<int32>& CalculatedValues)
{
	FFormatNamedArguments Args;
	Args.Reserve(CalculatedValues.Num());
	for (int i = 0; i < CalculatedValues.Num(); i++)
	{
		Args.Emplace(TEXT("MaxAllowed"), FText::AsNumber(CalculatedValues[i], &NumberFormattingOptions));
	}
	TooltipText = FText::Format(FormattedText, Args);
}

TArray<FString> FTooltipData::GetFormattedTextArgs() const
{
	TArray<FString> Names;
	FormattedText.GetFormatArgumentNames(Names);
	return Names;
}

int32 FTooltipData::GetNumberOfFormattedTextArgs() const
{
	TArray<FString> Names;
	FormattedText.GetFormatArgumentNames(Names);
	return Names.Num();
}

bool FTooltipData::HasFormattedText() const
{
	return GetNumberOfFormattedTextArgs() > 0;
}
