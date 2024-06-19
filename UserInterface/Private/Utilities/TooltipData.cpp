// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/TooltipData.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "Utilities/BSCommon.h"
#include "Utilities/TooltipIcon.h"

int32 FTooltipData::GId = 0;

FTooltipData::FTooltipData(): Id(GId++), bAllowTextWrap(false)
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

void FTooltipData::SetFormattedTooltipText(const FValidationCheckData& Data)
{
	FFormatNamedArguments Args;
	Args.Reserve(Data.CalculatedValues.Num());

	if (Data.GridSnapSize > 0 && Data.bCalculatedValuesAreIntegers)
	{
		for (int i = 0; i < Data.CalculatedValues.Num(); i++)
		{
			Args.Emplace(TEXT("MaxAllowed"),
				FText::AsNumber(
					BSCommon::GridSnapToZero(static_cast<int32>(Data.CalculatedValues[i]), Data.GridSnapSize),
					&Data.NumberFormattingOptions));
		}
	}
	else if (Data.GridSnapSize > 0)
	{
		for (int i = 0; i < Data.CalculatedValues.Num(); i++)
		{
			Args.Emplace(TEXT("MaxAllowed"),
				FText::AsNumber(BSCommon::GridSnapToZero(Data.CalculatedValues[i], Data.GridSnapSize),
					&Data.NumberFormattingOptions));
		}
	}
	else if (Data.bCalculatedValuesAreIntegers)
	{
		for (int i = 0; i < Data.CalculatedValues.Num(); i++)
		{
			Args.Emplace(TEXT("MaxAllowed"),
				FText::AsNumber(static_cast<int32>(Data.CalculatedValues[i]), &Data.NumberFormattingOptions));
		}
	}
	else
	{
		for (int i = 0; i < Data.CalculatedValues.Num(); i++)
		{
			Args.Emplace(TEXT("MaxAllowed"), FText::AsNumber(Data.CalculatedValues[i], &Data.NumberFormattingOptions));
		}
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
