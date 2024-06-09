// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CreatorViewWidget.h"
#include "CommonWidgetCarousel.h"
#include "GameModes/CustomGameModeCategoryWidget.h"
#include "Utilities/BSCarouselNavBar.h"

void UCreatorViewWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Carousel->SetActiveWidgetIndex(0);
	CarouselNavBar->SetLinkedCarousel(Carousel);
	for (int i = 0; i < Carousel->GetChildrenCount(); i++)
	{
		if (UWidget* Widget = Carousel->GetWidgetAtIndex(i))
		{
			if (const auto CategoryWidget = Cast<UCustomGameModeCategoryWidget>(Widget))
			{
				GameModeCategoryWidgetIndexMap.Add(CategoryWidget->GetGameModeCategory(), i);
			}
		}
	}
}

void UCreatorViewWidget::UpdateNotificationIcons(const TMap<EGameModeCategory, TPair<int32, int32>>& IconMap)
{
	for (const auto& [Category, Pair] : IconMap)
	{
		if (const int32* Index = GameModeCategoryWidgetIndexMap.Find(Category))
		{
			CarouselNavBar->UpdateNotifications(*Index, Pair.Key, Pair.Value);
		}
	}
}
