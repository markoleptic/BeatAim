// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/ComboBox/BSTaggedComboBoxEntry.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Utilities/GameModeCategoryTagWidget.h"

void UBSTaggedComboBoxEntry::AddGameModeCategoryTagWidget(TArray<UGameModeCategoryTagWidget*>& InParentTagWidgets,
	TArray<UGameModeCategoryTagWidget*>& InTagWidgets, const FMargin InMargin, const EVerticalAlignment InVAlign,
	const EHorizontalAlignment InHAlign)
{
	InTagWidgets.Sort([&](const UGameModeCategoryTagWidget& Widget, const UGameModeCategoryTagWidget& Widget2)
	{
		return Widget.GetGameModeCategoryTag().ToString() < Widget2.GetGameModeCategoryTag().ToString();
	});
	for (UGameModeCategoryTagWidget* Widget : InTagWidgets)
	{
		UHorizontalBoxSlot* BoxSlot = Box_TagWidgets->AddChildToHorizontalBox(Cast<UWidget>(Widget));
		BoxSlot->SetPadding(InMargin);
		BoxSlot->SetVerticalAlignment(InVAlign);
		BoxSlot->SetHorizontalAlignment(InHAlign);
	}

	InParentTagWidgets.Sort([&](const UGameModeCategoryTagWidget& Widget, const UGameModeCategoryTagWidget& Widget2)
	{
		return Widget.GetGameModeCategoryTag().ToString() < Widget2.GetGameModeCategoryTag().ToString();
	});
	for (UGameModeCategoryTagWidget* Widget : InParentTagWidgets)
	{
		UHorizontalBoxSlot* BoxSlot = Box_TagWidgets->AddChildToHorizontalBox(Cast<UWidget>(Widget));
		BoxSlot->SetPadding(InMargin);
		BoxSlot->SetVerticalAlignment(InVAlign);
		BoxSlot->SetHorizontalAlignment(InHAlign);
	}
}
