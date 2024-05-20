// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "RangeActors/WallMenu.h"
#include "BSGameInstance.h"
#include "Text3DComponent.h"
#include "AbilitySystem/BSAbilitySystemComponent.h"
#include "BeatShot/BSGameplayTags.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RangeActors/TimeOfDayManager.h"
#include "SaveGames/SaveGamePlayerSettings.h"

AWallMenu::AWallMenu()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	Corkboard = CreateDefaultSubobject<UStaticMeshComponent>("Corkboard");
	Corkboard->SetupAttachment(RootComponent);
	Corkboard->SetRelativeLocation(Position_Corkboard);
	Corkboard->SetGenerateOverlapEvents(false);
	Corkboard->SetCollisionProfileName(FName("BlockAll"));

	// Create ability system component, and set it to be explicitly replicated
	if (!AbilitySystemComponent)
	{
		AbilitySystemComponent = CreateDefaultSubobject<UBSAbilitySystemComponent>("Ability System Component");
		// Minimal Mode means that no GameplayEffects will replicate. They will only live on the Server.
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	}

	// Main Light Visualizer Enable
	MainText_Enable_LightVisualizers = CreateDefaultSubobject<UText3DComponent>(FName("Enable Light Visualizers"));
	SetupMainText(MainText_Enable_LightVisualizers, Corkboard, true, "WallMenu_LightVisualizer");

	ToggleText_LightVisualizers_On = CreateDefaultSubobject<UText3DComponent>(FName("Light Visualizers On"));
	ToggleText_LightVisualizers_Off = CreateDefaultSubobject<UText3DComponent>(FName("Light Visualizers Off"));
	Box_LightVisualizers_On = CreateDefaultSubobject<UBoxComponent>(FName("Box Light Visualizers On"));
	Box_LightVisualizers_Off = CreateDefaultSubobject<UBoxComponent>(FName("Box Light Visualizers Off"));

	SetupToggleText(MainText_Enable_LightVisualizers, ToggleText_LightVisualizers_On, ToggleText_LightVisualizers_Off,
		Box_LightVisualizers_On, Box_LightVisualizers_Off);

	// Front Beam Visualizer
	MainText_Enable_LVFrontBeam = CreateDefaultSubobject<UText3DComponent>(FName("Enable Front Beam"));
	SetupMainText(MainText_Enable_LVFrontBeam, MainText_Enable_LightVisualizers, false, "WallMenu_LVFrontBeam", Indent);

	ToggleText_LVFrontBeam_On = CreateDefaultSubobject<UText3DComponent>(FName("Front Beam On"));
	ToggleText_LVFrontBeam_Off = CreateDefaultSubobject<UText3DComponent>(FName("Front Beam Off"));
	Box_LVFrontBeam_On = CreateDefaultSubobject<UBoxComponent>(FName("Box Front Beam On"));
	Box_LVFrontBeam_Off = CreateDefaultSubobject<UBoxComponent>(FName("Box Front Beam Off"));

	SetupToggleText(MainText_Enable_LVFrontBeam, ToggleText_LVFrontBeam_On, ToggleText_LVFrontBeam_Off,
		Box_LVFrontBeam_On, Box_LVFrontBeam_Off, -Indent);

	// Top Beam Visualizer
	MainText_EnableLV_TopBeam = CreateDefaultSubobject<UText3DComponent>(FName("Enable Top Beam"));
	SetupMainText(MainText_EnableLV_TopBeam, MainText_Enable_LightVisualizers, false, "WallMenu_LVTopBeam",
		Offset_MainText + Indent);

	ToggleText_LV_TopBeam_On = CreateDefaultSubobject<UText3DComponent>(FName("Top Beam On"));
	ToggleText_LV_TopBeam_Off = CreateDefaultSubobject<UText3DComponent>(FName("Top Beam Off"));
	Box_LV_TopBeam_On = CreateDefaultSubobject<UBoxComponent>(FName("Box Top Beam On"));
	Box_LV_TopBeam_Off = CreateDefaultSubobject<UBoxComponent>(FName("Box Top Beam Off"));

	SetupToggleText(MainText_EnableLV_TopBeam, ToggleText_LV_TopBeam_On, ToggleText_LV_TopBeam_Off, Box_LV_TopBeam_On,
		Box_LV_TopBeam_Off, -Indent);

	// Left Beam Visualizer
	MainText_Enable_LVLeftBeam = CreateDefaultSubobject<UText3DComponent>(FName("Enable Left Beam"));
	SetupMainText(MainText_Enable_LVLeftBeam, MainText_Enable_LightVisualizers, false, "WallMenu_LVLeftBeam",
		(Offset_MainText * 2) + Indent);

	ToggleText_LVLeftBeam_On = CreateDefaultSubobject<UText3DComponent>(FName("Left Beam On"));
	ToggleText_LVLeftBeam_Off = CreateDefaultSubobject<UText3DComponent>(FName("Left Beam Off"));
	Box_LVLeftBeam_On = CreateDefaultSubobject<UBoxComponent>(FName("Box Left Beam On"));
	Box_LVLeftBeam_Off = CreateDefaultSubobject<UBoxComponent>(FName("Box Left Beam Off"));

	SetupToggleText(MainText_Enable_LVLeftBeam, ToggleText_LVLeftBeam_On, ToggleText_LVLeftBeam_Off, Box_LVLeftBeam_On,
		Box_LVLeftBeam_Off, -Indent);

	// Right Beam Visualizer
	MainText_Enable_LVRightBeam = CreateDefaultSubobject<UText3DComponent>(FName("Enable Right Beam"));
	SetupMainText(MainText_Enable_LVRightBeam, MainText_Enable_LightVisualizers, false, "WallMenu_LVRightBeam",
		(Offset_MainText * 3) + Indent);

	ToggleText_LVRightBeam_On = CreateDefaultSubobject<UText3DComponent>(FName("Right Beam On"));
	ToggleText_LVRightBeam_Off = CreateDefaultSubobject<UText3DComponent>(FName("Right Beam Off"));
	Box_LVRightBeam_On = CreateDefaultSubobject<UBoxComponent>(FName("Box Right Beam On"));
	Box_LVRightBeam_Off = CreateDefaultSubobject<UBoxComponent>(FName("Box Right Beam Off"));

	SetupToggleText(MainText_Enable_LVRightBeam, ToggleText_LVRightBeam_On, ToggleText_LVRightBeam_Off,
		Box_LVRightBeam_On, Box_LVRightBeam_Off, -Indent);

	// Left Cube Visualizer
	MainText_EnableLV_LeftCube = CreateDefaultSubobject<UText3DComponent>(FName("Enable Left Cube"));
	SetupMainText(MainText_EnableLV_LeftCube, MainText_Enable_LightVisualizers, false, "WallMenu_LVLeftCube",
		(Offset_MainText * 4) + Indent);

	ToggleText_LV_LeftCube_On = CreateDefaultSubobject<UText3DComponent>(FName("Left Cube On"));
	ToggleText_LV_LeftCube_Off = CreateDefaultSubobject<UText3DComponent>(FName("Left Cube Off"));
	Box_LV_LeftCube_On = CreateDefaultSubobject<UBoxComponent>(FName("Box Left Cube On"));
	Box_LV_LeftCube_Off = CreateDefaultSubobject<UBoxComponent>(FName("Box Left Cube Off"));

	SetupToggleText(MainText_EnableLV_LeftCube, ToggleText_LV_LeftCube_On, ToggleText_LV_LeftCube_Off,
		Box_LV_LeftCube_On, Box_LV_LeftCube_Off, -Indent);

	// Right Cube Visualizer
	MainText_EnableLV_RightCube = CreateDefaultSubobject<UText3DComponent>(FName("Enable Right Cube"));
	SetupMainText(MainText_EnableLV_RightCube, MainText_Enable_LightVisualizers, false, "WallMenu_LVRightCube",
		(Offset_MainText * 5) + Indent);

	ToggleText_LV_RightCube_On = CreateDefaultSubobject<UText3DComponent>(FName("Right Cube On"));
	ToggleText_LV_RightCube_Off = CreateDefaultSubobject<UText3DComponent>(FName("Right Cube Off"));
	Box_LV_RightCube_On = CreateDefaultSubobject<UBoxComponent>(FName("Box Right Cube On"));
	Box_LV_RightCube_Off = CreateDefaultSubobject<UBoxComponent>(FName("Box Right Cube Off"));

	SetupToggleText(MainText_EnableLV_RightCube, ToggleText_LV_RightCube_On, ToggleText_LV_RightCube_Off,
		Box_LV_RightCube_On, Box_LV_RightCube_Off, -Indent);


	// Night Mode Enable
	MainText_Enable_NightMode = CreateDefaultSubobject<UText3DComponent>(FName("Enable Night Mode"));
	SetupMainText(MainText_Enable_NightMode, MainText_Enable_LightVisualizers, false, "WallMenu_NightMode",
		Offset_MainText * 6);

	ToggleText_NightMode_On = CreateDefaultSubobject<UText3DComponent>(FName("Night Mode On"));
	ToggleText_NightMode_Off = CreateDefaultSubobject<UText3DComponent>(FName("Night Mode Off"));
	Box_NightMode_On = CreateDefaultSubobject<UBoxComponent>(FName("Box Night Mode On"));
	Box_NightMode_Off = CreateDefaultSubobject<UBoxComponent>(FName("Box Night Mode Off"));

	SetupToggleText(MainText_Enable_NightMode, ToggleText_NightMode_On, ToggleText_NightMode_Off, Box_NightMode_On,
		Box_NightMode_Off);
}

