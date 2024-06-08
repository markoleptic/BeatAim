// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/TooltipIcon.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Utilities/TooltipData.h"
#include "Utilities/TooltipWidget.h"

UTooltipIcon::UTooltipIcon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), Button(nullptr),
                                                                          Image(nullptr),
                                                                          TooltipIconType(ETooltipIconType::Default)
{
}

void UTooltipIcon::NativePreConstruct()
{
	Super::NativePreConstruct();
	SetTooltipIconType(TooltipIconType);
}

void UTooltipIcon::NativeConstruct()
{
	Super::NativeConstruct();
	SetTooltipIconType(TooltipIconType);
	Button->OnHovered.AddDynamic(this, &UTooltipIcon::HandleTooltipHovered);
}

void UTooltipIcon::PostInitProperties()
{
	Super::PostInitProperties();
	TooltipData.SetTooltipIcon(this);
}

UTooltipIcon* UTooltipIcon::CreateTooltipIcon(UUserWidget* InOwningObject, const ETooltipIconType Type)
{
	UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr,
		TEXT("/Game/UserInterface/Utilities/Tooltips/WBP_TooltipIcon.WBP_TooltipIcon"));
	const UBlueprint* Blueprint = Cast<UBlueprint>(LoadedObject);
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return nullptr;
	}
	const TSubclassOf<UUserWidget> WidgetClass = TSubclassOf<UUserWidget>(Blueprint->GeneratedClass);
	if (UTooltipIcon* Icon = CreateWidget<UTooltipIcon>(InOwningObject, WidgetClass))
	{
		Icon->SetTooltipIconType(Type);
		Icon->OnTooltipIconHovered.AddUObject(UTooltipWidget::Get(), &UTooltipWidget::HandleTooltipIconHovered);
		return Icon;
	}
	return nullptr;
}

void UTooltipIcon::SetTooltipIconType(const ETooltipIconType Type)
{
	TooltipIconType = Type;
	if (Image)
	{
		if (const FSlateBrush* Brush = TooltipIconBrushMap.Find(Type))
		{
			Image->SetBrush(*Brush);
		}
	}
}

void UTooltipIcon::HandleTooltipHovered()
{
	OnTooltipIconHovered.Broadcast(TooltipData);
}

void UTooltipIcon::SetTooltipText(const FText& InText, const bool bAllowTextWrap)
{
	TooltipData.SetTooltipText(InText);
	TooltipData.SetAllowTextWrap(bAllowTextWrap);
}

FTooltipData& UTooltipIcon::GetTooltipData()
{
	return TooltipData;
}

ETooltipIconType UTooltipIcon::GetType() const
{
	return TooltipIconType;
}
