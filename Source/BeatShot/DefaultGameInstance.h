// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModeActorStruct.h"
#include "Engine/GameInstance.h"
#include "Interfaces/IHttpRequest.h"
#include "DefaultGameInstance.generated.h"

class USaveGamePlayerScore;
class USaveGamePlayerSettings;
class ADefaultGameMode;
class ATargetSpawner;
class ADefaultCharacter;
class AGameModeBase;
class ASphereTarget;
class AGameModeActorBase;
class ADefaultPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAASettingsChange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerSettingsChange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvalidRefreshToken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoginResponse, FString, ResponseMsg, int32, ResponseCode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAccessTokenResponse, FString, ResponseMsg, int32, ResponseCode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostPlayerScoresResponse, FString, ResponseMsg, int32, ResponseCode);

/**
 *
 */
UCLASS()
class BEATSHOT_API UDefaultGameInstance : public UGameInstance
{
	GENERATED_BODY()

		virtual void Init() override;

public:

	//References

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
		ADefaultCharacter* DefaultCharacterRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
		ATargetSpawner* TargetSpawnerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
		ASphereTarget* SphereTargetRef;

	// Only used to make sure all targets are destroyed at the end of game
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
		TArray<ASphereTarget*> SphereTargetArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
		AGameModeBase* GameModeBaseRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
		AGameModeActorBase* GameModeActorBaseRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
		ADefaultPlayerController* DefaultPlayerControllerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
		FGameModeActorStruct GameModeActorStruct;

	// Register Functions

	UFUNCTION(BlueprintCallable, Category = "References")
		void RegisterDefaultCharacter(ADefaultCharacter* DefaultCharacter);

	UFUNCTION(BlueprintCallable, Category = "References")
		void RegisterTargetSpawner(ATargetSpawner* TargetSpawner);

	UFUNCTION(BlueprintCallable, Category = "References")
		void RegisterSphereTarget(ASphereTarget* SphereTarget);

	UFUNCTION(BlueprintCallable, Category = "References")
		void RegisterGameModeBase(AGameModeBase* GameModeBase);

	UFUNCTION(BlueprintCallable, Category = "References")
		void RegisterGameModeActorBase(AGameModeActorBase* GameModeActorBase);

	UFUNCTION(BlueprintCallable, Category = "References")
		void RegisterPlayerController(ADefaultPlayerController* DefaultPlayerController);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
		EGameModeActorName GameModeActorName;

	// Audio Analyzer Settings

	UFUNCTION(BlueprintCallable, Category = "AA Settings")
		FAASettingsStruct LoadAASettings();

	UFUNCTION(BlueprintCallable, Category = "AA Settings")
		void SaveAASettings(FAASettingsStruct AASettingsToSave);

	// Player Scores Loading / Saving

	UFUNCTION(BlueprintCallable, Category = "Scoring")
		TMap<FGameModeActorStruct, FPlayerScoreArrayWrapper> LoadPlayerScores();

	// local saving of scores
	UFUNCTION(BlueprintCallable, Category = "Scoring")
		void SavePlayerScores(TMap<FGameModeActorStruct, FPlayerScoreArrayWrapper> PlayerScoreMapToSave);

	// database saving of scores. First sends an access token request, then calls savescores with the accesstoken
	UFUNCTION(BlueprintCallable, Category = "DataBase")
		void SavePlayerScoresToDatabase(TMap<FGameModeActorStruct, FPlayerScoreArrayWrapper> PlayerScoreMapToSave);

	// Player Settings Loading / Saving

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Settings")
		FPlayerSettings PlayerSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Settings")
		USaveGamePlayerSettings* SaveGamePlayerSettings;

	UFUNCTION(BlueprintCallable, Category = "Player Settings")
		void SavePlayerSettings(FPlayerSettings PlayerSettingsToSave);

	UFUNCTION(BlueprintCallable, Category = "Player Settings")
		FPlayerSettings LoadPlayerSettings();

	// delegates

	UPROPERTY(BlueprintAssignable)
		FOnAASettingsChange OnAASettingsChange;

	UPROPERTY(BlueprintAssignable)
		FOnPlayerSettingsChange OnPlayerSettingsChange;

	//UPROPERTY(BlueprintAssignable)
	//	FOnPlayerScoresChange OnPlayerScoresChange;

	UPROPERTY(BlueprintAssignable)
		FOnLoginResponse OnLoginResponse;

	UPROPERTY(BlueprintAssignable)
		FOnAccessTokenResponse OnAccessTokenResponse;

	UPROPERTY(BlueprintAssignable)
		FOnPostPlayerScoresResponse OnPostPlayerScoresResponse;

	UPROPERTY(BlueprintAssignable)
		FOnInvalidRefreshToken OnInvalidRefreshToken;

	UFUNCTION(BlueprintCallable, Category = "Authorization")
		void LoginUser(FLoginPayload LoginPayload);

	UFUNCTION(BlueprintCallable, Category = "Authorization")
		void RequestAccessToken(FString RefreshToken);

	UFUNCTION(BlueprintCallable, Category = "Authorization")
		void PostPlayerScores(FString AccessTokenFString, int32 ResponseCode);

	UFUNCTION(BlueprintCallable, Category = "Authorization")
		bool IsRefreshTokenValid();

	void OnLoginResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void OnAccessTokenResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void OnPostPlayerScoresResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

private:

	UPROPERTY(BlueprintReadOnly, Category = "Authorization", meta = (AllowPrivateAccess = true))
		FString Username;

	UPROPERTY(BlueprintReadOnly, Category = "Authorization", meta = (AllowPrivateAccess = true))
		FString LoginEndpoint = "https://beatshot.gg/api/login";

	const FString RefreshEndpoint = "https://beatshot.gg/api/refresh";

	const FString SaveScoresEndpoint = "https://beatshot.gg/api/profile/";
};

