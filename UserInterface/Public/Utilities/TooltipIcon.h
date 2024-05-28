// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Utilities/TooltipData.h"
#include "TooltipIcon.generated.h"

class UImage;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTooltipIconHovered, const FTooltipData&, TooltipData);

/** A button and image representing an icon that executes a delegate when hovered over. */
UCLASS()
class USERINTERFACE_API UTooltipIcon : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

	/** Broadcasts OnTooltipHovered delegate. */
	UFUNCTION()
	void HandleTooltipHovered();

public:
	UTooltipIcon(const FObjectInitializer& ObjectInitializer);

	/** Static creator function based. */
	static UTooltipIcon* CreateTooltipIcon(UUserWidget* InOwningObject, ETooltipIconType Type);

	/** Modifies the appearance of the icon based on the type. */
	void SetTooltipIconType(ETooltipIconType Type);

	/** Sets the tooltip text that is accessed when the tooltip icon is hovered over. */
	void SetTooltipText(const FText& InText, const bool bAllowTextWrap = false);

	/** Called when Button is hovered over, provides the tooltip data that should be displayed. */
	FOnTooltipIconHovered OnTooltipIconHovered;

protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UButton* Button;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UImage* Image;

	/** Data broadcast to the tooltip widget. */
	UPROPERTY()
	FTooltipData TooltipData;

	UPROPERTY(EditDefaultsOnly, Category="TooltipIcon")
	TMap<ETooltipIconType, FSlateBrush> TooltipIconBrushMap;

	UPROPERTY(EditAnywhere, Category = "TooltipIcon")
	ETooltipIconType TooltipIconType;
};
