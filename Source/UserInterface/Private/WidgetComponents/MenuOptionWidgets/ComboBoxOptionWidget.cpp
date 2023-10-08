﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "WidgetComponents/MenuOptionWidgets/ComboBoxOptionWidget.h"
#include "EnumTagMap.h"
#include "WidgetComponents/Boxes/BSComboBoxEntry.h"
#include "WidgetComponents/Boxes/BSComboBoxEntry_Tagged.h"
#include "WidgetComponents/GameModeCategoryTagWidget.h"
#include "WidgetComponents/Boxes/BSComboBoxString.h"
#include "Styles/MenuOptionStyle.h"

void UComboBoxOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ComboBox->OnGenerateWidgetEventDelegate.BindDynamic(this, &ThisClass::OnGenerateWidgetEvent);
	ComboBox->OnSelectionChanged_GenerateWidgetForMultiSelection.BindDynamic(this,
		&ThisClass::OnSelectionChanged_GenerateMultiSelectionItem);
}

UBSComboBoxEntry* UComboBoxOptionWidget::ConstructComboBoxEntryWidget()
{
	return CreateWidget<UBSComboBoxEntry>(this, ComboBox->GetComboboxEntryWidget());
}

UWidget* UComboBoxOptionWidget::OnGenerateWidgetEvent(const UBSComboBoxString* ComboBoxString, FString Method)
{
	UWidget* Widget = IBSWidgetInterface::OnGenerateWidgetEvent(ComboBoxString, Method);

	UBSComboBoxEntry_Tagged* ComboBoxEntry = Cast<UBSComboBoxEntry_Tagged>(Widget);
	if (!ComboBoxEntry)
	{
		return Widget;
	}

	return AddGameModeCategoryTagWidgets(ComboBoxEntry);
}

UWidget* UComboBoxOptionWidget::OnSelectionChanged_GenerateMultiSelectionItem(const UBSComboBoxString* ComboBoxString,
	const TArray<FString>& SelectedOptions)
{
	return IBSWidgetInterface::OnSelectionChanged_GenerateMultiSelectionItem(ComboBoxString, SelectedOptions);
}

FString UComboBoxOptionWidget::GetStringTableKeyFromComboBox(const UBSComboBoxString* ComboBoxString,
	const FString& EnumString)
{
	if (GetComboBoxEntryTooltipStringTableKey.IsBound())
	{
		return GetComboBoxEntryTooltipStringTableKey.Execute(EnumString);
	}
	return IBSWidgetInterface::GetStringTableKeyFromComboBox(ComboBoxString, EnumString);
}

UWidget* UComboBoxOptionWidget::AddGameModeCategoryTagWidgets(UBSComboBoxEntry_Tagged* ComboBoxEntry)
{
	if (GameplayTagWidgetMap.IsEmpty())
	{
		return ComboBoxEntry;
	}
	const FString CompareString = ComboBoxEntry->GetEntryText().ToString();

	const FEnumTagPair* Found = EnumTagMapping.EnumTagPairs.FindByPredicate(
		[this, &CompareString](const FEnumTagPair& EnumTagPair)
		{
			return EnumTagPair.DisplayName.Equals(CompareString);
		});

	if (!Found)
	{
		return ComboBoxEntry;
	}

	const FEnumTagPair& EnumTagPair = EnumTagMapping.EnumTagPairs[Found->Index];
	TArray<UGameModeCategoryTagWidget*> ParentTagWidgetsToAdd;
	TArray<UGameModeCategoryTagWidget*> TagWidgetsToAdd;

	for (const FGameplayTag& Tag : EnumTagPair.ParentTags)
	{
		const TSubclassOf<UGameModeCategoryTagWidget>* SubClass = GameplayTagWidgetMap.Find(Tag);
		if (!SubClass)
		{
			continue;
		}
		UGameModeCategoryTagWidget* TagWidget = CreateWidget<UGameModeCategoryTagWidget>(this, *SubClass);
		ParentTagWidgetsToAdd.Add(TagWidget);
	}

	for (const FGameplayTag& Tag : EnumTagPair.Tags)
	{
		const TSubclassOf<UGameModeCategoryTagWidget>* SubClass = GameplayTagWidgetMap.Find(Tag);
		if (!SubClass)
		{
			continue;
		}
		UGameModeCategoryTagWidget* TagWidget = CreateWidget<UGameModeCategoryTagWidget>(this, *SubClass);
		TagWidgetsToAdd.Add(TagWidget);
	}

	ComboBoxEntry->AddGameModeCategoryTagWidget(ParentTagWidgetsToAdd, TagWidgetsToAdd,
		MenuOptionStyle->Padding_TagWidget, MenuOptionStyle->VerticalAlignment_TagWidget,
		MenuOptionStyle->HorizontalAlignment_TagWidget);
	return ComboBoxEntry;
}

void UComboBoxOptionWidget::SortAndAddOptions(TArray<FString>& InOptions)
{
	InOptions.Sort([](const FString& FirstOption, const FString& SecondOption)
	{
		return FirstOption < SecondOption;
	});
	for (const FString Option : InOptions)
	{
		ComboBox->AddOption(Option);
	}
}

void UComboBoxOptionWidget::SetGameplayTagWidgetMap(
	const TMap<FGameplayTag, TSubclassOf<UGameModeCategoryTagWidget>>& InMap)
{
	GameplayTagWidgetMap = InMap;
}

void UComboBoxOptionWidget::SetEnumTagMap(const TObjectPtr<UEnumTagMap> InEnumTagMap)
{
	EnumTagMap = InEnumTagMap;
}