void AWallMenu::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->InitAbilityActorInfo(this, nullptr);
		GetAbilitySystemComponent()->OnGameplayEffectAppliedDelegateToSelf.AddUObject(this,
			&ThisClass::OnGameplayEffectAppliedToSelf);
	}
}

void AWallMenu::BeginPlay()
{
	Super::BeginPlay();

	if (UBSGameInstance* GI = Cast<UBSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		GI->RegisterPlayerSettingsUpdaters(OnPlayerSettingsChangedDelegate_Game);
		GI->RegisterPlayerSettingsSubscriber<
			AWallMenu, FPlayerSettings_Game>(this, &AWallMenu::OnPlayerSettingsChanged);
		GI->RegisterPlayerSettingsSubscriber<
			AWallMenu, FPlayerSettings_User>(this, &AWallMenu::OnPlayerSettingsChanged);
	}

	if (TimeOfDayManager)
	{
		TimeOfDayManager->OnTimeOfDayTransitionCompleted.BindUObject(this, &ThisClass::OnTimeOfDayChangeCompleted);
	}

	BoxToTextMap.Add(Box_LightVisualizers_On, FText3DToggle(MainText_Enable_LightVisualizers, Box_LightVisualizers_On,
		ToggleText_LightVisualizers_On, ToggleText_LightVisualizers_Off, true, "bShowLightVisualizers"));
	BoxToTextMap.Add(Box_LightVisualizers_Off, FText3DToggle(MainText_Enable_LightVisualizers, Box_LightVisualizers_Off,
		ToggleText_LightVisualizers_On, ToggleText_LightVisualizers_Off, false, "bShowLightVisualizers"));

	BoxToTextMap.Add(Box_LVFrontBeam_On, FText3DToggle(MainText_Enable_LVFrontBeam, Box_LVFrontBeam_On,
		ToggleText_LVFrontBeam_On, ToggleText_LVFrontBeam_Off, true, "bShow_LVFrontBeam"));
	BoxToTextMap.Add(Box_LVFrontBeam_Off, FText3DToggle(MainText_Enable_LVFrontBeam, Box_LVFrontBeam_Off,
		ToggleText_LVFrontBeam_On, ToggleText_LVFrontBeam_Off, false, "bShow_LVFrontBeam"));

	BoxToTextMap.Add(Box_LVLeftBeam_On, FText3DToggle(MainText_Enable_LVLeftBeam, Box_LVLeftBeam_On,
		ToggleText_LVLeftBeam_On, ToggleText_LVLeftBeam_Off, true, "bShow_LVLeftBeam"));
	BoxToTextMap.Add(Box_LVLeftBeam_Off, FText3DToggle(MainText_Enable_LVLeftBeam, Box_LVLeftBeam_Off,
		ToggleText_LVLeftBeam_On, ToggleText_LVLeftBeam_Off, false, "bShow_LVLeftBeam"));

	BoxToTextMap.Add(Box_LVRightBeam_On, FText3DToggle(MainText_Enable_LVRightBeam, Box_LVRightBeam_On,
		ToggleText_LVRightBeam_On, ToggleText_LVRightBeam_Off, true, "bShow_LVRightBeam"));
	BoxToTextMap.Add(Box_LVRightBeam_Off, FText3DToggle(MainText_Enable_LVRightBeam, Box_LVRightBeam_Off,
		ToggleText_LVRightBeam_On, ToggleText_LVRightBeam_Off, false, "bShow_LVRightBeam"));

	BoxToTextMap.Add(Box_LV_TopBeam_On, FText3DToggle(MainText_EnableLV_TopBeam, Box_LV_TopBeam_On,
		ToggleText_LV_TopBeam_On, ToggleText_LV_TopBeam_Off, true, "bShow_LVTopBeam"));
	BoxToTextMap.Add(Box_LV_TopBeam_Off, FText3DToggle(MainText_EnableLV_TopBeam, Box_LV_TopBeam_Off,
		ToggleText_LV_TopBeam_On, ToggleText_LV_TopBeam_Off, false, "bShow_LVTopBeam"));

	BoxToTextMap.Add(Box_LV_LeftCube_On, FText3DToggle(MainText_EnableLV_LeftCube, Box_LV_LeftCube_On,
		ToggleText_LV_LeftCube_On, ToggleText_LV_LeftCube_Off, true, "bShow_LVLeftCube"));
	BoxToTextMap.Add(Box_LV_LeftCube_Off, FText3DToggle(MainText_EnableLV_LeftCube, Box_LV_LeftCube_Off,
		ToggleText_LV_LeftCube_On, ToggleText_LV_LeftCube_Off, false, "bShow_LVLeftCube"));

	BoxToTextMap.Add(Box_LV_RightCube_On, FText3DToggle(MainText_EnableLV_RightCube, Box_LV_RightCube_On,
		ToggleText_LV_RightCube_On, ToggleText_LV_RightCube_Off, true, "bShow_LVRightCube"));
	BoxToTextMap.Add(Box_LV_RightCube_Off, FText3DToggle(MainText_EnableLV_RightCube, Box_LV_RightCube_Off,
		ToggleText_LV_RightCube_On, ToggleText_LV_RightCube_Off, false, "bShow_LVRightCube"));

	BoxToTextMap.Add(Box_NightMode_On, FText3DToggle(MainText_Enable_NightMode, Box_NightMode_On,
		ToggleText_NightMode_On, ToggleText_NightMode_Off, true, "bNightModeSelected"));
	BoxToTextMap.Add(Box_NightMode_Off, FText3DToggle(MainText_Enable_NightMode, Box_NightMode_Off,
		ToggleText_NightMode_On, ToggleText_NightMode_Off, false, "bNightModeSelected"));

	for (TPair<TObjectPtr<UBoxComponent>, FText3DToggle>& Pair : BoxToTextMap)
	{
		if (Pair.Value.Parent)
		{
			if (UText3DComponent* CastedParent = Cast<UText3DComponent>(Pair.Value.Parent))
			{
				if (Material_Main_Front_Text3D)
				{
					CastedParent->SetFrontMaterial(Material_Main_Front_Text3D);
				}
				if (Material_Bevel)
				{
					CastedParent->SetBevelMaterial(Material_Bevel);
				}
				if (Material_Extrude)
				{
					CastedParent->SetExtrudeMaterial(Material_Extrude);
				}
				if (Font_Text3D)
				{
					CastedParent->SetFont(Font_Text3D);
				}
			}
		}
		if (!Pair.Value.OnText || !Pair.Value.OffText)
		{
			continue;
		}

		if (Material_Bevel)
		{
			Pair.Value.OnText->SetBevelMaterial(Material_Bevel);
			Pair.Value.OffText->SetBevelMaterial(Material_Bevel);
		}
		if (Material_Extrude)
		{
			Pair.Value.OnText->SetExtrudeMaterial(Material_Extrude);
			Pair.Value.OffText->SetExtrudeMaterial(Material_Extrude);
		}
		if (Font_Text3D)
		{
			Pair.Value.OnText->SetFont(Font_Text3D);
			Pair.Value.OffText->SetFont(Font_Text3D);
		}

		if (!Material_Toggle)
		{
			continue;
		}

		UMaterialInstanceDynamic* DynamicMatFrontOn = UMaterialInstanceDynamic::Create(Material_Toggle,
			Pair.Value.OnText.Get());
		UMaterialInstanceDynamic* DynamicMatFrontOff = UMaterialInstanceDynamic::Create(Material_Toggle,
			Pair.Value.OffText.Get());

		if (!DynamicMatFrontOn || !DynamicMatFrontOff)
		{
			continue;
		}

		Pair.Value.OnText->SetFrontMaterial(DynamicMatFrontOn);
		Pair.Value.OffText->SetFrontMaterial(DynamicMatFrontOff);
	}

	const FPlayerSettings PlayerSettings = LoadPlayerSettings();
	Init(PlayerSettings.Game, PlayerSettings.User, false);
	bIsWaitingOnTimeOfDayTransition = false;
}

