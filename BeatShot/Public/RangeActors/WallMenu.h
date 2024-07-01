// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "BSPlayerSettingsInterface.h"
#include "GameFramework/Actor.h"
#include "WallMenu.generated.h"

enum class ETimeOfDay : uint8;
class ATimeOfDayManager;
struct FActiveGameplayEffectHandle;
struct FGameplayEffectSpec;
class UBSAbilitySystemComponent;
class UBoxComponent;
class UText3DComponent;

USTRUCT(BlueprintType, meta=(ShowOnlyInnerProperties))
struct FText3DToggle
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USceneComponent> Parent;
	TObjectPtr<UBoxComponent> CollisionReceiver;
	TObjectPtr<UText3DComponent> OnText;
	TObjectPtr<UText3DComponent> OffText;
	FGuid Guid;
	bool bIsOnText;
	FString SettingType;

	FText3DToggle()
	{
		bIsOnText = false;
		Parent = nullptr;
		CollisionReceiver = nullptr;
		OnText = nullptr;
		OffText = nullptr;
		Guid = FGuid::NewGuid();
		SettingType = FString();
	}

	FText3DToggle(USceneComponent* InParent, UBoxComponent* Collision, UText3DComponent* InOnText,
		UText3DComponent* InOffText, const bool bInIsOnText, const FString InSettingType)
	{
		bIsOnText = bInIsOnText;
		Parent = InParent;
		CollisionReceiver = Collision;
		OnText = InOnText;
		OffText = InOffText;
		Guid = FGuid::NewGuid();
		SettingType = InSettingType;
	}

	~FText3DToggle()
	{
		Parent = nullptr;
		CollisionReceiver = nullptr;
		OnText = nullptr;
		OffText = nullptr;
		Guid.Invalidate();
		SettingType.Empty();
	}

	FORCEINLINE bool operator==(const FText3DToggle& Other) const
	{
		if (!Guid.IsValid() || !Other.Guid.IsValid())
		{
			return false;
		}
		return Guid == Other.Guid;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FText3DToggle& Text3DToggle)
	{
		return GetTypeHash(Text3DToggle.Guid);
	}
};


/** Displays some settings on a wall that can be changed by shooting them. */
UCLASS()
class BEATSHOT_API AWallMenu : public AActor, public IBSPlayerSettingsInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

	AWallMenu();

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	/* ~Begin IAbilitySystemInterface */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	/* ~End IAbilitySystemInterface */

	void OnGameplayEffectAppliedToSelf(UAbilitySystemComponent* ABS, const FGameplayEffectSpec& EffectSpec,
		FActiveGameplayEffectHandle EffectHandle);

public:
	void OnTimeOfDayChangeCompleted(const ETimeOfDay NewTimeOfDay);

protected:
	UPROPERTY()
	UBSAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* Corkboard;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* MainText_Enable_LightVisualizers;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LightVisualizers_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LightVisualizers_Off;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LightVisualizers_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LightVisualizers_Off;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* MainText_Enable_NightMode;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_NightMode_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_NightMode_Off;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_NightMode_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_NightMode_Off;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* MainText_Enable_LVFrontBeam;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LVFrontBeam_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LVFrontBeam_Off;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LVFrontBeam_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LVFrontBeam_Off;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* MainText_Enable_LVLeftBeam;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LVLeftBeam_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LVLeftBeam_Off;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LVLeftBeam_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LVLeftBeam_Off;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* MainText_Enable_LVRightBeam;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LVRightBeam_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LVRightBeam_Off;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LVRightBeam_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LVRightBeam_Off;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* MainText_EnableLV_TopBeam;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LV_TopBeam_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LV_TopBeam_Off;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LV_TopBeam_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LV_TopBeam_Off;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* MainText_EnableLV_LeftCube;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LV_LeftCube_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LV_LeftCube_Off;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LV_LeftCube_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LV_LeftCube_Off;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* MainText_EnableLV_RightCube;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LV_RightCube_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UText3DComponent* ToggleText_LV_RightCube_Off;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LV_RightCube_On;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Box_LV_RightCube_Off;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|References")
	TSoftObjectPtr<ATimeOfDayManager> TimeOfDayManager;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Text3D|Materials")
	UMaterialInterface* Material_Main_Front_Text3D;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Text3D|Materials")
	UMaterialInterface* Material_Toggle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Text3D|Materials")
	UMaterialInterface* Material_Extrude;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Text3D|Materials")
	UMaterialInterface* Material_Bevel;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Text3D")
	UFont* Font_Text3D;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Text3D")
	FVector Offset_MainText = {0.f, 0.f, -42.f};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Text3D")
	FVector Indent = {0.f, 40.f, 0.f};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Text3D")
	float MaxHeightMainText = 55.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BeatShot|Text3D")
	float MaxHeightIndentedText = 55.f;


	virtual void OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings) override;
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_User& UserSettings) override;

	void SetupMainText(UText3DComponent* InComponent, USceneComponent* InParent, const bool bFirstText,
		const FString& Key, const FVector& AdditionalOffset = FVector::ZeroVector) const;

	void SetupToggleText(USceneComponent* InParent, UText3DComponent* InToggleTextOn, UText3DComponent* InToggleTextOff,
		UBoxComponent* InBoxOn, UBoxComponent* InBoxOff, const FVector& AdditionalOffset = FVector::ZeroVector);

	/** Displays which settings are on/off etc. by lighting the correct words. */
	void ToggleAllText(const FPlayerSettings_Game& GameSettings, bool bNightModeUnlocked) const;

	void ToggleText(const bool bIsOn, const UText3DComponent* InToggleTextOn,
		const UText3DComponent* InToggleTextOff) const;

	void ToggleNightModeText(const bool bIsOn) const;

	void SetNightModeTextVisibility(bool bVisible) const;

	static void SetDynamicMaterialPulseValue(const UText3DComponent* InText3D, float Value);

	static void SetDynamicMaterialEmissiveColor(const UText3DComponent* InText3D, const FLinearColor& Color);

	FVector Position_Corkboard = {300.f, 50.f, 0.f};

	/** Offset from main text to the on text. */
	FVector Offset_OnText = {0, 747.f, 0};
	/** Offset from main text to the off text. */
	FVector Offset_OffText = {0, 944.f, 0};

	/** Offset from Corkboard to first main text. */
	FVector Position_FirstMainText = {335.f, 55.f, 55.f};
	/** Rotation first main text */
	FRotator Rotation_FirstMainText = {-90.f, 0.f, 0.f};

	/** Offset from Toggle text to box component. */
	FVector Position_BoxCollision = {2.f, 0, 16.75f};
	/** Box component extents. */
	FVector Extents_BoxCollision = {1.f, 32.f, 32.f};
	/** Box component scale for on. */
	FVector Scale_BoxCollision_On = {3.f, 1.2f, 0.65f};
	/** Box component scale for off. */
	FVector Scale_BoxCollision_Off = {3.f, 1.4f, 0.65f};

	TMap<TObjectPtr<UBoxComponent>, FText3DToggle> BoxToTextMap;

	mutable bool bIsWaitingOnTimeOfDayTransition = false;

	bool bNightModeUnlockedAtInit = false;
};
