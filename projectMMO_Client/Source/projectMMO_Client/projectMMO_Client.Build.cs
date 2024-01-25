// Fill out your copyright notice in the Description page of Project Settings.

using System;
using System.IO;
using UnrealBuildTool;

public class projectMMO_Client : ModuleRules
{
	public projectMMO_Client(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

        // Add IocpSocketContoller Library
		string RootPath = "D:/_Project/UE5_project/ToyProject_MMO";
		string HeaderPath = Path.Combine(RootPath, "IocpSocketController");
		string LibraryPath = Path.Combine(RootPath, "Library/Network/x64/Release/");
		string LibraryName = "IocpSocketController.lib";

		PublicIncludePaths.Add(HeaderPath);
		PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, LibraryName));
	}
}
