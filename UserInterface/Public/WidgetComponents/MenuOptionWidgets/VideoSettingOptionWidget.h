// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MenuOptionWidget.h"
#include "WidgetComponents/Buttons/VideoSettingButton.h"
#include "VideoSettingOptionWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnVideoSettingQualityButtonPressed, const EVideoSettingType, const uint8)

UCLASS()
class USERINTERFACE_API UVideoSettingOptionWidget : public UMenuOptionWidget
{
	GENERATED_BODY()

public:
	void Init(const EVideoSettingType InVideoSettingType);
	void SetActiveButton(const int32 InQuality);
	FOnVideoSettingQualityButtonPressed OnVideoSettingQualityButtonPressed;

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void SetStyling() override;

	/** Changes video settings quality depending on input button */
	void OnBSButtonPressed_VideoQuality(const UBSButton* Button);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingButton* Button_0;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingButton* Button_1;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingButton* Button_2;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoSettingButton* Button_3;

	UPROPERTY(EditDefaultsOnly, Category="VideoSettingOptionWidget")
	FText Button_0_Text = FText::FromString("Low");
	UPROPERTY(EditDefaultsOnly, Category="VideoSettingOptionWidget")
	FText Button_1_Text = FText::FromString("Medium");
	UPROPERTY(EditDefaultsOnly, Category="VideoSettingOptionWidget")
	FText Button_2_Text = FText::FromString("High");
	UPROPERTY(EditDefaultsOnly, Category="VideoSettingOptionWidget")
	FText Button_3_Text = FText::FromString("Ultra");
};
