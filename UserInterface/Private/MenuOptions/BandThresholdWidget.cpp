// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/BandThresholdWidget.h"

#include "GlobalConstants.h"
#include "Components/TextBlock.h"

void UBandThresholdWidget::NativeConstruct()
{
	Super::NativeConstruct();
	OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged_Threshold);
	SetValues(Constants::MinValue_BandFrequencyThreshold, Constants::MaxValue_BandFrequencyThreshold,
		Constants::SnapSize_BandFrequencyThreshold);
	SetShowTooltipImage(false);
}

void UBandThresholdWidget::SetDefaultValue(const float Value, const int32 ChannelIndex)
{
	SetValue(Value);
	const TArray ChannelNumber = {
		FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", "AA_BandChannelText"),
		FText::FromString(FString::FromInt(ChannelIndex + 1))
	};
	TextBlock_Description->SetText(FText::Join(FText::FromString(" "), ChannelNumber));
	Index = ChannelIndex;
}

void UBandThresholdWidget::OnSliderTextBoxValueChanged_Threshold(USingleRangeInputWidget* SliderTextBoxOptionWidget,
	const float Value)
{
	if (!OnThresholdValueCommitted.ExecuteIfBound(this, Index, Value))
	{
		UE_LOG(LogTemp, Display, TEXT("OnThresholdValueCommitted not bound."));
	}
}
