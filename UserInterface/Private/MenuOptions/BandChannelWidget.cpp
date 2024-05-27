// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/BandChannelWidget.h"
#include "BSConstants.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetStringLibrary.h"
#include "Styles/MenuOptionStyle.h"

void UBandChannelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetShowTooltipIcon(false);
	Value_BandChannelMin->OnTextCommitted.AddDynamic(this, &UBandChannelWidget::OnValueCommitted_Min);
	Value_BandChannelMax->OnTextCommitted.AddDynamic(this, &UBandChannelWidget::OnValueCommitted_Max);
}

void UBandChannelWidget::SetStyling()
{
	Super::SetStyling();
	if (MenuOptionStyle)
	{
		if (TextBlock_Lower)
		{
			TextBlock_Lower->SetFont(MenuOptionStyle->Font_DescriptionText);
			UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(TextBlock_Lower->Slot);
			if (HorizontalBoxSlot)
			{
				HorizontalBoxSlot->SetPadding(
					MenuOptionStyle->Padding_LeftBox + MenuOptionStyle->Padding_DescriptionText);
			}
		}
		if (TextBlock_Upper)
		{
			TextBlock_Upper->SetFont(MenuOptionStyle->Font_DescriptionText);
			UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(TextBlock_Upper->Slot);
			if (HorizontalBoxSlot)
			{
				HorizontalBoxSlot->SetPadding(
					MenuOptionStyle->Padding_LeftBox + MenuOptionStyle->Padding_DescriptionText);
			}
		}
		if (Value_BandChannelMin)
		{
			Value_BandChannelMin->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		}
		if (Value_BandChannelMax)
		{
			Value_BandChannelMax->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		}
	}
}

void UBandChannelWidget::SetDefaultValues(const FVector2d Values, const int32 ChannelIndex)
{
	Value_BandChannelMin->SetText(FText::AsNumber(Values.X));
	Value_BandChannelMax->SetText(FText::AsNumber(Values.Y));
	const TArray ChannelNumber = {
		FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", "AA_BandChannelText"),
		FText::FromString(FString::FromInt(ChannelIndex + 1)),
		FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", "AA_BandChannelUnit")
	};
	TextBlock_Description->SetText(FText::Join(FText::FromString(" "), ChannelNumber));
	Index = ChannelIndex;
}

void UBandChannelWidget::OnValueCommitted_Min(const FText& NewValue, ETextCommit::Type CommitType)
{
	const float NewFloatValue = FCString::Atof(*UKismetStringLibrary::Replace(NewValue.ToString(), ",", ""));
	if (NewFloatValue < Constants::MinValue_BandFrequency)
	{
		Value_BandChannelMin->SetText(FText::AsNumber(Constants::MinValue_BandFrequency));
		return;
	}
	if (NewFloatValue > Constants::MaxValue_BandFrequency)
	{
		Value_BandChannelMin->SetText(FText::AsNumber(Constants::MaxValue_BandFrequency));
		return;
	}
	if (!OnChannelValueCommitted.ExecuteIfBound(this, Index, NewFloatValue, true))
	{
		UE_LOG(LogTemp, Display, TEXT("OnChannelValueCommitted not bound."));
	}
}

void UBandChannelWidget::OnValueCommitted_Max(const FText& NewValue, ETextCommit::Type CommitType)
{
	const float NewFloatValue = FCString::Atof(*UKismetStringLibrary::Replace(NewValue.ToString(), ",", ""));
	if (NewFloatValue < Constants::MinValue_BandFrequency)
	{
		Value_BandChannelMax->SetText(FText::AsNumber(0));
		return;
	}
	if (NewFloatValue > Constants::MaxValue_BandFrequency)
	{
		Value_BandChannelMax->SetText(FText::AsNumber(Constants::MaxValue_BandFrequency));
		return;
	}
	if (!OnChannelValueCommitted.ExecuteIfBound(this, Index, NewFloatValue, false))
	{
		UE_LOG(LogTemp, Display, TEXT("OnChannelValueCommitted not bound."));
	}
}
