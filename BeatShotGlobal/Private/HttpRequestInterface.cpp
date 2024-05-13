// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


// ReSharper disable StringLiteralTypo
#include "HttpRequestInterface.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "SaveGamePlayerScore.h"
#include "Interfaces/IHttpResponse.h"

bool IHttpRequestInterface::IsRefreshTokenValid(const FString RefreshToken)
{
	if (RefreshToken.IsEmpty())
	{
		return false;
	}
	FDateTime CookieExpireDate;
	const int32 ExpiresStartPos = RefreshToken.Find("Expires=", ESearchCase::CaseSensitive, ESearchDir::FromStart, 0);
	const FString RightChopped = RefreshToken.RightChop(ExpiresStartPos + 8);
	const FString CookieExpireString = RightChopped.Left(RightChopped.Find(";", ESearchCase::IgnoreCase,
		ESearchDir::FromStart, 0));
	FDateTime::ParseHttpDate(CookieExpireString, CookieExpireDate);

	if (FDateTime::UtcNow() + FTimespan::FromDays(1) < CookieExpireDate)
	{
		return true;
	}
	return false;
}

void IHttpRequestInterface::RequestAccessToken(const FString RefreshToken,
	TSharedPtr<FAccessTokenResponse, ESPMode::ThreadSafe> AccessTokenResponse)
{
	const FHttpRequestRef HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Constants::Endpoint_Refresh);
	HttpRequest->SetVerb("GET");
	HttpRequest->SetTimeout(5.f);
	HttpRequest->SetHeader("Cookie", RefreshToken);
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[AccessTokenResponse](FHttpRequestPtr Request, const FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			check(AccessTokenResponse.IsValid());
			AccessTokenResponse->bConnectedSuccessfully = bConnectedSuccessfully;
			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				AccessTokenResponse->HttpStatus = 502;
				UE_LOG(LogTemp, Warning, TEXT("Access Token Request failed to successfully connect."));
			}
			else
			{
				AccessTokenResponse->HttpStatus = Response->GetResponseCode();
				if (AccessTokenResponse->HttpStatus >= 200 && AccessTokenResponse->HttpStatus <= 300)
				{
					AccessTokenResponse->OK = true;
					TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
					const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(
						Response->GetContentAsString());
					FJsonSerializer::Deserialize(JsonReader, JsonObject);
					AccessTokenResponse->AccessToken = JsonObject->GetStringField("accessToken");
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Request Access Token failed Http Status: %d"),
						AccessTokenResponse->HttpStatus);
				}
			}
			if (AccessTokenResponse->OnHttpResponseReceived.IsBound())
			{
				AccessTokenResponse->OnHttpResponseReceived.Execute();
			}
		});
	HttpRequest->ProcessRequest();
}

void IHttpRequestInterface::LoginUser(const FLoginPayload LoginPayload,
	TSharedPtr<FLoginResponse, ESPMode::ThreadSafe> LoginResponse)
{
	FString ContentString;
	const TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	FJsonObjectConverter::UStructToJsonObject(FLoginPayload::StaticStruct(), &LoginPayload, JsonObject);
	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&ContentString);
	FJsonSerializer::Serialize(JsonObject, JsonWriter);

	const FHttpRequestRef HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Constants::Endpoint_Login);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetTimeout(5.f);
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(ContentString);
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[LoginResponse](FHttpRequestPtr Request, const FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			check(LoginResponse.IsValid());
			LoginResponse->bConnectedSuccessfully = bConnectedSuccessfully;
			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				LoginResponse->ResponseMsg = "Failed to Connect";
				LoginResponse->HttpStatus = 502;
				UE_LOG(LogTemp, Warning, TEXT("Login Request failed to successfully connect."));
			}
			else
			{
				LoginResponse->HttpStatus = Response->GetResponseCode();
				TSharedPtr<FJsonObject> ResponseJsonObject = MakeShareable(new FJsonObject());
				const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(
					Response->GetContentAsString());
				FJsonSerializer::Deserialize(JsonReader, ResponseJsonObject);

				if (LoginResponse->HttpStatus >= 200 && LoginResponse->HttpStatus <= 300)
				{
					LoginResponse->OK = true;
					LoginResponse->UserID = ResponseJsonObject->GetStringField("userID");
					LoginResponse->DisplayName = ResponseJsonObject->GetStringField("displayName");
					LoginResponse->AccessToken = ResponseJsonObject->GetStringField("accessToken");
					LoginResponse->RefreshToken = Response->GetHeader("set-cookie");
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Login Request failed Http Status: %d"), LoginResponse->HttpStatus);
				}
			}
			if (LoginResponse->OnHttpResponseReceived.IsBound())
			{
				LoginResponse->OnHttpResponseReceived.Execute();
			}
		});
	HttpRequest->ProcessRequest();
}

