// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/Buttons/ButtonNotificationWidget.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"

void UButtonNotificationWidget::SetNumWarnings(const uint32 InNumWarnings)
{
	if (InNumWarnings == 0)
	{
		Overlay_Warnings->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		Overlay_Warnings->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	TextBlock_NumWarnings->SetText(FText::AsNumber(InNumWarnings));
}

void UButtonNotificationWidget::SetNumCautions(const uint32 InNumCautions)
{
	if (InNumCautions == 0)
	{
		Overlay_Cautions->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		Overlay_Cautions->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	TextBlock_NumCautions->SetText(FText::AsNumber(InNumCautions));
}
