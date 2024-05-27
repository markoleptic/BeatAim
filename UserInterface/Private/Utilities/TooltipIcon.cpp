// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/TooltipIcon.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Styles/MenuOptionStyle.h"
#include "Utilities/BSWidgetInterface.h"

/*UTooltipIcon::UTooltipIcon(): Button(nullptr), Image(nullptr), TooltipData(FTooltipData()),
                              TooltipIconType(ETooltipIconType::Default)
{
}*/

void UTooltipIcon::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UTooltipIcon::NativeConstruct()
{
	Super::NativeConstruct();
	TooltipData.TooltipIcon = this;
	Button->OnHovered.AddDynamic(this, &UTooltipIcon::HandleTooltipHovered);
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
	OnTooltipHovered.Broadcast(TooltipData);
}

void UTooltipIcon::SetTooltipText(const FText& InText, const bool bAllowTextWrap)
{
	TooltipData.TooltipText = InText;
	TooltipData.bAllowTextWrap = bAllowTextWrap;
}
