// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSVerticalBox.h"
#include "BSWidgetInterface.h"
#include "Blueprint/UserWidget.h"
#include "BSSettingCategoryWidget.generated.h"

/** Base Setting Category widget, with a BSVerticalBox acting as the main container. The background colors of
 *  BSHorizontalBoxes inside the MainContainer are synced. Additional containers can be added, but will not be synced
 *  with MainContainer or each other. */
UCLASS()
class USERINTERFACE_API UBSSettingCategoryWidget : public UUserWidget, public IBSWidgetInterface
{
	GENERATED_BODY()

public:
	/** Calls UpdateBrushColors on all BSVerticalBoxes in Containers array. For each box, it iterates through the
	 *  widget tree and alternates the Brush Color of each widget. */
	void UpdateBrushColors() const;

	/** Returns the BSVerticalBox, the MainContainer for this widget. */
	UBSVerticalBox* GetMainContainer() const { return MainContainer; }

protected:
	virtual void NativeConstruct() override;

	/** Override this function to set up more specific settings, such as calling AddWidgetBoxPair inside it. */
	virtual void InitSettingCategoryWidget();

	/** Add another container separate from MainContainer that will also be called to update background colors,
	 *  but will sync with MainContainers, or with each other. */
	void AddContainer(const TArray<TObjectPtr<UBSVerticalBox>>& InContainers);

	/** Add another container separate from MainContainer that will also be called to update background colors,
	 *	but will sync with MainContainers, or with each other. */
	void AddContainer(const TObjectPtr<UBSVerticalBox>& InContainer);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSVerticalBox* MainContainer;

private:
	UPROPERTY()
	TArray<TObjectPtr<UBSVerticalBox>> Containers;
};
