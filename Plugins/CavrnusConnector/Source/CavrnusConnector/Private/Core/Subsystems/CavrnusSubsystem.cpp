// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Core/Subsystems/CavrnusSubsystem.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusConnectorSettings.h"
#include "Core/CavrnusAppLifecycleHandler.h"
#include "Core/Contexts/CavrnusEditorContext.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "Managers/CavrnusEditorAuthenticationManager.h"
#include "Managers/Login/CavrnusLoginManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Blueprint.h"
#include "Modes/CavrnusModeManager.h"
#include "Modes/CavrnusExploreMode.h"

#if WITH_EDITOR && ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
#include "Misc/MessageDialog.h"
#endif

#if WITH_EDITOR
#include "Editor.h"
#include "Editor/EditorEngine.h"
#endif

UCavrnusSubsystem* UCavrnusSubsystem::Get()
{
	if (GEngine)
		return GEngine->GetEngineSubsystem<UCavrnusSubsystem>();
	
	return nullptr;
}

void UCavrnusSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (bInitialized)
		return;

	CreateEditorContext();
	
	AppHandler = NewObject<UCavrnusAppLifecycleHandler>(this);
	AppHandler->AwaitAppStart([this](const bool IsEditor, UWorld* World)
	{
		OnAppStart(IsEditor, World);
	});
	AppHandler->AwaitAppEnd([this](const bool IsEditor)
	{
		OnAppEnd(IsEditor);
	});

	bInitialized = true;
}

void UCavrnusSubsystem::DisposeEditorCtx()
{
	if (EditorContext)
	{
		EditorContext->Dispose();
		EditorContext = nullptr;
	}
	
	UE_LOG(LogTemp, Log, TEXT("DisposeEditorCtx called!"));
}

void UCavrnusSubsystem::DisposeRuntimeCtx()
{
	if (RuntimeContext)
	{
		RuntimeContext->Dispose();
		RuntimeContext = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("DisposeRuntimeCtx called!"));
}

void UCavrnusSubsystem::Deinitialize()
{
	if (IsValid(AppHandler))
	{
		AppHandler->Dispose();
		AppHandler = nullptr;
	}
	
	DisposeEditorCtx();
	DisposeRuntimeCtx();
	
	Super::Deinitialize();
}

void UCavrnusSubsystem::OnAppStart(const bool IsEditor, UWorld* World)
{
	UE_LOG(LogTemp, Log, TEXT("CavrnusSubsystem::OnAppStart called - IsEditor: %s, World: %s"), 
		IsEditor ? TEXT("true") : TEXT("false"), 
		World ? *World->GetName() : TEXT("nullptr"));
	
	CreateRuntimeContext(World);

	if (!IsRuntimeContextReady())
	{
		UE_LOG(LogTemp, Error, TEXT("CavrnusSubsystem::OnAppStart - RuntimeContext creation failed!"));
		return;
	}

	if (IsEditor)
	{
		if (const auto* Auth = EditorContext->Get<UCavrnusEditorAuthenticationManager>())
		{
			if (Auth->HasEditorAuthenticated())
			{
				UE_LOG(LogCavrnusConnector, Log, TEXT("[Subsystem] OnAppStart -- PIE mode, editor authenticated, triggering DoPieLogin"));
				if (auto* LoginManager = RuntimeContext->Get<UCavrnusLoginManager>())
				{
					LoginManager->DoPieLogin();
				}
				else
				{
					UE_LOG(LogCavrnusConnector, Error, TEXT("[Subsystem] OnAppStart - LoginManager not available"));
				}
			}
			else if (UCavrnusConnectorSettings::Get()->ConnectOnStart)
			{
				UE_LOG(LogCavrnusConnector, Log, TEXT("[Subsystem] OnAppStart -- PIE mode, not editor-authenticated, ConnectOnStart=true, triggering DoPluginSettingsLogin"));
				if (auto* LoginManager = RuntimeContext->Get<UCavrnusLoginManager>())
				{
					LoginManager->DoPluginSettingsLogin();
				}
				else
				{
					UE_LOG(LogCavrnusConnector, Error, TEXT("[Subsystem] OnAppStart - LoginManager not available"));
				}
			}
			else
			{
				UE_LOG(LogCavrnusConnector, Log, TEXT("[Subsystem] OnAppStart -- PIE mode, not editor-authenticated, ConnectOnStart=false, awaiting Blueprint login call"));
			}
		}
	}
	else
	{
		UE_LOG(LogCavrnusConnector, Log, TEXT("[Subsystem] OnAppStart -- Runtime mode, ConnectOnStart: %s, AuthMethod: %s"),
			UCavrnusConnectorSettings::Get()->ConnectOnStart ? TEXT("true") : TEXT("false"),
			*UEnum::GetValueAsString(UCavrnusConnectorSettings::Get()->AuthMethod));

		if (UCavrnusConnectorSettings::Get()->ConnectOnStart)
		{
			UE_LOG(LogCavrnusConnector, Log, TEXT("[Subsystem] OnAppStart -- Runtime mode, ConnectOnStart=true, triggering DoPluginSettingsLogin"));
			if (auto* LoginManager = RuntimeContext->Get<UCavrnusLoginManager>())
			{
				LoginManager->DoPluginSettingsLogin();
			}
			else
			{
				UE_LOG(LogCavrnusConnector, Error, TEXT("[Subsystem] OnAppStart - LoginManager not available"));
			}
		}
		else
		{
			UE_LOG(LogCavrnusConnector, Log, TEXT("[Subsystem] OnAppStart -- Runtime mode, ConnectOnStart=false, awaiting Blueprint login call"));
		}
	}
}

void UCavrnusSubsystem::OnAppEnd(const bool IsEditor)
{
	DisposeRuntimeCtx();
}

void UCavrnusSubsystem::CreateEditorContext()
{
	if (IsValid(EditorContext))
	{
		EditorContext->Dispose();
		EditorContext = nullptr;
	}
	EditorContext = NewObject<UCavrnusEditorContext>(this);
	EditorContext->Initialize();
}

void UCavrnusSubsystem::CreateRuntimeContext(UWorld* World)
{
	UE_LOG(LogTemp, Log, TEXT("CavrnusSubsystem::CreateRuntimeContext called - World: %s"), 
		World ? *World->GetName() : TEXT("nullptr"));
	
	DisposeRuntimeCtx();
	RuntimeContext = NewObject<UCavrnusRuntimeContext>(this);
	if (RuntimeContext)
	{
		RuntimeContext->Initialize(World);
		UE_LOG(LogTemp, Log, TEXT("CavrnusSubsystem::CreateRuntimeContext - RuntimeContext initialized successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CavrnusSubsystem::CreateRuntimeContext - Failed to create RuntimeContext!"));
	}
}
