// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/GameModeCategoryTagWidget.h"
#include "Components/Image.h"


void UGameModeCategoryTagWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	SetText(CategoryText);
	SetColor(TagColor);
}

void UGameModeCategoryTagWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetText(CategoryText);
	SetColor(TagColor);
}

void UGameModeCategoryTagWidget::SetText(const FText& InText)
{
	CategoryText = InText;
	if (TextBlock_Category)
	{
		TextBlock_Category->SetText(CategoryText);
	}
}

void UGameModeCategoryTagWidget::SetColor(const FLinearColor& InColor)
{
	if (!Image_Material)
	{
		return;
	}
	UMaterialInstanceDynamic* Inst = Image_Material->GetDynamicMaterial();
	if (!Inst)
	{
		return;
	}
	TagColor = InColor;
	Inst->SetVectorParameterValue("Button color 1", TagColor);
}
