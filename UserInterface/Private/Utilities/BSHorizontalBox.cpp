// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/BSHorizontalBox.h"
#include "Utilities/BSWidgetInterface.h"
#include "Components/Border.h"
#include "Components/HorizontalBoxSlot.h"

TSharedRef<SWidget> UBSHorizontalBox::RebuildWidget()
{
	MyHorizontalBox = SNew(SHorizontalBox);

	for (const TObjectPtr<UPanelSlot>& PanelSlot : Slots)
	{
		if (UHorizontalBoxSlot* TypedSlot = Cast<UHorizontalBoxSlot>(PanelSlot))
		{
			TypedSlot->Parent = this;
			TypedSlot->BuildSlot(MyHorizontalBox.ToSharedRef());
		}
		if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(PanelSlot->Content))
		{
			if (UBorder* FoundBorder = IBSWidgetInterface::DescendWidgetReturnFirst<UBorder>(PanelWidget))
			{
				Borders.AddUnique(FoundBorder);
			}
		}
	}
	return MyHorizontalBox.ToSharedRef();
}

void UBSHorizontalBox::SetLeftBorderBrushTint(const FLinearColor& Color)
{
	if (Borders.IsValidIndex(0))
	{
		Borders[0]->SetBrushColor(Color);
	}
}

void UBSHorizontalBox::SetRightBorderBrushTint(const FLinearColor& Color)
{
	if (Borders.IsValidIndex(1))
	{
		Borders[1]->SetBrushColor(Color);
	}
}

void UBSHorizontalBox::RefreshBorders()
{
	for (const TObjectPtr<UPanelSlot>& PanelSlot : Slots)
	{
		if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(PanelSlot->Content))
		{
			if (UBorder* FoundBorder = IBSWidgetInterface::DescendWidgetReturnFirst<UBorder>(PanelWidget))
			{
				Borders.AddUnique(FoundBorder);
			}
		}
	}
}
