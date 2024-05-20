// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerSettingsInterface.h"
#include "Blueprint/UserWidget.h"
#include "CrossHairWidget.generated.h"

class UCanvasPanelSlot;
class UOverlay;
class UImage;

/** Widget used to show the CrossHair. */
UCLASS()
class USERINTERFACE_API UCrossHairWidget : public UUserWidget, public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

public:
	void InitializeCrossHair(const FPlayerSettings_CrossHair& CrossHairSettings);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_Left;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_Top;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_Right;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_Bottom;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_Dot;

	UPROPERTY()
	UCanvasPanelSlot* Slot_Left;
	UPROPERTY()
	UCanvasPanelSlot* Slot_Right;
	UPROPERTY()
	UCanvasPanelSlot* Slot_Top;
	UPROPERTY()
	UCanvasPanelSlot* Slot_Bottom;
	UPROPERTY()
	UCanvasPanelSlot* Slot_Dot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_LeftOutline;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_TopOutline;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_RightOutline;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_BottomOutline;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_Left;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_Top;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_Right;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_Bottom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_Dot;

public:
	/** Sets the size of lines. */
	void SetLineSize(const FVector2d VerticalLineSize);

	/** Sets the size of the outlines. */
	void SetOutlineSize(const int32 NewSize, const FVector2d VerticalLineSize);

	/** Sets the color of the Outline Images. */
	void SetOutlineColor(const FLinearColor NewColor);

	/** Sets the color of the lines. */
	void SetLineColor(const FLinearColor NewColor);

	/** Sets the offset between the middle of the screen and the start of the lines. */
	void SetInnerOffset(const int32 NewOffset);

	/** Sets the visibility of the CrossHair Dot. */
	void SetShowCrossHairDot(const bool bShow);

	/** Sets the color of the CrossHair Dot. */
	void SetCrossHairDotColor(const FLinearColor NewColor);

	/** Sets the size of the CrossHair Dot. */
	void SetCrossHairDotSize(const int32 NewSize, const int32 OutlineSize);

	/** Sets the color of the CrossHair Dot Outline. */
	void SetCrossHairDotOutlineColor(const FLinearColor NewColor);

	/** Reinitialize CrossHair if Player Settings change. */
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_CrossHair& CrossHairSettings) override;
};
