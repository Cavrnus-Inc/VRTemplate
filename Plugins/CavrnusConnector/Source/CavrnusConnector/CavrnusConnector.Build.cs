// Copyright (c) 2025 Cavrnus. All rights reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class CavrnusConnector : ModuleRules
{

    public string[] ProtobufLibRelease
    {
        get
        {
            string[] relevantLibFiles = {                
                "libprotobuf.lib"
            };

            return relevantLibFiles;
        }
    }

    private string ThirdPartyPath
    {
        get { return Path.Combine(PluginDirectory, "Source", "ThirdParty"); }
    }

    private string ProtobufPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "protobuf")); }
    }
	
    private string CollaborationSystemPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "Collab")); }
    }

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

        // Add the directory with the generated protos
        string CommDirectory = Path.Combine(PrivateDirectory, "Comm");
        if (Directory.Exists(CommDirectory))
        { 
            PrivateIncludePaths.Add(CommDirectory);
        }

        // Add the include directory for CavrnusRelay
        string RelayDirectory = Path.Combine(PrivateDirectory, "Relay");
        if (Directory.Exists(RelayDirectory))
        {
            PrivateIncludePaths.Add(RelayDirectory);
        }

        // Add the include directory for Protobuf 
        string ProtobufIncludeDirectory = Path.Combine(ProtobufPath, "include");
        if (Directory.Exists(ProtobufIncludeDirectory))
        {
            PrivateIncludePaths.Add(ProtobufIncludeDirectory);
        }
    }

    public CavrnusConnector(ReadOnlyTargetRules Target) : base(Target)
    {
        Console.WriteLine("====== Entering CavrnusConnector.Build.cs ====== ");
        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "AssetRegistry",
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
            "Core",
            "CoreUObject",
            "Engine",
            "UMG",
            "InputCore",
            "Slate",
            "SlateCore",
            "RenderCore",
            "RHI",
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
//#if UE_5_4_OR_LATER
//            "ImageCore",
//#endif
        }
        );
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Archive"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Archive/ZipUtilities"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Archive/ZipUtilities/ThirdParty"));

        PublicIncludePathModuleNames.AddRange(
        new string[]
        {
            "Settings",
        }
        );
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[] { "UnrealEd", "BlueprintGraph", "KismetCompiler","DesktopPlatform" }
            );
        }

        //Add include path
        string Platform = string.Empty;
        string CollabPlatform = string.Empty;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            Platform = "Win64";
            CollabPlatform = "win-x64";
        }
        else
        {
            throw new Exception(string.Format("Unsupported platform {0}", Target.Platform.ToString()));
        }

		string ProtobufLibraryDirectory = Path.Combine(ProtobufPath, "lib");
        foreach (string libFileName in ProtobufLibRelease)
        {
            string FullPathToLib = "";
            if (Target.WindowsPlatform.Compiler == WindowsCompiler.VisualStudio2022)
            {
                FullPathToLib = Path.Combine(ProtobufLibraryDirectory, Platform, "v143", libFileName);
            }
#if !UE_5_4_OR_LATER
            else if (Target.WindowsPlatform.Compiler == WindowsCompiler.VisualStudio2019)
            {
                FullPathToLib = Path.Combine(ProtobufLibraryDirectory, Platform, "v142", libFileName);
            }
#endif
            else
            {
                throw new Exception("Unknown compiler; libraries not selected");
            }

            if (!File.Exists(FullPathToLib))
            {
                throw new Exception(string.Format("File not found {0}", FullPathToLib));
            }

            PublicAdditionalLibraries.Add(FullPathToLib);
        }

        RuntimeDependencies.Add(Path.Combine(CollaborationSystemPath, CollabPlatform, "..."));

        Console.WriteLine("====== Exiting CavrnusConnector.Build.cs ====== ");
    }
}
