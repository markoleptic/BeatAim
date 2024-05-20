// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CreatorViewWidget.h"
#include "CommonWidgetCarousel.h"
#include "GameModes/CustomGameModeCategoryWidget.h"
#include "GameModes/CustomGameModeStartWidget.h"
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
	UpdateOptionsFromConfig();
}

void UCreatorViewWidget::UpdateAllChildWidgetOptionsValid()
{
	Super::UpdateAllChildWidgetOptionsValid();
	for (const TPair<TObjectPtr<UCustomGameModeCategoryWidget>, FCustomGameModeCategoryInfo*>& ChildWidgetValidity :
	     ChildWidgetValidityMap)
	{
		// Widget_Start has a separate validity check
		if (ChildWidgetValidity.Key.IsA<UCustomGameModeStartWidget>())
		{
			continue;
		}
		CarouselNavBar->UpdateNotifications(ChildWidgetValidity.Key->GetIndex(), ChildWidgetValidity.Value->NumCautions,
			ChildWidgetValidity.Value->NumWarnings);
	}
}