UAbilitySystemComponent* AWallMenu::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AWallMenu::OnGameplayEffectAppliedToSelf(UAbilitySystemComponent* ABS, const FGameplayEffectSpec& EffectSpec,
	FActiveGameplayEffectHandle EffectHandle)
{
	FGameplayTagContainer Container;
	EffectSpec.GetAllAssetTags(Container);
	const FHitResult* Hit = EffectSpec.GetEffectContext().GetHitResult();

	if (!Hit || !Container.HasTagExact(BSGameplayTags::Ability_Fire))
	{
		return;
	}

	if (UPrimitiveComponent* HitComponent = Hit->GetComponent())
	{
		UBoxComponent* Box = Cast<UBoxComponent>(HitComponent);
		const FText3DToggle* Found = BoxToTextMap.Find(Box);
		if (Found)
		{
			bool bRequiresSave = false;
			FPlayerSettings PlayerSettings = LoadPlayerSettings();
			if (Found->SettingType == "bNightModeSelected" && PlayerSettings.User.bNightModeUnlocked)
			{
				if (!bIsWaitingOnTimeOfDayTransition)
				{
					bRequiresSave = (PlayerSettings.Game.bNightModeSelected != Found->bIsOnText) || bRequiresSave;
					PlayerSettings.Game.bNightModeSelected = Found->bIsOnText;
				}
			}
			else if (Found->SettingType == "bShowLightVisualizers")
			{
				// Don't do anything if its already on
				if (Found->bIsOnText && PlayerSettings.Game.bShowLightVisualizers)
				{
					return;
				}

				bRequiresSave = true;
				PlayerSettings.Game.bShowLightVisualizers = Found->bIsOnText;

				// By default, only turn on 3 lights when initially toggling on
				if (Found->bIsOnText)
				{
					ToggleText(true, ToggleText_LV_TopBeam_On, ToggleText_LV_TopBeam_Off);
					ToggleText(true, ToggleText_LV_LeftCube_On, ToggleText_LV_LeftCube_Off);
					ToggleText(true, ToggleText_LV_RightCube_On, ToggleText_LV_RightCube_Off);
					ToggleText(false, ToggleText_LVFrontBeam_On, ToggleText_LVFrontBeam_Off);
					ToggleText(false, ToggleText_LVLeftBeam_On, ToggleText_LVLeftBeam_Off);
					ToggleText(false, ToggleText_LVRightBeam_On, ToggleText_LVRightBeam_Off);

					PlayerSettings.Game.bShow_LVTopBeam = true;
					PlayerSettings.Game.bShow_LVLeftCube = true;
					PlayerSettings.Game.bShow_LVRightCube = true;
					PlayerSettings.Game.bShow_LVFrontBeam = false;
					PlayerSettings.Game.bShow_LVLeftBeam = false;
					PlayerSettings.Game.bShow_LVRightBeam = false;
				}
				// Toggle all off if disabled
				else if (!Found->bIsOnText)
				{
					ToggleText(false, ToggleText_LV_TopBeam_On, ToggleText_LV_TopBeam_Off);
					ToggleText(false, ToggleText_LV_LeftCube_On, ToggleText_LV_LeftCube_Off);
					ToggleText(false, ToggleText_LV_RightCube_On, ToggleText_LV_RightCube_Off);
					ToggleText(false, ToggleText_LVFrontBeam_On, ToggleText_LVFrontBeam_Off);
					ToggleText(false, ToggleText_LVLeftBeam_On, ToggleText_LVLeftBeam_Off);
					ToggleText(false, ToggleText_LVRightBeam_On, ToggleText_LVRightBeam_Off);

					PlayerSettings.Game.bShow_LVTopBeam = false;
					PlayerSettings.Game.bShow_LVLeftCube = false;
					PlayerSettings.Game.bShow_LVRightCube = false;
					PlayerSettings.Game.bShow_LVFrontBeam = false;
					PlayerSettings.Game.bShow_LVLeftBeam = false;
					PlayerSettings.Game.bShow_LVRightBeam = false;
				}
			}
			else if (Found->SettingType == "bShow_LVFrontBeam")
			{
				bRequiresSave = (PlayerSettings.Game.bShow_LVFrontBeam != Found->bIsOnText) || bRequiresSave;
				PlayerSettings.Game.bShow_LVFrontBeam = Found->bIsOnText;
			}
			else if (Found->SettingType == "bShow_LVLeftBeam")
			{
				bRequiresSave = (PlayerSettings.Game.bShow_LVLeftBeam != Found->bIsOnText) || bRequiresSave;
				PlayerSettings.Game.bShow_LVLeftBeam = Found->bIsOnText;
			}
			else if (Found->SettingType == "bShow_LVRightBeam")
			{
				bRequiresSave = (PlayerSettings.Game.bShow_LVRightBeam != Found->bIsOnText) || bRequiresSave;
				PlayerSettings.Game.bShow_LVRightBeam = Found->bIsOnText;
			}
			else if (Found->SettingType == "bShow_LVTopBeam")
			{
				bRequiresSave = (PlayerSettings.Game.bShow_LVTopBeam != Found->bIsOnText) || bRequiresSave;
				PlayerSettings.Game.bShow_LVTopBeam = Found->bIsOnText;
			}
			else if (Found->SettingType == "bShow_LVLeftCube")
			{
				bRequiresSave = (PlayerSettings.Game.bShow_LVLeftCube != Found->bIsOnText) || bRequiresSave;
				PlayerSettings.Game.bShow_LVLeftCube = Found->bIsOnText;
			}
			else if (Found->SettingType == "bShow_LVRightCube")
			{
				bRequiresSave = (PlayerSettings.Game.bShow_LVRightCube != Found->bIsOnText) || bRequiresSave;
				PlayerSettings.Game.bShow_LVRightCube = Found->bIsOnText;
			}

			if (bRequiresSave)
			{
				SavePlayerSettings(PlayerSettings.Game);
				ToggleText(Found->bIsOnText, Found->OnText.Get(), Found->OffText.Get());
			}
		}
	}
}

