// Copyright (c) 2025 Cavrnus. All rights reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class CavrnusCVT: ModuleRules
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

    public CavrnusCVT(ReadOnlyTargetRules Target) : base(Target)
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
            "CavrnusApplication"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd"
            });
        }

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "CavrnusConnector",
            "Core",
            "CoreUObject",
            "Engine",
            "UMG",
            "Networking",
            "Sockets",
            "JsonBlueprintUtilities",
            "Projects",
            "DeveloperSettings",
            "HTTP",            
            "ImageWrapper",
            "DatasmithRuntime",
            "zlib",
            "EnhancedInput",
            "Json",
            "JsonUtilities",
            "LevelSequence",
            "MovieScene",
            "SequencerScripting",
            "XmlParser",
            "Projects",
        });

        if (Target.Platform != UnrealTargetPlatform.Win64)
        {
            throw new Exception($"Unsupported platform {Target.Platform}");
        }
    }
}
