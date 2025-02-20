﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeWidget.h"
#include "CreatorViewWidget.generated.h"

class USavedTextWidget;
class UScrollBox;
class UBSCarouselNavBar;
class UCustomGameModeCategoryWidget;
class UCustomGameModePreviewWidget;
class UCommonWidgetCarousel;

UCLASS()
class USERINTERFACE_API UCreatorViewWidget : public UCustomGameModeWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	/** Updates icons on the carousel. */
	void UpdateNotificationIcons(const TMap<EGameModeCategory, TPair<int32, int32>>& IconMap);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCustomGameModePreviewWidget* Widget_Preview;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USavedTextWidget* SavedTextWidget_CreatorView;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* ScrollBox;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonWidgetCarousel* Carousel;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSCarouselNavBar* CarouselNavBar;

	TMap<EGameModeCategory, int32> GameModeCategoryWidgetIndexMap;
};
