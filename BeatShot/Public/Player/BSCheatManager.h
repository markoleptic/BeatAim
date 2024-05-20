#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameFramework/CheatManager.h"
#include "HAL/IConsoleManager.h"
#include "BSCheatManager.generated.h"

class UGameplayAbility;

/** Creates a UObject delegate binding a TAutoConsoleVariable to the given object and function. */
#define BIND_BS_CVAR(Variable, Object, Function) \
Variable.AsVariable()->SetOnChangedCallback(FConsoleVariableDelegate::CreateUObject(Object, Function))

/** Creates a lambda delegate binding a TAutoConsoleVariable, which calls the function supplied. The function is
 *  takes a reference to the MemberVariable. */
#define BIND_BS_COMP_CVAR(Variable, Object, Function, MemberVariable) \
Variable->AsVariable()->SetOnChangedCallback(FConsoleVariableDelegate::CreateLambda([Object] (IConsoleVariable* Var) \
{ \
	Object->Function(Var, MemberVariable); \
}));
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

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Debug")
	TSubclassOf<UGameplayAbility> AimBotAbility;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Debug")
	float SpawnBoxLineThickness = 6.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Debug")
	float StaticExtentsBoxLineThickness = 6.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Debug")
	float SpawnVolumeLineThickness = 6.f;

	UPROPERTY()
	FGameplayAbilitySpecHandle AimBotSpecHandle;
};
