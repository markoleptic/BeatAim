// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "System/SteamManager.h"
#include <steam/isteamutils.h>
#include "Containers/UnrealString.h"
#include "BSGameInstance.h"

DEFINE_LOG_CATEGORY(LogSteamManager);

// Stats array which will hold data about the stats and their state
TMap<const char*, FSteamStat> g_Stats = {
	// Number of games played MultiBeat
	{
		"NumGamesPlayed_GM1",
		FSteamStat("NumGamesPlayed_GM1", Stat_Int, {
			EBaseGameMode::MultiBeat, EBaseGameMode::MultiBeatPrecision, EBaseGameMode::MultiBeatSpeed
		})
	},
	// Number of games played SingleBeat
	{"NumGamesPlayed_GM2", FSteamStat("NumGamesPlayed_GM2", Stat_Int, {EBaseGameMode::SingleBeat})},
	// Number of games played BeatTrack
	{"NumGamesPlayed_GM3", FSteamStat("NumGamesPlayed_GM3", Stat_Int, {EBaseGameMode::BeatTrack})},
	// Number of games played BeatGrid
	{"NumGamesPlayed_GM4", FSteamStat("NumGamesPlayed_GM4", Stat_Int, {EBaseGameMode::BeatGrid})},
	// Number of games played ClusterBeat
	{"NumGamesPlayed_GM5", FSteamStat("NumGamesPlayed_GM5", Stat_Int, {EBaseGameMode::ClusterBeat})},
	// Number of games played ChargedBeatTrack
	{"NumGamesPlayed_GM6", FSteamStat("NumGamesPlayed_GM6", Stat_Int, {EBaseGameMode::ChargedBeatTrack})},
	// Number of games played Custom Game Mode
	{"NumGamesPlayed_Custom", FSteamStat("NumGamesPlayed_Custom", Stat_Int, {EBaseGameMode::None})},
};

// Achievement array which will hold data about the achievements and their state
TSet g_Achievements = {
	FSteamAchievement("PlayAnyGM", nullptr, false),
	FSteamAchievement("Participant_GM1", g_Stats.Find("NumGamesPlayed_GM1"), false),
	FSteamAchievement("Participant_GM2", g_Stats.Find("NumGamesPlayed_GM2"), false),
	FSteamAchievement("Participant_GM3", g_Stats.Find("NumGamesPlayed_GM3"), false),
	FSteamAchievement("Participant_GM4", g_Stats.Find("NumGamesPlayed_GM4"), false),
	FSteamAchievement("Participant_GM5", g_Stats.Find("NumGamesPlayed_GM5"), false),
	FSteamAchievement("Participant_GM6", g_Stats.Find("NumGamesPlayed_GM6"), false),
	FSteamAchievement("Enthusiast_GM1", g_Stats.Find("NumGamesPlayed_GM1"), false),
	FSteamAchievement("Enthusiast_GM2", g_Stats.Find("NumGamesPlayed_GM2"), false),
	FSteamAchievement("Enthusiast_GM3", g_Stats.Find("NumGamesPlayed_GM3"), false),
	FSteamAchievement("Enthusiast_GM4", g_Stats.Find("NumGamesPlayed_GM4"), false),
	FSteamAchievement("Enthusiast_GM5", g_Stats.Find("NumGamesPlayed_GM5"), false),
	FSteamAchievement("Enthusiast_GM6", g_Stats.Find("NumGamesPlayed_GM6"), false),
	FSteamAchievement("Enjoyer_GM1", g_Stats.Find("NumGamesPlayed_GM1"), false),
	FSteamAchievement("Enjoyer_GM2", g_Stats.Find("NumGamesPlayed_GM2"), false),
	FSteamAchievement("Enjoyer_GM3", g_Stats.Find("NumGamesPlayed_GM3"), false),
	FSteamAchievement("Enjoyer_GM4", g_Stats.Find("NumGamesPlayed_GM4"), false),
	FSteamAchievement("Enjoyer_GM5", g_Stats.Find("NumGamesPlayed_GM5"), false),
	FSteamAchievement("Enjoyer_GM6", g_Stats.Find("NumGamesPlayed_GM6"), false),
	FSteamAchievement("Participant_Custom", g_Stats.Find("NumGamesPlayed_Custom"), false),
	FSteamAchievement("Enthusiast_Custom", g_Stats.Find("NumGamesPlayed_Custom"), false),
	FSteamAchievement("Enjoyer_Custom", g_Stats.Find("NumGamesPlayed_Custom"), false),
};

USteamManager::USteamManager()
{
	AppId = 0;
	StatsData = TMap<const char*, FSteamStat>();
	AchievementData = TSet<FSteamAchievement>();
	bInitializedStats = false;
	DefaultGameInstance = nullptr;
}

