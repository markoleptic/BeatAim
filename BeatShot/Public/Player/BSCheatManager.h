#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameFramework/CheatManager.h"
#include "HAL/IConsoleManager.h"
#include "BSCheatManager.generated.h"

class UGameplayAbility;

/** Base CheatManager for this game. */
UCLASS(Blueprintable)
class BEATSHOT_API UBSCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
#if !UE_BUILD_SHIPPING

	virtual void InitCheatManager() override;
	void Cheat_AimBot(IConsoleVariable* Variable);
	void ShowDebug(IConsoleVariable* Variable);
	void ShowDebug_SpawnBox(IConsoleVariable* Variable);
	void ShowDebug_SpawnVolume(IConsoleVariable* Variable);
	void ShowDebug_DirectionalBoxes(IConsoleVariable* Variable);
	void ShowDebug_ReinforcementLearningWidget(IConsoleVariable* Variable);
	void ShowDebug_SpotLightFront(IConsoleVariable* Variable);
	void ShowDebug_MovingTargetDirectionModeAny(IConsoleVariable* Variable);
	void SetTimeOfDay(IConsoleVariable* Variable);

	/** Sets the ComponentClass's value type to the type matching the console variable. */
	template <typename T, class ComponentClass>
	void SetComponentDebugValue(IConsoleVariable* Variable, T ComponentClass::* DebugVar);

#endif

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BeatShot|Debug")
	TSubclassOf<UGameplayAbility> AimBotAbility;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BeatShot|Debug")
	float SpawnBoxLineThickness = 6.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BeatShot|Debug")
	float StaticExtentsBoxLineThickness = 6.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BeatShot|Debug")
	float SpawnVolumeLineThickness = 6.f;

	UPROPERTY()
	FGameplayAbilitySpecHandle AimBotSpecHandle;
};
