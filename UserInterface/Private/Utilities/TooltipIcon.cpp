// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/TooltipIcon.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Styles/MenuOptionStyle.h"
#include "Utilities/BSWidgetInterface.h"
#include "Utilities/TooltipData.h"

UTooltipIcon::UTooltipIcon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), Button(nullptr),
                                                                          Image(nullptr),
                                                                          TooltipIconType(ETooltipIconType::Default)
{
	TooltipData = MakeShareable(NewObject<UTooltipData>(this));
	TooltipData->SetTooltipIcon(this);
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

void UTooltipIcon::BeginDestroy()
{
	Super::BeginDestroy();
	if (TooltipData)
	{
		TooltipData.Reset();
	}
}

UTooltipIcon* UTooltipIcon::CreateTooltipIcon(UUserWidget* InOwningObject, const ETooltipIconType Type)
{
	const UMenuOptionStyle* MenuOptionStyle = GetDefault<UMenuOptionStyle>();
	if (UTooltipIcon* Icon = CreateWidget<UTooltipIcon>(InOwningObject, MenuOptionStyle->TooltipIconClass))
	{
		Icon->SetTooltipIconType(Type);
		return Icon;
	}
	return nullptr;
}

void UTooltipIcon::SetTooltipIconType(const ETooltipIconType Type)
{
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

void UTooltipIcon::SetTooltipText(FText&& InText, const bool bAllowTextWrap) const
{
	TooltipData->SetTooltipText(MoveTemp(InText));
	TooltipData->SetAllowTextWrap(bAllowTextWrap);
}

TSharedPtr<UTooltipData> UTooltipIcon::GetTooltipData() const
{
	return TooltipData;
}
