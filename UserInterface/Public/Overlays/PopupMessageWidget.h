// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PopupMessageWidget.generated.h"

class UGridPanel;
class UBSButton;
class UTextBlock;
class UWidgetAnimation;

/** Provides a simple pop up dialog for other widgets. */
UCLASS()
class USERINTERFACE_API UPopupMessageWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, Category = "PopupMessageWidget")
	TSubclassOf<UBSButton> ButtonClass;
	UPROPERTY(EditDefaultsOnly, Category = "PopupMessageWidget")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill;
	UPROPERTY(EditDefaultsOnly, Category = "PopupMessageWidget")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment = VAlign_Fill;

public:
	TArray<UBSButton*> InitPopup(const FText& TitleInput, const FText& MessageInput, const int32 NumButtons);

	UFUNCTION()
	virtual void FadeIn();
	UFUNCTION()
	virtual void FadeOut();

	void SetShouldRemoveFromParentOnFadeOut(const bool bShouldRemove) { bRemoveFromParentOnFadeOut = bShouldRemove; }

	virtual void ChangeMessageText(const FText& MessageInput);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UGridPanel* GridPanel;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_Title;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_Message;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutMessage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeInMessage;

	FWidgetAnimationDynamicEvent FadeOutPopupMessageDelegate;

	UFUNCTION()
	void OnFadeOutPopupMessageFinish();

	bool bRemoveFromParentOnFadeOut = true;
};
