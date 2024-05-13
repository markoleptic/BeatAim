// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SubmenuStyle.generated.h"

/** Defines styles used in Settings widgets and Game Modes widgets. */
UCLASS(Abstract, Blueprintable, BlueprintType)
class USERINTERFACE_API USubmenuStyle : public UObject
{
	GENERATED_BODY()

public:
	/** The padding applied to the content window. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SubmenuStyle|Content")
	FMargin Padding_Content;

	/** The padding applied to the object containing the buttons. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SubmenuStyle|Sidebar")
	FMargin Padding_Sidebar;

	/** The font applied to NavigationButtons. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SubmenuStyle|Buttons")
	FSlateFontInfo NavigationButtonFont;

	/** The font applied to first level headers. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SubmenuStyle|Headers")
	FSlateFontInfo Font_Header1;

	/** The padding applied to first level headers. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SubmenuStyle|Headers")
	FSlateFontInfo Padding_Header1;

	/** The font applied to second level headers. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SubmenuStyle|Headers")
	FSlateFontInfo Font_Header2;

	/** The padding applied to second level headers. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SubmenuStyle|Headers")
	FSlateFontInfo Padding_Header2;

	/** The font applied to third level headers. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SubmenuStyle|Headers")
	FSlateFontInfo Font_Header3;

	/** The padding applied to third level headers. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SubmenuStyle|Headers")
	FSlateFontInfo Padding_Header3;
};
