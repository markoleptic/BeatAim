// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "Overlays/QTableWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/UniformGridPanel.h"
#include "Kismet/KismetMathLibrary.h"

void UQTableWidget::NativeConstruct()
{
	Super::NativeConstruct();
	NumberFormattingOptions.MaximumFractionalDigits = 2;
	NumberFormattingOptions.MinimumFractionalDigits = 2;
	NumberFormattingOptions.MinimumIntegralDigits = 1;
	NumberFormattingOptions.MaximumIntegralDigits = 2;
}

void UQTableWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UQTableWidget::UpdatePanel(const TArray<float>& QTable)
{
	UpdateExtrema(QTable);
	for (int i = 0; i < QTable.Num(); i++)
	{
		Points[i].Text->SetText(FText::AsNumber(QTable[i], &NumberFormattingOptions));
		if (i == QTable.Num() / 2)
		{
			Points[i].Image->SetColorAndOpacity(InterpColor((MinValue + MaxValue) / 2));
			continue;
		}
		Points[i].Image->SetColorAndOpacity(InterpColor(QTable[i]));
	}
}

void UQTableWidget::UpdatePanel2(const TArray<float>& QTable)
{
	UpdateExtrema(QTable);
	for (int i = 0; i < QTable.Num(); i++)
	{
		Points2[i].Text->SetText(FText::AsNumber(QTable[i], &NumberFormattingOptions));
		Points2[i].Image->SetColorAndOpacity(InterpColor(QTable[i]));
	}
}

void UQTableWidget::InitQTable(const int32 Rows, const int32 Columns, const TArray<float>& QTable)
{
	for (int j = 0; j < Columns; j++)
	{
		for (int i = 0; i < Rows; i++)
		{
			UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
			UImage* Image = WidgetTree->ConstructWidget<UImage>(DefaultImage);
			UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(DefaultText);
			Text->SetColorAndOpacity(FSlateColor(FLinearColor(FColor::Black)));
			Overlay->AddChildToOverlay(Image);
			UOverlaySlot* TextSlot = Overlay->AddChildToOverlay(Text);
			TextSlot->SetVerticalAlignment(VAlign_Center);
			TextSlot->SetHorizontalAlignment(HAlign_Center);
			GridPanel->AddChildToUniformGrid(Overlay, i, j);
			Points.Add(FAgentPoint(Image, Text));
		}
	}
	UpdatePanel(QTable);
}

void UQTableWidget::InitQTable2(const int32 Rows, const int32 Columns, const TArray<float>& QTable)
{
	for (int j = 0; j < Columns; j++)
	{
		for (int i = 0; i < Rows; i++)
		{
			UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
			UImage* Image = WidgetTree->ConstructWidget<UImage>(DefaultImage);
			UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(DefaultText);
			Text->SetColorAndOpacity(FSlateColor(FLinearColor(FColor::Black)));
			Overlay->AddChildToOverlay(Image);
			Overlay->AddChildToOverlay(Text);
			GridPanel2->AddChildToUniformGrid(Overlay, i, j);
			Points2.Add(FAgentPoint(Image, Text));
		}
	}
	UpdatePanel2(QTable);
}

FLinearColor UQTableWidget::InterpColor(const float Value) const
{
	return UKismetMathLibrary::LinearColorLerp(FLinearColor(FColor::Red), FLinearColor(FColor::Green),
		UKismetMathLibrary::MapRangeClamped(Value, MinValue, MaxValue, 0, 1));
}

void UQTableWidget::UpdateExtrema(const TArray<float>& QTable)
{
	const int32 Middle = QTable.Num() / 2;
	for (int i = 0; i < QTable.Num(); i++)
	{
		if (i == Middle)
		{
			continue;
		}
		if (QTable[i] > MinValue)
		{
			MinValue = QTable[i];
		}
		if (QTable[i] < MaxValue)
		{
			MaxValue = QTable[i];
		}
	}
}
