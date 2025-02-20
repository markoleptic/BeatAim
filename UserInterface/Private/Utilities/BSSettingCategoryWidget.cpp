// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/BSSettingCategoryWidget.h"
#include "Blueprint/WidgetTree.h"
#include "MenuOptions/MenuOptionWidget.h"
#include "Utilities/BSVerticalBox.h"

void UBSSettingCategoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitSettingCategoryWidget();
}

void UBSSettingCategoryWidget::InitSettingCategoryWidget()
{
	AddContainer(MainContainer);
	WidgetTree->ForEachWidget([&](UWidget* Widget)
	{
		if (const auto MenuOption = Cast<UMenuOptionWidget>(Widget))
		{
			const FText Text = MenuOption->GetDescriptionTooltipIconText();
			if (MenuOption->ShouldShowDescriptionTooltipIcon() && !Text.IsEmpty())
			{
				if (UTooltipIcon* TooltipIcon = MenuOption->GetDescriptionTooltipIcon())
				{
					SetupTooltip(TooltipIcon, Text);
				}
			}
		}
	});
	UpdateBrushColors();
}

void UBSSettingCategoryWidget::AddContainer(const TArray<TObjectPtr<UBSVerticalBox>>& InContainers)
{
	for (const TObjectPtr<UBSVerticalBox>& InContainer : InContainers)
	{
		AddContainer(InContainer);
	}
}

void UBSSettingCategoryWidget::AddContainer(const TObjectPtr<UBSVerticalBox>& InContainer)
{
	Containers.AddUnique(InContainer);
}

void UBSSettingCategoryWidget::UpdateBrushColors() const
{
	for (const TObjectPtr<UBSVerticalBox>& Container : Containers)
	{
		if (Container)
		{
			Container->UpdateBrushColors();
		}
	}
}
