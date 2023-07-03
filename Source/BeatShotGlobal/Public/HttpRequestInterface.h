﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "GlobalStructs.h"
#include "HttpRequestInterface.generated.h"

/** Broadcast if refresh token is invalid */
DECLARE_DELEGATE_OneParam(FOnAccessTokenResponse, const FString& AccessToken);

/** Broadcast when a login response is received from BeatShot website */
DECLARE_DELEGATE_ThreeParams(FOnLoginResponse, const FPlayerSettings_User& PlayerSettings, const FString& ResponseMsg, const int32 ResponseCode);

/** Broadcast when a response is received from posting player scores to database */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPostScoresResponse, const ELoginState& LoginState);

/** Broadcast when a response is received from posting player scores to database */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPostFeedbackResponse, const bool bSuccess);

/** Used to convert PlayerScoreArray to database scores */
USTRUCT(BlueprintType)
struct FJsonScore
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPlayerScore> Scores;
};

/** Used to create a feedback Json object */
USTRUCT(BlueprintType)
struct FJsonFeedback
{
	GENERATED_BODY()

	UPROPERTY()
	FString Title;

	UPROPERTY()
	FString Content;

	FJsonFeedback()
	{
		Title = FString();
		Content = FString();
	}

	FJsonFeedback(const FString& InTitle, const FString& InContent)
	{
		Title = InTitle;
		Content = InContent;
	}
	
};

/** Interface to allow all other classes in this game to use HTTP request functions */
UINTERFACE()
class UHttpRequestInterface : public UInterface
{
	GENERATED_BODY()
};

/** Interface to allow all other classes in this game to use HTTP request functions */
class BEATSHOTGLOBAL_API IHttpRequestInterface
{
	GENERATED_BODY()

public:
	/** Sends an http post login request to BeatShot website given a LoginPayload. Executes supplied OnLoginResponse
	 *  with a login cookie */
	void LoginUser(const FLoginPayload& LoginPayload, FOnLoginResponse& OnLoginResponse) const;

	/** Requests a short lived access token given a valid login cookie. Executes supplied OnAccessTokenResponse
	 *  with an access token */
	void RequestAccessToken(const FString& LoginCookie, FOnAccessTokenResponse& OnAccessTokenResponse) const;

	/** Checks to see if the user has a refresh token and if it has expired or not */
	static bool IsRefreshTokenValid(const FPlayerSettings_User& PlayerSettings);

	/* Converts ScoresToPost to a JSON string and sends an http post request to BeatShot website given a valid
	 * access token. Executes supplied OnPostResponse with the login state */
	void PostPlayerScores(const TArray<FPlayerScore>& ScoresToPost, const FString& Username, const FString& AccessToken, FOnPostScoresResponse& OnPostResponse) const;

	void PostFeedback(const FJsonFeedback& InFeedback, FOnPostFeedbackResponse& OnPostFeedbackResponse) const;

private:
	FString LoginEndpoint = "https://beatshot.gg/api/login";
	const FString RefreshEndpoint = "https://beatshot.gg/api/refresh";
	const FString SaveScoresEndpoint = "https://beatshot.gg/api/profile/";
	const FString SendFeedbackEndpoint = "https://beatshot.gg/api/sendfeedback";
};
