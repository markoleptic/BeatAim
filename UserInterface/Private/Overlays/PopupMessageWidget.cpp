// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Overlays/PopupMessageWidget.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/TextBlock.h"
#include "Utilities/Buttons/BSButton.h"

void UPopupMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

TArray<UBSButton*> UPopupMessageWidget::InitPopup(const FText& TitleInput, const FText& MessageInput,
	const int32 NumButtons)
{
	TArray<UBSButton*> Buttons;
	TextBlock_Title->SetText(TitleInput);
	if (MessageInput.IsEmpty())
	{
		TextBlock_Message->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		TextBlock_Message->SetText(MessageInput);
		TextBlock_Message->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	for (int i = 0; i < NumButtons; i++)
	{
		UBSButton* Button = CreateWidget<UBSButton>(this, ButtonClass);
		UGridSlot* GridSlot = GridPanel->AddChildToGrid(Button, 0, i);
		GridSlot->SetHorizontalAlignment(HorizontalAlignment);
		GridSlot->SetVerticalAlignment(VerticalAlignment);
		Buttons.Add(Button);
	}
	return Buttons;
}

void UPopupMessageWidget::FadeIn()
{
	PlayAnimationForward(FadeInMessage);
}

void UPopupMessageWidget::FadeOut()
{
	FadeOutPopupMessageDelegate.BindDynamic(this, &UPopupMessageWidget::OnFadeOutPopupMessageFinish);
	BindToAnimationFinished(FadeOutMessage, FadeOutPopupMessageDelegate);
	PlayAnimationForward(FadeOutMessage);
}

void UPopupMessageWidget::ChangeMessageText(const FText& MessageInput)
{
	if (MessageInput.IsEmpty())
	{
		TextBlock_Message->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		TextBlock_Message->SetText(MessageInput);
		TextBlock_Message->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UPopupMessageWidget::OnFadeOutPopupMessageFinish()
{
	if (bRemoveFromParentOnFadeOut)
	{
		RemoveFromParent();
	}
}
