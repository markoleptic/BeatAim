// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/ColorSelectWidget.h"
#include "Utilities/BSWidgetInterface.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Styles/MenuOptionStyle.h"

void UColorSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ColorAValue->OnTextCommitted.AddDynamic(this, &UColorSelectWidget::OnColorAValueChange);
	ColorRValue->OnTextCommitted.AddDynamic(this, &UColorSelectWidget::OnColorRValueChange);
	ColorGValue->OnTextCommitted.AddDynamic(this, &UColorSelectWidget::OnColorGValueChange);
	ColorBValue->OnTextCommitted.AddDynamic(this, &UColorSelectWidget::OnColorBValueChange);
	HexValue->OnTextCommitted.AddDynamic(this, &UColorSelectWidget::OnHexValueChange);

	ColorASlider->OnValueChanged.AddDynamic(this, &UColorSelectWidget::OnColorASliderChange);
	ColorRSlider->OnValueChanged.AddDynamic(this, &UColorSelectWidget::OnColorRSliderChange);
	ColorGSlider->OnValueChanged.AddDynamic(this, &UColorSelectWidget::OnColorGSliderChange);
	ColorBSlider->OnValueChanged.AddDynamic(this, &UColorSelectWidget::OnColorBSliderChange);
}

void UColorSelectWidget::SetStyling()
{
	Super::SetStyling();
	if (MenuOptionStyle)
	{
		const FMargin NewMargin(0.f,
			MenuOptionStyle->Padding_DescriptionText.Top + MenuOptionStyle->Padding_LeftBox.Top, 0.f,
			MenuOptionStyle->Padding_DescriptionText.Bottom + MenuOptionStyle->Padding_LeftBox.Bottom);
		if (ColorAValue)
		{
			ColorAValue->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		}
		if (ColorRValue)
		{
			ColorRValue->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		}
		if (ColorGValue)
		{
			ColorGValue->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		}
		if (ColorBValue)
		{
			ColorBValue->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		}
		if (HexValue)
		{
			ColorBValue->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		}
		if (ColorRText)
		{
			ColorRText->SetFont(MenuOptionStyle->Font_DescriptionText);
			UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(ColorRText->Slot);
			if (HorizontalBoxSlot)
			{
				HorizontalBoxSlot->SetPadding(NewMargin);
			}
		}
		if (ColorGText)
		{
			ColorGText->SetFont(MenuOptionStyle->Font_DescriptionText);
			UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(ColorGText->Slot);
			if (HorizontalBoxSlot)
			{
				HorizontalBoxSlot->SetPadding(NewMargin);
			}
		}
		if (ColorBText)
		{
			ColorBText->SetFont(MenuOptionStyle->Font_DescriptionText);
			UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(ColorBText->Slot);
			if (HorizontalBoxSlot)
			{
				HorizontalBoxSlot->SetPadding(NewMargin);
			}
		}
		if (ColorAText)
		{
			ColorAText->SetFont(MenuOptionStyle->Font_DescriptionText);
			UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(ColorAText->Slot);
			if (HorizontalBoxSlot)
			{
				HorizontalBoxSlot->SetPadding(NewMargin);
			}
		}
		if (HexText)
		{
			HexText->SetFont(MenuOptionStyle->Font_DescriptionText);
			UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(HexText->Slot);
			if (HorizontalBoxSlot)
			{
				HorizontalBoxSlot->SetPadding(NewMargin);
			}
		}
	}
}

void UColorSelectWidget::InitializeColor(const FLinearColor& NewColor)
{
	Color = NewColor;

	ColorAValue->SetText(FText::AsNumber(roundf(Color.A * 100)));
	ColorRValue->SetText(FText::AsNumber(roundf(Color.R * 255)));
	ColorGValue->SetText(FText::AsNumber(roundf(Color.G * 255)));
	ColorBValue->SetText(FText::AsNumber(roundf(Color.B * 255)));
	HexValue->SetText(FText::FromString(Color.ToFColor(false).ToHex()));

	ColorASlider->SetValue(roundf(Color.A * 100));
	ColorRSlider->SetValue(roundf(Color.R * 255));
	ColorGSlider->SetValue(roundf(Color.G * 255));
	ColorBSlider->SetValue(roundf(Color.B * 255));

	ColorPreview->SetColorAndOpacity(Color);
}

void UColorSelectWidget::OnColorAValueChange(const FText& NewValue, ETextCommit::Type CommitType)
{
	const float SnappedValue = IBSWidgetInterface::OnEditableTextBoxChanged(NewValue, ColorAValue, ColorASlider, 1, 0,
		100);
	Color.A = FMath::GridSnap(SnappedValue / 100, 0.01);
	ColorPreview->SetColorAndOpacity(Color);
	HexValue->SetText(FText::FromString(Color.ToFColor(false).ToHex()));
	if (!OnColorChanged.ExecuteIfBound(Color))
	{
		UE_LOG(LogTemp, Display, TEXT("OnColorChanged not bound."));
	}
}

