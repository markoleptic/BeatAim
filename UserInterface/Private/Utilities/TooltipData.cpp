// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/TooltipData.h"
#include "Utilities/TooltipIcon.h"

int32 UTooltipData::GId = 0;

UTooltipData::UTooltipData(): Id(GId++), bAllowTextWrap(false)
{
	NumberFormattingOptions.SetAlwaysSign(false).SetRoundingMode(HalfFromZero).SetMaximumFractionalDigits(2);
}

FText UTooltipData::GetTooltipText() const
{
	return TooltipText;
}

bool UTooltipData::GetAllowTextWrap() const
{
	return bAllowTextWrap;
}

TWeakObjectPtr<UTooltipIcon> UTooltipData::GetTooltipIcon() const
{
	return TooltipIcon;
}

void UTooltipData::SetTooltipIcon(const TWeakObjectPtr<UTooltipIcon>& InTooltipIcon)
{
	TooltipIcon = InTooltipIcon;
}

void UTooltipData::SetTooltipText(const FText& InTooltipText)
{
	TooltipText = InTooltipText;
}

void UTooltipData::SetAllowTextWrap(const bool InbAllowTextWrap)
{
	bAllowTextWrap = InbAllowTextWrap;
}

void UTooltipData::CreateFormattedText(const FText& InText)
{
	FormattedText = FTextFormat(InText);
}

void UTooltipData::SetFormattedTooltipText(const TArray<int32>& CalculatedValues)
{
	FFormatNamedArguments Args;
	Args.Reserve(CalculatedValues.Num());
	for (int i = 0; i < CalculatedValues.Num(); i++)
	{
		Args.Emplace(TEXT("MaxAllowed"), FText::AsNumber(CalculatedValues[i], &NumberFormattingOptions));
	}
	TooltipText = FText::Format(FormattedText, Args);
}

TArray<FString> UTooltipData::GetFormattedTextArgs() const
{
	TArray<FString> Names;
	FormattedText.GetFormatArgumentNames(Names);
	return Names;
}

int32 UTooltipData::GetNumberOfFormattedTextArgs() const
{
	TArray<FString> Names;
	FormattedText.GetFormatArgumentNames(Names);
	return Names.Num();
}

bool UTooltipData::HasFormattedText() const
{
	return FormattedText.IsValid();
}
