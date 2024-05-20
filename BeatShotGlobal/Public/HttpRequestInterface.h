// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "SaveGames/SaveGamePlayerScore.h"
#include "HttpRequestInterface.generated.h"

/** Executed when a response is received from a request. */
DECLARE_DELEGATE(FOnHttpResponseReceived);

/** Base struct for Http requests in this class. */
USTRUCT(BlueprintType)
struct FBSHttpResponse
{
	GENERATED_BODY()

	int32 HttpStatus;
	bool bConnectedSuccessfully;
	bool OK;
	FOnHttpResponseReceived OnHttpResponseReceived;

	FBSHttpResponse(): HttpStatus(0), bConnectedSuccessfully(false), OK(false)
	{
	}
};

USTRUCT(BlueprintType)
struct FAccessTokenResponse : public FBSHttpResponse
{
	GENERATED_BODY()

	FString AccessToken;

	FAccessTokenResponse()
	{
	}
};

USTRUCT(BlueprintType)
struct FDeleteScoresResponse : public FBSHttpResponse
{
	GENERATED_BODY()

	int32 NumRemoved;

	FDeleteScoresResponse(): NumRemoved(0)
	{
	}
};

/** Simple login payload. */
USTRUCT(BlueprintType)
struct FLoginPayload
{
	GENERATED_BODY()

	UPROPERTY()
	FString Username;

	UPROPERTY()
	FString Email;

	UPROPERTY()
	FString Password;

	FLoginPayload()
	{
		Username = "";
		Email = "";
		Password = "";
	}

	FLoginPayload(const FString& InUsername, const FString& InEmail, const FString& InPassword)
	{
		Username = InUsername;
		Email = InEmail;
		Password = InPassword;
	}
};

/** Login Response object. */
USTRUCT(BlueprintType)
struct FLoginResponse : public FBSHttpResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserID;

	UPROPERTY()
	FString AccessToken;

	UPROPERTY()
	FString RefreshToken;

	UPROPERTY()
	FString DisplayName;

	UPROPERTY()
	FString ResponseMsg;

	FLoginResponse()
	{
	}
};

/** Response object returned as JSON from authentication using SteamAuthTicket. */
USTRUCT(BlueprintType)
struct FSteamAuthTicketResponse : public FBSHttpResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString Result;

	UPROPERTY()
	FString SteamID;

	UPROPERTY()
	FString OwnerSteamID;

	UPROPERTY()
	bool VacBanned;

	UPROPERTY()
	bool PublisherBanned;

	UPROPERTY()
	FString DisplayName;

	UPROPERTY()
	FString RefreshCookie;

	UPROPERTY()
	FString ErrorCode;

	UPROPERTY()
	FString ErrorDesc;

	FSteamAuthTicketResponse(): VacBanned(false), PublisherBanned(false)
	{
	}
};

/** Used to convert PlayerScoreArray to database scores. */
USTRUCT(BlueprintType)
struct FJsonScore
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPlayerScore> Scores;
};

/** Used to create a feedback Json object. */
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

/** Used to pass the json body containing the CustomGameModeName to delete. */
USTRUCT(BlueprintType)
struct FJsonDeleteScores
{
	GENERATED_BODY()

	UPROPERTY()
	FString CustomGameModeName;

	FJsonDeleteScores()
	{
		CustomGameModeName = "";
	}

	FJsonDeleteScores(const FString& InCustomGameModeName)
	{
		CustomGameModeName = InCustomGameModeName;
	}
};

/** Interface to allow all other classes in this game to use HTTP request functions. */
UINTERFACE()
class UHttpRequestInterface : public UInterface
{
	GENERATED_BODY()
};

/** Interface to allow all other classes in this game to use HTTP request functions. */
class BEATSHOTGLOBAL_API IHttpRequestInterface
{
	GENERATED_BODY()

public:
	/** Checks to see if the user has a refresh token and if it has expired or not. */
	static bool IsRefreshTokenValid(const FString RefreshToken);

	/** Makes a GET request for a short lived access token given a valid refresh token. Executes supplied
	 *  OnAccessTokenResponse with the access token.
	 *  
	 *  @param RefreshToken the refresh token obtained when logging in
	 *  @param AccessTokenResponse struct containing callback delegate and response info
	 */
	static void RequestAccessToken(const FString RefreshToken,
		TSharedPtr<FAccessTokenResponse, ESPMode::ThreadSafe> AccessTokenResponse);

	/** Sends a POST login request to BeatShot website given a LoginPayload. Executes delegate in struct on completion.
	 *
	 *  @param LoginPayload login info to send with request
	 *  @param LoginResponse struct containing callback delegate and response info
	 */
	static void LoginUser(const FLoginPayload LoginPayload,
		TSharedPtr<FLoginResponse, ESPMode::ThreadSafe> LoginResponse);

	/** Converts ScoresToPost to a JSON string and sends an http POST request to BeatShot website given a valid
	 *  access token. Executes delegate in struct on completion.
	 * 
	 *  @param ScoresToPost scores to send with the request
	 *  @param UserID userID of the BeatShot account
	 *  @param AccessToken access token obtained using refresh token
	 *  @param PostScoresResponse struct containing callback delegate and response info
	 */
	static void PostPlayerScores(const TArray<FPlayerScore> ScoresToPost, const FString UserID,
		const FString AccessToken, TSharedPtr<FBSHttpResponse, ESPMode::ThreadSafe> PostScoresResponse);

	/** Makes a POST request to BeatShot website which emails the feedback. Executes supplied OnPostFeedbackResponse.
	 *
	 *  @param InFeedback struct to send with the request
	 *  @param FeedbackResponse struct containing callback delegate and response info
	 */
	static void PostFeedback(const FJsonFeedback InFeedback,
		TSharedPtr<FBSHttpResponse, ESPMode::ThreadSafe> FeedbackResponse);

	/** Makes a DELETE request to BeatShot website which deletes all scores matching the CustomGameModeName and userID.
	 *
	 *  @param CustomGameModeName CustomGameModeName to send with the request
	 *  @param UserID userID of the BeatShot account
	 *  @param AccessToken access token obtained using refresh token
	 *  @param DeleteScoresResponse struct containing callback delegate and response info
	 */
	static void DeleteScores(const FString CustomGameModeName, const FString UserID, const FString AccessToken,
		TSharedPtr<FDeleteScoresResponse, ESPMode::ThreadSafe> DeleteScoresResponse);

	/** Makes a GET request to BeatShot website that uses the AuthenticateUserTicket request from the
	 *  ISteamUserAuthInterface. Executes supplied OnTicketWebApiResponse.
	 *
	 *  @param AuthTicket auth ticket obtained from GetAuthTicketFromWebApi Steam API call
	 *  @param SteamAuthTicketResponse struct containing callback delegate and response info
	 */
	static void AuthenticateSteamUser(const FString AuthTicket,
		TSharedPtr<FSteamAuthTicketResponse, ESPMode::ThreadSafe> SteamAuthTicketResponse);
};