void AWallMenu::OnTimeOfDayChangeCompleted(const ETimeOfDay NewTimeOfDay)
{
	if (ToggleText_NightMode_On)
	{
		UMaterialInterface* FrontMat = ToggleText_NightMode_On->GetFrontMaterial();
		if (UMaterialInstanceDynamic* Dynamic = Cast<UMaterialInstanceDynamic>(FrontMat))
		{
			Dynamic->SetScalarParameterValue(FName("bEnablePulse"), 0.0f);
		}
	}
	if (ToggleText_NightMode_Off)
	{
		UMaterialInterface* FrontMat = ToggleText_NightMode_Off->GetFrontMaterial();
		if (UMaterialInstanceDynamic* Dynamic = Cast<UMaterialInstanceDynamic>(FrontMat))
		{
			Dynamic->SetScalarParameterValue(FName("bEnablePulse"), 0.0f);
		}
	}
	bIsWaitingOnTimeOfDayTransition = false;
}

void AWallMenu::Init(const FPlayerSettings_Game& GameSettings, const FPlayerSettings_User& UserSettings,
	const bool bFromSettingsUpdate)
{
	MainText_Enable_NightMode->SetVisibility(UserSettings.bNightModeUnlocked);
	ToggleText_NightMode_On->SetVisibility(UserSettings.bNightModeUnlocked);
	ToggleText_NightMode_Off->SetVisibility(UserSettings.bNightModeUnlocked);

	if (bFromSettingsUpdate)
	{
		return;
	}

	ToggleText(GameSettings.bShowLightVisualizers, ToggleText_LightVisualizers_On, ToggleText_LightVisualizers_Off);
	ToggleText(UserSettings.bNightModeUnlocked && GameSettings.bNightModeSelected, ToggleText_NightMode_On,
		ToggleText_NightMode_Off);
	ToggleText(GameSettings.bShow_LVTopBeam, ToggleText_LV_TopBeam_On, ToggleText_LV_TopBeam_Off);
	ToggleText(GameSettings.bShow_LVLeftCube, ToggleText_LV_LeftCube_On, ToggleText_LV_LeftCube_Off);
	ToggleText(GameSettings.bShow_LVRightCube, ToggleText_LV_RightCube_On, ToggleText_LV_RightCube_Off);
	ToggleText(GameSettings.bShow_LVFrontBeam, ToggleText_LVFrontBeam_On, ToggleText_LVFrontBeam_Off);
	ToggleText(GameSettings.bShow_LVLeftBeam, ToggleText_LVLeftBeam_On, ToggleText_LVLeftBeam_Off);
	ToggleText(GameSettings.bShow_LVRightBeam, ToggleText_LVRightBeam_On, ToggleText_LVRightBeam_Off);

	if (ToggleText_NightMode_On)
	{
		UMaterialInterface* FrontMat = ToggleText_NightMode_On->GetFrontMaterial();
		if (UMaterialInstanceDynamic* Dynamic = Cast<UMaterialInstanceDynamic>(FrontMat))
		{
			Dynamic->SetScalarParameterValue(FName("bEnablePulse"), 0.0f);
		}
	}
	if (ToggleText_NightMode_Off)
	{
		UMaterialInterface* FrontMat = ToggleText_NightMode_Off->GetFrontMaterial();
		if (UMaterialInstanceDynamic* Dynamic = Cast<UMaterialInstanceDynamic>(FrontMat))
		{
			Dynamic->SetScalarParameterValue(FName("bEnablePulse"), 0.0f);
		}
	}
}

