// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusCVT.h"
#include "UI/CavrnusCVTToolbarButtonRegistry.h"
#include "Bookmarks/CavrnusBookmarkMode.h"
#include "Bookmarks/CavrnusBookmarkManager.h"
#include "CavrnusConnector/Public/CavrnusLog.h"
#include "CavrnusConnector/Public/UI/Systems/Panels/Types/Toolbar/CavrnusToolbarPanelWidget.h"
#include "CavrnusConnector/Public/Core/Subsystems/CavrnusSubsystem.h"
#include "CavrnusConnector/Public/Core/Contexts/CavrnusRuntimeContext.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/App.h"
#include "Misc/CoreDelegates.h"
#include "Interfaces/IPluginManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#define LOCTEXT_NAMESPACE "CavrnusCVT"

IMPLEMENT_MODULE(FCavrnusCVT, CavrnusCVT)
DEFINE_LOG_CATEGORY(LogCavrnusCVT);

// Static function to register all button factories - called by registry when it initializes
static void RegisterAllButtonFactories(UCavrnusCVTToolbarButtonRegistry* Registry)
{
    if (!Registry)
        return;

    // Register button factories from mode classes
    Registry->RegisterButtonFactory(
        UCavrnusBookmarkMode::GetButtonName(),
        [](UCavrnusToolbarPanelWidget* ToolbarWidget) -> UCavrnusUIToggleButton*
        {
            return UCavrnusBookmarkMode::CreateToolbarButton(ToolbarWidget);
        },
        UCavrnusBookmarkMode::GetDefaultInsertIndex()
    );
    
    // Future button registrations will go here:
    // Registry->RegisterButtonFactory("FileImport", [](UCavrnusToolbarPanelWidget* Widget) { return UCavrnusFileImportMode::CreateToolbarButton(Widget); }, 1);
    // Registry->RegisterButtonFactory("MeasurementTool", [](UCavrnusToolbarPanelWidget* Widget) { return UCavrnusMeasurementToolMode::CreateToolbarButton(Widget); }, 2);
    // etc.
    
    UE_LOG(LogCavrnusCVT, Log, TEXT("Registered toolbar button factories"));
}

FCavrnusCVT::FCavrnusCVT() {}
FCavrnusCVT::~FCavrnusCVT() {}

void FCavrnusCVT::MountCustomContentFolder()
{
    // Path to your module-specific content folder
    FString ContentDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("CavrnusConnector"))->GetBaseDir(), TEXT("Source/CavrnusCVT/Content"));

    // Mount it under a custom virtual path
    FString VirtualMountPoint = TEXT("/CavrnusCVT");

    FPackageName::RegisterMountPoint(VirtualMountPoint, ContentDir);

    // Optionally tell the AssetRegistry to rescan
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().ScanPathsSynchronous({ ContentDir }, true);
}

void FCavrnusCVT::StartupModule()
{
    IModuleInterface::StartupModule();
    UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusCVT module active."));

    // Set the static registration function so the registry can call it when it initializes
    UCavrnusCVTToolbarButtonRegistry::SetFactoryRegistrationCallback(RegisterAllButtonFactories);

    // Helper lambda to start the registration ticker
    auto StartRegistrationTicker = [this]()
    {
        // Check if service is already registered before starting a ticker
        UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
        if (Subsystem && Subsystem->RuntimeContext)
        {
            UCavrnusServiceLocator* ServiceLocator = Subsystem->RuntimeContext->GetServiceLocator();
            if (ServiceLocator && ServiceLocator->Get<UCavrnusCVTToolbarButtonRegistry>())
            {
                // Service already registered, no need to start ticker
                return;
            }
        }

        // Only start a new ticker if we don't already have one running
        if (!TickerHandle.IsValid())
        {
            TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
                FTickerDelegate::CreateLambda([this](float DeltaTime) -> bool
                {
                    if (TryRegisterToolbarButtonFactories())
                    {
                        // Successfully registered, stop ticking
                        TickerHandle.Reset();
                        return false; // Stop ticking
                    }
                    return true; // Continue ticking
                })
            );
        }
    };

#if WITH_EDITOR
    // Hook into PIE start event so we register the service on each PIE session
    FEditorDelegates::PostPIEStarted.AddLambda([StartRegistrationTicker](bool bIsSimulating)
    {
        // Start ticker to wait for RuntimeContext to be ready
        StartRegistrationTicker();
    });
#endif

    // Also try to register on engine init (for non-PIE scenarios)
    FCoreDelegates::OnPostEngineInit.AddLambda([StartRegistrationTicker]()
    {
        // Use a ticker to poll for RuntimeContext availability
        // RuntimeContext is initialized asynchronously in UCavrnusSubsystem::OnAppStart
        StartRegistrationTicker();
    });
    MountCustomContentFolder();
}

void FCavrnusCVT::ShutdownModule()
{
    if (TickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
        TickerHandle.Reset();
    }
    
    // Clear the registration callback
    UCavrnusCVTToolbarButtonRegistry::SetFactoryRegistrationCallback(nullptr);
    
    IModuleInterface::ShutdownModule();
}

bool FCavrnusCVT::TryRegisterToolbarButtonFactories()
{
    UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
    if (!Subsystem || !Subsystem->RuntimeContext)
    {
        // RuntimeContext not ready yet, will retry
        return false;
    }

    // Register the CVT toolbar button registry service
    // The registry will automatically register factories in its Initialize() method
    if (UCavrnusServiceLocator* ServiceLocator = Subsystem->RuntimeContext->GetServiceLocator())
    {
        ServiceLocator->RegisterService<UCavrnusCVTToolbarButtonRegistry>();
        
        // Register bookmark manager service
        ServiceLocator->RegisterService<UCavrnusBookmarkManager>();
    }

    UCavrnusCVTToolbarButtonRegistry* Registry = Subsystem->RuntimeContext->Get<UCavrnusCVTToolbarButtonRegistry>();
    if (!Registry)
    {
        // Registry not available yet, will retry
        return false;
    }

    // Factories are now registered automatically in the registry's Initialize() method
    return true; // Success
}

#undef LOCTEXT_NAMESPACE