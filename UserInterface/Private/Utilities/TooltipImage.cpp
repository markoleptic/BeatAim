// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/TooltipImage.h"
#include "Utilities/BSWidgetInterface.h"
#include "Components/Button.h"


// FDynamicTooltipData

FDynamicTooltipData::FDynamicTooltipData()
{
	MinAllowed = 0.f;
	Precision = 0;
	FallbackText = FText();
	TryChangeString = "Try lowering this value to <= ";
}

FDynamicTooltipData::FDynamicTooltipData(const float InMin, const FString& InFallbackStringTableKey)
{
	MinAllowed = InMin;
	Precision = 0;
	FallbackText = IBSWidgetInterface::GetTooltipTextFromKey(InFallbackStringTableKey);;
	TryChangeString = "Try lowering this value to <= ";
}


// FTooltipData

FTooltipData::FTooltipData()
{
	TooltipImage = TWeakObjectPtr<UTooltipImage>(nullptr);
	TooltipText = FText();
	bAllowTextWrap = false;
	TooltipType = ETooltipImageType::Default;
	TooltipStringTableKey = FString();
	AdditionalTooltipText = FText();
	bHasBeenInitialized = false;
	DynamicTooltipData = FDynamicTooltipData();
	bShouldShowTooltipImage = false;
	LastActual = -1.f;
	LastMaxAllowed = -1.f;
	bIsDirty = false;
	bIsDynamic = false;
}

FTooltipData::FTooltipData(const FText& InTooltipText, const bool InbAllowTextWrap)
{
	TooltipImage = TWeakObjectPtr<UTooltipImage>(nullptr);
	TooltipText = InTooltipText;
	bAllowTextWrap = InbAllowTextWrap;
	TooltipType = ETooltipImageType::Default;
	TooltipStringTableKey = FString();
	AdditionalTooltipText = FText();
	DynamicTooltipData = FDynamicTooltipData();
	bShouldShowTooltipImage = false;
	LastActual = -1.f;
	LastMaxAllowed = -1.f;
	bIsDirty = false;
	bIsDynamic = false;

	bHasBeenInitialized = true;
}

FTooltipData::FTooltipData(const FString& InStringTableKey, const ETooltipImageType& InType,
	const FText& InAdditionalTooltipText)
{
	TooltipImage = TWeakObjectPtr<UTooltipImage>(nullptr);
	TooltipText = FText();
	bAllowTextWrap = false;
	TooltipType = InType;
	TooltipStringTableKey = InStringTableKey;
	AdditionalTooltipText = InAdditionalTooltipText;
	DynamicTooltipData = FDynamicTooltipData();
	InitTooltipText();
	bShouldShowTooltipImage = false;
	LastActual = -1.f;
	LastMaxAllowed = -1.f;
	bIsDirty = false;
	bIsDynamic = false;

	bHasBeenInitialized = true;
}

void FTooltipData::InitTooltipText()
{
	if (!HasStringTableKey())
	{
		UE_LOG(LogTemp, Display, TEXT("Tried to init TooltipText without a StringTableKey."));
		return;
	}
	if (!AdditionalTooltipText.IsEmpty())
	{
		TooltipText = FText::Join(FText::FromString(" "),
			IBSWidgetInterface::GetTooltipTextFromKey(TooltipStringTableKey), AdditionalTooltipText);
	}
	else
	{
		TooltipText = IBSWidgetInterface::GetTooltipTextFromKey(TooltipStringTableKey);
	}
}

void FTooltipData::SetDynamicData(const float InMin, const FString& InFallbackStringTableKey, const int32 InPrecision)
{
	DynamicTooltipData = FDynamicTooltipData(InMin, InFallbackStringTableKey);
	DynamicTooltipData.Precision = InPrecision;
	bIsDynamic = true;
}

void FTooltipData::UpdateTooltipData(const FTooltipData& InUpdateData)
{
	if (!HasStringTableKey())
	{
		UE_LOG(LogTemp, Display, TEXT("Tried to update TooltipData without a StringTableKey."));
		return;
	}
	if (!InUpdateData.AdditionalTooltipText.IsEmpty())
	{
		TooltipText = FText::Join(FText::FromString(" "),
			IBSWidgetInterface::GetTooltipTextFromKey(InUpdateData.TooltipStringTableKey),
			InUpdateData.AdditionalTooltipText);
	}
	else
	{
		TooltipText = IBSWidgetInterface::GetTooltipTextFromKey(InUpdateData.TooltipStringTableKey);
	}
}

void FTooltipData::SetShouldShowTooltipImage(const bool bShow)
{
	if (bShouldShowTooltipImage != bShow)
	{
		bIsDirty = true;
		bShouldShowTooltipImage = bShow;
	}
}

void FTooltipData::UpdateDynamicTooltipText(const FDynamicTooltipState& InUpdateData)
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

	if (InUpdateData.bOverride) SetShouldShowTooltipImage(false);
	else SetShouldShowTooltipImage(InUpdateData.Actual > InUpdateData.MaxAllowed);
}

bool FTooltipData::IsDirty() const
{
	return bIsDirty || (!bShouldShowTooltipImage && TooltipImage.IsValid());
}

void FTooltipData::RemoveTooltipImage()
{
	if (TooltipImage.IsValid())
	{
		TooltipImage->RemoveFromParent();
		TooltipImage.Reset();
	}
}


// UTooltipImage

void UTooltipImage::NativeConstruct()
{
	Super::NativeConstruct();
	Button->OnHovered.AddDynamic(this, &UTooltipImage::OnTooltipImageHoveredCallback);
}

void UTooltipImage::OnTooltipImageHoveredCallback()
{
	OnTooltipHovered.Broadcast(this, TooltipData);
}

void UTooltipImage::SetupTooltipImage(const FText& InText, const bool bAllowTextWrap)
{
	TooltipData = FTooltipData(InText, bAllowTextWrap);
	TooltipData.TooltipImage = this;
}

void UTooltipImage::SetTooltipData(const FTooltipData& InTooltipData)
{
	TooltipData = InTooltipData;
	TooltipData.TooltipImage = this;
}
