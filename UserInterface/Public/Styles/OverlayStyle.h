// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OverlayStyle.generated.h"

/** Defines styles used in Overlay Widgets. */
UCLASS(Abstract, Blueprintable, BlueprintType)
class USERINTERFACE_API UOverlayStyle : public UObject
{
	GENERATED_BODY()

public:
	/** The padding applied to main content windows. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OverlayStyle|Content")
	FMargin Padding_Content;

	/** The padding applied to the title. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OverlayStyle|Title")
	FMargin Padding_Title;

	/** The font applied to the title. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OverlayStyle|Title")
	FSlateFontInfo Font_Title;

	/** The font applied to the body. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OverlayStyle|Body")
	FSlateFontInfo Font_Body;

	/** The padding applied to buttons. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OverlayStyle|Buttons")
	FMargin Padding_Buttons;

	/** The font applied to buttons. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OverlayStyle|Buttons")
	FSlateFontInfo Font_Button;
};
