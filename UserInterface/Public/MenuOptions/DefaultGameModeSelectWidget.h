// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MenuOptionWidget.h"
#include "BSGameModeConfig/DefiningConfig.h"
#include "DefaultGameModeSelectWidget.generated.h"

class UCommonTextBlock;
class UBSButton;

UCLASS()
class USERINTERFACE_API UDefaultGameModeSelectWidget : public UMenuOptionWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual void SetStyling() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

public:
	EBaseGameMode GetBaseGameMode() const { return BaseGameMode; }

	void SetBaseGameMode(const EBaseGameMode InBaseGameMode) { BaseGameMode = InBaseGameMode; }

	void SetAltDescriptionText(const FText& Text);

	UPROPERTY(EditDefaultsOnly, meta=(BindWidget))
	UBSButton* Button;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DefaultGameModeOptionWidget")
	EBaseGameMode BaseGameMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(BindWidget))
	UCommonTextBlock* AltDescriptionText;
};
