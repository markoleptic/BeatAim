// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FAQWidget.generated.h"

class UVerticalBox;
class UBSButton;
class UBorder;
class UWidgetSwitcher;
class UMenuButton;

/** FAQ widget. */
UCLASS()
class USERINTERFACE_API UFAQWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_GameModes;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_Scoring;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_AudioAnalyzer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* FAQSwitcher;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_GameModes;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_Scoring;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_AudioAnalyzer;

private:
	UFUNCTION()
	void OnButtonClicked_BSButton(const UBSButton* Button);
};
