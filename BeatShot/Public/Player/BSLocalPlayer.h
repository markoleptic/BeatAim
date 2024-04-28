// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "BSLocalPlayer.generated.h"

struct FSwapAudioOutputResult;
class UBSGameUserSettings;

/** Local machine-specific player. */
UCLASS()
class BEATSHOT_API UBSLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	UBSLocalPlayer();

	//~UObject interface
	virtual void PostInitProperties() override;
	//~End of UObject interface

	/** Gets the local settings for this player, this is read from config files at process startup and is always valid */
	UFUNCTION()
	UBSGameUserSettings* GetLocalSettings() const;

protected:
	void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);

	UFUNCTION()
	void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);

private:
	UPROPERTY(Transient)
	mutable TObjectPtr<UBSGameUserSettings> BSGameUserSettings;
};
