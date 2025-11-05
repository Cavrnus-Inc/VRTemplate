// Copyright (c) 2025 Cavrnus. All rights reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class CavrnusConnectorEditor : ModuleRules
{
    private void AddDefaultIncludePaths()
    {
        //Add Public dir for this module
        string PublicDirectory = Path.Combine(ModuleDirectory, "Public");
        if (Directory.Exists(PublicDirectory))
        {
            PublicIncludePaths.Add(PublicDirectory);
        }

        // Add the base private directory for this module
        string PrivateDirectory = Path.Combine(ModuleDirectory, "Private");
        if (Directory.Exists(PrivateDirectory))
        {
            PrivateIncludePaths.Add(PrivateDirectory);
        }
    }

    public CavrnusConnectorEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "WorkspaceMenuStructure",
            "UnrealEd", 
            "EditorFramework",
            "EditorSubsystem",
            "EditorStyle",
            "RenderCore",
            "BlueprintGraph",
            "LevelEditor",
            "Kismet",
            "KismetWidgets",
            "StaticMeshEditor",
            "BlueprintGraph",
            "KismetCompiler",
            "EditorScriptingUtilities"
        });
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseRTTI = true;
        bEnableExceptions = true;
#if UE_5_2_OR_LATER
        IWYUSupport = IWYUSupport.Full;
#else
        bEnforceIWYU = true;
#endif
        AddDefaultIncludePaths();

        PublicDependencyModuleNames.AddRange(
        new string[]
        {
            "WorkspaceMenuStructure",
            "UnrealEd",
            "EditorFramework",
            "EditorSubsystem",
            "EditorStyle",
            "RenderCore",
            "BlueprintGraph",
            "LevelEditor",
            "Core",
            "CoreUObject",
            "EnhancedInput",
            "Engine",
            "UMG",
            "UMGEditor",
            "Slate",
            "SlateCore",
            "AssetTools",
            "ToolMenus",
            "SubobjectDataInterface",
            "CavrnusConnector",
            "Blutility",
            "Projects",
            "Json",
            "JsonUtilities",
            "HTTP",
                }
        );

        //Add include path
        string Platform = string.Empty;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            Platform = "Win64";
        }
        else
        {
            throw new Exception(string.Format("Unsupported platform {0}", Target.Platform.ToString()));
        }
    }
}
