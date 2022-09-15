// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameModeActorBase.h"
#include "DefaultGameMode.h"
#include "DefaultGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetTextLibrary.h"
#include "Kismet/KismetStringLibrary.h"

void UPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();
	GI = Cast<UDefaultGameInstance>(UGameplayStatics::GetGameInstance(this));
	GI->GameModeActorBaseRef->UpdateScoresToHUD.AddDynamic(this, &UPlayerHUD::UpdateAllElements);
	UpdateAllElements(GI->GameModeActorBaseRef->PlayerScores);
}

void UPlayerHUD::SetTargetBar(float TargetsHit, float ShotsFired)
{
	if (isnan(TargetsHit / ShotsFired))
	{
		TargetBar->SetPercent(0.f);
	}
	else
	{
		TargetBar->SetPercent(TargetsHit / ShotsFired);
	}
}

void UPlayerHUD::SetTargetsHit(float TargetsHit)
{
	TargetsHitText->SetText(FText::AsNumber(TargetsHit));
}

void UPlayerHUD::SetAccuracy(float TargetsHit, float ShotsFired)
{
	if (isnan(TargetsHit / ShotsFired))
	{
		Accuracy->SetText(FText::AsPercent(0.f));
	}
	else
	{
		Accuracy->SetText(FText::AsPercent(TargetsHit / ShotsFired));
	}
}

void UPlayerHUD::SetShotsFired(float ShotsFired)
{
	ShotsFiredText->SetText(FText::AsNumber(ShotsFired));
}

void UPlayerHUD::SetTargetsSpawned(float TargetsSpawned)
{
	//TargetSubsystem->SetTotalTargetsSpawned(TargetsSpawned);
	TargetsSpawnedText->SetText(FText::AsNumber(TargetsSpawned));
}

void UPlayerHUD::SetCurrentScore(float Score)
{
	CurrentScoreText->SetText(FText::AsNumber(Score));
}

void UPlayerHUD::SetHighScore(float HighScore)
{
	HighScoreText->SetText(FText::AsNumber(HighScore));
}

void UPlayerHUD::UpdateAllElements(FPlayerScore NewPlayerScoreStruct)
{
	// display default game mode names if not custom
	if (NewPlayerScoreStruct.CustomGameModeName.Equals(""))
	{
		GameModeNameText->SetText(UEnum::GetDisplayValueAsText(NewPlayerScoreStruct.GameModeActorName));
	}
	else
	{
		GameModeNameText->SetText(UKismetTextLibrary::Conv_StringToText(NewPlayerScoreStruct.CustomGameModeName));
	}
	// show song title and total song length
	SongTitle->SetText(UKismetTextLibrary::Conv_StringToText(NewPlayerScoreStruct.SongTitle));
	TotalSongLength->SetText(UKismetTextLibrary::Conv_StringToText(UKismetStringLibrary::LeftChop(UKismetStringLibrary::TimeSecondsToString(NewPlayerScoreStruct.SongLength),3)));

	if (NewPlayerScoreStruct.IsBeatTrackMode == true)
	{
		float Score = round(NewPlayerScoreStruct.Score);
		float TotalPossibleDamage = NewPlayerScoreStruct.TotalPossibleDamage;
		float HighScore = round(NewPlayerScoreStruct.HighScore);


		if (isnan(Score / TotalPossibleDamage))
		{
			TargetBar->SetPercent(0.f);
			Accuracy->SetText(FText::AsPercent(0.f));
		}
		else
		{
			TargetBar->SetPercent(Score / TotalPossibleDamage);
			Accuracy->SetText(FText::AsPercent(Score / TotalPossibleDamage));
		}

		HighScoreText->SetText(FText::AsNumber(Score));

		if (HighScore < Score)
		{
			HighScoreText->SetText(FText::AsNumber(Score));
		}
		else
		{
			HighScoreText->SetText(FText::AsNumber(HighScore));
		}
	}
	else
	{
		float TargetsHit = NewPlayerScoreStruct.TargetsHit;
		float Score = round(NewPlayerScoreStruct.Score);
		float ShotsFired = NewPlayerScoreStruct.ShotsFired;
		float TargetsSpawned = NewPlayerScoreStruct.TargetsSpawned;
		float HighScore = round(NewPlayerScoreStruct.HighScore);

		if (isnan(TargetsHit / ShotsFired))
		{
			TargetBar->SetPercent(0.f);
		}
		else
		{
			TargetBar->SetPercent(TargetsHit / ShotsFired);
		}

		TargetsHitText->SetText(FText::AsNumber(TargetsHit));

		if (isnan(TargetsHit / ShotsFired))
		{
			Accuracy->SetText(FText::AsPercent(0.f));
		}
		else
		{
			Accuracy->SetText(FText::AsPercent(TargetsHit / ShotsFired));
		}

		ShotsFiredText->SetText(FText::AsNumber(ShotsFired));

		TargetsSpawnedText->SetText(FText::AsNumber(TargetsSpawned));

		CurrentScoreText->SetText(FText::AsNumber(Score));

		if (HighScore < Score)
		{
			HighScoreText->SetText(FText::AsNumber(Score));
		}
		else
		{
			HighScoreText->SetText(FText::AsNumber(HighScore));
		}
	}

}

