// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Overlays/CrossHairWidget.h"
#include "SaveGamePlayerSettings.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"

void UCrossHairWidget::InitializeCrossHair(const FPlayerSettings_CrossHair& CrossHairSettings)
{
	SetLineSize(FVector2d(CrossHairSettings.LineWidth, CrossHairSettings.LineLength));
	SetInnerOffset(CrossHairSettings.InnerOffset);
	SetOutlineSize(CrossHairSettings.OutlineSize, FVector2d(CrossHairSettings.LineWidth, CrossHairSettings.LineLength));

	SetLineColor(CrossHairSettings.CrossHairColor);
	SetOutlineColor(CrossHairSettings.OutlineColor);
	SetCrossHairDotColor(CrossHairSettings.CrossHairDotColor);
	SetCrossHairDotOutlineColor(CrossHairSettings.OutlineColor);

	SetCrossHairDotSize(CrossHairSettings.CrossHairDotSize, CrossHairSettings.OutlineSize);
	SetShowCrossHairDot(CrossHairSettings.bShowCrossHairDot);
}

void UCrossHairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Slot_Left = Cast<UCanvasPanelSlot>(Overlay_Left->Slot);
	Slot_Right = Cast<UCanvasPanelSlot>(Overlay_Right->Slot);
	Slot_Top = Cast<UCanvasPanelSlot>(Overlay_Top->Slot);
	Slot_Bottom = Cast<UCanvasPanelSlot>(Overlay_Bottom->Slot);
	Slot_Dot = Cast<UCanvasPanelSlot>(Overlay_Dot->Slot);

	InitializeCrossHair(LoadPlayerSettings().CrossHair);
}

void UCrossHairWidget::SetLineSize(const FVector2d VerticalLineSize)
{
	Image_Top->SetDesiredSizeOverride(VerticalLineSize);
	Image_Bottom->SetDesiredSizeOverride(VerticalLineSize);
	Image_Left->SetDesiredSizeOverride(FVector2d(VerticalLineSize.Y, VerticalLineSize.X));
	Image_Right->SetDesiredSizeOverride(FVector2d(VerticalLineSize.Y, VerticalLineSize.X));
}

void UCrossHairWidget::SetOutlineSize(const int32 NewSize, const FVector2d VerticalLineSize)
{
	Image_TopOutline->SetDesiredSizeOverride(VerticalLineSize + NewSize);
	Image_BottomOutline->SetDesiredSizeOverride(VerticalLineSize + NewSize);
	Image_LeftOutline->SetDesiredSizeOverride(FVector2d(VerticalLineSize.Y, VerticalLineSize.X) + NewSize);
	Image_RightOutline->SetDesiredSizeOverride(FVector2d(VerticalLineSize.Y, VerticalLineSize.X) + NewSize);
}

void UCrossHairWidget::SetOutlineColor(const FLinearColor NewColor)
{
	Image_TopOutline->SetColorAndOpacity(NewColor);
	Image_BottomOutline->SetColorAndOpacity(NewColor);
	Image_LeftOutline->SetColorAndOpacity(NewColor);
	Image_RightOutline->SetColorAndOpacity(NewColor);
}

void UCrossHairWidget::SetLineColor(const FLinearColor NewColor)
{
	Image_Top->SetColorAndOpacity(NewColor);
	Image_Bottom->SetColorAndOpacity(NewColor);
	Image_Left->SetColorAndOpacity(NewColor);
	Image_Right->SetColorAndOpacity(NewColor);
}

void UCrossHairWidget::SetInnerOffset(const int32 NewOffsetValue)
{
	if (!Slot_Left || !Slot_Right || !Slot_Bottom || !Slot_Top)
	{
		return;
	}

	Slot_Top->SetPosition(FVector2d(0, -NewOffsetValue));
	Slot_Bottom->SetPosition(FVector2d(0, NewOffsetValue));
	Slot_Left->SetPosition(FVector2d(-NewOffsetValue, 0));
	Slot_Right->SetPosition(FVector2d(NewOffsetValue, 0));
}

void UCrossHairWidget::SetShowCrossHairDot(const bool bShow)
{
	Image_Dot->SetVisibility(bShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UCrossHairWidget::SetCrossHairDotColor(const FLinearColor NewColor)
{
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(Image_Dot->GetDynamicMaterial());
	if (MaterialInstance)
	{
		MaterialInstance->SetVectorParameterValue("Color", NewColor);
	}
}

void UCrossHairWidget::SetCrossHairDotSize(const int32 NewSize, const int32 OutlineSize)
{
	Image_Dot->SetDesiredSizeOverride(FVector2d(NewSize + OutlineSize));
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(Image_Dot->GetDynamicMaterial());
	if (MaterialInstance)
	{
		MaterialInstance->SetScalarParameterValue("FillOutlineRatio",
			static_cast<float>(NewSize) / static_cast<float>(NewSize + OutlineSize));
	}
}

void UCrossHairWidget::SetCrossHairDotOutlineColor(const FLinearColor NewColor)
{
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(Image_Dot->GetDynamicMaterial());
	if (MaterialInstance)
	{
		MaterialInstance->SetVectorParameterValue("OutlineColor", NewColor);
	}
}

void UCrossHairWidget::OnPlayerSettingsChanged(const FPlayerSettings_CrossHair& CrossHairSettings)
{
	InitializeCrossHair(CrossHairSettings);
}
