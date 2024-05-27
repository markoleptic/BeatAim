// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Utilities/TooltipData.h"
#include "TooltipIcon.generated.h"

class UImage;
class UButton;
class UTooltipIcon;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTooltipHovered, const FTooltipData&, TooltipData);

/** A button and image representing an icon that executes a delegate when hovered over. */
UCLASS()
class USERINTERFACE_API UTooltipIcon : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	/*UTooltipIcon();*/

	static UTooltipIcon* CreateTooltipIcon(UUserWidget* InOwningObject, ETooltipIconType Type);

	void SetTooltipIconType(ETooltipIconType Type);

	UFUNCTION()
	void HandleTooltipHovered();

	/** Sets the tooltip text that is accessed when the tooltip icon is hovered over. */
	void SetTooltipText(const FText& InText, const bool bAllowTextWrap = false);

	/** Called when Button is hovered over, provides the tooltip data that should be displayed. */
	FOnTooltipHovered OnTooltipHovered;

protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UButton* Button;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UImage* Image;

	/** Info about what this Tooltip image should display. */
	UPROPERTY()
	FTooltipData TooltipData;

	UPROPERTY(EditDefaultsOnly, Category="TooltipIcon")
	TMap<ETooltipIconType, FSlateBrush> TooltipIconBrushMap;

	UPROPERTY(EditAnywhere, Category = "TooltipIcon")
	ETooltipIconType TooltipIconType;
};