void UColorSelectWidget::OnColorRValueChange(const FText& NewValue, ETextCommit::Type CommitType)
{
	const float SnappedValue = IBSWidgetInterface::OnEditableTextBoxChanged(NewValue, ColorRValue, ColorRSlider, 1, 0,
		255);
	Color.R = SnappedValue / 255;
	ColorPreview->SetColorAndOpacity(Color);
	HexValue->SetText(FText::FromString(Color.ToFColor(false).ToHex()));
	if (!OnColorChanged.ExecuteIfBound(Color))
	{
		UE_LOG(LogTemp, Display, TEXT("OnColorChanged not bound."));
	}
}

void UColorSelectWidget::OnColorGValueChange(const FText& NewValue, ETextCommit::Type CommitType)
{
	const float SnappedValue = IBSWidgetInterface::OnEditableTextBoxChanged(NewValue, ColorGValue, ColorGSlider, 1, 0,
		255);
	Color.G = SnappedValue / 255;
	ColorPreview->SetColorAndOpacity(Color);
	HexValue->SetText(FText::FromString(Color.ToFColor(false).ToHex()));
	if (!OnColorChanged.ExecuteIfBound(Color))
	{
		UE_LOG(LogTemp, Display, TEXT("OnColorChanged not bound."));
	}
}

void UColorSelectWidget::OnColorBValueChange(const FText& NewValue, ETextCommit::Type CommitType)
{
	const float SnappedValue = IBSWidgetInterface::OnEditableTextBoxChanged(NewValue, ColorBValue, ColorBSlider, 1, 0,
		255);
	Color.B = SnappedValue / 255;
	ColorPreview->SetColorAndOpacity(Color);
	HexValue->SetText(FText::FromString(Color.ToFColor(false).ToHex()));
	if (!OnColorChanged.ExecuteIfBound(Color))
	{
		UE_LOG(LogTemp, Display, TEXT("OnColorChanged not bound."));
	}
}

void UColorSelectWidget::OnHexValueChange(const FText& NewValue, ETextCommit::Type CommitType)
{
	const FString String = FColor::FromHex(NewValue.ToString()).ToString();
	Color.InitFromString(String);
	Color.R = Color.R / 255;
	Color.G = Color.G / 255;
	Color.B = Color.B / 255;
	Color.A = Color.A / 255;
	ColorAValue->SetText(FText::AsNumber(roundf(Color.A * 100)));
	ColorRValue->SetText(FText::AsNumber(Color.R * 255));
	ColorGValue->SetText(FText::AsNumber(Color.G * 255));
	ColorBValue->SetText(FText::AsNumber(Color.B * 255));
	ColorASlider->SetValue(roundf(Color.A * 100));
	ColorRSlider->SetValue(Color.R * 255);
	ColorGSlider->SetValue(Color.G * 255);
	ColorBSlider->SetValue(Color.B * 255);
	ColorPreview->SetColorAndOpacity(Color);
	if (!OnColorChanged.ExecuteIfBound(Color))
	{
		UE_LOG(LogTemp, Display, TEXT("OnColorChanged not bound."));
	}
}

void UColorSelectWidget::OnColorASliderChange(const float NewValue)
{
	const float SnappedValue = IBSWidgetInterface::OnSliderChanged(NewValue, ColorAValue, 1);
	Color.A = SnappedValue / 100;
	ColorPreview->SetColorAndOpacity(Color);
	HexValue->SetText(FText::FromString(Color.ToFColor(false).ToHex()));
	if (!OnColorChanged.ExecuteIfBound(Color))
	{
		UE_LOG(LogTemp, Display, TEXT("OnColorChanged not bound."));
	}
}

void UColorSelectWidget::OnColorRSliderChange(const float NewValue)
{
	const float SnappedValue = IBSWidgetInterface::OnSliderChanged(NewValue, ColorRValue, 1);
	Color.R = SnappedValue / 255;
	ColorPreview->SetColorAndOpacity(Color);
	HexValue->SetText(FText::FromString(Color.ToFColor(false).ToHex()));
	if (!OnColorChanged.ExecuteIfBound(Color))
	{
		UE_LOG(LogTemp, Display, TEXT("OnColorChanged not bound."));
	}
}

void UColorSelectWidget::OnColorGSliderChange(const float NewValue)
{
	const float SnappedValue = IBSWidgetInterface::OnSliderChanged(NewValue, ColorGValue, 1);
	Color.G = SnappedValue / 255;
	ColorPreview->SetColorAndOpacity(Color);
	HexValue->SetText(FText::FromString(Color.ToFColor(false).ToHex()));
	if (!OnColorChanged.ExecuteIfBound(Color))
	{
		UE_LOG(LogTemp, Display, TEXT("OnColorChanged not bound."));
	}
}

void UColorSelectWidget::OnColorBSliderChange(const float NewValue)
{
	const float SnappedValue = IBSWidgetInterface::OnSliderChanged(NewValue, ColorBValue, 1);
	Color.B = SnappedValue / 255;
	ColorPreview->SetColorAndOpacity(Color);
	HexValue->SetText(FText::FromString(Color.ToFColor(false).ToHex()));
	if (!OnColorChanged.ExecuteIfBound(Color))
	{
		UE_LOG(LogTemp, Display, TEXT("OnColorChanged not bound."));
	}
}
