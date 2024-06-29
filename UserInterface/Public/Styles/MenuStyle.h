// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MenuStyle.generated.h"

/** Defines styles used in Menu Widgets. */
UCLASS(Abstract, Blueprintable, BlueprintType)
class USERINTERFACE_API UMenuStyle : public UObject
{
	GENERATED_BODY()

public:
	/** The padding applied to main content windows. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MenuStyle|Content")
	FMargin Padding_MainContent;

	/** The padding applied to sidebar windows. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MenuStyle|Sidebar")
	FMargin Padding_Sidebar;

	/** The font applied to SidebarButtons. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MenuStyle|Sidebar")
	FSlateFontInfo Font_SidebarButton;
};
