// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BoxBoundsWidget.generated.h"

class USizeBox;
class UImage;

/** Represents the BoxBounds of a game mode, to be used with CustomGameModePreviewWidget. */
UCLASS()
class USERINTERFACE_API UBoxBoundsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Sets the Width and Height overrides of the BoxBounds, which adjust the BoxBoundsImage size. */
	void SetBoxBounds(const FVector2d& InBounds) const;

	/** Sets the vertical padding of the BoxBounds. */
	void SetBoxBoundsPosition(const float VerticalOffset) const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	USizeBox* BoxBounds;
	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	UImage* BoxBoundsImage;
};
