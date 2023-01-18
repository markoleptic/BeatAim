// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetComponents/BandChannelWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Kismet/KismetStringLibrary.h"

void UBandChannelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BandChannelMin->OnTextCommitted.AddDynamic(this, &UBandChannelWidget::OnMinValueCommitted);
	BandChannelMax->OnTextCommitted.AddDynamic(this, &UBandChannelWidget::OnMaxValueCommitted);
}

void UBandChannelWidget::SetDefaultValues(const FVector2d Values, const int32 ChannelIndex)
{
	BandChannelMin->SetText(FText::AsNumber(Values.X));
	BandChannelMax->SetText(FText::AsNumber(Values.Y));
	ChannelText->SetText(FText::FromString("Channel " + FString::FromInt(ChannelIndex + 1) + " (Hertz)"));
	Index = ChannelIndex;

	FSlateBrush LightBrush = FSlateBrush();
	LightBrush.TintColor = FLinearColor(0,0,0,0.1);
	FSlateBrush DarkBrush = FSlateBrush();
	DarkBrush.TintColor = FLinearColor(0,0,0,0.2);
	
	if (ChannelIndex == 0 || ChannelIndex % 2 == 0)
	{
		ChannelTextBorder->SetBrush(DarkBrush);
		LowerBorder->SetBrush(LightBrush);
		UpperBorder->SetBrush(LightBrush);
	}
	else
	{
		ChannelTextBorder->SetBrush(LightBrush);
		LowerBorder->SetBrush(DarkBrush);
		UpperBorder->SetBrush(DarkBrush);
	}
}

void UBandChannelWidget::OnMinValueCommitted(const FText& NewValue, ETextCommit::Type CommitType)
{
	const float NewFloatValue = FCString::Atof(*UKismetStringLibrary::Replace(NewValue.ToString(), "," ,""));
	if (!OnChannelValueCommitted.ExecuteIfBound(this, Index, NewFloatValue, true))
	{
		UE_LOG(LogTemp, Display, TEXT("OnChannelValueCommitted not bound."));
	}
}

void UBandChannelWidget::OnMaxValueCommitted(const FText& NewValue, ETextCommit::Type CommitType)
{
	const float NewFloatValue = FCString::Atof(*UKismetStringLibrary::Replace(NewValue.ToString(), "," ,""));
	if (!OnChannelValueCommitted.ExecuteIfBound(this, Index, NewFloatValue, false))
	{
		UE_LOG(LogTemp, Display, TEXT("OnChannelValueCommitted not bound."));
	}
}
