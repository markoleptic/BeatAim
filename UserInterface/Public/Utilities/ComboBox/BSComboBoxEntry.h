// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BSComboBoxEntry.generated.h"

class UBorder;
class UTextBlock;
class UTooltipIcon;
class UMenuOptionStyle;

/** Simple class used for ComboBox entries. Allows for tooltips to be added to entries. */
UCLASS()
class USERINTERFACE_API UBSComboBoxEntry : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void SetStyles();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "BSComboBoxEntry|Style")
	TSubclassOf<UMenuOptionStyle> MenuOptionStyleClass;

	UPROPERTY()
	const UMenuOptionStyle* MenuOptionStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor NotSelectedColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor SelectedColor;

	/** The image to the left of the text, that will display a tooltip when hovered. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTooltipIcon* TooltipIcon;

	/** The main text of the entry. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_Entry;

	/** The background of the entry. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBorder* Background;

	/** Whether to always hide the tooltip image. */
	mutable bool bAlwaysHideTooltipIcon = false;

public:
	/** Sets the main text of the entry. */
	void SetEntryText(const FText& InText) const;

	FText GetEntryText() const;

	/** Returns the TooltipIcon. */
	UTooltipIcon* GetTooltipIcon() const { return TooltipIcon; }

	/** Returns the string form of the main entry text. */
	FString GetEntryTextAsString() const;

	/** Shows or hides the TooltipIcon */
	void SetTooltipIconVisibility(const bool bIsVisible) const;

	/** Sets whether to always hide the TooltipIcon. */
	void SetAlwaysHideTooltipIcon(const bool bShouldAlwaysHide) const
	{
		bAlwaysHideTooltipIcon = bShouldAlwaysHide;
	}

	/** Sets the Brush tint for the Border. */
	void SetBackgroundBrushTint(const FLinearColor& Color) const;
};