void AWallMenu::OnPlayerSettingsChanged(const FPlayerSettings_Game& GameSettings)
{
	Init(GameSettings, LoadPlayerSettings().User, true);
}

void AWallMenu::OnPlayerSettingsChanged(const FPlayerSettings_User& UserSettings)
{
	Init(LoadPlayerSettings().Game, UserSettings, true);
}

void AWallMenu::SetupMainText(UText3DComponent* InComponent, USceneComponent* InParent, const bool bFirstText,
	const FString& Key, const FVector& AdditionalOffset) const
{
	InComponent->SetupAttachment(InParent);

	if (bFirstText)
	{
		InComponent->SetRelativeLocation(Position_FirstMainText);
		InComponent->SetRelativeRotation(Rotation_FirstMainText);
	}
	else
	{
		InComponent->SetRelativeLocation(Offset_MainText + AdditionalOffset);
	}

	InComponent->SetScaleProportionally(true);
	InComponent->SetHasMaxHeight(true);

	if (AdditionalOffset == FVector::ZeroVector)
	{
		InComponent->SetMaxHeight(MaxHeightMainText);
	}
	else
	{
		InComponent->SetMaxHeight(MaxHeightIndentedText);
	}

	InComponent->SetText(FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", Key));
	InComponent->SetExtrude(5.f);
	InComponent->SetBevel(2.f);
	InComponent->SetCastShadow(false);
	InComponent->SetHorizontalAlignment(EText3DHorizontalTextAlignment::Left);
}

void AWallMenu::SetupToggleText(USceneComponent* InParent, UText3DComponent* InToggleTextOn,
	UText3DComponent* InToggleTextOff, UBoxComponent* InBoxOn, UBoxComponent* InBoxOff, const FVector& AdditionalOffset)
{
	InToggleTextOn->SetupAttachment(InParent);
	InToggleTextOn->SetRelativeLocation(Offset_OnText + AdditionalOffset);
	InToggleTextOn->SetExtrude(5.f);
	InToggleTextOn->SetBevel(2.f);
	InToggleTextOn->SetText(FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", "WallMenu_On"));
	InToggleTextOn->SetCastShadow(false);
	InToggleTextOn->SetHorizontalAlignment(EText3DHorizontalTextAlignment::Center);

	InToggleTextOff->SetupAttachment(InParent);
	InToggleTextOff->SetRelativeLocation(Offset_OffText + AdditionalOffset);
	InToggleTextOff->SetExtrude(5.f);
	InToggleTextOff->SetBevel(2.f);
	InToggleTextOff->SetText(FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", "WallMenu_Off"));
	InToggleTextOff->SetCastShadow(false);
	InToggleTextOff->SetHorizontalAlignment(EText3DHorizontalTextAlignment::Center);

	InToggleTextOn->SetScaleProportionally(true);
	InToggleTextOff->SetScaleProportionally(true);
	InToggleTextOn->SetHasMaxHeight(true);
	InToggleTextOff->SetHasMaxHeight(true);

	if (AdditionalOffset == FVector::ZeroVector)
	{
		InToggleTextOn->SetMaxHeight(MaxHeightMainText);
		InToggleTextOff->SetMaxHeight(MaxHeightMainText);
	}
	else
	{
		InToggleTextOn->SetMaxHeight(MaxHeightIndentedText);
		InToggleTextOff->SetMaxHeight(MaxHeightIndentedText);
	}

	InBoxOn->SetupAttachment(InToggleTextOn);
	InBoxOn->SetRelativeLocation(Position_BoxCollision);
	InBoxOn->SetRelativeScale3D(Scale_BoxCollision_On);
	InBoxOn->SetBoxExtent(Extents_BoxCollision);
	InBoxOn->SetGenerateOverlapEvents(false);
	InBoxOn->SetCollisionProfileName(FName("BlockAll"));

	InBoxOff->SetupAttachment(InToggleTextOff);
	InBoxOff->SetRelativeLocation(Position_BoxCollision);
	InBoxOff->SetRelativeScale3D(Scale_BoxCollision_Off);
	InBoxOff->SetBoxExtent(Extents_BoxCollision);
	InBoxOff->SetGenerateOverlapEvents(false);
	InBoxOff->SetCollisionProfileName(FName("BlockAll"));
}

void AWallMenu::ToggleText(const bool bIsOn, UText3DComponent* InToggleTextOn, UText3DComponent* InToggleTextOff) const
{
	UMaterialInterface* MatFrontOn = InToggleTextOn->GetFrontMaterial();
	UMaterialInterface* MatFrontOff = InToggleTextOff->GetFrontMaterial();

	if (!MatFrontOn || !MatFrontOff)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMatFrontOn = Cast<UMaterialInstanceDynamic>(MatFrontOn);
	UMaterialInstanceDynamic* DynamicMatFrontOff = Cast<UMaterialInstanceDynamic>(MatFrontOff);

	if (!DynamicMatFrontOn || !DynamicMatFrontOff)
	{
		return;
	}

	if (InToggleTextOn == ToggleText_NightMode_On)
	{
		if (!bIsWaitingOnTimeOfDayTransition)
		{
			if (bIsOn)
			{
				DynamicMatFrontOn->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Green);
				DynamicMatFrontOff->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Red);
				DynamicMatFrontOn->SetScalarParameterValue(FName("bEnablePulse"), 1.0f);
				DynamicMatFrontOff->SetScalarParameterValue(FName("bEnablePulse"), 0.0f);
			}
			else
			{
				DynamicMatFrontOn->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Red);
				DynamicMatFrontOff->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Green);
				DynamicMatFrontOn->SetScalarParameterValue(FName("bEnablePulse"), 0.0f);
				DynamicMatFrontOff->SetScalarParameterValue(FName("bEnablePulse"), 1.0f);
			}
			bIsWaitingOnTimeOfDayTransition = true;
		}
	}
	else
	{
		if (bIsOn)
		{
			DynamicMatFrontOn->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Green);
			DynamicMatFrontOff->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Red);
		}
		else
		{
			DynamicMatFrontOn->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Red);
			DynamicMatFrontOff->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Green);
		}
	}
}
