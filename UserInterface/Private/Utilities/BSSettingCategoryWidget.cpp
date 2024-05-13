// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/BSSettingCategoryWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Utilities/BSVerticalBox.h"
#include "MenuOptions/MenuOptionWidget.h"
#include "Utilities/TooltipWidget.h"

void UBSSettingCategoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ActiveTooltipWidget = ConstructTooltipWidget();
	InitSettingCategoryWidget();
}

void UBSSettingCategoryWidget::InitSettingCategoryWidget()
{
	AddContainer(MainContainer);
	WidgetTree->ForEachWidget([&](UWidget* Widget)
	{
		if (const auto MenuOption = Cast<UMenuOptionWidget>(Widget))
		{
			if (MenuOption->ShouldShowTooltip())
			{
				if (UTooltipImage* TooltipImage = MenuOption->GetTooltipImage())
				{
					SetupTooltip(TooltipImage, MenuOption->GetTooltipImageText());
				}
			}
		}
	});
	UpdateBrushColors();
}

UTooltipWidget* UBSSettingCategoryWidget::ConstructTooltipWidget()
{
	return CreateWidget<UTooltipWidget>(this, TooltipWidgetClass);
}

UTooltipWidget* UBSSettingCategoryWidget::GetTooltipWidget() const
{
	return ActiveTooltipWidget;
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
