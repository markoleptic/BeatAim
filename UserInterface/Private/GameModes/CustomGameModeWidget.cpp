// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModeWidget.h"
#include "Blueprint/WidgetTree.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "GameModes/CustomGameModeCategoryWidget.h"
#include "GameModes/CustomGameModeStartWidget.h"

UCustomGameModeStartWidget* UCustomGameModeWidget::GetStartWidget() const
{
	if (const TObjectPtr<UCustomGameModeCategoryWidget>* Widget = GameModeCategoryWidgetMap.Find(
		EGameModeCategory::Start))
	{
		return Cast<UCustomGameModeStartWidget>(Widget->Get());
	}
	return nullptr;
}

TArray<TObjectPtr<UCustomGameModeCategoryWidget>> UCustomGameModeWidget::GetCustomGameModeCategoryWidgets() const
{
	TArray<TObjectPtr<UCustomGameModeCategoryWidget>> Out;
	GameModeCategoryWidgetMap.GenerateValueArray(Out);
	return Out;
}

void UCustomGameModeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	WidgetTree->ForEachWidget([&](UWidget* Widget)
	{
		if (UCustomGameModeCategoryWidget* Component = Cast<UCustomGameModeCategoryWidget>(Widget))
		{
			Component->OnPropertyChanged.BindUObject(this, &UCustomGameModeWidget::HandlePropertyChanged);
			GameModeCategoryWidgetMap.Add(Component->GetGameModeCategory(), Component);
		}
	});
}

void UCustomGameModeWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UCustomGameModeWidget::HandlePropertyChanged(const TSet<uint32>& Properties)
{
	OnPropertyChanged.Execute(Properties);
}

void UCustomGameModeWidget::Init(const TSharedPtr<FBSConfig>& InConfig)
{
	for (const auto [EGameModeCategory, Widget] : GameModeCategoryWidgetMap)
	{
		Widget->InitComponent(InConfig);
	}
}

void UCustomGameModeWidget::UpdateOptionsFromConfig()
{
	for (const auto& [Category, Widget] : GameModeCategoryWidgetMap)
	{
		Widget->UpdateOptionsFromConfig();
	}
}
