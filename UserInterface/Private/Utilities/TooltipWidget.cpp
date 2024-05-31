// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/TooltipWidget.h"
#include "Components/TextBlock.h"
#include "Utilities/TooltipData.h"
#include "Utilities/TooltipIcon.h"

UTooltipWidget* UTooltipWidget::TooltipWidget = nullptr;

UTooltipWidget::UTooltipWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Don't set up Singleton instance in CDO constructor or other Unreal-style constructors
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad | RF_NeedPostLoad | RF_NeedPostLoadSubobjects))
	{
		ensure(TooltipWidget == nullptr);
		TooltipWidget = this;
	}
}

UTooltipWidget* UTooltipWidget::Get()
{
	if (!TooltipWidget)
	{
		TooltipWidget = InitializeTooltipWidget();
	}
	return TooltipWidget;
}

void UTooltipWidget::BeginDestroy()
{
	if (TooltipWidget != nullptr)
	{
		if (ensure(TooltipWidget == this))
		{
			TooltipWidget = nullptr;
		}
		RemoveFromRoot();
	}
	Super::BeginDestroy();
}

void UTooltipWidget::SetText(const FText& InText, const bool bAllowTextWrap) const
{
	TooltipDescriptor->SetText(InText);
	TooltipDescriptor->SetAutoWrapText(bAllowTextWrap);
}

void UTooltipWidget::HandleTooltipIconHovered(const TObjectPtr<UTooltipData>& InTooltipData)
{
	SetText(InTooltipData->GetTooltipText(), InTooltipData->GetAllowTextWrap());
	if (const TWeakObjectPtr<UTooltipIcon> TooltipIcon = InTooltipData->GetTooltipIcon(); TooltipIcon.IsValid())
	{
		TooltipIcon->SetToolTip(this);
	}
}

void UTooltipWidget::Cleanup()
{
	if (TooltipWidget)
	{
		TooltipWidget->RemoveFromRoot();
		TooltipWidget->MarkAsGarbage();
		TooltipWidget = nullptr;
	}
}

UTooltipWidget* UTooltipWidget::InitializeTooltipWidget()
{
	if (!TooltipWidget)
	{
		UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr,
			TEXT("/Game/UserInterface/Utilities/Tooltips/WBP_Tooltip.WBP_Tooltip"));
		const UBlueprint* Blueprint = Cast<UBlueprint>(LoadedObject);
		if (!Blueprint || !Blueprint->GeneratedClass)
		{
			return nullptr;
		}
		const TSubclassOf<UUserWidget> WidgetClass = TSubclassOf<UUserWidget>(Blueprint->GeneratedClass);
		if (GEngine && GEngine->GameViewport && WidgetClass)
		{
			TooltipWidget = CreateWidget<UTooltipWidget>(GEngine->GameViewport->GetWorld(), WidgetClass);
			TooltipWidget->AddToRoot();
		}
	}
	return TooltipWidget;
}
