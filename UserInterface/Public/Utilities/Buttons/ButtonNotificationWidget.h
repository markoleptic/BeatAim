// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ButtonNotificationWidget.generated.h"

class UOverlay;
class UTextBlock;

UCLASS()
class USERINTERFACE_API UButtonNotificationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetNumWarnings(const uint32 InNumWarnings);
	void SetNumCautions(const uint32 InNumCautions);

protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock* TextBlock_NumWarnings;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock* TextBlock_NumCautions;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UOverlay* Overlay_Warnings;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UOverlay* Overlay_Cautions;
};
