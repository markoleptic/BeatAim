// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeWidget.h"
#include "PropertyViewWidget.generated.h"

UCLASS()
class USERINTERFACE_API UPropertyViewWidget : public UCustomGameModeWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
};
