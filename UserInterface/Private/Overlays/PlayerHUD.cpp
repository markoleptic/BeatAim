// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Overlays/PlayerHUD.h"
#include "BSGameModeConfig/BSConfig.h"
#include "Components/HorizontalBox.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetStringLibrary.h"
#include "Overlays/HitTimingWidget.h"
#include "SaveGames/SaveGamePlayerScore.h"
#include "SaveGames/SaveGamePlayerSettings.h"

void UPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	/** Initial value update */
	ProgressBar_Accuracy->SetPercent(0.f);
	TextBlock_Accuracy->SetText(FText::AsPercent(0.f));
	TextBlock_SongTimeElapsed->SetText(FText::FromString(UKismetStringLibrary::TimeSecondsToString(0).LeftChop(3)));
}

void UPlayerHUD::Init(const TSharedPtr<FBSConfig>& InConfig)
{
	Config = InConfig;
	switch (InConfig->TargetConfig.TargetDamageType)
	{
	case ETargetDamageType::Tracking:
		{
			Box_Accuracy->SetVisibility(ESlateVisibility::Collapsed);
			Box_TrackingAccuracy->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			Box_TargetsSpawned->SetVisibility(ESlateVisibility::Collapsed);
			Box_Streak->SetVisibility(ESlateVisibility::Collapsed);
			Box_TargetsHit->SetVisibility(ESlateVisibility::Collapsed);
			Box_ShotsFired->SetVisibility(ESlateVisibility::Collapsed);
			HitTimingWidget->SetIsEnabled(false);
			HitTimingWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		break;
	case ETargetDamageType::Combined:
		{
			const int32 Min = -static_cast<int32>(InConfig->TargetConfig.SpawnBeatDelay * 100.f);
			const FString MinTick = FString::FromInt(Min) + "ms";
			const int32 Max = (InConfig->TargetConfig.TargetMaxLifeSpan - InConfig->TargetConfig.SpawnBeatDelay) *
				100.f;
			const FString MaxTick = FString::FromInt(Max) + "ms";
			HitTimingWidget->Init(FText::FromString(MinTick), FText::FromString(MaxTick));
		}
	case ETargetDamageType::Hit:
		{
			Box_TrackingAccuracy->SetVisibility(ESlateVisibility::Collapsed);
			const int32 Min = -static_cast<int32>(InConfig->TargetConfig.SpawnBeatDelay * 100.f);
			const FString MinTick = FString::FromInt(Min) + "ms";
			const int32 Max = (InConfig->TargetConfig.TargetMaxLifeSpan - InConfig->TargetConfig.SpawnBeatDelay) *
				100.f;
			const FString MaxTick = FString::FromInt(Max) + "ms";
			HitTimingWidget->Init(FText::FromString(MinTick), FText::FromString(MaxTick));
		}
		break;
	case ETargetDamageType::None:
	case ETargetDamageType::Self:
		break;
	}

	// Display default game mode names if not custom
	if (InConfig->DefiningConfig.GameModeType == EGameModeType::Preset)
	{
		TextBlock_GameModeName->SetText(UEnum::GetDisplayValueAsText(InConfig->DefiningConfig.BaseGameMode));
	}
	// Display custom game mode if not a default game mode
	else
	{
		TextBlock_GameModeName->SetText(FText::FromString(InConfig->DefiningConfig.CustomGameModeName));
	}

	TextBlock_SongTitle->SetText(FText::FromString(InConfig->AudioConfig.SongTitle));
	const FString Time = UKismetStringLibrary::TimeSecondsToString(InConfig->AudioConfig.SongLength).LeftChop(3);
	TextBlock_TotalSongLength->SetText(FText::FromString(Time));
}

void UPlayerHUD::OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings)
{
	if (GameSettings.bShowHitTimingWidget && (Config->TargetConfig.TargetDamageType == ETargetDamageType::Hit || Config
		->TargetConfig.TargetDamageType == ETargetDamageType::Combined))
	{
		if (!HitTimingWidget->GetIsEnabled())
		{
			HitTimingWidget->SetIsEnabled(true);
			HitTimingWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
	else if (!GameSettings.bShowHitTimingWidget && (Config->TargetConfig.TargetDamageType == ETargetDamageType::Hit ||
		Config->TargetConfig.TargetDamageType == ETargetDamageType::Combined))
	{
		if (HitTimingWidget->GetIsEnabled())
		{
			HitTimingWidget->SetIsEnabled(false);
			HitTimingWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UPlayerHUD::UpdateAllElements(const FPlayerScore& Scores, const float TimeOffsetNormalized, const float Error)
{
	const float Score = round(Scores.Score);
	const float HighScore = round(Scores.HighScore);

	TextBlock_CurrentScore->SetText(FText::AsNumber(Score));
	TextBlock_HighScore->SetText(FText::AsNumber(HighScore < Score ? Score : HighScore));

	switch (Config->TargetConfig.TargetDamageType)
	{
	case ETargetDamageType::Tracking:
		{
			if (!isnan(Score / Scores.TotalPossibleDamage))
			{
				ProgressBar_TrackingAccuracy->SetPercent(Score / Scores.TotalPossibleDamage);
				TextBlock_TrackingAccuracy->SetText(FText::AsPercent(Score / Scores.TotalPossibleDamage));
			}
		}
		break;
	case ETargetDamageType::Hit:
		{
			const float TargetsHit = Scores.TargetsHit;
			const float ShotsFired = Scores.ShotsFired;
			const float TargetsSpawned = Scores.TargetsSpawned;
			const float Percent = static_cast<float>(Scores.TargetsHit) / Scores.ShotsFired;
			if (!isnan(Percent))
			{
				ProgressBar_Accuracy->SetPercent(Percent);
				TextBlock_Accuracy->SetText(FText::AsPercent(Percent));
			}
			TextBlock_TargetsHit->SetText(FText::AsNumber(TargetsHit));
			TextBlock_ShotsFired->SetText(FText::AsNumber(ShotsFired));
			TextBlock_TargetsSpawned->SetText(FText::AsNumber(TargetsSpawned));
			TextBlock_CurrentScore->SetText(FText::AsNumber(Score));
			TextBlock_CurrentStreakBest->SetText(FText::AsNumber(Scores.Streak));
		}
		break;
	case ETargetDamageType::Combined:
		{
			if (!isnan(Score / Scores.TotalPossibleDamage))
			{
				ProgressBar_TrackingAccuracy->SetPercent(Score / Scores.TotalPossibleDamage);
				TextBlock_TrackingAccuracy->SetText(FText::AsPercent(Score / Scores.TotalPossibleDamage));
			}
			const float TargetsHit = Scores.TargetsHit;
			const float ShotsFired = Scores.ShotsFired;
			const float TargetsSpawned = Scores.TargetsSpawned;
			const float Percent = static_cast<float>(Scores.TargetsHit) / Scores.ShotsFired;
			if (!isnan(Percent))
			{
				ProgressBar_Accuracy->SetPercent(Percent);
				TextBlock_Accuracy->SetText(FText::AsPercent(Percent));
			}
			TextBlock_TargetsHit->SetText(FText::AsNumber(TargetsHit));
			TextBlock_ShotsFired->SetText(FText::AsNumber(ShotsFired));
			TextBlock_TargetsSpawned->SetText(FText::AsNumber(TargetsSpawned));
			TextBlock_CurrentScore->SetText(FText::AsNumber(Score));
			TextBlock_CurrentStreakBest->SetText(FText::AsNumber(Scores.Streak));
		}
		break;
	case ETargetDamageType::None:
	case ETargetDamageType::Self:
		break;
	}
	if (HitTimingWidget->GetIsEnabled() && TimeOffsetNormalized > 0.f)
	{
		HitTimingWidget->UpdateHitTiming(TimeOffsetNormalized, Error);
	}
}

void UPlayerHUD::UpdateSongProgress(const float PlaybackTime)
{
	const FString Time = UKismetStringLibrary::TimeSecondsToString(PlaybackTime).LeftChop(3);
	TextBlock_SongTimeElapsed->SetText(FText::FromString(Time));
}
