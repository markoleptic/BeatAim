// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TooltipWidget.generated.h"

class UTooltipData;
class UTextBlock;

/** Simple widget used for tooltips. */
UCLASS()
class USERINTERFACE_API UTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTooltipWidget(const FObjectInitializer& ObjectInitializer);

	virtual void BeginDestroy() override;

	/** @return static tooltip widget singleton, creating one if not instantiated yet. */
	static UTooltipWidget* Get();

	/** Sets the text and text wrapping for the tooltip. Only necessary to call this if not using a TooltipIcon to
	 *  store TooltipData.
	 *  @param InText the text to set on the tooltip.
	 *  @param bAllowTextWrap whether to allow auto text wrapping.
	 */
	void SetText(const FText& InText, const bool bAllowTextWrap = false) const;

	/** Sets the text of the tooltip, and sets this as the tooltip of the tooltip icon. */
	void HandleTooltipIconHovered(const TObjectPtr<UTooltipData>& InTooltipData);

	/** Removes the static tooltip widget from the root set, and marks as garbage. */
	static void Cleanup();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TooltipDescriptor;

private:
	/** Instantiates the static tooltip widget. */
	static UTooltipWidget* InitializeTooltipWidget();

	static UTooltipWidget* TooltipWidget;
};
