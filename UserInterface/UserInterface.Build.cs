// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class UserInterface : ModuleRules
{
	public UserInterface(ReadOnlyTargetRules Target) : base(Target)
	{
		// For some reason this is necessary, otherwise get errors for AudioMixerQuantizedCommands
		bUseUnity = false;
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "ApplicationCore", "CoreUObject", "Engine", "UMG", "Slate", "SlateCore", "WebBrowserWidget",
			"WebBrowser", "BeatShotGlobal", "GameplayTags",
			"InputCore", "CommonUI", "MoviePlayer"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"ParallelcubeAudioAnalyzer", "ParallelcubeTaglib", "EnhancedInput"
		});

		PublicIncludePaths.AddRange(new[]
		{
			"../Plugins/AudioAnalyzer/Source/AudioAnalyzer/Private",
			"../Plugins/AudioAnalyzer/Source/AudioAnalyzer/Thirdparty/KissFFT_130",
			"../Plugins/AudioAnalyzer/Source/AudioAnalyzer/Thirdparty/KissFFT_130/tools",
			"../Plugins/AudioAnalyzer/Source/Thirdparty/miniaudio/include",
			"../Plugins/AudioAnalyzer/Source/AudioAnalyzer/Thirdparty/stb"
		});
	}
}