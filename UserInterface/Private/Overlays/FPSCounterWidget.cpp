// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Components/TextBlock.h"
#include "Overlays/FrameCounterWidget.h"

void UFrameCounterWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// average the last 40 ticks
	if (CounterUpdateInterval < 40)
	{
		SumOfTicks += InDeltaTime;
		CounterUpdateInterval++;
	}
	else
	{
		// update frame rate
		TextBlock_FPSCounter->SetText(FText::AsNumber(round(CounterUpdateInterval / SumOfTicks)));
		SumOfTicks = 0;
		CounterUpdateInterval = 0;
	}
}
