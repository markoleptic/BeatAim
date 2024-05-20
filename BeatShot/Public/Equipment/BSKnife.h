// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerSettingsInterface.h"
#include "GameFramework/Actor.h"
#include "BSKnife.generated.h"

struct FPlayerSettings_Game;

UCLASS()
class BEATSHOT_API ABSKnife : public AActor, public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

public:
	ABSKnife();

protected:
	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** ~IBSPlayerSettingsInterface begin */
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings) override;
	/** ~IBSPlayerSettingsInterface end */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Knife;

	/** Sets whether the weapon mesh is visible, updating its GameplayTags. */
	UFUNCTION(BlueprintCallable)
	void SetShowKnifeMesh(const bool bShow);
};
