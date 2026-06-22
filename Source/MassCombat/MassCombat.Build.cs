// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MassCombat : ModuleRules
{
	public MassCombat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Add the module root so subsystem folders can be included via module-root-relative paths.
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
