// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/VerticalBox.h"
#include "BSVerticalBox.generated.h"

class UBSHorizontalBox;
/** Container holding BSHorizontalBoxes. Primarily used to update brush tints if some boxes have been collapsed. */
UCLASS()
class USERINTERFACE_API UBSVerticalBox : public UVerticalBox
{
	GENERATED_BODY()

	friend class UBSSettingCategoryWidget;

	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	/** Updates the brush colors of the BS horizontal boxes one level below any BS vertical box inside this widget. */
	void UpdateBrushColors();

private:
	/** Iterates through all slots in the widget, and changes the Brush Color of the borders in each BSHorizontalBox.
	 *  Returns true if the last left side border was dark, so that the next slot can do the opposite. Skips collapsed
	 *  boxes. */
	bool UpdateBrushColors_Private(TArray<TObjectPtr<UPanelSlot>>& InSlots, bool bLastLeftBorderDark);

	void UpdateHorizontalBoxBorderColors(UBSHorizontalBox* HorizontalBox, bool& bLastLeftBorderDark);
};
