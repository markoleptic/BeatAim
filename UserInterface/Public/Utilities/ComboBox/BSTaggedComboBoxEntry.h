// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSComboBoxEntry.h"
#include "BSTaggedComboBoxEntry.generated.h"


class UGameModeCategoryTagWidget;
class UHorizontalBox;

UCLASS()
class USERINTERFACE_API UBSTaggedComboBoxEntry : public UBSComboBoxEntry
{
	GENERATED_BODY()

public:
	/** Adds widgets to Box_TagWidgets. */
	void AddGameModeCategoryTagWidget(TArray<UGameModeCategoryTagWidget*>& InParentTagWidgets,
		TArray<UGameModeCategoryTagWidget*>& InTagWidgets, const FMargin InMargin, const EVerticalAlignment InVAlign,
		const EHorizontalAlignment InHAlign);

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UHorizontalBox* Box_TagWidgets;
};