void USteamManager::InitializeSteamManager()
{
	if (SteamUtils())
	{
		AppId = SteamUtils()->GetAppID();
	}
	OnSteamOverlayActiveDelegate.Register(this, &USteamManager::OnSteamOverlayActive);
	OnAuthTicketForWebApiResponseDelegate.Register(this, &USteamManager::OnAuthTicketForWebApiResponse);
	OnUserStatsReceivedDelegate.Register(this, &USteamManager::OnUserStatsReceived);
	OnUserStatsStoredDelegate.Register(this, &USteamManager::OnUserStatsStored);
	OnAchievementStoredDelegate.Register(this, &USteamManager::OnAchievementStored);
	StatsData = g_Stats;
	AchievementData = g_Achievements;
	bInitializedStats = RequestStats();

	// Online Subsystem Steam calls these
	// SteamAPI_Init()
	// SteamAPI_RunCallbacks();
}

void USteamManager::AssignGameInstance(UBSGameInstance* InDefaultGameInstance)
{
	DefaultGameInstance = InDefaultGameInstance;
}

bool USteamManager::CreateAuthTicketForWebApi(
	TSharedPtr<FOnAuthTicketForWebApiResponseCallbackHandler, ESPMode::ThreadSafe> CallbackHandler)
{
	if (!SteamUser()) return false;
	ActiveCallbacks.Enqueue(CallbackHandler);
	CallbackHandler->Handle = SteamUser()->GetAuthTicketForWebApi("kxuhYhcZQyDdFtpS");
	return true;
}

/** Callback for when a ticket for a WebApiResponse is generated */
void USteamManager::OnAuthTicketForWebApiResponse(GetTicketForWebApiResponse_t* pParam)
{
	TSharedPtr<FOnAuthTicketForWebApiResponseCallbackHandler> CallbackHandler;
	if (ActiveCallbacks.Dequeue(CallbackHandler))
	{
		BytesToHex(pParam->m_rgubTicket, pParam->m_cubTicket, CallbackHandler->Ticket);
		CallbackHandler->Result = pParam->m_eResult;
		CallbackHandler->OnAuthTicketForWebApiReady.ExecuteIfBound();
	}
	else
	{
		UE_LOG(LogSteamManager, Warning, TEXT("Failed to dequeue a CallbackHandler."));
	}
}

/** Callback for when steam overlay is toggled */
void USteamManager::OnSteamOverlayActive(GameOverlayActivated_t* pParam)
{
	const bool bIsCurrentOverlayActive = pParam->m_bActive != 0;
	if (bIsCurrentOverlayActive)
	{
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			DefaultGameInstance->OnSteamOverlayIsOn();
		});
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			DefaultGameInstance->OnSteamOverlayIsOff();
		});
	}
}

bool USteamManager::RequestStats()
{
	if (!SteamUserStats() || !SteamUser() || !SteamUser()->BLoggedOn())
	{
		return false;
	}
	return SteamUserStats()->RequestCurrentStats();
}

/** Callback for anytime RequestStats() is called. Updates the member variables AchievementData
 *  and StatsData to reflect the latest stats and achievement data returned from Steam. */
void USteamManager::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
	if (pCallback->m_nGameID != AppId || pCallback->m_eResult != k_EResultOK) return;

	for (FSteamAchievement& Achievement : AchievementData)
	{
		SteamUserStats()->GetAchievement(Achievement.APIName, &Achievement.bAchieved);
	}

	for (TPair<const char*, FSteamStat>& Stat : StatsData)
	{
		switch (Stat.Value.StatType)
		{
		case Stat_Int:
			SteamUserStats()->GetStat(Stat.Value.APIName, &Stat.Value.IntValue);
			break;
		case Stat_Float:
		case Stat_AvgRate:
			SteamUserStats()->GetStat(Stat.Value.APIName, &Stat.Value.FloatValue);
			break;
		default:
			break;
		}
	}

	UE_LOG(LogSteamManager, Warning, TEXT("Received stats from Steam"));
	bInitializedStats = true;
}

bool USteamManager::StoreStats()
{
	if (!bInitializedStats || !SteamUserStats()) return false;

	// load stats
	for (TPair<const char*, FSteamStat>& Stat : StatsData)
	{
		switch (Stat.Value.StatType)
		{
		case Stat_Int:
			SteamUserStats()->SetStat(Stat.Key, Stat.Value.IntValue);
			break;
		case Stat_Float:
			SteamUserStats()->SetStat(Stat.Key, Stat.Value.FloatValue);
			break;
		case Stat_AvgRate:
			SteamUserStats()->UpdateAvgRateStat(Stat.Key, Stat.Value.FloatAvgNumerator, Stat.Value.FloatAvgDenominator);
		// The averaged result is calculated for us
			SteamUserStats()->GetStat(Stat.Key, &Stat.Value.FloatValue);
			break;
		}
	}
	return SteamUserStats()->StoreStats();
}

