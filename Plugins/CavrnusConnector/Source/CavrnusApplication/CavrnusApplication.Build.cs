// Copyright (c) 2025 Cavrnus. All rights reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class CavrnusApplication: ModuleRules
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
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Archive"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Archive/ZipUtilities"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Archive/ZipUtilities/ThirdParty"));
    }

    public CavrnusApplication(ReadOnlyTargetRules Target) : base(Target)
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



        // Editor-only dependencies
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "StaticMeshDescription",
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
            "XmlParser"

        });

        // Editor-only dependencies
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "Kismet",
            });
            PublicDependencyModuleNames.AddRange(new string[]
            {
                "SequencerScripting",
            });
        }

        if (Target.Platform != UnrealTargetPlatform.Win64)
        {
            throw new Exception($"Unsupported platform {Target.Platform}");
        }
    }
}
