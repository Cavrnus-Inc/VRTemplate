// Copyright (c) 2025 Cavrnus. All rights reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class CavrnusBlueprintModule : ModuleRules
{
    private void AddDefaultIncludePaths()
    {
        string PublicDirectory = Path.Combine(ModuleDirectory, "Public");
        if (Directory.Exists(PublicDirectory))
        {
            PublicIncludePaths.Add(PublicDirectory);
        }

        string PrivateDirectory = Path.Combine(ModuleDirectory, "Private");
        if (Directory.Exists(PrivateDirectory))
        {
            PrivateIncludePaths.Add(PrivateDirectory);
        }
    }

    public CavrnusBlueprintModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseRTTI = true;
        bEnableExceptions = true;

#if UE_5_2_OR_LATER
        IWYUSupport = IWYUSupport.Full;
#else
        bEnforceIWYU = true;
#endif

        AddDefaultIncludePaths();

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "BlueprintGraph",
            "KismetCompiler",
            "Kismet",
            "EditorSubsystem",
            "EditorFramework",
            "UnrealEd",
            "Projects"
        });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "UMG",
            "Json",
            "JsonUtilities",
            "CavrnusConnector"
        });

        if (Target.Platform != UnrealTargetPlatform.Win64)
        {
            throw new Exception($"Unsupported platform {Target.Platform}");
        }
    }
}