/** Callback for anytime you attempt to store stats on Steam */
void USteamManager::OnUserStatsStored(UserStatsStored_t* pCallback)
{
	// Ignore callbacks for other games
	if (pCallback->m_nGameID != AppId) return;

	if (k_EResultOK == pCallback->m_eResult)
	{
		UE_LOG(LogSteamManager, Warning, TEXT("Stored stats for Steam"));
	}
	else if (pCallback->m_eResult == k_EResultInvalidParam)
	{
		// One or more stats broke a constraint. They've been reverted, so re-iterate the values now to keep in sync
		UE_LOG(LogSteamManager, Warning, TEXT("StoreStats - some failed to validate"));
		// Fake callback so that values reload
		UserStatsReceived_t FakeCallback;
		FakeCallback.m_eResult = k_EResultOK;
		FakeCallback.m_nGameID = AppId;
		OnUserStatsReceived(&FakeCallback);
	}
	else
	{
		UE_LOG(LogSteamManager, Warning, TEXT("StatsStored - failed, %d" ), pCallback->m_eResult);
	}
}

template <typename T>
void USteamManager::UpdateStat(const char* StatAPIName, T Value)
{
	if (!bInitializedStats) return;

	// Update local stats
	FSteamStat* FoundSteamStat = StatsData.Find(StatAPIName);
	if (!FoundSteamStat) return;

	int NewValue = 0;
	switch (FoundSteamStat->StatType)
	{
	case Stat_Int:
		{
			FoundSteamStat->IntValue += static_cast<int>(Value);
			NewValue = FoundSteamStat->IntValue;
		}
		break;
	case Stat_Float:
	case Stat_AvgRate:
		{
			FoundSteamStat->FloatValue += static_cast<float>(Value);
			NewValue = static_cast<int>(FoundSteamStat->FloatValue);
		}
		break;
	}

	// Store local stats
	if (!StoreStats()) return;

	FSteamAchievement* MinNotYetAchieved = nullptr;
	int32 MinMaxProgressLimit = INT_MAX;

	// Show achievement progress
	for (auto& Achievement : AchievementData)
	{
		if (!Achievement.ProgressStat || *Achievement.ProgressStat != *FoundSteamStat) continue;

		int32 MinProgressLimit = 0;
		int32 MaxProgressLimit = 0;
		SteamUserStats()->GetAchievementProgressLimits(Achievement.APIName, &MinProgressLimit, &MaxProgressLimit);

		// Do not show progress if already achieved
		if (NewValue >= MaxProgressLimit) continue;

		// Only show progress for one game mode achievement
		if (!FoundSteamStat->BaseGameModes.IsEmpty())
		{
			// Only show progress in increments of 5, starting at 10 and ending at 45, skipping 25
			if ((NewValue > 5 && NewValue < 25) || (NewValue > 25 && NewValue < 50))
			{
				if (NewValue % 5 == 0)
				{
					if (MaxProgressLimit < MinMaxProgressLimit)
					{
						MinMaxProgressLimit = MaxProgressLimit;
						MinNotYetAchieved = &Achievement;
					}
				}
			}
		}
	}

	if (MinNotYetAchieved)
	{
		SteamUserStats()->IndicateAchievementProgress(MinNotYetAchieved->APIName, FoundSteamStat->IntValue,
			MinMaxProgressLimit);
	}

	switch (FoundSteamStat->StatType)
	{
	case Stat_Int: UE_LOG(LogSteamManager, Warning, TEXT("Updated Stat %s Old Value: %d New Value: %d"),
			*FString(FoundSteamStat->APIName), FoundSteamStat->IntValue - Value, FoundSteamStat->IntValue);
		break;
	case Stat_Float:
	case Stat_AvgRate: UE_LOG(LogSteamManager, Warning, TEXT("Updated Stat %s Old Value: %.2f New Value: %.2f"),
			*FString(FoundSteamStat->APIName), FoundSteamStat->FloatValue - Value, FoundSteamStat->FloatValue);
		break;
	}
}

void USteamManager::UpdateStat_NumGamesPlayed(const EBaseGameMode GameMode, int IntValue)
{
	if (!bInitializedStats) return;
	const char* APIName = GetStat_NumGamesPlayed(GameMode);
	if (!APIName)
	{
		UE_LOG(LogSteamManager, Warning, TEXT("Failed to find FSteamStat for GameMode"));
		return;
	}
	UpdateStat(APIName, IntValue);
}

const char* USteamManager::GetStat_NumGamesPlayed(const EBaseGameMode GameMode)
{
	for (TPair<const char*, FSteamStat>& Stat : StatsData)
	{
		if (Stat.Value.BaseGameModes.Contains(GameMode))
		{
			return Stat.Key;
		}
	}
	return nullptr;
}

/** Callback for anytime Achievements are successfully stored on Steam */
void USteamManager::OnAchievementStored(UserAchievementStored_t* pCallback)
{
	if (AppId == pCallback->m_nGameID)
	{
		UE_LOG(LogSteamManager, Warning, TEXT("Stored Achievement for Steam"));
	}
}

bool USteamManager::SetAchievement(const char* ID) const
{
	if (!bInitializedStats || !SteamUserStats()) return false;
	SteamUserStats()->SetAchievement(ID);
	return SteamUserStats()->StoreStats();
}
