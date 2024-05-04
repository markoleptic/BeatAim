// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class BeatShot : ModuleRules
{
	public BeatShot(ReadOnlyTargetRules Target) : base(Target)
	{
		bEnableUndefinedIdentifierWarnings = false;
		bEnableExceptions = true;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrecompileForTargets = PrecompileTargetsType.Any;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "Niagara", "EnhancedInput", "UserInterface", "UMG",
			"BeatShotGlobal", "OnlineSubsystem", "OnlineSubsystemUtils",
			"Steamworks", "GameplayAbilities", "GameplayTags", "GameplayTasks", "NetCore", "PhysicsCore",
			"ModularGameplay", "Text3D", "DLSSBlueprint", "NISBlueprint",
			"StreamlineBlueprint", "Slate", "SlateCore", "MoviePlayer", "NumCpp", "Synthesis", "AudioMixer"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"ParallelcubeAudioAnalyzer", "ParallelcubeTaglib", "EnhancedInput", "MoviePlayer", "DeveloperSettings",
			"AudioModulation", "MetasoundEngine"
		});

		PublicIncludePaths.AddRange(new[]
		{
			"../Plugins/AudioAnalyzer/Source/AudioAnalyzer/Private",
			"../Plugins/AudioAnalyzer/Source/AudioAnalyzer/Thirdparty/KissFFT_130",
			"../Plugins/AudioAnalyzer/Source/AudioAnalyzer/Thirdparty/KissFFT_130/tools",
			"../Plugins/AudioAnalyzer/Source/Thirdparty/miniaudio/include",
			"../Plugins/AudioAnalyzer/Source/AudioAnalyzer/Thirdparty/stb"
		});

		PublicIncludePaths.Add(Path.Combine(EngineDirectory, "Plugins/Marketplace/DLSS/Source/ThirdParty/NGX/Include"));
		PublicIncludePaths.Add(Path.Combine(EngineDirectory,
			"Plugins/Marketplace/Streamline/Source/ThirdParty/Streamline/include"));

		DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
	}
}