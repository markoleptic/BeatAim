// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeCategoryWidget.h"
#include "CustomGameModePreviewWidget.generated.h"

class UOverlay;
class UTextBlock;
class USpacer;
class USizeBox;
class UBoxBoundsWidget;
class UTargetWidget;
class UBSButton;

UCLASS()
class USERINTERFACE_API UCustomGameModePreviewWidget : public UCustomGameModeCategoryWidget
{
public:
	UCustomGameModePreviewWidget();

private:
	GENERATED_BODY()

public:
	/** Sets the bounds and position of the BoxBounds_Current widget. */
	void SetBoxBounds_Current(const FVector2d& InBounds, const float VerticalOffset) const;

	/** Sets the bounds and position of the BoxBounds_Max widget. */
	void SetBoxBounds_Max(const FVector2d& InBounds, const float VerticalOffset) const;

	/** Sets the bounds and position of the BoxBounds_Min widget. */
	void SetBoxBounds_Min(const FVector2d& InBounds, const float VerticalOffset) const;

	/** Sets the visibility of the BoxBounds_Max. */
	void SetBoxBoundsVisibility_Max(const ESlateVisibility& InVisibility) const;

	/** Sets the visibility of the BoxBounds_Min. */
	void SetBoxBoundsVisibility_Min(const ESlateVisibility& InVisibility) const;

	/** Sets the height override of the StaticBounds SizeBox. */
	void SetStaticBoundsHeight(const float InHeight) const;

	/** Sets the height override of the FloorDistance SizeBox. */
	void SetFloorDistanceHeight(const float InHeight) const;

	/** Sets the text of TextBlock_FloorDistance. */
	void SetText_FloorDistance(const FText& InText) const;

	/** Sets the Overlay and BottomOverlay's left and right padding,
	 * and sets the InMargin.Top to TopSpacer's Y value. */
	void SetOverlayPadding(const FMargin InMargin) const;

	/** Returns the TopSpacer Y value. */
	float GetSpacerOffset() const;

	/** Returns a UTargetWidget that was created and added to the Overlay. */
	UTargetWidget* ConstructTargetWidget();

	/** Shows or hides the BoxBounds Current, Min, Max and TextBlock_GameModePreviewAvailability */
	void ToggleGameModePreview(const bool bEnable) const;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UBoxBoundsWidget* BoxBounds_Current;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UBoxBoundsWidget* BoxBounds_Max;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UBoxBoundsWidget* BoxBounds_Min;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	USizeBox* StaticBounds;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	USizeBox* FloorDistance;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UVerticalBox* Box_StaticBoundsAndIndicators;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	USpacer* TopSpacer;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* TextBlock_FloorDistance;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_RefreshPreview;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Create;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_Start;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UTargetWidget> TargetWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UOverlay* Overlay;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UOverlay* Overlay_Bottom;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* TextBlock_GameModePreviewAvailability;
};
