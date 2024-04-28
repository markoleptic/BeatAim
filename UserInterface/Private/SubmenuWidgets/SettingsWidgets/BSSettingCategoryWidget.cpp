// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "SubmenuWidgets/SettingsWidgets/BSSettingCategoryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "WidgetComponents/Boxes/BSVerticalBox.h"
#include "WidgetComponents/MenuOptionWidgets/MenuOptionWidget.h"
#include "WidgetComponents/Tooltips/TooltipWidget.h"

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
		if (auto Component = Cast<UMenuOptionWidget>(Widget))
		{
			if (!Component->ShouldShowTooltip())
			{
				return;
			}
			if (UTooltipImage* TooltipImage = Component->GetTooltipImage())
			{
				SetupTooltip(TooltipImage, Component->GetTooltipImageText());
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
