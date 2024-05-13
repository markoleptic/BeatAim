// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/ComboBoxWidget.h"
#include "EnumTagMap.h"
#include "GameModeCategoryTagMap.h"
#include "Utilities/ComboBox/BSComboBoxEntry.h"
#include "Utilities/ComboBox/BSTaggedComboBoxEntry.h"
#include "Utilities/GameModeCategoryTagWidget.h"
#include "Utilities/ComboBox/BSComboBoxString.h"
#include "Styles/MenuOptionStyle.h"

void UComboBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ComboBox->OnGenerateWidgetEventDelegate.BindDynamic(this, &UComboBoxWidget::OnGenerateWidgetEvent);
	ComboBox->OnSelectionChanged_GenerateWidgetForMultiSelection.BindDynamic(this,
		&IBSWidgetInterface::OnSelectionChanged_GenerateMultiSelectionItem);
	ComboBox->SetSelectionMode(SelectionMode);
	ComboBox->SetCanSelectNone(bCanSelectNone);
	ComboBox->SetCloseComboBoxOnSelectionChanged(bCloseComboBoxOnSelectionChanged);
}

TSharedRef<SWidget> UComboBoxWidget::RebuildWidget()
{
	if (ComboBox)
	{
		ComboBox->SetSelectionMode(SelectionMode);
		ComboBox->SetCanSelectNone(bCanSelectNone);
		ComboBox->SetCloseComboBoxOnSelectionChanged(bCloseComboBoxOnSelectionChanged);
	}
	return Super::RebuildWidget();
}

UBSComboBoxEntry* UComboBoxWidget::ConstructComboBoxEntryWidget()
{
	return CreateWidget<UBSComboBoxEntry>(this, ComboBox->GetComboboxEntryWidget());
}

UWidget* UComboBoxWidget::OnGenerateWidgetEvent(const UBSComboBoxString* ComboBoxString, FString Method)
{
	UWidget* Widget = IBSWidgetInterface::OnGenerateWidgetEvent(ComboBoxString, Method);

	UBSTaggedComboBoxEntry* ComboBoxEntry = Cast<UBSTaggedComboBoxEntry>(Widget);
	if (!ComboBoxEntry)
	{
		return Widget;
	}

	return AddGameModeCategoryTagWidgets(ComboBoxEntry);
}

FString UComboBoxWidget::GetStringTableKeyFromComboBox(const UBSComboBoxString* ComboBoxString,
	const FString& EnumString)
{
	if (GetComboBoxEntryTooltipStringTableKey.IsBound())
	{
		return GetComboBoxEntryTooltipStringTableKey.Execute(EnumString);
	}
	return IBSWidgetInterface::GetStringTableKeyFromComboBox(ComboBoxString, EnumString);
}

UWidget* UComboBoxWidget::AddGameModeCategoryTagWidgets(UBSTaggedComboBoxEntry* ComboBoxEntry)
{
	if (!GameModeCategoryTagMap)
	{
		return ComboBoxEntry;
	}
	const FString CompareString = ComboBoxEntry->GetEntryText().ToString();

	for (auto& [Index, EnumTagPair] : EnumTagMapping.NewEnumTagPairs)
	{
		if (EnumTagPair.DisplayName.Equals(CompareString))
		{
			TArray<UGameModeCategoryTagWidget*> ParentTagWidgetsToAdd;
			TArray<UGameModeCategoryTagWidget*> TagWidgetsToAdd;

			for (const FGameplayTag& Tag : EnumTagPair.ParentTags)
			{
				const TSubclassOf<UUserWidget> SubClass = GameModeCategoryTagMap->GetWidgetByGameModeCategoryTag(Tag);
				if (!SubClass) continue;

				UGameModeCategoryTagWidget* TagWidget = CreateWidget<UGameModeCategoryTagWidget>(this, SubClass);
				ParentTagWidgetsToAdd.Add(TagWidget);
			}

			for (const FGameplayTag& Tag : EnumTagPair.Tags)
			{
				const TSubclassOf<UUserWidget> SubClass = GameModeCategoryTagMap->GetWidgetByGameModeCategoryTag(Tag);
				if (!SubClass) continue;

				UGameModeCategoryTagWidget* TagWidget = CreateWidget<UGameModeCategoryTagWidget>(this, SubClass);
				TagWidgetsToAdd.Add(TagWidget);
			}

			ComboBoxEntry->AddGameModeCategoryTagWidget(ParentTagWidgetsToAdd, TagWidgetsToAdd,
				MenuOptionStyle->Padding_TagWidget, MenuOptionStyle->VerticalAlignment_TagWidget,
				MenuOptionStyle->HorizontalAlignment_TagWidget);

			break;
		}
	}

	return ComboBoxEntry;
}

void UComboBoxWidget::SortAndAddOptions(TArray<FString>& InOptions)
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

void UComboBoxWidget::SetGameplayTagWidgetMap(const TObjectPtr<UGameModeCategoryTagMap> InMap)
{
	GameModeCategoryTagMap = InMap;
}

void UComboBoxWidget::SetEnumTagMap(const TObjectPtr<UEnumTagMap> InMap)
{
	EnumTagMap = InMap;
}
