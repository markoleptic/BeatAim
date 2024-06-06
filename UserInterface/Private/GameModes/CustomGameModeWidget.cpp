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
			//const bool bIndexOnCarousel = Component->ShouldIndexOnCarousel();
			//Component->InitComponent(InConfig, bIndexOnCarousel ? Index : -1);
			//Component->RequestComponentUpdate.AddUObject(this, &ThisClass::OnRequestComponentUpdate);
			//Component->RequestGameModePreviewUpdate.AddUObject(this, &ThisClass::OnRequestGameModePreviewUpdate);
			Component->OnPropertyChanged.BindUObject(this, &UCustomGameModeWidget::HandlePropertyChanged);
			GameModeCategoryWidgetMap.Add(Component->GetGameModeCategory(), Component);
			/*ChildWidgetValidityMap.FindOrAdd(Component) = Component->GetCustomGameModeCategoryInfo();
			if (bIndexOnCarousel)
			{
				Index++;
			}*/
		}
	});
	//Widget_Start->RequestGameModeTemplateUpdate.AddUObject(this, &ThisClass::OnRequestGameModeTemplateUpdate);
	//Widget_Start->OnCustomGameModeNameChanged.AddUObject(this, &ThisClass::OnStartWidget_CustomGameModeNameChanged);
}

void UCustomGameModeWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UCustomGameModeWidget::HandlePropertyChanged(const TSet<FPropertyHash>& Properties)
{
	OnPropertyChanged.Execute(Properties);
}

void UCustomGameModeWidget::Init(const TSharedPtr<FBSConfig>& InConfig)
{
	for (const auto [EGameModeCategory, Widget] : GameModeCategoryWidgetMap)
	{
		Widget->InitComponent(InConfig);
	}
	UpdateOptionsFromConfig();
}

void UCustomGameModeWidget::UpdateOptionsFromConfig()
{
	for (const auto& [Category, Widget] : GameModeCategoryWidgetMap)
	{
		Widget->UpdateOptionsFromConfig();
	}
	//UpdateAllChildWidgetOptionsValid();
}

/*void UCustomGameModeWidget::UpdateAllChildWidgetOptionsValid()
{
	bool bAtLeastOneWarningPresent = false;
	uint8 TotalWarnings = 0;
	uint8 TotalCautions = 0;
	for (const TPair<TObjectPtr<UCustomGameModeCategoryWidget>, FCustomGameModeCategoryInfo*>& ChildWidgetValidity :
	     ChildWidgetValidityMap)
	{
		if (const TObjectPtr<UCustomGameModeCategoryWidget> Component = ChildWidgetValidity.Key)
		{
			if (Component->IsInitialized() && Component != Widget_Start)
			{
				Component->UpdateAllOptionsValid();
				TotalWarnings += ChildWidgetValidity.Value->NumWarnings;
				TotalCautions += ChildWidgetValidity.Value->NumCautions;
				if (ChildWidgetValidity.Value->NumWarnings > 0)
				{
					bAtLeastOneWarningPresent = true;
				}
				if (ChildWidgetValidity.Value->NumWarnings > 0 || ChildWidgetValidity.Value->NumCautions)
				{
					UE_LOG(LogTemp, Display, TEXT("%s has %d cautions and %d warnings"), *Component->GetName(),
						ChildWidgetValidity.Value->NumCautions, ChildWidgetValidity.Value->NumWarnings);
				}
			}
		}
	}
	UE_LOG(LogTemp, Display, TEXT("TotalWarnings: %d TotalCautions: %d"), TotalWarnings, TotalCautions);
	UpdateContainsGameModeBreakingOption(bAtLeastOneWarningPresent);
	RequestButtonStateUpdate.Broadcast();
}*/

/*FString UCustomGameModeWidget::GetNewCustomGameModeName() const
{
	return Widget_Start->GetNewCustomGameModeName();
}*/

/*void UCustomGameModeWidget::SetNewCustomGameModeName(const FString& InCustomGameModeName) const
{
	Widget_Start->SetNewCustomGameModeName(InCustomGameModeName);
}*/

/*FStartWidgetProperties UCustomGameModeWidget::GetStartWidgetProperties() const
{
	return Widget_Start->GetStartWidgetProperties();
}*/

/*void UCustomGameModeWidget::SetStartWidgetProperties(const FStartWidgetProperties& InProperties)
{
	Widget_Start->SetStartWidgetProperties(InProperties);
}*/

/*bool UCustomGameModeWidget::GetAllNonStartChildWidgetOptionsValid() const
{
	for (const TPair<TObjectPtr<UCustomGameModeCategoryWidget>, FCustomGameModeCategoryInfo*>& ChildWidgetValidity :
	     ChildWidgetValidityMap)
	{
		if (ChildWidgetValidity.Key == Widget_Start)
		{
			continue;
		}
		if (!ChildWidgetValidity.Value->GetAllOptionsValid())
		{
			return false;
		}
	}
	return true;
}*/

/*void UCustomGameModeWidget::RefreshGameModeTemplateComboBoxOptions(const TArray<FBSConfig>& CustomGameModes) const
{
	Widget_Start->RefreshGameModeTemplateComboBoxOptions(CustomGameModes);
}*/

/*void UCustomGameModeWidget::OnRequestGameModeTemplateUpdate(const FString& InGameMode,
	const EGameModeDifficulty& Difficulty)
{
	RequestGameModeTemplateUpdate.Broadcast(InGameMode, Difficulty);
}*/

/*void UCustomGameModeWidget::OnStartWidget_CustomGameModeNameChanged()
{
	RequestButtonStateUpdate.Broadcast();
}*/

/*void UCustomGameModeWidget::OnRequestComponentUpdate()
{
	if (!bIsUpdatingFromComponentRequest)
	{
		bIsUpdatingFromComponentRequest = true;
		UpdateAllChildWidgetOptionsValid();
		bIsUpdatingFromComponentRequest = false;
	}
}*/

/*void UCustomGameModeWidget::OnRequestGameModePreviewUpdate()
{
	RequestGameModePreviewUpdate.Broadcast();
}*/

/*void UCustomGameModeWidget::UpdateContainsGameModeBreakingOption(const bool bGameModeBreakingOptionPresent)
{
	if (bGameModeBreakingOptionPresent == bContainsGameModeBreakingOption)
	{
		return;
	}
	if (bGameModeBreakingOptionPresent)
	{
		UE_LOG(LogTemp, Display,
			TEXT("bContainsGameModeBreakingOption changed from false to true inside UCustomGameModeWidget"));
	}
	else
	{
		UE_LOG(LogTemp, Display,
			TEXT("bContainsGameModeBreakingOption changed from true to false inside UCustomGameModeWidget"));
	}
	bContainsGameModeBreakingOption = bGameModeBreakingOptionPresent;
	OnGameModeBreakingChange.Broadcast(bContainsGameModeBreakingOption);
}*/
