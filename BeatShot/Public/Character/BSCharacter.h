// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSCharacterBase.h"
#include "BSCharacter.generated.h"

class UBSRecoilComponent;

/** Base Character for this game. */
UCLASS()
class BEATSHOT_API ABSCharacter : public ABSCharacterBase
{
	GENERATED_BODY()

public:
	ABSCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** The skeletal mesh for hands that hold the gun. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BeatShot|Components")
	TObjectPtr<USkeletalMeshComponent> HandsMesh;

public:
	UFUNCTION(BlueprintPure, Category = "BeatShot|Character")
	USkeletalMeshComponent* GetHandsMesh() const;
};
