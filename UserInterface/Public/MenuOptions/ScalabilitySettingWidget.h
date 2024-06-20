// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MenuOptionWidget.h"
#include "ScalabilitySettingWidget.generated.h"

class UBSButton;
enum class EVideoSettingType : uint8;
class UScalabilitySettingButton;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnVideoSettingQualityButtonPressed, const EVideoSettingType, const uint8)

UCLASS()
class USERINTERFACE_API UScalabilitySettingWidget : public UMenuOptionWidget
{
	GENERATED_BODY()

public:
	void Init(const EVideoSettingType InVideoSettingType);
	void SetActiveButton(const int32 InQuality);
	FOnVideoSettingQualityButtonPressed OnVideoSettingQualityButtonPressed;

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void SetStyling() override;

	/** Changes video settings quality depending on input button. */
	void OnBSButtonPressed_VideoQuality(const UBSButton* Button);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingButton* Button_0;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingButton* Button_1;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingButton* Button_2;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScalabilitySettingButton* Button_3;

	FText Button_0_Text;
	FText Button_1_Text;
	FText Button_2_Text;
	FText Button_3_Text;
};
