// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include <steam/isteamfriends.h>
#include <steam/isteamuser.h>
#include <steam/isteamuserstats.h>
#include <steam/steam_api_common.h>
#include "SteamManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSteamManager, Log, All);

DECLARE_DELEGATE(FOnAuthTicketForWebApiReady);

enum class EBaseGameMode : uint8;
class UBSGameInstance;

UENUM()
enum ESteamStatType
{
	Stat_Int     = 0,
	Stat_Float   = 1,
	Stat_AvgRate = 2,
};

USTRUCT()
struct FSteamStat
{
	GENERATED_BODY()

	const char* APIName;
	ESteamStatType StatType;
	int IntValue;
	float FloatValue;
	float FloatAvgNumerator;
	float FloatAvgDenominator;
	TSet<EBaseGameMode> BaseGameModes;

	FSteamStat(): APIName(nullptr), StatType(Stat_Int), IntValue(0), FloatValue(0), FloatAvgNumerator(0),
	              FloatAvgDenominator(0)
	{
	}

	FSteamStat(const char* InAPIName, const ESteamStatType InStatType, const TSet<EBaseGameMode>& InBaseGameModes)
	{
		StatType = InStatType;
		APIName = InAPIName;
		BaseGameModes = InBaseGameModes;
		IntValue = 0;
		FloatValue = 0;
		FloatAvgNumerator = 0;
		FloatAvgDenominator = 0;
	}

	FORCEINLINE bool operator==(const FSteamStat& Other) const
	{
		return APIName == Other.APIName;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FSteamStat& Stat)
	{
		return GetTypeHash(Stat.APIName);
	}
};

USTRUCT()
struct FSteamAchievement
{
	GENERATED_BODY()

	const char* APIName;
	FSteamStat* ProgressStat;
	bool bAchieved;

	FSteamAchievement(): APIName(nullptr), ProgressStat(nullptr), bAchieved(false)
	{
	}

	FSteamAchievement(const char* InAPIName, FSteamStat* InProgressStat, const bool bInAchieved)
	{
		APIName = InAPIName;
		ProgressStat = InProgressStat;
		bAchieved = bInAchieved;
	}

	FORCEINLINE bool operator==(const FSteamAchievement& Other) const
	{
		return APIName == Other.APIName;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FSteamAchievement& Achievement)
	{
		return GetTypeHash(Achievement.APIName);
	}
};

USTRUCT()
struct FOnAuthTicketForWebApiResponseCallbackHandler
{
	GENERATED_BODY()

	FOnAuthTicketForWebApiReady OnAuthTicketForWebApiReady;
	HAuthTicket Handle;
	EResult Result;
	FString Ticket;

	FOnAuthTicketForWebApiResponseCallbackHandler(): Handle(0), Result(k_EResultFail)
	{
	}
};

/** Class responsible for linking to Steam. */
UCLASS()
class BEATSHOT_API USteamManager : public UObject
{
	GENERATED_BODY()

public:
	USteamManager();

	void InitializeSteamManager();

	void AssignGameInstance(UBSGameInstance* InDefaultGameInstance);

	/** Updates locally stored StatData with new IntValue/FloatValue for matching StatAPIName, calls StoreStats(). */
	template <typename T>
	void UpdateStat(const char* StatAPIName, const T Value);

	/** Updates locally stored NumGamesPlayed StatData with new IntValue/FloatValue for matching GameMode, calls
	 *  StoreStats(). */
	void UpdateStat_NumGamesPlayed(const EBaseGameMode GameMode, int IntValue);

	UPROPERTY()
	UBSGameInstance* DefaultGameInstance;

	/** Calls GetAuthTicketForWebApi using Steam Api, callback is OnTicketForWebApiResponse. */
	bool CreateAuthTicketForWebApi(
		TSharedPtr<FOnAuthTicketForWebApiResponseCallbackHandler, ESPMode::ThreadSafe> CallbackHandler);

private:
	/** Delegate registered with Steam to trigger when a user activates the Steam Overlay. */
	STEAM_CALLBACK_MANUAL(USteamManager, OnSteamOverlayActive, GameOverlayActivated_t, OnSteamOverlayActiveDelegate);
	/** Delegate registered with Steam to trigger when a response is received from GetAuthTicketForWebApi. */
	STEAM_CALLBACK_MANUAL(USteamManager, OnAuthTicketForWebApiResponse, GetTicketForWebApiResponse_t,
		OnAuthTicketForWebApiResponseDelegate);
	/** Delegate registered with Steam to trigger anytime RequestStats() is called. */
	STEAM_CALLBACK_MANUAL(USteamManager, OnUserStatsReceived, UserStatsReceived_t, OnUserStatsReceivedDelegate);
	/** Delegate registered with Steam to trigger anytime you attempt to store stats on Steam. */
	STEAM_CALLBACK_MANUAL(USteamManager, OnUserStatsStored, UserStatsStored_t, OnUserStatsStoredDelegate);
	/** Delegate registered with Steam to trigger anytime you attempt to store achievements on Steam. */
	STEAM_CALLBACK_MANUAL(USteamManager, OnAchievementStored, UserAchievementStored_t, OnAchievementStoredDelegate);

	/** Wraps an asynchronous call to steam, ISteamUserStats::RequestCurrentStats, requesting the stats
	 *  and achievements of the current user. Needs to be called before setting any stats or achievements. */
	static bool RequestStats();

	/** Wraps an asynchronous call to steam, ISteamUserStats::StoreStats, that stores the stats of the current
	 *  user on the server. Needs to be called to update the stats of the user. */
	bool StoreStats();

	/** Sets a given achievement to achieved and sends the results to Steam. You can set a given achievement multiple
	 *  times, so you don't need to worry about only setting achievements that aren't already set. This is an
	 *  asynchronous call which will trigger two callbacks: OnUserStatsStored() and OnAchievementStored(). */
	bool SetAchievement(const char* ID) const;

	/** Returns a pointer to the NumGamesPlayed FSteamStat element corresponding to GameMode. */
	const char* GetStat_NumGamesPlayed(const EBaseGameMode GameMode);

	/** The Steam AppID for this game. */
	int64 AppId;

	/** Locally stored and updated Steam Stats struct array. */
	TMap<const char*, FSteamStat> StatsData;

	/** Locally stored and updated Steam Achievement struct array. */
	TSet<FSteamAchievement> AchievementData;

	/** If Steam Stats were successfully initialized. */
	bool bInitializedStats;

	/** A queue of AuthTicket callbacks that are executed in order. */
	TQueue<TSharedPtr<FOnAuthTicketForWebApiResponseCallbackHandler>> ActiveCallbacks;
};
