// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/Buttons/NotificationButtonCombo.h"
#include "Components/Overlay.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"

void UNotificationButtonCombo::SetNumWarnings(const uint32 InNumWarnings)
{
	FVector2d CurrentSize = Spacer_Mid->GetSize();
	if (InNumWarnings == 0)
	{
		Overlay_Warnings->SetVisibility(ESlateVisibility::Collapsed);
		CurrentSize.X = 0.f;
		Spacer_Mid->SetSize(CurrentSize);
	}
	else
	{
		Overlay_Warnings->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CurrentSize.X = Overlay_Cautions->IsVisible() ? NotificationImageSpacing : 0.f;
		Spacer_Mid->SetSize(CurrentSize);
	}
	TextBlock_NumWarnings->SetText(FText::AsNumber(InNumWarnings));
}

void UNotificationButtonCombo::SetNumCautions(const uint32 InNumCautions)
{
	FVector2d CurrentSize = Spacer_Mid->GetSize();
	if (InNumCautions == 0)
	{
		Overlay_Cautions->SetVisibility(ESlateVisibility::Collapsed);
		CurrentSize.X = 0.f;
		Spacer_Mid->SetSize(CurrentSize);
	}
	else
	{
		Overlay_Cautions->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CurrentSize.X = Overlay_Warnings->IsVisible() ? NotificationImageSpacing : 0.f;
		Spacer_Mid->SetSize(CurrentSize);
	}
	TextBlock_NumCautions->SetText(FText::AsNumber(InNumCautions));
}
