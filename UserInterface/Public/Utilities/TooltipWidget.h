// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TooltipWidget.generated.h"

struct FTooltipData;
class UTextBlock;

/** Simple widget used for tooltips. */
UCLASS()
class USERINTERFACE_API UTooltipWidget : public UUserWidget
{
	GENERATED_BODY()
	UTooltipWidget(const FObjectInitializer& ObjectInitializer);

public:
	/** Singleton access function. */
	static UTooltipWidget* Get();

	virtual void BeginDestroy() override;

	/** Sets the text and text wrapping for the tooltip. */
	void SetText(const FText& InText, const bool bAllowTextWrap = false) const;

	/** Sets the text of the tooltip, and sets this as the tooltip of the tooltip icon. */
	UFUNCTION()
	void HandleTooltipIconHovered(const FTooltipData& InTooltipData);

	static void Cleanup();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TooltipDescriptor;

private:
	/** Creates the static tooltip widget. */
	static UTooltipWidget* InitializeTooltipWidget();

	static UTooltipWidget* TooltipWidget;
};
