﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "WidgetComponents/BSCarouselNavBar.h"
#include "WidgetComponents/Buttons/BSButton.h"
#include "CommonWidgetCarousel.h"
#include "WidgetComponents/ButtonNotificationWidget.h"

void UBSCarouselNavBar::SetLinkedCarousel(UCommonWidgetCarousel* CommonCarousel)
{
	if (LinkedCarousel) LinkedCarousel->OnCurrentPageIndexChanged.RemoveAll(this);
	

	LinkedCarousel = CommonCarousel;

	if (LinkedCarousel)
	{
		LinkedCarousel->OnCurrentPageIndexChanged.AddDynamic(this, &UBSCarouselNavBar::HandlePageChanged);
	}

	RebuildButtons();
}

void UBSCarouselNavBar::SetNavButtonText(const TArray<FText>& InButtonText)
{
	ButtonText = InButtonText;
}

void UBSCarouselNavBar::UpdateNotifications(const int32 Index, const int32 NumCautions, const int32 NumWarnings)
{
	if (bShowNotificationWidgets && Notifications.IsValidIndex(Index))
	{
		Notifications[Index]->SetNumWarnings(NumWarnings);
		Notifications[Index]->SetNumCautions(NumCautions);
	}
}

void UBSCarouselNavBar::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyContainer.Reset();
}

void UBSCarouselNavBar::HandlePageChanged(UCommonWidgetCarousel* CommonCarousel, int32 PageIndex)
{
	if (Buttons.IsValidIndex(PageIndex))
	{
		Buttons[PageIndex]->SetActive();
	}
}

void UBSCarouselNavBar::HandleButtonClicked(const UBSButton* AssociatedButton)
{
	if (LinkedCarousel && LinkedCarousel->GetActiveWidgetIndex() != AssociatedButton->GetEnumValue())
	{
		LinkedCarousel->EndAutoScrolling();
		const uint8 EnumValue = AssociatedButton->GetEnumValue();
		LinkedCarousel->SetActiveWidgetIndex(EnumValue);
	}
}

TSharedRef<SWidget> UBSCarouselNavBar::RebuildWidget()
{
	MyContainer = SNew(SHorizontalBox);

	return MyContainer.ToSharedRef();
}

void UBSCarouselNavBar::RebuildButtons()
{
	if (ensure(MyContainer) && LinkedCarousel)
	{
		MyContainer->ClearChildren();

		const int32 NumPages = LinkedCarousel->GetChildrenCount();
		for (int32 CurPage = 0; CurPage < NumPages; CurPage++)
		{
			TSharedRef<SVerticalBox> MainVerticalBox = SNew(SVerticalBox);
			if (NotificationWidgetType && bShowNotificationWidgets)
			{
				UButtonNotificationWidget* NotificationWidget =
					Cast<UButtonNotificationWidget>(CreateWidget(GetOwningPlayer(), NotificationWidgetType));
				if (NotificationWidget)
				{
					Notifications.Add(NotificationWidget);
					TSharedRef<SWidget> NotificationSWidget = NotificationWidget->TakeWidget();
					MainVerticalBox->AddSlot()
						.HAlign(HAlign_Fill).VAlign(VAlign_Fill)
						.AutoHeight().Padding(NotificationWidgetContainerPadding)
						[NotificationSWidget];
				}
			}
			
			if (ButtonWidgetType)
			{
				UBSButton* ButtonUserWidget = Cast<UBSButton>(CreateWidget(GetOwningPlayer(), ButtonWidgetType));
				if (ButtonUserWidget)
				{
					Buttons.Add(ButtonUserWidget);
					TSharedRef<SWidget> ButtonSWidget = ButtonUserWidget->TakeWidget();
					
					if (ButtonText.IsValidIndex(CurPage))
					{
						ButtonUserWidget->SetButtonText(ButtonText[CurPage]);
					}
					
					MainVerticalBox->AddSlot()
						.HAlign(HAlign_Fill).VAlign(VAlign_Fill)
						.FillHeight(1.f).Padding(ButtonPadding)
						[ButtonSWidget];
				}
			}
			
			MyContainer->AddSlot()
				.HAlign(HAlign_Fill).VAlign(VAlign_Center)
				.FillWidth(1.f).Padding(0)
				[MainVerticalBox];
			
		}
		if (NumPages > 0)
		{
			for (int32 CurPage = 0; CurPage < NumPages; CurPage++)
			{
				const int32 NextPage = CurPage + 1;

				// Default to 0 warnings and cautions
				if (bShowNotificationWidgets && Notifications.IsValidIndex(CurPage))
				{
					Notifications[CurPage]->SetNumCautions(0);
					Notifications[CurPage]->SetNumWarnings(0);
				}

				// Bind button clicking to function, link buttons together to deactivate when not clicked
				if (Buttons.IsValidIndex(CurPage))
				{
					Buttons[CurPage]->OnBSButtonPressed.AddDynamic(this, &UBSCarouselNavBar::HandleButtonClicked);
					if (Buttons.IsValidIndex(NextPage))
					{
						Buttons[CurPage]->SetDefaults(CurPage, Buttons[NextPage]);
					}
				}
			}

			Buttons.Last()->SetDefaults(static_cast<uint8>(Buttons.Num() - 1), Buttons[0]);
			Buttons[LinkedCarousel->GetActiveWidgetIndex()]->SetActive();
		}
	}
}
