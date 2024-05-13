// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/BSVerticalBox.h"
#include "GlobalConstants.h"
#include "Utilities/BSWidgetInterface.h"
#include "Blueprint/WidgetTree.h"
#include "Utilities/BSHorizontalBox.h"

TSharedRef<SWidget> UBSVerticalBox::RebuildWidget()
{
	return Super::RebuildWidget();
}

void UBSVerticalBox::UpdateBrushColors()
{
	UpdateBrushColors_Private(Slots, false);
}

bool UBSVerticalBox::UpdateBrushColors_Private(TArray<TObjectPtr<UPanelSlot>>& InSlots, bool bLastLeftBorderDark)
{
	for (const TObjectPtr<UPanelSlot>& BoxSlot : InSlots)
	{
		// Slot is a BSVerticalBox
		if (UBSVerticalBox* SubVerticalBox = Cast<UBSVerticalBox>(BoxSlot->Content))
		{
			// First slot of every Vertical Box has the same border color
			bLastLeftBorderDark = false;
			bLastLeftBorderDark = UpdateBrushColors_Private(SubVerticalBox->Slots, bLastLeftBorderDark);
			continue;
		}

		// Slot is a UUserWidget
		if (const UUserWidget* UserWidget = Cast<UUserWidget>(BoxSlot->Content))
		{
			if (UserWidget->GetVisibility() != ESlateVisibility::Collapsed)
			{
				UserWidget->WidgetTree->ForEachWidget([this, &bLastLeftBorderDark](UWidget* Widget)
				{
					if (UBSVerticalBox* VerticalBox = Cast<UBSVerticalBox>(Widget))
					{
						bLastLeftBorderDark = VerticalBox->UpdateBrushColors_Private(
							Cast<UBSVerticalBox>(Widget)->Slots, bLastLeftBorderDark);
					}
					// UUserWidget contains a HorizontalBox
					else if (UBSHorizontalBox* HorizontalBox = Cast<UBSHorizontalBox>(Widget))
					{
						UpdateHorizontalBoxBorderColors(HorizontalBox, bLastLeftBorderDark);
					}
				});
			}
			continue;
		}

		// Slot is a subclass of PanelWidget
		if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(BoxSlot->Content))
		{
			if (TArray<UBSVerticalBox*> Boxes = IBSWidgetInterface::DescendWidget<UBSVerticalBox>(PanelWidget); !Boxes.
				IsEmpty())
			{
				for (UBSVerticalBox* Box : Boxes)
				{
					// First slot of every Vertical Box has the same border color
					bLastLeftBorderDark = false;
					bLastLeftBorderDark = UpdateBrushColors_Private(Box->Slots, bLastLeftBorderDark);
				}
				continue;
			}
		}

		// Slot is a BSHorizontalBox
		if (UBSHorizontalBox* HorizontalBox = Cast<UBSHorizontalBox>(BoxSlot->Content))
		{
			UpdateHorizontalBoxBorderColors(HorizontalBox, bLastLeftBorderDark);
		}
	}
	return bLastLeftBorderDark;
}

void UBSVerticalBox::UpdateHorizontalBoxBorderColors(UBSHorizontalBox* HorizontalBox, bool& bLastLeftBorderDark)
{
	if (HorizontalBox->GetVisibility() != ESlateVisibility::Collapsed)
	{
		if (HorizontalBox->ShouldForceSameBorderBrushTintAsPrevious())
		{
			if (bLastLeftBorderDark)
			{
				HorizontalBox->SetLeftBorderBrushTint(Constants::DarkMenuBrushColor);
				HorizontalBox->SetRightBorderBrushTint(Constants::LightMenuBrushColor);
			}
			else
			{
				HorizontalBox->SetLeftBorderBrushTint(Constants::LightMenuBrushColor);
				HorizontalBox->SetRightBorderBrushTint(Constants::DarkMenuBrushColor);
			}
			return;
		}
		if (bLastLeftBorderDark)
		{
			HorizontalBox->SetLeftBorderBrushTint(Constants::LightMenuBrushColor);
			HorizontalBox->SetRightBorderBrushTint(Constants::DarkMenuBrushColor);
			bLastLeftBorderDark = false;
		}
		else
		{
			HorizontalBox->SetLeftBorderBrushTint(Constants::DarkMenuBrushColor);
			HorizontalBox->SetRightBorderBrushTint(Constants::LightMenuBrushColor);
			bLastLeftBorderDark = true;
		}
	}
}
