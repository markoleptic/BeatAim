// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TargetWidget.generated.h"

class UImage;

/** Represents a spawned target, to be used with CustomGameModePreviewWidget. */
UCLASS()
class USERINTERFACE_API UTargetWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	/** Sets the desired size override of the TargetImage. */
	void SetTargetScale(const FVector& NewScale) const;

	/** Sets the color of the TargetImage. */
	void SetTargetColor(const FLinearColor& Color) const;

	/** Sets the OverlaySlot padding of the TargetImage. */
	void SetTargetPosition(const FVector2d& InPosition) const;

protected:
	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	UImage* TargetImage;

	UPROPERTY()
	UMaterialInstanceDynamic* TargetImageMaterial;
};
