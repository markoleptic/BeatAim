// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Overlays/CountdownWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/TimelineComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UCountdownWidget::NativeConstruct()
{
	Super::NativeConstruct();
	OnCountdownTick.BindDynamic(this, &UCountdownWidget::CountdownTick);
	CountdownTimeline.AddInterpFloat(CountdownCurve, OnCountdownTick);
	OnCountdownComplete.BindDynamic(this, &UCountdownWidget::CountdownComplete);
	CountdownTimeline.SetTimelineFinishedFunc(OnCountdownComplete);
	CountdownTimeline.SetPlayRate(1.f);
}

void UCountdownWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	CountdownTimeline.TickTimeline(InDeltaTime);
}

void UCountdownWidget::StartCountdown(const float CountdownLength, const float PlayerDelay)
{
	TextBlock_Click->SetVisibility(ESlateVisibility::Collapsed);
	CountdownContainer->SetVisibility(ESlateVisibility::Visible);
	CountdownTimeline.SetTimelineLength(CountdownLength);
	GetWorld()->GetTimerManager().SetTimer(StartAAManagerPlaybackTimer, this,
		&UCountdownWidget::StartAAManagerPlaybackTimerComplete, CountdownLength - PlayerDelay, false);
	CountdownTimeline.PlayFromStart();
}

void UCountdownWidget::StartAAManagerPlaybackTimerComplete() const
{
	if (StartAAManagerPlayback.IsBound())
	{
		StartAAManagerPlayback.Execute();
	}
}

void UCountdownWidget::CountdownTick(const float Value)
{
	TextBlock_Counter->SetText(FText::AsNumber(ceil(CountdownTimeline.GetTimelineLength() - Value)));
	Image_Countdown->GetDynamicMaterial()->SetScalarParameterValue(FName("Progress"),
		UKismetMathLibrary::Fraction(CountdownTimeline.GetTimelineLength() - Value));
}

void UCountdownWidget::CountdownComplete()
{
	if (OnCountdownCompleted.IsBound())
	{
		OnCountdownCompleted.Execute();
	}
}

void UCountdownWidget::SetCountdownPaused(const bool bIsPaused)
{
	if (bIsPaused && GetWorld()->GetTimerManager().IsTimerActive(StartAAManagerPlaybackTimer) && CountdownTimeline.
		IsPlaying())
	{
		CountdownTimeline.Stop();
		GetWorld()->GetTimerManager().PauseTimer(StartAAManagerPlaybackTimer);
	}
	else if (!bIsPaused && GetWorld()->GetTimerManager().IsTimerPaused(StartAAManagerPlaybackTimer) && !
		CountdownTimeline.IsPlaying())
	{
		CountdownTimeline.Play();
		GetWorld()->GetTimerManager().UnPauseTimer(StartAAManagerPlaybackTimer);
	}
}
