// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CreatorViewWidget.h"
#include "CommonWidgetCarousel.h"
#include "GameModes/CustomGameModeCategoryWidget.h"
#include "Utilities/BSCarouselNavBar.h"

void UCreatorViewWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Carousel->OnCurrentPageIndexChanged.AddUniqueDynamic(this, &ThisClass::OnCarouselWidgetIndexChanged);
	Carousel->SetActiveWidgetIndex(0);
	CarouselNavBar->SetLinkedCarousel(Carousel);
}

void UCreatorViewWidget::OnCarouselWidgetIndexChanged(UCommonWidgetCarousel* InCarousel, const int32 NewIndex)
{
	//UpdateOptionsFromConfig();
}

void UCreatorViewWidget::UpdateNotificationIcons(const TMap<EGameModeCategory, TPair<int32, int32>>& IconMap)
{
	for (const auto& [Category, Pair] : IconMap)
	{
		if (const TObjectPtr<UCustomGameModeCategoryWidget>* Widget = GameModeCategoryWidgetMap.Find(Category))
		{
			CarouselNavBar->UpdateNotifications(static_cast<int32>((*Widget)->GetGameModeCategory()) - 1, Pair.Key,
				Pair.Value);
		}
	}
}
