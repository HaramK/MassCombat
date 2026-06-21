// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MassCombat : ModuleRules
{
	public MassCombat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 서브폴더(Combat/, AI/, Actor/) 간 인클루드를 위해 모듈 루트를 검색 경로에 추가
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "MassEntity",
			"MassCommon", "MassSpawner", "MassActors", "MassMovement", "MassNavigation", "MassNavMeshNavigation", "MassLOD", "MassRepresentation", "MassSignals",
			"StateTreeModule", "MassAIBehavior", "NavigationSystem", "NavCorridor", "AnimToTexture", "GameplayTags", "DeveloperSettings"});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
