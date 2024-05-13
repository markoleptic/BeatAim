// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Utilities/TooltipImage.h"
#include "MenuOptionStyle.generated.h"

/** Defines styles used in MenuOptionWidgets */
UCLASS(Abstract, Blueprintable, BlueprintType)
class USERINTERFACE_API UMenuOptionStyle : public UObject
{
	GENERATED_BODY()

public:
	/** The amount to indent the widget to the left. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle")
	float IndentAmount = 50.f;

	/** The vertical alignment used when placing a new GameModeCategoryTagWidget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Alignment")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment_TagWidget = VAlign_Center;

	/** The horizontal alignment used when placing a new GameModeCategoryTagWidget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Alignment")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment_TagWidget = HAlign_Right;

	/** The class used for Tooltip Warnings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Classes")
	TSubclassOf<UTooltipImage> TooltipWarningImageClass;

	/** The class used for Tooltip Cautions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Classes")
	TSubclassOf<UTooltipImage> TooltipCautionImageClass;

	/** The font applied to the DescriptionText. Also applied to ComboBoxes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Fonts")
	FSlateFontInfo Font_DescriptionText;

	/** The font applied to EditableTextBoxes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Fonts")
	FSlateFontInfo Font_EditableText;

	/** The font applied to the EditableTextBoxes beside a slider. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Fonts")
	FSlateFontInfo Font_EditableTextBesideSlider;

	/** The padding applied to the DescriptionText. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Padding")
	FMargin Padding_DescriptionText = FMargin(0.f, 4.f);

	/** The padding applied to the left horizontal box inside the left border. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Padding")
	FMargin Padding_LeftBox = FMargin(10.f, 5.f);

	/** The padding applied to the right horizontal box inside the right border. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Padding")
	FMargin Padding_RightBox = FMargin(0.f, 0.f);

	/** The padding applied to the box containing the GameModeCategoryTags and TooltipImages. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Padding")
	FMargin Padding_TagsAndTooltips = FMargin(10.f, 5.f);

	/** The padding used when placing a new Tooltip Warning/Caution. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Padding")
	FMargin Padding_TooltipWarning = FMargin(10.f, 0.f, 0.f, 0.f);

	/** The padding used when placing a new GameModeCategoryTagWidget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Padding")
	FMargin Padding_TagWidget = FMargin(0.f, 0.f, 0.f, 0.f);

	/** The padding applied to EditableTextBoxes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MenuOptionStyle|Padding")
	FMargin Padding_EditableText = FMargin(13.f, 3.f);
};
