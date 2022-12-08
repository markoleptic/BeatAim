// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModeActorBase.h"
#include "SaveGameAASettings.h"
#include "SaveGamePlayerSettings.h"
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
class USoundClass;
class USoundMix;
class ADefaultPlayerController;

/* Used to convert PlayerScoreMap to database scores */
USTRUCT(BlueprintType)
struct FJsonScore
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPlayerScore> Scores;
};

/* Simple login payload */
USTRUCT(BlueprintType)
struct FLoginPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString Username;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString Email;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	FString Password;
};

/** Broadcast when AudioAnalyzer settings are changed and saved */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAASettingsChange, FAASettingsStruct, RefreshedAASettings);

/** Broadcast when Player Settings are changed and saved */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerSettingsChange, FPlayerSettings, RefreshedPlayerSettings);

/** Broadcast if refresh token is invalid NYI */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRefreshTokenResponse, bool, bSuccessful);

/** Broadcast when a login response is received from Beatshot website */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoginResponse, FString, ResponseMsg, int32, ResponseCode);

/** Broadcast when a response is received from posting player scores to database */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostPlayerScoresResponse, FString, ResponseMsg, int32, ResponseCode);

/** Broadcast when target spawner is registered */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetSpawnerInit, ATargetSpawner*, TargetSpawner);

UCLASS()
class BEATSHOT_API UDefaultGameInstance : public UGameInstance
{
	GENERATED_BODY()

#pragma region References

public:
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

	UFUNCTION(BlueprintCallable, Category = "Sound Settings")
	void ChangeVolume(USoundClass* SoundClassToChange, USoundMix* SoundMix, float Volume, float GlobalVolume) const;

	UPROPERTY(BlueprintReadWrite, Category = "References")
	ADefaultCharacter* DefaultCharacterRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "References")
	ATargetSpawner* TargetSpawnerRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "References")
	ASphereTarget* SphereTargetRef;

	/* Used to make sure all targets are destroyed at the end of game */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "References")
	TArray<ASphereTarget*> SphereTargetArray;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "References")
	AGameModeBase* GameModeBaseRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "References")
	AGameModeActorBase* GameModeActorBaseRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "References")
	ADefaultPlayerController* DefaultPlayerControllerRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "References")
	FGameModeActorStruct GameModeActorStruct;

#pragma endregion

#pragma region Settings
	
	UFUNCTION(BlueprintCallable, Category = "AA Settings")
	FAASettingsStruct LoadAASettings() const;

	UFUNCTION(BlueprintCallable, Category = "AA Settings")
	void SaveAASettings(FAASettingsStruct AASettingsToSave) const;

	UFUNCTION(BlueprintCallable, Category = "Player Settings")
	FPlayerSettings LoadPlayerSettings() const;
	
	UFUNCTION(BlueprintCallable, Category = "Player Settings")
	void SavePlayerSettings(FPlayerSettings PlayerSettingsToSave) const;

#pragma endregion

#pragma region Scoring
	
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	TMap<FGameModeActorStruct, FPlayerScoreArrayWrapper> LoadPlayerScores() const;

	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void SavePlayerScores(TMap<FGameModeActorStruct, FPlayerScoreArrayWrapper> PlayerScoreMapToSave,
	                      bool bSaveToDatabase);

	/* Database saving of scores. First sends an access token request, then calls save scores with the access token */
	UFUNCTION(BlueprintCallable, Category = "DataBase")
	void SavePlayerScoresToDatabase();

private:
	
	/* Bound to onAccessTokenResponse and removed when this function calls PostPlayerScores */
	void PostPlayerScores(FString AccessTokenFString, int32 ResponseCode);

#pragma endregion

#pragma region HttpRequests
	
public:
	UFUNCTION(BlueprintCallable, Category = "Authorization")
	void LoginUser(FLoginPayload LoginPayload);

	UFUNCTION(BlueprintCallable, Category = "Authorization")
	void RequestAccessToken(FString RefreshToken);

	UFUNCTION(BlueprintCallable, Category = "Authorization")
	bool IsRefreshTokenValid() const;

private:
	/* Bound to OnProcessRequestComplete Login Response */
	void OnLoginResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully) const;

	/* Bound to OnProcessRequestComplete RequestAccessToken Response */
	void OnAccessTokenResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	/* Bound to OnProcessRequestComplete PostPlayerScores Response */
	void OnPostPlayerScoresResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
											bool bConnectedSuccessfully);

#pragma endregion

#pragma region Delegates
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnAASettingsChange OnAASettingsChange;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerSettingsChange OnPlayerSettingsChange;

	UPROPERTY(BlueprintAssignable)
	FOnLoginResponse OnLoginResponse;

	UPROPERTY(BlueprintAssignable)
	FOnPostPlayerScoresResponse OnPostPlayerScoresResponse;

	UPROPERTY(BlueprintAssignable)
	FOnRefreshTokenResponse OnRefreshTokenResponse;

	UPROPERTY(BlueprintAssignable)
	FOnTargetSpawnerInit OnTargetSpawnerInit;

#pragma endregion

private:

	UPROPERTY(BlueprintReadOnly, Category = "Authorization", meta = (AllowPrivateAccess = true))
	FString Username;

	UPROPERTY(BlueprintReadOnly, Category = "Authorization", meta = (AllowPrivateAccess = true))
	FString LoginEndpoint = "https://beatshot.gg/api/login";

	const FString RefreshEndpoint = "https://beatshot.gg/api/refresh";

	const FString SaveScoresEndpoint = "https://beatshot.gg/api/profile/";

	bool bIsSavingScores = false;
};
