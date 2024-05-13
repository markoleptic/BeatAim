// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "SavedTextWidget.generated.h"

class UTextBlock;

/** Widget that displays text with an animation. */
UCLASS()
class USERINTERFACE_API USavedTextWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* SavedText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeInFadeOut;

public:
	UFUNCTION()
	void PlayFadeInFadeOut();

	/** Sets the text that should appear when PlayFadeInFadeOut is called. */
	void SetSavedText(const FText& Text) { SavedText->SetText(Text); }
};
