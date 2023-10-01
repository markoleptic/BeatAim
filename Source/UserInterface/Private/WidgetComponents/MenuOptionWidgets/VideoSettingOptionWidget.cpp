﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "WidgetComponents/MenuOptionWidgets/VideoSettingOptionWidget.h"
#include "Styles/MenuOptionStyle.h"

void UVideoSettingOptionWidget::Init(const EVideoSettingType InVideoSettingType)
{
	Button_0->SetDefaults(InVideoSettingType, 0, Button_1);
	Button_1->SetDefaults(InVideoSettingType, 1, Button_2);
	Button_2->SetDefaults(InVideoSettingType, 2, Button_3);
	Button_3->SetDefaults(InVideoSettingType, 3, Button_0);
}

void UVideoSettingOptionWidget::SetActiveButton(const int32 InQuality)
{
	UVideoSettingButton* Head = Button_0;
	if (Head && InQuality >= 0 && InQuality <= 4)
	{
		while (Head->GetVideoSettingQuality() != InQuality)
		{
			Head = Head->GetNext();
		}
	}
	if (Head)
	{
		Head->SetActive();
	}
}

void UVideoSettingOptionWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	Button_0->SetButtonText(Button_0_Text);
	Button_1->SetButtonText(Button_1_Text);
	Button_2->SetButtonText(Button_2_Text);
	Button_3->SetButtonText(Button_3_Text);
}

void UVideoSettingOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_0->SetButtonText(Button_0_Text);
	Button_1->SetButtonText(Button_1_Text);
	Button_2->SetButtonText(Button_2_Text);
	Button_3->SetButtonText(Button_3_Text);
	
	Button_0->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnBSButtonPressed_VideoQuality);
	Button_1->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnBSButtonPressed_VideoQuality);
	Button_2->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnBSButtonPressed_VideoQuality);
	Button_3->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnBSButtonPressed_VideoQuality);
}

void UVideoSettingOptionWidget::SetStyling()
{
	Super::SetStyling();
	if (MenuOptionStyle)
	{
		Button_0->SetButtonFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		Button_1->SetButtonFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		Button_2->SetButtonFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		Button_3->SetButtonFont(MenuOptionStyle->Font_EditableTextBesideSlider);
	}
}

void UVideoSettingOptionWidget::OnBSButtonPressed_VideoQuality(const UBSButton* Button)
{
	const UVideoSettingButton* VideoSettingButton = Cast<UVideoSettingButton>(Button);
	if (!VideoSettingButton)
	{
		return;
	}
	OnVideoSettingQualityButtonPressed.Broadcast(VideoSettingButton->GetVideoSettingType(), VideoSettingButton->GetVideoSettingQuality());
}
