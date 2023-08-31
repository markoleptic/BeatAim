﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "WidgetComponents/Buttons/BSButton.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UBSButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (TextBlock)
	{
		TextBlock->SetText(ButtonText);
		TextBlock->SetFont(DefaultFontInfo);
	}
	if (ImageMaterial)
	{
		DynamicImageMaterial = ImageMaterial->GetDynamicMaterial();
		DynamicImageMaterial->SetScalarParameterValue(HoveredScalarParameterName, 0.f);
		DynamicImageMaterial->SetScalarParameterValue(PressedScalarParameterName, 0.f);
	}
}

void UBSButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (TextBlock)
	{
		TextBlock->SetText(ButtonText);
		TextBlock->SetFont(DefaultFontInfo);
	}
	if (ImageMaterial)
	{
		DynamicImageMaterial = ImageMaterial->GetDynamicMaterial();
		DynamicImageMaterial->SetScalarParameterValue(HoveredScalarParameterName, 0.f);
		DynamicImageMaterial->SetScalarParameterValue(PressedScalarParameterName, 0.f);
	}
	if (Button)
	{
		Button->OnPressed.AddDynamic(this, &UBSButton::OnPressed_Button);
		Button->OnHovered.AddDynamic(this, &UBSButton::OnHovered_Button);
		Button->OnUnhovered.AddDynamic(this, &UBSButton::OnUnhovered_Button);
	}
	if (bRestoreClickedButtonState)
	{
		OnAnimFinished_OnPressedEvent.BindDynamic(this, &ThisClass::OnAnimFinished_OnPressed);
		BindToAnimationFinished(Anim_OnPressed, OnAnimFinished_OnPressedEvent);
	}
}

void UBSButton::SetInActive()
{
	if (!bIsClicked)
	{
		return;
	}
	if (IsAnimationPlaying(Anim_OnPressed))
	{
		ReverseAnimation(Anim_OnPressed);
	}
	else
	{
		PlayAnimReverse_OnPressed();
	}
	bIsClicked = false;
}

void UBSButton::SetActive()
{
	OnPressed_Button();
}

void UBSButton::SetDefaults(const uint8 InEnum, UBSButton* NextButton)
{
	Next = NextButton;
	EnumValue = InEnum;
	SetHasSetDefaults(true);
}

void UBSButton::SetButtonText(const FText& InText)
{
	ButtonText = InText;
	TextBlock->SetText(ButtonText);
}

void UBSButton::OnPressed_Button()
{
	bIsClicked = true;
	if (OnBSButtonPressed.IsBound())
	{
		OnBSButtonPressed.Broadcast(this);
	}
	if (OnBSButtonButtonClicked_NoParams.IsBound())
	{
		OnBSButtonButtonClicked_NoParams.Broadcast();
	}
	PlayAnim_OnPressed();
	UBSButton* Head = GetNext();
	while (Head != this)
	{
		if (!Head)
		{
			return;
		}
		Head->SetInActive();
		Head = Head->GetNext();
	}
}

void UBSButton::OnHovered_Button()
{
	/** Is animation reversing */
	if (IsAnimationPlaying(Anim_OnHovered) && !IsAnimationPlayingForward(Anim_OnHovered))
	{
		const float PlaybackPosition = GetAnimationCurrentTime(Anim_OnHovered);
		StopAnimation(Anim_OnHovered);
		PlayAnimation(Anim_OnHovered, PlaybackPosition, 1, EUMGSequencePlayMode::Forward, PlaybackSpeed, false);
	}
	else
	{
		PlayAnim_OnHovered();
	}
}

void UBSButton::OnUnhovered_Button()
{
	if (IsAnimationPlaying(Anim_OnHovered))
	{
		ReverseAnimation(Anim_OnHovered);
	}
	else
	{
		PlayAnimReverse_OnHovered();
	}
}

void UBSButton::PlayAnim_OnPressed()
{
	PlayAnimationForward(Anim_OnPressed, PlaybackSpeed, false);
}

void UBSButton::PlayAnim_OnHovered()
{
	PlayAnimationForward(Anim_OnHovered, PlaybackSpeed, false);
}

void UBSButton::PlayAnimReverse_OnPressed()
{
	PlayAnimationReverse(Anim_OnPressed, PlaybackSpeed, false);
}

void UBSButton::PlayAnimReverse_OnHovered()
{
	PlayAnimationReverse(Anim_OnHovered, PlaybackSpeed, false);
}

void UBSButton::OnAnimFinished_OnPressed()
{
	DynamicImageMaterial->SetScalarParameterValue(PressedScalarParameterName, 0.f);
	OnPressedAnimFinished.Broadcast();
}
