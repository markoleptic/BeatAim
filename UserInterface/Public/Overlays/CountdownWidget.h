// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TimelineComponent.h"
#include "CountdownWidget.generated.h"

class USizeBox;
DECLARE_DELEGATE(FOnCountdownCompleted);
DECLARE_DELEGATE(FStartAAManagerPlayback);

class UImage;
class UTextBlock;

/** Widget used to display a countdown before the game mode begins. */
UCLASS()
class USERINTERFACE_API UCountdownWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	/** Called from Blueprint when user clicks on screen. */
	void StartCountdown(const float CountdownLength, const float PlayerDelay);

	/** Called when StartAAManagerPlayback has finished. */
	UFUNCTION()
	void StartAAManagerPlaybackTimerComplete() const;

	/** Bound to CountdownTimeline, sets material parameters. */
	UFUNCTION()
	void CountdownTick(const float Value);

	/** Bound to CountdownTimeline completion. */
	UFUNCTION()
	void CountdownComplete();

	/** Pauses/Unpauses the countdown. */
	void SetCountdownPaused(const bool bIsPaused);

	/** Executes when the countdown timeline has completed. */
	FOnCountdownCompleted OnCountdownCompleted;

	/** Executes when then the StartAAManagerPlaybackTimer has completed. */
	FStartAAManagerPlayback StartAAManagerPlayback;

protected:
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* CountdownCurve;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_Countdown;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_Counter;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_Click;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	USizeBox* CountdownContainer;

private:
	UPROPERTY()
	FTimerHandle StartAAManagerPlaybackTimer;

	FTimeline CountdownTimeline;
	FOnTimelineFloat OnCountdownTick;
	FOnTimelineEvent OnCountdownComplete;
};
