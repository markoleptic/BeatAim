// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CarouselNavButtonStyle.generated.h"

class UBSButton;

USTRUCT(BlueprintType)
struct FNavButtonStyle
{
	GENERATED_BODY()

	/** The display text of the button. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Text;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMargin Padding;

	/** The horizontal alignment of the button. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<EHorizontalAlignment> HAlign = HAlign_Fill;

	/** The vertical alignment of the button. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<EVerticalAlignment> VAlign = VAlign_Fill;

	/** Whether to fill the width or use Auto Width. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ScriptName="DoFillWidth"))
	bool bFillWidth = true;

	/** Fill width coefficient. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition="bFillWidth", EditConditionHides))
	float FillWidth = 1.f;
};

/** Defines styles used for the buttons of the Carousel Nav Bar. */
UCLASS(Abstract, Blueprintable, BlueprintType)
class USERINTERFACE_API UCarouselNavButtonStyle : public UObject
{
	GENERATED_BODY()

public:
	/** The button widget to use. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UBSButton> ButtonWidgetType;

	/** The style and text for each button widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ForceInlineRow))
	TArray<FNavButtonStyle> ButtonStyles;
};