void IHttpRequestInterface::PostPlayerScores(const TArray<FPlayerScore> ScoresToPost, const FString UserID,
	const FString AccessToken, TSharedPtr<FBSHttpResponse, ESPMode::ThreadSafe> PostScoresResponse)
{
	FJsonScore JsonScores;
	// Add all elements that haven't been saved to database to the JsonScores Scores array
	for (const FPlayerScore& PlayerScoreObject : ScoresToPost)
	{
		if (!PlayerScoreObject.bSavedToDatabase)
		{
			JsonScores.Scores.Add(PlayerScoreObject);
		}
	}

	FString ContentString;
	const TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	FJsonObjectConverter::UStructToJsonObject(FJsonScore::StaticStruct(), &JsonScores, JsonObject);
	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&ContentString);
	FJsonSerializer::Serialize(JsonObject, JsonWriter);
	UE_LOG(LogTemp, Display, TEXT("FJsonScore: %s"), *ContentString);

	const FHttpRequestRef HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Constants::Segment_ApiProfile + UserID + "/savescores");
	HttpRequest->SetVerb("POST");
	HttpRequest->SetTimeout(5.f);
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetHeader("Authorization", "Bearer " + AccessToken);
	HttpRequest->SetContentAsString(ContentString);
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[PostScoresResponse](FHttpRequestPtr Request, const FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			check(PostScoresResponse.IsValid());
			PostScoresResponse->bConnectedSuccessfully = bConnectedSuccessfully;
			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				PostScoresResponse->HttpStatus = 502;
				UE_LOG(LogTemp, Warning, TEXT("PostPlayerScores Request failed to successfully connect."));
			}
			else
			{
				PostScoresResponse->HttpStatus = Response->GetResponseCode();
				if (PostScoresResponse->HttpStatus >= 200 && PostScoresResponse->HttpStatus <= 300)
				{
					PostScoresResponse->OK = true;
					UE_LOG(LogTemp, Warning, TEXT("Successfully saved scores to database."));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Send Scores Request failed Http Status: %d"),
						PostScoresResponse->HttpStatus);
				}
			}
			if (PostScoresResponse->OnHttpResponseReceived.IsBound())
			{
				PostScoresResponse->OnHttpResponseReceived.Execute();
			}
		});
	HttpRequest->ProcessRequest();
}

void IHttpRequestInterface::PostFeedback(const FJsonFeedback InFeedback,
	TSharedPtr<FBSHttpResponse, ESPMode::ThreadSafe> FeedbackResponse)
{
	FString ContentString;
	const TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	FJsonObjectConverter::UStructToJsonObject(FJsonFeedback::StaticStruct(), &InFeedback, JsonObject);
	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&ContentString);
	FJsonSerializer::Serialize(JsonObject, JsonWriter);

	const FHttpRequestRef HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Constants::Endpoint_SendFeedback);
	HttpRequest->SetTimeout(5.f);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(ContentString);
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[FeedbackResponse](FHttpRequestPtr Request, const FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			check(FeedbackResponse.IsValid());
			FeedbackResponse->bConnectedSuccessfully = bConnectedSuccessfully;
			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				FeedbackResponse->HttpStatus = 502;
				UE_LOG(LogTemp, Warning, TEXT("Failed to connect to send feedback."));
			}
			else
			{
				FeedbackResponse->HttpStatus = Response->GetResponseCode();
				if (FeedbackResponse->HttpStatus >= 200 && FeedbackResponse->HttpStatus <= 300)
				{
					FeedbackResponse->OK = true;
					UE_LOG(LogTemp, Warning, TEXT("Successfully sent feedback."));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to send feedback Http Status: %d"),
						FeedbackResponse->HttpStatus);
				}
			}
			if (FeedbackResponse->OnHttpResponseReceived.IsBound())
			{
				FeedbackResponse->OnHttpResponseReceived.Execute();
			}
		});
	HttpRequest->ProcessRequest();
}

