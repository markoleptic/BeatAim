// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class BeatShotGlobal : ModuleRules
{
	public BeatShotGlobal(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "InputCore", "Engine", "HTTP", "Json", "JsonUtilities",
			"DLSSBlueprint", "NISBlueprint", "StreamlineBlueprint", "GameplayTags", "UMG", "EnhancedInput",
			"PhysicsCore", "Slate", "SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"DeveloperSettings", "AudioModulation"
		});

		PublicIncludePaths.Add(Path.Combine(EngineDirectory, "Plugins/Marketplace/DLSS/Source/ThirdParty/NGX/Include"));
		PublicIncludePaths.Add(Path.Combine(EngineDirectory,
			"Plugins/Marketplace/Streamline/Source/ThirdParty/Streamline/include"));
	}
}