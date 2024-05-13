// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameModeCategoryTagWidget.generated.h"

class UImage;
/** Visual tag that can applied to a Game mode or Game Mode setting. */
UCLASS()
class USERINTERFACE_API UGameModeCategoryTagWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetText(const FText& InText);
	void SetColor(const FLinearColor& InColor);
	FGameplayTag GetGameModeCategoryTag() const { return GameModeCategoryTag; }
	FText GetGameModeCategoryText() const { return CategoryText; }

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_Category;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UImage* Image_Material;

	UPROPERTY(EditDefaultsOnly, Category="GameModeCategoryTagWidget")
	FText CategoryText = FText();

	UPROPERTY(EditDefaultsOnly, Category="GameModeCategoryTagWidget")
	FGameplayTag GameModeCategoryTag;

	UPROPERTY(EditDefaultsOnly, Category="GameModeCategoryTagWidget")
	FLinearColor TagColor;
};