void IHttpRequestInterface::DeleteScores(const FString CustomGameModeName, const FString UserID,
	const FString AccessToken, TSharedPtr<FDeleteScoresResponse, ESPMode::ThreadSafe> DeleteScoresResponse)
{
	FString ContentString;
	const FJsonDeleteScores JsonDelete = FJsonDeleteScores(CustomGameModeName);
	const TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	FJsonObjectConverter::UStructToJsonObject(FJsonDeleteScores::StaticStruct(), &JsonDelete, JsonObject);
	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&ContentString);
	FJsonSerializer::Serialize(JsonObject, JsonWriter);

	const FHttpRequestRef HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Constants::Segment_ApiProfile + UserID + "/deletescores");
	HttpRequest->SetTimeout(5.f);
	HttpRequest->SetVerb("DELETE");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetHeader("Authorization", "Bearer " + AccessToken);
	HttpRequest->SetContentAsString(ContentString);
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[DeleteScoresResponse](FHttpRequestPtr Request, const FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			check(DeleteScoresResponse.IsValid());
			DeleteScoresResponse->bConnectedSuccessfully = bConnectedSuccessfully;
			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				DeleteScoresResponse->HttpStatus = 502;
				UE_LOG(LogTemp, Warning, TEXT("Failed to connect to server while deleting scores."));
			}
			else
			{
				DeleteScoresResponse->HttpStatus = Response->GetResponseCode();

				if (DeleteScoresResponse->HttpStatus >= 200 && DeleteScoresResponse->HttpStatus <= 300)
				{
					DeleteScoresResponse->OK = true;
					TSharedPtr<FJsonObject> ResponseJsonObject = MakeShareable(new FJsonObject());
					const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(
						Response->GetContentAsString());
					FJsonSerializer::Deserialize(JsonReader, ResponseJsonObject);

					DeleteScoresResponse->NumRemoved = static_cast<int32>(ResponseJsonObject->GetNumberField(
						"Number Removed"));
					UE_LOG(LogTemp, Display, TEXT("Successfully deleted scores from database."));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to delete scores Http Status: %d"),
						DeleteScoresResponse->HttpStatus);
				}
			}
			if (DeleteScoresResponse->OnHttpResponseReceived.IsBound())
			{
				DeleteScoresResponse->OnHttpResponseReceived.Execute();
			}
		});
	HttpRequest->ProcessRequest();
}

void IHttpRequestInterface::AuthenticateSteamUser(const FString AuthTicket,
	TSharedPtr<FSteamAuthTicketResponse, ESPMode::ThreadSafe> SteamAuthTicketResponse)
{
	const FHttpRequestRef HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Constants::Endpoint_AuthenticateUserTicketNoRedirect + AuthTicket);
	HttpRequest->SetTimeout(5.f);
	HttpRequest->SetVerb("GET");
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[SteamAuthTicketResponse](FHttpRequestPtr Request, const FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			check(SteamAuthTicketResponse.IsValid());
			SteamAuthTicketResponse->bConnectedSuccessfully = bConnectedSuccessfully;
			if (!SteamAuthTicketResponse->bConnectedSuccessfully || !Response.IsValid())
			{
				SteamAuthTicketResponse->HttpStatus = 502;
				SteamAuthTicketResponse->ErrorCode = "502";
				SteamAuthTicketResponse->ErrorDesc = "Failed to connect";

				UE_LOG(LogTemp, Warning, TEXT("Failed to connect to server while trying to authenticate."));
			}
			else
			{
				SteamAuthTicketResponse->HttpStatus = Response->GetResponseCode();

				TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
				const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(
					Response->GetContentAsString());
				FJsonSerializer::Deserialize(JsonReader, JsonObject);

				if (SteamAuthTicketResponse->HttpStatus >= 200 && SteamAuthTicketResponse->HttpStatus <= 300)
				{
					SteamAuthTicketResponse->OK = true;
					SteamAuthTicketResponse->Result = JsonObject->GetStringField("result");
					SteamAuthTicketResponse->SteamID = JsonObject->GetStringField("steamid");
					SteamAuthTicketResponse->OwnerSteamID = JsonObject->GetStringField("ownersteamid");
					SteamAuthTicketResponse->VacBanned = JsonObject->GetBoolField("vacbanned");
					SteamAuthTicketResponse->PublisherBanned = JsonObject->GetBoolField("publisherbanned");
					SteamAuthTicketResponse->DisplayName = JsonObject->GetStringField("displayname");
					SteamAuthTicketResponse->RefreshCookie = Response->GetHeader("set-cookie");
				}
				else
				{
					SteamAuthTicketResponse->ErrorCode = JsonObject->HasField("errorcode")
						? JsonObject->GetStringField("errorcode")
						: "Unknown Error Code";
					SteamAuthTicketResponse->ErrorDesc = JsonObject->HasField("errordesc")
						? JsonObject->GetStringField("errordesc")
						: "Unknown Error Description";
				}
			}
			if (SteamAuthTicketResponse->OnHttpResponseReceived.IsBound())
			{
				SteamAuthTicketResponse->OnHttpResponseReceived.Execute();
			}
		});
	HttpRequest->ProcessRequest();
}
