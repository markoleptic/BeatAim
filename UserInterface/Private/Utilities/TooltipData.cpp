// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/TooltipData.h"
#include "Utilities/TooltipIcon.h"

int32 UTooltipData::GId = 0;

UTooltipData::UTooltipData(): Id(GId++), bAllowTextWrap(false)
{
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

void UTooltipData::SetTooltipText(FText&& InTooltipText)
{
	TooltipText = std::move(InTooltipText);
}

void UTooltipData::SetAllowTextWrap(const bool InbAllowTextWrap)
{
	bAllowTextWrap = InbAllowTextWrap;
}

void UTooltipData::CreateFormattedText(const FText& InText)
{
	FormattedText = FTextFormat(InText);
}

template <typename... ArgTypes>
void UTooltipData::SetFormattedTooltipText(ArgTypes&&... Args)
{
	TooltipText = FText::Format(FormattedText, MoveTemp(Args));
}

TArray<FString> UTooltipData::GetFormattedTextArgs() const
{
	TArray<FString> Junk;
	FormattedText.GetFormatArgumentNames(Junk);
	return Junk;
}

int32 UTooltipData::GetNumberOfFormattedTextArgs() const
{
	TArray<FString> Junk;
	FormattedText.GetFormatArgumentNames(Junk);
	return Junk.Num();
}


/*void UTooltipData::UpdateDynamicTooltipText(const FDynamicTooltipState& InUpdateData)
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


void Func(const FText& StringTableText, FTextFormat& StringTableTextFormat)
{
	FNumberFormattingOptions Options = FNumberFormattingOptions();
	FText Text;
	FTextFormat txtfmt(Text);
	TArray<FString> ArgNames;
	StringTableTextFormat.GetFormatArgumentNames(ArgNames);
	FFormatNamedArguments Args;

	FText FormattedText = FText::Format(StringTableTextFormat, Args);
}
