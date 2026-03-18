// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusFunctionLibrary.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusConnectorSettings.h"
#include <Containers/Ticker.h>
#include <Engine/Engine.h>
#include <Engine/GameViewportClient.h>
#include <GameFramework/PlayerController.h>
#include <HAL/FileManager.h>
#include <Misc/Paths.h>
#include "Types/CavrnusCallbackTypes.h"
#include "LivePropertyUpdates/CavrnusLiveBoolPropertyUpdate.h"
#include "LivePropertyUpdates/CavrnusLiveColorPropertyUpdate.h"
#include "LivePropertyUpdates/CavrnusLiveFloatPropertyUpdate.h"
#include "LivePropertyUpdates/CavrnusLiveStringPropertyUpdate.h"
#include "LivePropertyUpdates/CavrnusLiveTransformPropertyUpdate.h"
#include "LivePropertyUpdates/CavrnusLiveVectorPropertyUpdate.h"
#include "Types\CavrnusPropertyValue.h"
#include "RelayModel\CavrnusRelayModel.h"
#include "RelayModel\RelayCallbackModel.h"
#include "RelayModel\DataState.h"
#include "RelayModel\SpacePropertyModel.h"
#include "Types\AbsolutePropertyId.h"
#include "RelayModel\SpacePermissionsModel.h"
#include "Translation\CavrnusProtoTranslation.h"

#include "RestAPI/CavrnusRestApiClient.h"
#include "RelayModel/CavrnusBindingModel.h"
#include "Managers/SpawnedObjects/SpawnObjectHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "Managers/SpawnedObjects/CavrnusPendingSpawnObject.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "UObject/UnrealType.h"
#include "CavrnusGCManager.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Managers/Login/CavrnusLoginConfig.h"
#include "Managers/Login/CavrnusLoginManager.h"
#include "Pawns/CavrnusPawnManager.h"
#include "UI/CavrnusUI.h"
#include "Abstract/FileImporter/CavrnusLoaderRegistry_Abstract.h"

#pragma region Authentication

// ============================================
// Authentication Functions
// ============================================

void UCavrnusFunctionLibrary::SetForceKeepAlive()
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildSetForceKeepAlive());
}

void UCavrnusFunctionLibrary::EndForceKeepAlive()
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildEndForceKeepAlive());
}

FDelegateHandle UCavrnusFunctionLibrary::UiFlowTeardownHandle;


void UCavrnusFunctionLibrary::SetupUiFlowManager(const FCavrnusLoginConfig& LoginConfig)
{
	UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
	if (!Subsystem || !Subsystem->IsRuntimeContextReady())
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[SetupUiFlowManager] RuntimeContext not ready"));
		return;
	}

	UCavrnusLoginManager* LoginManager = Subsystem->RuntimeContext->Get<UCavrnusLoginManager>();
	if (!LoginManager)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[SetupUiFlowManager] LoginManager not available"));
		return;
	}

	if (LoginManager->HasLoginBeenInitiated())
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[SetupUiFlowManager] Login has already been initiated."))
		return;
	}

	LoginManager->DoLogin(LoginConfig);
}

void UCavrnusFunctionLibrary::CavrnusLogin()
{
	FCavrnusLoginConfig Config = FCavrnusLoginConfig::FromPluginSettings();
	SetupUiFlowManager(Config);
}

void UCavrnusFunctionLibrary::CavrnusLoginAsMember(const FString& Server, const FString& Email, const FString& Password, const FString& SpaceId)
{
	FCavrnusLoginConfig Config = FCavrnusLoginConfig::ForMember(Server, Email, Password, SpaceId);
	SetupUiFlowManager(Config);
}

void UCavrnusFunctionLibrary::CavrnusLoginAsGuest(const FString& Server, const FString& GuestName, const FString& SpaceId)
{
	FCavrnusLoginConfig Config = FCavrnusLoginConfig::ForGuest(Server, GuestName, SpaceId);
	SetupUiFlowManager(Config);
}

void UCavrnusFunctionLibrary::CavrnusLoginAllowBoth(const FString& Server, const FString& SpaceId, ECavrnusPreferredLoginTab PreferredTab, const FString& MemberEmail, const FString& MemberPassword, const FString& GuestName)
{
	UCavrnusConnectorSettings::Get()->PreferredLoginTab = PreferredTab;
	FCavrnusLoginConfig Config = FCavrnusLoginConfig::ForAllowBoth(Server, SpaceId, GuestName, MemberEmail, MemberPassword);
	SetupUiFlowManager(Config);
}

// --- Deprecated functions (still functional) ---

void UCavrnusFunctionLibrary::CavrnusLoginMemberFlow(const FString& OptionalServer)
{
	FCavrnusLoginConfig Config = FCavrnusLoginConfig();
	Config.AuthMethod = ECavrnusAuthMethod::JoinAsMember;
	Config.Server = OptionalServer;
	
	SetupUiFlowManager(Config);
}

void UCavrnusFunctionLibrary::CavrnusLoginGuestFlow(const FString& OptionalServer)
{
	FCavrnusLoginConfig Config = FCavrnusLoginConfig();
	Config.AuthMethod = ECavrnusAuthMethod::JoinAsGuest;
	Config.Server = OptionalServer;
	
	SetupUiFlowManager(Config);
}

void UCavrnusFunctionLibrary::CavrnusLoginMemberFlowWithConfig(const FString& OptionalServer, const FString& OptionalEmail, const FString& OptionalPassword, const FString& OptionalSpaceToAutoJoin)
{
	FCavrnusLoginConfig Config = FCavrnusLoginConfig();
	Config.AuthMethod = ECavrnusAuthMethod::JoinAsMember;
	Config.Server = OptionalServer;
	Config.MemberLoginEmail = OptionalEmail;
	Config.MemberLoginPassword = OptionalPassword;
	Config.SpaceJoinId = OptionalSpaceToAutoJoin;
	
	SetupUiFlowManager(Config);
}

void UCavrnusFunctionLibrary::CavrnusLoginGuestFlowWithConfig(const FString& OptionalServer, const FString& OptionalGuestName, const FString& OptionalSpaceToAutoJoin)
{
	FCavrnusLoginConfig Config = FCavrnusLoginConfig();
	Config.AuthMethod = ECavrnusAuthMethod::JoinAsGuest;
	Config.Server = OptionalServer;
	Config.GuestName = OptionalGuestName;
	Config.SpaceJoinId = OptionalSpaceToAutoJoin;
	
	SetupUiFlowManager(Config);
}

void UCavrnusFunctionLibrary::CavrnusLoginGlobalSettingsFlow()
{
	UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
	if (!Subsystem || !Subsystem->IsRuntimeContextReady())
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusLoginGlobalSettingsFlow - RuntimeContext not ready"));
		return;
	}

	UCavrnusLoginManager* LoginManager = Subsystem->RuntimeContext->Get<UCavrnusLoginManager>();
	if (!LoginManager)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusLoginGlobalSettingsFlow - LoginManager not available"));
		return;
	}

	LoginManager->DoPluginSettingsLogin();
}

bool UCavrnusFunctionLibrary::HooksSetUp = false;
static bool bSpaceJoinBound = false;

void UCavrnusFunctionLibrary::SetupCavrnusEventHooks()
{
	// Always register the shutdown hook first — it only needs FWorldDelegates,
	// not the viewport.  This ensures KillDataModel runs even when the viewport
	// isn't ready yet (e.g. early PIE frames).
	UCavrnusFunctionLibrary::HookCavrnusShutdown();

	// BindSpaceJoin doesn't need the viewport — register it once, early, so
	// pawn management and user callbacks work even when viewport is deferred.
	if (!bSpaceJoinBound)
	{
		BindSpaceJoin();
		bSpaceJoinBound = true;
	}

	if (HooksSetUp)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("Event hooks have already been established for Cavrnus. This only needs to happen once."));
		return;
	}

	if (!GEngine)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("SetupCavrnusEventHooks aborted: GEngine is null."));
		return;
	}

	UGameViewportClient* Viewport = GEngine->GameViewport;
	if (!Viewport)
	{
		UE_LOG(LogCavrnusConnector, Verbose, TEXT("SetupCavrnusEventHooks: GameViewport not yet available -- deferring object hooks until viewport is ready."));
		FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([](float) -> bool
			{
				if (GEngine && GEngine->GameViewport)
				{
					SetupCavrnusEventHooks();
					return false; // stop ticking
				}
				return true; // keep ticking
			}), 0.0f);
		return;
	}

	HooksSetUp = true;

	auto GetSafeWorld = [Viewport]() -> UWorld*
		{
			return Viewport ? Viewport->GetWorld() : nullptr;
		};

	// Creation callback: try loader (async) first, fall back to new spawn system
	TFunction<AActor* (FCavrnusSpawnedObject, FString)> OnObjectCreation =
		[GetSafeWorld](const FCavrnusSpawnedObject& Ob, const FString& UniqueId) -> AActor*
	{
		UWorld* World = GetSafeWorld();
		if (!World)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("OnObjectCreation: world is null, cannot handle object %s."), *UniqueId);
			return nullptr;
		}

		UE_LOG(LogCavrnusConnector, Warning, TEXT("OnObjectCreation: received id=%s"), *UniqueId);

		// Check if this object already exists in SpawnedObjects (prevents duplicates)
		// This handles the case where CavrnusSpawnActorById already created a pending spawn
		Cavrnus::SpacePropertyModel* PropertyModel = 
			Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(Ob.SpaceConnection);
		if (PropertyModel && PropertyModel->SpawnedObjects.Contains(Ob.PropertiesContainerName))
		{
			UE_LOG(LogCavrnusConnector, Log, TEXT("OnObjectCreation: Object %s already exists in SpawnedObjects, skipping duplicate creation"), *Ob.PropertiesContainerName);
			return nullptr; // Already handled
		}

		UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
		if (!Subsystem || !Subsystem->IsRuntimeContextReady())
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("OnObjectCreation: RuntimeContext not ready"));
			return nullptr;
		}
		
		USpawnedObjectsManager* Manager = Subsystem->RuntimeContext->Get<USpawnedObjectsManager>();
		if (!Manager)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("OnObjectCreation: SpawnedObjectsManager is invalid"));
			return nullptr;
		}

		// Try async loader via registry first
		UCavrnusBaseLoader_Abstract* CandidateLoader =
			FCavrnusLoaderRegistry_Abstract::Get().CreateMatchingLoader(UniqueId, GetTransientPackage());
		if (CandidateLoader)
		{
			UE_LOG(LogCavrnusConnector, Log, TEXT("OnObjectCreation: dispatching async loader %s for id %s"), *CandidateLoader->GetClass()->GetName(), *UniqueId);
			if (Manager)
			{
				Manager->RegisterSpawnedObjectAsync(Ob, UniqueId, World);
			}
			return nullptr; // Async path: no actor is available synchronously
		}

		// No loader found -> try new DataAsset-based spawn system
		TSubclassOf<AActor> ActorClass = nullptr;
		UStaticMesh* StaticMesh = nullptr; // Unused, kept for function signature compatibility
		TSubclassOf<UObject> ObjectClass = nullptr;
		
		// Check for Actor class (matching Unreal Engine's SpawnActor - only AActor classes)
		if (Manager->FindSpawnableClassOrMesh(UniqueId, nullptr, ActorClass, StaticMesh))
		{
			if (ActorClass)
			{
				// Use new pending spawn system for Actors
				ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				UCavrnusPendingSpawnObject* PendingSpawn = Manager->CreatePendingSpawnObjectWithActorClass(Ob, UniqueId, ActorClass, CollisionHandling);
				
				if (PendingSpawn)
				{
					PendingSpawn->Initialize();
					return nullptr; // Pending spawn, no actor yet
				}
			}
		}
		// Check for UObject class (for ConstructObjectFromClass)
		else if (Manager->FindSpawnableObjectClass(UniqueId, nullptr, ObjectClass))
		{
			// Use new pending construct system for UObjects
			UCavrnusPendingSpawnObject* PendingSpawn = Manager->CreatePendingConstructObject(Ob, UniqueId, ObjectClass);
			
			if (PendingSpawn)
			{
				PendingSpawn->Initialize();
				return nullptr; // Pending construct, no object yet
			}
		}

		// Fall back to old system for backwards compatibility
		UE_LOG(LogCavrnusConnector, Warning, TEXT("OnObjectCreation: no loader or DataAsset entry for %s, falling back to old spawn system."), *UniqueId);
		if (Manager)
		{
			return Manager->RegisterSpawnedObject(Ob, UniqueId, World);
		}
		UE_LOG(LogCavrnusConnector, Error, TEXT("OnObjectCreation: Manager not available for fallback"));
		return nullptr;
	};

	Cavrnus::CavrnusRelayModel::GetDataModel()->RegisterObjectCreationCallback(OnObjectCreation);

	// Object destruction callback remains synchronous
	TFunction<void(FCavrnusSpawnedObject)> OnObjectDestruction =
		[GetSafeWorld](const FCavrnusSpawnedObject& Ob)
		{
			UWorld* World = GetSafeWorld();
			if (!World)
			{
				UE_LOG(LogCavrnusConnector, Warning, TEXT("OnObjectDestruction: world is null, unable to unregister object %s."), *Ob.PropertiesContainerName);
				return;
			}

			UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
			if (!Subsystem || !Subsystem->IsRuntimeContextReady())
			{
				UE_LOG(LogCavrnusConnector, Warning, TEXT("OnObjectDestruction: RuntimeContext not ready, unable to unregister object %s."), *Ob.PropertiesContainerName);
				return;
			}

			USpawnedObjectsManager* Manager = Subsystem->RuntimeContext->Get<USpawnedObjectsManager>();
			if (Manager)
			{
				Manager->UnregisterSpawnedObject(Ob, World);
			}
			else
			{
				UE_LOG(LogCavrnusConnector, Error, TEXT("OnObjectDestruction: SpawnedObjectsManager not available"));
			}
		};

	Cavrnus::CavrnusRelayModel::GetDataModel()->RegisterObjectDestructionCallback(OnObjectDestruction);

	UE_LOG(LogCavrnusConnector, Log, TEXT("Cavrnus event hooks set up successfully (async-first)."));
}

bool UCavrnusFunctionLibrary::IsLoggedIn()
{
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->CurrentAuthentication != nullptr;
}

void UCavrnusFunctionLibrary::SetServer(const FString& Server)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->SetServer(Server);
}

const FString& UCavrnusFunctionLibrary::GetServer()
{
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->CurrentServer;
}

void UCavrnusFunctionLibrary::AwaitServerSet(FCavrnusServerSet OnServerSet)
{
	CavrnusServerSet callback = [OnServerSet](const FString& val)
		{
			OnServerSet.ExecuteIfBound(val);
		};
	AwaitServerSet(callback);
}

void UCavrnusFunctionLibrary::AwaitServerSet(CavrnusServerSet OnServerSet)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterServerSetCallback(OnServerSet);
}

void UCavrnusFunctionLibrary::AuthenticateWithPassword(const FString& Server, const FString& Email, const FString& Password, FCavrnusAuthRecv OnSuccess, FCavrnusError OnFailure)
{
	CavrnusAuthRecv callback = [OnSuccess](const FCavrnusAuthentication& val)
	{
		OnSuccess.ExecuteIfBound(val);
	};
	CavrnusError errorCallback = [OnFailure](const FString& error)
	{
		OnFailure.ExecuteIfBound(error);
	};
	AuthenticateWithPassword(Server, Email, Password, callback, errorCallback);
}

void UCavrnusFunctionLibrary::AuthenticateWithPassword(const FString& Server, const FString& Email, const FString& Password, CavrnusAuthRecv OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterAuthenticationCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildAuthenticateWithPassword(RequestId, Server, Email, Password));
}

void UCavrnusFunctionLibrary::AuthenticateAsGuest(const FString& Server, const FString& UserName, FCavrnusAuthRecv OnSuccess, FCavrnusError OnFailure)
{
	CavrnusAuthRecv callback = [OnSuccess](const FCavrnusAuthentication& val)
	{
		OnSuccess.ExecuteIfBound(val);
	};
	CavrnusError errorCallback = [OnFailure](const FString& error)
	{
		OnFailure.ExecuteIfBound(error);
	};
	AuthenticateAsGuest(Server, UserName, callback, errorCallback);
}

void UCavrnusFunctionLibrary::AuthenticateAsGuest(const FString& Server, const FString& UserName, CavrnusAuthRecv OnSuccess, CavrnusError OnFailure)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->bAuthenticatedAsGuest = true;
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterAuthenticationCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildAuthenticateGuest(RequestId, Server, UserName));
}


void UCavrnusFunctionLibrary::AuthenticateWithApiKey(const FString& Server, const FString& AccessKey, const FString& AccessToken, FCavrnusAuthRecv OnSuccess, FCavrnusError OnFailure)
{
	CavrnusAuthRecv callback = [OnSuccess](const FCavrnusAuthentication& val)
		{
			OnSuccess.ExecuteIfBound(val);
		};
	CavrnusError errorCallback = [OnFailure](const FString& error)
		{
			OnFailure.ExecuteIfBound(error);
		};
	AuthenticateWithApiKey(Server, AccessKey, AccessToken, callback, errorCallback);
}

void UCavrnusFunctionLibrary::AuthenticateWithApiKey(const FString& Server, const FString& AccessKey, const FString& AccessToken, CavrnusAuthRecv OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterAuthenticationCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildAuthenticateWithApiKey(RequestId, Server, AccessKey, AccessToken));
}

void UCavrnusFunctionLibrary::AuthenticateWithDeviceCodeBegin(const FString& Server, const FString& Source, const FString& CustomActivatedMessage, bool AutoOpenUrl, FCavrnusDeviceCodeBeginRecv OnSuccess, FCavrnusError OnFailure)
{
	CavrnusDeviceCodeRecv callback = [OnSuccess](const FCavrnusDeviceCodeData& val)
		{
			OnSuccess.ExecuteIfBound(val);
		};
	CavrnusError errorCallback = [OnFailure](const FString& error)
		{
			OnFailure.ExecuteIfBound(error);
		};
	AuthenticateWithDeviceCodeBegin(Server, Source, CustomActivatedMessage, AutoOpenUrl, callback, errorCallback);
}
void UCavrnusFunctionLibrary::AuthenticateWithDeviceCodeBegin(const FString& Server, const FString& Source, const FString& CustomActivatedMessage, bool AutoOpenUrl, CavrnusDeviceCodeRecv OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterDeviceCodeBeginCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildAuthenticateWithDeviceCodeBegin(RequestId, Server, Source, CustomActivatedMessage, AutoOpenUrl));

}

void UCavrnusFunctionLibrary::AuthenticateWithDeviceCodeConclude(const FString& Server, const FString& DeviceCode, const FString& UserCode, FCavrnusAuthRecv OnSuccess, FCavrnusError OnFailure)
{
	CavrnusAuthRecv callback = [OnSuccess](const FCavrnusAuthentication& val)
		{
			OnSuccess.ExecuteIfBound(val);
		};
	CavrnusError errorCallback = [OnFailure](const FString& error)
		{
			OnFailure.ExecuteIfBound(error);
		};
	AuthenticateWithDeviceCodeConclude(Server, DeviceCode, UserCode, callback, errorCallback);
}
void UCavrnusFunctionLibrary::AuthenticateWithDeviceCodeConclude(const FString& Server, const FString& DeviceCode, const FString& UserCode, CavrnusAuthRecv OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterAuthenticationCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildAuthenticateWithDeviceCodeConclude(RequestId, Server, DeviceCode, UserCode));
}

void UCavrnusFunctionLibrary::CreateApiKey(const FString& Name, FCavrnusApiKeyRecv OnSuccess, FCavrnusError OnFailure)
{
	CavrnusApiKeyRecv callback = [OnSuccess](const FCavrnusApiKeyData& val)
		{
			OnSuccess.ExecuteIfBound(val);
		};
	CavrnusError errorCallback = [OnFailure](const FString& error)
		{
			OnFailure.ExecuteIfBound(error);
		};
	CreateApiKey(Name, callback, errorCallback);
}
void UCavrnusFunctionLibrary::CreateApiKey(const FString& Name, CavrnusApiKeyRecv OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterConstructApiKeyCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildConstructApiKey(RequestId, Name));
}


void UCavrnusFunctionLibrary::AwaitAuthentication(FCavrnusAuthRecv OnAuth)
{
	CavrnusAuthRecv callback = [OnAuth](const FCavrnusAuthentication& val)
	{
		OnAuth.ExecuteIfBound(val);
	};
	AwaitAuthentication(callback);
}

void UCavrnusFunctionLibrary::AwaitAuthentication(CavrnusAuthRecv OnAuth)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterAuthCallback(OnAuth);
}

static void TryLoadLevel(const TSoftObjectPtr<UWorld>& Level);

static void DeauthenticateInternal(TSoftObjectPtr<UWorld> LevelToLoad)
{
	// Exit all active spaces
	if (Cavrnus::CavrnusRelayModel::IsAlive())
	{
		TArray<FCavrnusSpaceConnectionInfo> SpaceConnections = UCavrnusFunctionLibrary::GetCurrentSpaceConnections();
		for (const FCavrnusSpaceConnectionInfo& ConnInfo : SpaceConnections)
		{
			UCavrnusFunctionLibrary::ExitSpace(FCavrnusSpaceConnection(ConnInfo.SpaceConnectionId));
		}
	}

	// Kill the relay connection — a fresh login will re-create it
	Cavrnus::CavrnusRelayModel::KillDataModel();

	// Reset the login manager so a new login flow can be initiated
	if (UCavrnusSubsystem* Sub = UCavrnusSubsystem::Get())
	{
		if (Sub->IsRuntimeContextReady())
		{
			if (UCavrnusLoginManager* LoginManager = Sub->RuntimeContext->Get<UCavrnusLoginManager>())
			{
				LoginManager->ResetLoginState();
			}
		}
	}

	// Clear pawn spawners while the world is still alive (before any level load tears it down)
	if (UCavrnusSubsystem* PawnSub = UCavrnusSubsystem::Get())
	{
		if (PawnSub->IsRuntimeContextReady())
		{
			if (UCavrnusPawnManager* PawnManager = PawnSub->RuntimeContext->Get<UCavrnusPawnManager>())
				PawnManager->Clear();
		}
	}

	UE_LOG(LogCavrnusConnector, Log, TEXT("[UCavrnusFunctionLibrary::Deauthenticate] Exited all spaces, killed relay, reset login state. Fresh login required."));

	// Resolve level to load: parameter > setting > none
	TSoftObjectPtr<UWorld> ResolvedLevel = LevelToLoad;
	if (ResolvedLevel.IsNull())
	{
		if (const auto* Settings = UCavrnusConnectorSettings::Get())
		{
			if (Settings->bLoadLevelOnDeauthenticate)
				ResolvedLevel = Settings->DeauthenticateLevel;
		}
	}

	TryLoadLevel(ResolvedLevel);
}

UCavrnusConnectorSettings* UCavrnusFunctionLibrary::GetCavrnusSettings()
{
	return UCavrnusConnectorSettings::Get();
}

void UCavrnusFunctionLibrary::SetSpaceExitLevel(bool bEnabled, TSoftObjectPtr<UWorld> Level)
{
	UCavrnusConnectorSettings* Settings = UCavrnusConnectorSettings::Get();
	if (!Settings) return;

	Settings->bLoadLevelOnSpaceExit = bEnabled;
	Settings->SpaceExitLevel = Level;
}

void UCavrnusFunctionLibrary::SetDeauthenticateLevel(bool bEnabled, TSoftObjectPtr<UWorld> Level)
{
	UCavrnusConnectorSettings* Settings = UCavrnusConnectorSettings::Get();
	if (!Settings) return;

	Settings->bLoadLevelOnDeauthenticate = bEnabled;
	Settings->DeauthenticateLevel = Level;
}

void UCavrnusFunctionLibrary::Deauthenticate()
{
	DeauthenticateInternal(nullptr);
}

void UCavrnusFunctionLibrary::Deauthenticate(TSoftObjectPtr<UWorld> LevelToLoad)
{
	DeauthenticateInternal(LevelToLoad);
}

void UCavrnusFunctionLibrary::CheckServerStatus(const FString& Server, FCavrnusServerStatusRecv OnStatus)
{
	CavrnusServerStatusRecv callback = [OnStatus](const FCavrnusServerStatus& val)
	{
		OnStatus.ExecuteIfBound(val);
	};
	CheckServerStatus(Server, callback);
}

void UCavrnusFunctionLibrary::CheckServerStatus(const FString& Server, CavrnusServerStatusRecv OnStatus)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterServerStatusCallback(OnStatus);

	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildCheckServerStatus(RequestId, Server));

}

#pragma endregion

#pragma region Spaces

// ============================================
// Space Functions
// ============================================

void UCavrnusFunctionLibrary::FetchJoinableSpaces(FCavrnusAllSpacesInfoEvent onRecvCurrentJoinableSpaces)
{
	CavrnusAllSpacesInfoEvent callback = [onRecvCurrentJoinableSpaces](const TArray<FCavrnusSpaceInfo>& val)
	{
		onRecvCurrentJoinableSpaces.ExecuteIfBound(val);
	};
	FetchJoinableSpaces(callback);
}

void UCavrnusFunctionLibrary::FetchJoinableSpaces(CavrnusAllSpacesInfoEvent OnRecvCurrentJoinableSpaces)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterFetchAvailableSpacesCallback(OnRecvCurrentJoinableSpaces);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildFetchAvailableSpaces(RequestId));
}

void UCavrnusFunctionLibrary::FetchSpaceInfo(FString spaceId, FCavrnusSpaceInfoEvent OnRecvSpaceInfo)
{
	CavrnusSpaceInfoEvent callback = [OnRecvSpaceInfo](const FCavrnusSpaceInfo& val)
		{
			OnRecvSpaceInfo.ExecuteIfBound(val);
		};
	FetchSpaceInfo(spaceId, callback);
}

void UCavrnusFunctionLibrary::FetchSpaceInfo(FString spaceId, CavrnusSpaceInfoEvent OnRecvSpaceInfo)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterFetchSpaceInfoCallback(OnRecvSpaceInfo);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildFetchSpaceInfo(RequestId, spaceId));
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindJoinableSpaces(FCavrnusSpaceInfoEvent SpaceAdded, FCavrnusSpaceInfoEvent SpaceUpdated, FCavrnusSpaceInfoEvent SpaceRemoved)
{
	CavrnusSpaceInfoEvent added = [SpaceAdded](const FCavrnusSpaceInfo& val)
	{
		SpaceAdded.ExecuteIfBound(val);
	};
	CavrnusSpaceInfoEvent updated = [SpaceUpdated](const FCavrnusSpaceInfo& val)
	{
		SpaceUpdated.ExecuteIfBound(val);
	};
	CavrnusSpaceInfoEvent removed = [SpaceRemoved](const FCavrnusSpaceInfo& val)
	{
		SpaceRemoved.ExecuteIfBound(val);
	};

	return BindJoinableSpaces(added, updated, removed);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindJoinableSpaces(CavrnusSpaceInfoEvent SpaceAdded, CavrnusSpaceInfoEvent SpaceUpdated, CavrnusSpaceInfoEvent SpaceRemoved)
{
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->BindJoinableSpaces(SpaceAdded, SpaceUpdated, SpaceRemoved);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindSpaceInfoChanged(int32 ChangeMask, FCavrnusSpaceInfoChangedDelegate OnChanged)
{
	ESpaceInfoChangeFlags mask = static_cast<ESpaceInfoChangeFlags>(ChangeMask);
	CavrnusSpaceInfoChangedEvent callback = [OnChanged](const FCavrnusSpaceInfo& spaceInfo, ESpaceInfoChangeFlags changedFlags)
	{
		OnChanged.ExecuteIfBound(spaceInfo, static_cast<int32>(changedFlags));
	};

	return BindSpaceInfoChanged(mask, callback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindSpaceInfoChanged(ESpaceInfoChangeFlags ChangeMask, CavrnusSpaceInfoChangedEvent OnChanged)
{
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->BindSpaceInfoChanged(ChangeMask, OnChanged);
}

bool UCavrnusFunctionLibrary::IsConnectedToAnySpace()
{
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->GetCurrentSpaceConnections().Num() > 0;
}

TArray<FCavrnusSpaceConnectionInfo>& UCavrnusFunctionLibrary::GetCurrentSpaceConnections()
{
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->GetCurrentSpaceConnections();
}

void UCavrnusFunctionLibrary::GetCavrnusSpaceConnectionInfo(const FCavrnusSpaceConnection& SpaceConn, EValidInvalidOutputExecPins& Execs, FCavrnusSpaceConnectionInfo& SpaceConnectionInfo)
{
	FCavrnusSpaceConnectionInfo* Sci = GetCavrnusSpaceConnectionInfo(SpaceConn);
	if (Sci)
	{
		SpaceConnectionInfo = *Sci;
	}

	Execs = Sci ? EValidInvalidOutputExecPins::IsValid : EValidInvalidOutputExecPins::IsInvalid;
}

FCavrnusSpaceConnectionInfo* UCavrnusFunctionLibrary::GetCavrnusSpaceConnectionInfo(const FCavrnusSpaceConnection& SpaceConn)
{
	TArray<FCavrnusSpaceConnectionInfo>& SpaceConnections = Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->GetCurrentSpaceConnections();
	for (int i = 0; i < SpaceConnections.Num(); i++)
	{
		if (SpaceConnections[i].SpaceConnectionId == SpaceConn.SpaceConnectionId)
		{
			return &(SpaceConnections[i]);
		}
	}

	return nullptr;
}

void UCavrnusFunctionLibrary::JoinSpace(FString SpaceId, FCavrnusSpaceConnected OnConnected, FCavrnusError OnFailure)
{
	CavrnusSpaceConnected callback = [OnConnected](const FCavrnusSpaceConnection& val)
	{
		OnConnected.ExecuteIfBound(val);
	};
	CavrnusError errorCallback = [OnFailure](const FString& val)
	{
		OnFailure.ExecuteIfBound(val);
	};

	JoinSpace(SpaceId, callback, errorCallback);
}

void UCavrnusFunctionLibrary::JoinSpace(FString SpaceId, CavrnusSpaceConnected OnConnected, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterJoinSpaceCallback(OnConnected, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->HandleSpaceBeginLoading(SpaceId);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildJoinSpaceWithId(RequestId, SpaceId));
}


void UCavrnusFunctionLibrary::BindSpaceStatus(FCavrnusSpaceConnection space, FCavrnusSpaceStatusChanged OnStatus)
{
	CavrnusSpaceStatusChanged callback = [OnStatus](const FCavrnusConnectionStatus& status)
	{
		OnStatus.ExecuteIfBound(status);
	};

	BindSpaceStatus(space, callback);
}

void UCavrnusFunctionLibrary::BindSpaceStatus(FCavrnusSpaceConnection space, CavrnusSpaceStatusChanged OnStatus)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterSpaceStatusCallback(space.SpaceConnectionId, OnStatus);
}

void UCavrnusFunctionLibrary::BindSpaceServerMessages(FCavrnusSpaceConnection space, FCavrnusServerMessageEvent OnStatus)
{
	CavrnusServerMessageEvent callback = [OnStatus](const FCavrnusServerMessage& msg)
		{
			OnStatus.ExecuteIfBound(msg);
		};

	BindSpaceServerMessages(space, callback);
}

void UCavrnusFunctionLibrary::BindSpaceServerMessages(FCavrnusSpaceConnection space, CavrnusServerMessageEvent OnMessage)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterServerMessageCallback(space.SpaceConnectionId, OnMessage);
}


void UCavrnusFunctionLibrary::CreateSpace(FString SpaceName, FCavrnusSpaceCreated OnCreation, FCavrnusError OnFailure)
{
	CavrnusSpaceCreated callback = [OnCreation](const FCavrnusSpaceInfo& val)
	{
		OnCreation.ExecuteIfBound(val);
	};
	CavrnusError errorCallback = [OnFailure](const FString& val)
	{
		OnFailure.ExecuteIfBound(val);
	};

	CreateSpace(SpaceName, TArray<FString>(), callback, errorCallback);
}

void UCavrnusFunctionLibrary::CreateSpace(FString SpaceName, CavrnusSpaceCreated OnCreation, CavrnusError OnFailure)
{
	CreateSpace(SpaceName, TArray<FString>(), OnCreation, OnFailure);
}

void UCavrnusFunctionLibrary::CreateSpace(FString SpaceName, TArray<FString> Keywords, CavrnusSpaceCreated OnCreation, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterCreateSpaceCallback(OnCreation, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildCreateSpaceMsg(RequestId, SpaceName, Keywords));
}

void UCavrnusFunctionLibrary::DeleteSpace(FString SpaceId, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure)
{
	CavrnusSuccess successCallback = [OnSuccess]()
		{
			OnSuccess.ExecuteIfBound();
		};
	CavrnusError errorCallback = [OnFailure](const FString& val)
		{
			OnFailure.ExecuteIfBound(val);
		};

	DeleteSpace(SpaceId, successCallback, errorCallback);
}

void UCavrnusFunctionLibrary::DeleteSpace(FString SpaceId, CavrnusSuccess OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterGenericCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildDeleteSpaceMsg(RequestId, SpaceId));
}

void UCavrnusFunctionLibrary::RenameSpace(FString SpaceId, FString NewName, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure)
{
	CavrnusSuccess successCallback = [OnSuccess]()
		{
			OnSuccess.ExecuteIfBound();
		};
	CavrnusError errorCallback = [OnFailure](const FString& val)
		{
			OnFailure.ExecuteIfBound(val);
		};

	RenameSpace(SpaceId, NewName, successCallback, errorCallback);
}

void UCavrnusFunctionLibrary::RenameSpace(FString SpaceId, FString NewName, CavrnusSuccess OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterGenericCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRenameSpaceMsg(RequestId, SpaceId, NewName));
}

void UCavrnusFunctionLibrary::UploadSpaceThumbnail(FString SpaceId, FString ThumbnailFilePath, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure)
{
	CavrnusSuccess successCallback = [OnSuccess]()
		{
			OnSuccess.ExecuteIfBound();
		};
	CavrnusError errorCallback = [OnFailure](const FString& val)
		{
			OnFailure.ExecuteIfBound(val);
		};

	UploadSpaceThumbnail(SpaceId, ThumbnailFilePath, successCallback, errorCallback);
}

void UCavrnusFunctionLibrary::UploadSpaceThumbnail(FString SpaceId, FString thumbnailFilename, CavrnusSuccess OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterGenericCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildUploadThumbnailSpaceMsg(RequestId, SpaceId, thumbnailFilename));
}


void UCavrnusFunctionLibrary::FetchAllUserAccounts(FCavrnusUserAccountsFetched OnAccountsFetched)
{
	CavrnusUserAccountsFetched callback = [OnAccountsFetched](const TArray<FCavrnusUserAccount>& val)
		{
			OnAccountsFetched.ExecuteIfBound(val);
		};

	FetchAllUserAccounts(callback);
}

void UCavrnusFunctionLibrary::FetchAllUserAccounts(CavrnusUserAccountsFetched OnAccountsFetched)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterFetchCavrnusUserAccounts(OnAccountsFetched);

	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildFetchUserAccounts(RequestId));
}

void UCavrnusFunctionLibrary::InviteUserToSpace(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount)
{
	InviteUserToSpace(SpaceInfo, UserAccount, []() {}, [](const FString& val) {});
}

void UCavrnusFunctionLibrary::InviteUserToSpaceWithCallbacks(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure)
{
	CavrnusSuccess successCallback = [OnSuccess]()
		{
			OnSuccess.ExecuteIfBound();
		};
	CavrnusError errorCallback = [OnFailure](const FString& val)
		{
			OnFailure.ExecuteIfBound(val);
		};
	InviteUserToSpace(SpaceInfo, UserAccount, successCallback, errorCallback);
}
void UCavrnusFunctionLibrary::InviteUserToSpace(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, CavrnusSuccess OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterGenericCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildInviteUserToSpace(RequestId, SpaceInfo.SpaceId, UserAccount.UserEmail, ""));
}

void UCavrnusFunctionLibrary::InviteUserToSpaceWithRole(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, FString roleId)
{
	InviteUserToSpaceWithRole(SpaceInfo, UserAccount, roleId, []() {}, [](const FString& val) {});
}

void UCavrnusFunctionLibrary::InviteUserToSpaceWithRoleWithCallbacks(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, FString roleId, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure)
{
	CavrnusSuccess successCallback = [OnSuccess]()
		{
			OnSuccess.ExecuteIfBound();
		};
	CavrnusError errorCallback = [OnFailure](const FString& val)
		{
			OnFailure.ExecuteIfBound(val);
		};
	InviteUserToSpace(SpaceInfo, UserAccount, successCallback, errorCallback);
}
void UCavrnusFunctionLibrary::InviteUserToSpaceWithRole(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, FString roleId, CavrnusSuccess OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterGenericCallback(OnSuccess, OnFailure);

	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildInviteUserToSpace(RequestId, SpaceInfo.SpaceId, UserAccount.UserEmail, roleId));
}

void UCavrnusFunctionLibrary::RemoveUserFromSpace(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount)
{
	RemoveUserFromSpace(SpaceInfo, UserAccount, []() {}, [](const FString& val) {});

}
void UCavrnusFunctionLibrary::RemoveUserFromSpaceWithCallbacks(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure)
{
	CavrnusSuccess successCallback = [OnSuccess]()
		{
			OnSuccess.ExecuteIfBound();
		};
	CavrnusError errorCallback = [OnFailure](const FString& val)
		{
			OnFailure.ExecuteIfBound(val);
		};
	RemoveUserFromSpace(SpaceInfo, UserAccount, successCallback, errorCallback);
}
void UCavrnusFunctionLibrary::RemoveUserFromSpace(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, CavrnusSuccess OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterGenericCallback(OnSuccess, OnFailure);

	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRemoveUserFromSpace(RequestId, SpaceInfo.SpaceId, UserAccount.AccountId));
}

void UCavrnusFunctionLibrary::AwaitAnySpaceBeginLoading(FCavrnusSpaceBeginLoading OnBeginLoading)
{
	CavrnusSpaceBeginLoading callback = [OnBeginLoading](const FString& val)
	{
		OnBeginLoading.ExecuteIfBound(val);
	};

	AwaitAnySpaceBeginLoading(callback);
}

void UCavrnusFunctionLibrary::AwaitAnySpaceBeginLoading(CavrnusSpaceBeginLoading OnBeginLoading)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterBeginLoadingSpaceCallback(OnBeginLoading);
}

void UCavrnusFunctionLibrary::AwaitAnySpaceEndLoading(FCavrnusSpaceEndLoading OnEndLoading)
{
	CavrnusSpaceEndLoading callback = [OnEndLoading]()
		{
			OnEndLoading.ExecuteIfBound();
		};

	AwaitAnySpaceEndLoading(callback);
}

void UCavrnusFunctionLibrary::AwaitAnySpaceEndLoading(CavrnusSpaceEndLoading OnEndLoading)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterEndLoadingSpaceCallback(OnEndLoading);
}

void UCavrnusFunctionLibrary::AwaitAnySpaceConnection(FCavrnusSpaceConnected OnConnected)
{
	CavrnusSpaceConnected callback = [OnConnected](const FCavrnusSpaceConnection& SpaceConn)
	{
		OnConnected.ExecuteIfBound(SpaceConn);
	};

	AwaitAnySpaceConnection(callback);
}

void UCavrnusFunctionLibrary::AwaitAnySpaceConnection(CavrnusSpaceConnected OnConnected)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->AwaitAnySpaceConnection(OnConnected);
}

static void TryLoadLevel(const TSoftObjectPtr<UWorld>& Level)
{
	if (Level.IsNull())
		return;

	const FString LevelName = Level.GetLongPackageName();
	if (LevelName.IsEmpty())
		return;

	UWorld* World = GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetWorld() : nullptr;
	if (World)
	{
		UE_LOG(LogCavrnusConnector, Log, TEXT("[Cavrnus] Loading level '%s' after space exit"), *LevelName);
		UGameplayStatics::OpenLevel(World, FName(*LevelName));
	}
	else
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[Cavrnus] Cannot load level '%s' — no valid world context"), *LevelName);
	}
}

static void RequestLevelLoadOnSpaceExit(TSoftObjectPtr<UWorld> Level)
{
	UCavrnusFunctionLibrary::AwaitAnySpaceExited([Level]()
	{
		TryLoadLevel(Level);
	});
}

void UCavrnusFunctionLibrary::ExitSpace(FCavrnusSpaceConnection SpaceConnection)
{
	CheckErrors(SpaceConnection);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildExitSpaceMsg(SpaceConnection));

	if (const auto* Settings = UCavrnusConnectorSettings::Get())
	{
		if (Settings->bLoadLevelOnSpaceExit && !Settings->SpaceExitLevel.IsNull())
			RequestLevelLoadOnSpaceExit(Settings->SpaceExitLevel);
	}
}

void UCavrnusFunctionLibrary::ExitSpace(FCavrnusSpaceConnection SpaceConnection, TSoftObjectPtr<UWorld> LevelToLoad)
{
	CheckErrors(SpaceConnection);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildExitSpaceMsg(SpaceConnection));

	if (!LevelToLoad.IsNull())
		RequestLevelLoadOnSpaceExit(LevelToLoad);
	else if (const auto* Settings = UCavrnusConnectorSettings::Get())
	{
		if (Settings->bLoadLevelOnSpaceExit && !Settings->SpaceExitLevel.IsNull())
			RequestLevelLoadOnSpaceExit(Settings->SpaceExitLevel);
	}
}

void UCavrnusFunctionLibrary::AwaitAnySpaceExited(FCavrnusSpaceExited OnExit)
{
	CavrnusSpaceExited callback = [OnExit]()
		{
			OnExit.ExecuteIfBound();
		};

	AwaitAnySpaceExited(callback);
}

void UCavrnusFunctionLibrary::AwaitAnySpaceExited(CavrnusSpaceExited OnExit)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->AwaitAnySpaceExited(OnExit);
}

#pragma endregion

// ============================================
// Properties!
// ============================================

#pragma region Generic Prop Functions

void UCavrnusFunctionLibrary::DefineGenericPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, Cavrnus::FPropertyValue PropertyValue, bool overrideExistingDefault)
{
	CheckErrors(SpaceConnection);

	FAbsolutePropertyId AbsolutePropertyId(ContainerName, PropertyName);

	if (!overrideExistingDefault && Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->CurrDefinedProps.Contains(AbsolutePropertyId))
		return;

	Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->SetIsDefined(AbsolutePropertyId);
	int localChangeId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->SetLocalPropVal(AbsolutePropertyId, PropertyValue, -10);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildDefinePropMsg(SpaceConnection, AbsolutePropertyId, PropertyValue, localChangeId));
}

Cavrnus::FPropertyValue UCavrnusFunctionLibrary::GetGenericPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName)
{
	CheckErrors(SpaceConnection);
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->GetPropValue(FAbsolutePropertyId(ContainerName, PropertyName));
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindGenericPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusPropertyFunction& OnPropertyUpdated)
{
	CheckErrors(SpaceConnection);

	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->BindProperty(FAbsolutePropertyId(ContainerName, PropertyName), OnPropertyUpdated);
}

static FString PropTypeToString(Cavrnus::FPropertyValue::PropertyType Type)
{
	switch (Type)
	{
	case Cavrnus::FPropertyValue::PropertyType::String:    return TEXT("String");
	case Cavrnus::FPropertyValue::PropertyType::Bool:      return TEXT("Bool");
	case Cavrnus::FPropertyValue::PropertyType::Float:     return TEXT("Float");
	case Cavrnus::FPropertyValue::PropertyType::Color:     return TEXT("Color");
	case Cavrnus::FPropertyValue::PropertyType::Vector:    return TEXT("Vector");
	case Cavrnus::FPropertyValue::PropertyType::Transform: return TEXT("Transform");
	default:                                               return TEXT("Unset");
	}
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindContainerPropertyValues(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, FContainerPropertyUpdated PropertyUpdateEvent, bool bNewOnly)
{
	CavrnusPropertyFunction propUpdateCallback = [PropertyUpdateEvent](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		PropertyUpdateEvent.ExecuteIfBound(PropTypeToString(Prop.PropType), Prop.ToString(), ContainerName, PropertyName);
	};

	return BindContainerPropertyValues(SpaceConnection, FPropertiesContainer(ContainerName), propUpdateCallback, bNewOnly);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindContainerPropertyValues(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const CavrnusPropertyFunction& OnPropertyUpdated, bool bNewOnly)
{
	CheckErrors(SpaceConnection);

	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->BindContainerProperties(ContainerName.ContainerName, OnPropertyUpdated, !bNewOnly);
}

UCavrnusLivePropertyUpdate* UCavrnusFunctionLibrary::BeginTransientGenericPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, Cavrnus::FPropertyValue PropertyValue)
{
	CheckErrors(SpaceConnection);

	UCavrnusLivePropertyUpdate* res = nullptr;

	if(PropertyValue.PropType == Cavrnus::FPropertyValue::PropertyType::Bool)
		res = NewObject<UCavrnusLiveBoolPropertyUpdate>();
	if (PropertyValue.PropType == Cavrnus::FPropertyValue::PropertyType::String)
		res = NewObject<UCavrnusLiveStringPropertyUpdate>();
	if (PropertyValue.PropType == Cavrnus::FPropertyValue::PropertyType::Color)
		res = NewObject<UCavrnusLiveColorPropertyUpdate>();
	if (PropertyValue.PropType == Cavrnus::FPropertyValue::PropertyType::Float)
		res = NewObject<UCavrnusLiveFloatPropertyUpdate>();
	if (PropertyValue.PropType == Cavrnus::FPropertyValue::PropertyType::Vector)
		res = NewObject<UCavrnusLiveVectorPropertyUpdate>();
	if (PropertyValue.PropType == Cavrnus::FPropertyValue::PropertyType::Transform)
		res = NewObject<UCavrnusLiveTransformPropertyUpdate>();

	res->InitializeGeneric(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue);

	return res;
}

void UCavrnusFunctionLibrary::PostGenericPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, Cavrnus::FPropertyValue PropertyValue, const FPropertyPostOptions& options)
{
	CheckErrors(SpaceConnection);
	FAbsolutePropertyId AbsolutePropertyId(ContainerName, PropertyName);
	int localChangeId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->SetLocalPropVal(AbsolutePropertyId, PropertyValue, 1);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildUpdatePropMsg(SpaceConnection, AbsolutePropertyId, PropertyValue, localChangeId, options));
}

bool UCavrnusFunctionLibrary::PropertyValueExists(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName)
{
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->PropValueExists(FAbsolutePropertyId(ContainerName, PropertyName));
}

bool UCavrnusFunctionLibrary::PropertyValueExists(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName)
{
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->PropValueExists(FAbsolutePropertyId(ContainerName, PropertyName));
}

#pragma endregion


#pragma region Color Prop Functions

void UCavrnusFunctionLibrary::DefineColorPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FLinearColor PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::ColorPropValue(PropertyValue));
}

void UCavrnusFunctionLibrary::DefineColorPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FLinearColor PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::ColorPropValue(PropertyValue));
}

FLinearColor UCavrnusFunctionLibrary::GetColorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).ColorValue;
}

FLinearColor UCavrnusFunctionLibrary::GetColorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).ColorValue;
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindColorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FColorPropertyUpdated PropertyUpdateEvent)
{
	CavrnusPropertyFunction propUpdateCallback = [PropertyUpdateEvent](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		PropertyUpdateEvent.ExecuteIfBound(Prop.ColorValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindColorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusColorFunction& OnPropertyUpdated)
{
	CavrnusPropertyFunction propUpdateCallback = [OnPropertyUpdated](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		OnPropertyUpdated(Prop.ColorValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusLiveColorPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientColorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FLinearColor PropertyValue)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveColorPropertyUpdate* res = NewObject<UCavrnusLiveColorPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue);

	return res;
}

UCavrnusLiveColorPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientColorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FLinearColor PropertyValue)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveColorPropertyUpdate* res = NewObject<UCavrnusLiveColorPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue);

	return res;
}

void UCavrnusFunctionLibrary::PostColorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FLinearColor PropertyValue)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::ColorPropValue(PropertyValue));
}

void UCavrnusFunctionLibrary::PostColorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FLinearColor PropertyValue)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::ColorPropValue(PropertyValue));
}

#pragma endregion

#pragma region Bool Prop Functions

void UCavrnusFunctionLibrary::DefineBoolPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, bool PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::BoolPropValue(PropertyValue));
}

void UCavrnusFunctionLibrary::DefineBoolPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, bool PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::BoolPropValue(PropertyValue));
}

bool UCavrnusFunctionLibrary::GetBoolPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).BoolValue;
}

bool UCavrnusFunctionLibrary::GetBoolPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).BoolValue;
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindBooleanPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FBoolPropertyUpdated PropertyUpdateEvent)
{
	CavrnusPropertyFunction propUpdateCallback = [PropertyUpdateEvent](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		PropertyUpdateEvent.ExecuteIfBound(Prop.BoolValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindBooleanPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusBoolFunction& OnPropertyUpdated)
{
	CavrnusPropertyFunction propUpdateCallback = [OnPropertyUpdated](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		OnPropertyUpdated(Prop.BoolValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusLiveBoolPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientBoolPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, bool PropertyValue)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveBoolPropertyUpdate* res = NewObject<UCavrnusLiveBoolPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue);

	return res;
}

UCavrnusLiveBoolPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientBoolPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, bool PropertyValue)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveBoolPropertyUpdate* res = NewObject<UCavrnusLiveBoolPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue);

	return res;
}

void UCavrnusFunctionLibrary::PostBoolPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, bool PropertyValue)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::BoolPropValue(PropertyValue));
}

void UCavrnusFunctionLibrary::PostBoolPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, bool PropertyValue)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::BoolPropValue(PropertyValue));
}

#pragma endregion

#pragma region Float Prop Functions

void UCavrnusFunctionLibrary::DefineFloatPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, float PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::FloatPropValue(PropertyValue));
}

void UCavrnusFunctionLibrary::DefineFloatPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, float PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::FloatPropValue(PropertyValue));
}

float UCavrnusFunctionLibrary::GetFloatPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).FloatValue;
}

float UCavrnusFunctionLibrary::GetFloatPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).FloatValue;
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindFloatPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FFloatPropertyUpdated PropertyUpdateEvent)
{
	CavrnusPropertyFunction propUpdateCallback = [PropertyUpdateEvent](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		PropertyUpdateEvent.ExecuteIfBound(Prop.FloatValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindFloatPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusFloatFunction& OnPropertyUpdated)
{
	CavrnusPropertyFunction propUpdateCallback = [OnPropertyUpdated](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		OnPropertyUpdated(Prop.FloatValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusLiveFloatPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientFloatPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, float PropertyValue)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveFloatPropertyUpdate* res = NewObject<UCavrnusLiveFloatPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue);

	return res;
}

UCavrnusLiveFloatPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientFloatPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, float PropertyValue)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveFloatPropertyUpdate* res = NewObject<UCavrnusLiveFloatPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue);

	return res;
}

void UCavrnusFunctionLibrary::PostFloatPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, float PropertyValue)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::FloatPropValue(PropertyValue));
}

void UCavrnusFunctionLibrary::PostFloatPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, float PropertyValue)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::FloatPropValue(PropertyValue));
}

#pragma endregion

#pragma region String Prop Functions

void UCavrnusFunctionLibrary::DefineStringPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FString PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::StringPropValue(PropertyValue));
}

void UCavrnusFunctionLibrary::DefineStringPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FString PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::StringPropValue(PropertyValue));
}

FString UCavrnusFunctionLibrary::GetStringPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).StringValue;
}

FString UCavrnusFunctionLibrary::GetStringPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).StringValue;
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindStringPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FStringPropertyUpdated PropertyUpdateEvent)
{
	CavrnusPropertyFunction propUpdateCallback = [PropertyUpdateEvent](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		PropertyUpdateEvent.ExecuteIfBound(Prop.StringValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindStringPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusStringFunction& OnPropertyUpdated)
{
	CavrnusPropertyFunction propUpdateCallback = [OnPropertyUpdated](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		OnPropertyUpdated(Prop.StringValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusLiveStringPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientStringPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FString PropertyValue)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveStringPropertyUpdate* res = NewObject<UCavrnusLiveStringPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue);

	return res;
}

UCavrnusLiveStringPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientStringPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FString PropertyValue)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveStringPropertyUpdate* res = NewObject<UCavrnusLiveStringPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue);

	return res;
}

void UCavrnusFunctionLibrary::PostStringPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FString PropertyValue)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::StringPropValue(PropertyValue));
}

void UCavrnusFunctionLibrary::PostStringPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FString PropertyValue)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::StringPropValue(PropertyValue));
}

#pragma endregion

#pragma region Vector Prop Functions

void UCavrnusFunctionLibrary::DefineVectorPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FVector4 PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::VectorPropValue(PropertyValue));
}

void UCavrnusFunctionLibrary::DefineVectorPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FVector4 PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::VectorPropValue(PropertyValue));
}

FVector4 UCavrnusFunctionLibrary::GetVectorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).VectorValue;
}

FVector4 UCavrnusFunctionLibrary::GetVectorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).VectorValue;
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindVectorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FVectorPropertyUpdated PropertyUpdateEvent)
{
	CavrnusPropertyFunction propUpdateCallback = [PropertyUpdateEvent](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		PropertyUpdateEvent.ExecuteIfBound(Prop.VectorValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindVectorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusVectorFunction& OnPropertyUpdated)
{
	CavrnusPropertyFunction propUpdateCallback = [OnPropertyUpdated](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		OnPropertyUpdated(Prop.VectorValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusLiveVectorPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientVectorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FVector4 PropertyValue)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveVectorPropertyUpdate* res = NewObject<UCavrnusLiveVectorPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue);

	return res;
}

UCavrnusLiveVectorPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientVectorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FVector4 PropertyValue)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveVectorPropertyUpdate* res = NewObject<UCavrnusLiveVectorPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue);

	return res;
}

void UCavrnusFunctionLibrary::PostVectorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FVector4 PropertyValue)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::VectorPropValue(PropertyValue));
}

void UCavrnusFunctionLibrary::PostVectorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FVector4 PropertyValue)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::VectorPropValue(PropertyValue));
}

#pragma endregion

#pragma region Transform Prop Functions

void UCavrnusFunctionLibrary::DefineTransformPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FTransform PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::TransformPropValue(PropertyValue));
}

void UCavrnusFunctionLibrary::DefineTransformPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FTransform PropertyValue)
{
	DefineGenericPropertyDefaultValue(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::TransformPropValue(PropertyValue));
}

FTransform UCavrnusFunctionLibrary::GetTransformPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).TransformValue;
}

FTransform UCavrnusFunctionLibrary::GetTransformPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName)
{
	return GetGenericPropertyValue(SpaceConnection, ContainerName, PropertyName).TransformValue;
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindTransformPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FTransformPropertyUpdated PropertyUpdateEvent)
{
	CavrnusPropertyFunction propUpdateCallback = [PropertyUpdateEvent](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		PropertyUpdateEvent.ExecuteIfBound(Prop.TransformValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindTransformPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusTransformFunction& OnPropertyUpdated)
{
	CavrnusPropertyFunction propUpdateCallback = [OnPropertyUpdated](const Cavrnus::FPropertyValue& Prop, const FString& ContainerName, const FString& PropertyName)
	{
		OnPropertyUpdated(Prop.TransformValue, ContainerName, PropertyName);
	};

	return BindGenericPropertyValue(SpaceConnection, ContainerName, PropertyName, propUpdateCallback);
}

UCavrnusLiveTransformPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientTransformPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FTransform PropertyValue, const FPropertyPostOptions& options)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveTransformPropertyUpdate* res = NewObject<UCavrnusLiveTransformPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue, options);

	return res;
}

UCavrnusLiveTransformPropertyUpdate* UCavrnusFunctionLibrary::BeginTransientTransformPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FTransform PropertyValue, const FPropertyPostOptions& options)
{
	CheckErrors(SpaceConnection);

	UCavrnusLiveTransformPropertyUpdate* res = NewObject<UCavrnusLiveTransformPropertyUpdate>();
	res->Initialize(Cavrnus::CavrnusRelayModel::GetDataModel(), SpaceConnection, FAbsolutePropertyId(ContainerName, PropertyName), PropertyValue, options);

	return res;
}

void UCavrnusFunctionLibrary::PostTransformPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FTransform PropertyValue, const FPropertyPostOptions& options)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::TransformPropValue(PropertyValue), options);
}

void UCavrnusFunctionLibrary::PostTransformPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FTransform PropertyValue, const FPropertyPostOptions& options)
{
	PostGenericPropertyUpdate(SpaceConnection, ContainerName, PropertyName, Cavrnus::FPropertyValue::TransformPropValue(PropertyValue), options);
}

#pragma endregion

#pragma region Property Definitions

void UCavrnusFunctionLibrary::DefineStringPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FCavrnusStringPropertyDefinition Definition)
{
	DefineStringPropertyDefinition(SpaceConnection, FPropertiesContainer(ContainerName), PropertyName, Definition);
}

void UCavrnusFunctionLibrary::DefineStringPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const FCavrnusStringPropertyDefinition& Definition)
{
	FAbsolutePropertyId AbsolutePropertyId(ContainerName, PropertyName);
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->SetPropertyDefinition(AbsolutePropertyId, Definition.Metadata);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildDefineStringPropertyDefinitionMsg(SpaceConnection, AbsolutePropertyId, Definition));
}

void UCavrnusFunctionLibrary::DefineFloatPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FCavrnusFloatPropertyDefinition Definition)
{
	DefineFloatPropertyDefinition(SpaceConnection, FPropertiesContainer(ContainerName), PropertyName, Definition);
}

void UCavrnusFunctionLibrary::DefineFloatPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const FCavrnusFloatPropertyDefinition& Definition)
{
	FAbsolutePropertyId AbsolutePropertyId(ContainerName, PropertyName);
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->SetPropertyDefinition(AbsolutePropertyId, Definition.Metadata);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildDefineFloatPropertyDefinitionMsg(SpaceConnection, AbsolutePropertyId, Definition));
}

void UCavrnusFunctionLibrary::DefineColorPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FCavrnusColorPropertyDefinition Definition)
{
	DefineColorPropertyDefinition(SpaceConnection, FPropertiesContainer(ContainerName), PropertyName, Definition);
}

void UCavrnusFunctionLibrary::DefineColorPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const FCavrnusColorPropertyDefinition& Definition)
{
	FAbsolutePropertyId AbsolutePropertyId(ContainerName, PropertyName);
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->SetPropertyDefinition(AbsolutePropertyId, Definition.Metadata);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildDefineColorPropertyDefinitionMsg(SpaceConnection, AbsolutePropertyId, Definition));
}

void UCavrnusFunctionLibrary::DefineBoolPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FCavrnusBoolPropertyDefinition Definition)
{
	DefineBoolPropertyDefinition(SpaceConnection, FPropertiesContainer(ContainerName), PropertyName, Definition);
}

void UCavrnusFunctionLibrary::DefineBoolPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const FCavrnusBoolPropertyDefinition& Definition)
{
	FAbsolutePropertyId AbsolutePropertyId(ContainerName, PropertyName);
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->SetPropertyDefinition(AbsolutePropertyId, Definition.Metadata);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildDefineBoolPropertyDefinitionMsg(SpaceConnection, AbsolutePropertyId, Definition));
}

void UCavrnusFunctionLibrary::DefineVectorPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FCavrnusVectorPropertyDefinition Definition)
{
	DefineVectorPropertyDefinition(SpaceConnection, FPropertiesContainer(ContainerName), PropertyName, Definition);
}

void UCavrnusFunctionLibrary::DefineVectorPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const FCavrnusVectorPropertyDefinition& Definition)
{
	FAbsolutePropertyId AbsolutePropertyId(ContainerName, PropertyName);
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->SetPropertyDefinition(AbsolutePropertyId, Definition.Metadata);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildDefineVectorPropertyDefinitionMsg(SpaceConnection, AbsolutePropertyId, Definition));
}

void UCavrnusFunctionLibrary::DefineTransformPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FCavrnusTransformPropertyDefinition Definition)
{
	DefineTransformPropertyDefinition(SpaceConnection, FPropertiesContainer(ContainerName), PropertyName, Definition);
}

void UCavrnusFunctionLibrary::DefineTransformPropertyDefinition(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const FCavrnusTransformPropertyDefinition& Definition)
{
	FAbsolutePropertyId AbsolutePropertyId(ContainerName, PropertyName);
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->SetPropertyDefinition(AbsolutePropertyId, Definition.Metadata);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildDefineTransformPropertyDefinitionMsg(SpaceConnection, AbsolutePropertyId, Definition));
}

#pragma endregion

#pragma region Permissions

UPARAM(DisplayName = "Disposable")UCavrnusBinding* UCavrnusFunctionLibrary::BindPolicyActionPermitted(const FString& Action, FCavrnusPolicyUpdated OnPolicyUpdated)
{
	CavrnusPolicyUpdated callback = [OnPolicyUpdated](const FString& action, bool allowed)
	{
		OnPolicyUpdated.ExecuteIfBound(action, allowed);
	};

	return BindPolicyActionPermitted(Action, callback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindPolicyActionPermitted(FString Action, CavrnusPolicyUpdated OnPolicyUpdated)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRequestGlobalPermission(Action));

	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetGlobalPermissionsModel()->BindPolicyAllowed(Action, OnPolicyUpdated);
}

UPARAM(DisplayName = "Disposable")UCavrnusBinding* UCavrnusFunctionLibrary::BindSpacePolicyActionPermitted(FCavrnusSpaceConnection SpaceConnection, const FString& Action, FCavrnusPolicyUpdated OnPolicyUpdated)
{
	CavrnusPolicyUpdated callback = [OnPolicyUpdated](const FString& action, bool allowed)
	{
		OnPolicyUpdated.ExecuteIfBound(action, allowed);
	};

	return BindSpacePolicyActionPermitted(SpaceConnection, Action, callback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindSpacePolicyActionPermitted(FCavrnusSpaceConnection SpaceConnection, const FString& Action, CavrnusPolicyUpdated OnPolicyUpdated)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRequestSpacePermission(SpaceConnection, Action));

	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePermissionsModel(SpaceConnection)->BindPolicyAllowed(Action, OnPolicyUpdated);
}

#pragma endregion


#pragma region Spawned Objects

const FCavrnusSpawnedObject& UCavrnusFunctionLibrary::SpawnObject(FCavrnusSpaceConnection SpaceConnection, const FString& UniqueIdentifier)
{
	CheckErrors(SpaceConnection);

	FString InstanceId = Cavrnus::CavrnusProtoTranslation::CreateTransientId();

	// Use the new helper function instead of direct protobuf calls
	USpawnedObjectsManager::CreateAndRegisterObject(SpaceConnection, UniqueIdentifier, InstanceId);

	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->SpawnedObjects[InstanceId];
}

void UCavrnusFunctionLibrary::DestroyObject(const FCavrnusSpawnedObject& SpawnedObject)
{
	CheckErrors(SpawnedObject.SpaceConnection);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildDestroyOp(SpawnedObject.SpaceConnection, SpawnedObject.PropertiesContainerName));

	Cavrnus::CavrnusRelayModel::GetDataModel()->HandleSpaceObjectRemoved(Cavrnus::CavrnusProtoTranslation::BuildObjectRemoved(SpawnedObject.SpaceConnection, SpawnedObject.PropertiesContainerName));
}

void UCavrnusFunctionLibrary::RegisterSpawnableObjectType(const FString& Identifier, TSubclassOf<AActor> ClassType)
{
	UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
	if (!Subsystem || !Subsystem->IsRuntimeContextReady())
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("RegisterSpawnableObjectType - RuntimeContext not ready"));
		return;
	}

	USpawnedObjectsManager* Manager = Subsystem->RuntimeContext->Get<USpawnedObjectsManager>();
	if (Manager)
	{
		Manager->RegisterSpawnableObjectType(Identifier, ClassType);
	}
	else
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("RegisterSpawnableObjectType - SpawnedObjectsManager not available"));
	}
}

void UCavrnusFunctionLibrary::UnregisterSpawnableObjectType(const FString& Identifier)
{
	UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
	if (!Subsystem || !Subsystem->IsRuntimeContextReady())
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("UnregisterSpawnableObjectType - RuntimeContext not ready"));
		return;
	}

	USpawnedObjectsManager* Manager = Subsystem->RuntimeContext->Get<USpawnedObjectsManager>();
	if (Manager)
	{
		Manager->UnregisterSpawnableObjectType(Identifier);
	}
	else
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("UnregisterSpawnableObjectType - SpawnedObjectsManager not available"));
	}
}

#pragma endregion

#pragma region Space Users

// ============================================
// Space Users 
// ============================================

TArray<FCavrnusUser> UCavrnusFunctionLibrary::GetCurrentSpaceUsers(FCavrnusSpaceConnection SpaceConnection)
{
	CheckErrors(SpaceConnection);
	TArray<FCavrnusUser> users;
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->CurrSpaceUsers.GenerateValueArray(users);

	return users;
}

void UCavrnusFunctionLibrary::AwaitLocalUser(FCavrnusSpaceConnection SpaceConnection, FCavrnusSpaceUserEvent LocalUserArrived)
{
	CavrnusSpaceUserEvent callback = [LocalUserArrived](const FCavrnusUser& user)
	{
		LocalUserArrived.ExecuteIfBound(user);
	};

	AwaitLocalUser(SpaceConnection, callback);
}

void UCavrnusFunctionLibrary::AwaitLocalUser(FCavrnusSpaceConnection SpaceConnection, CavrnusSpaceUserEvent LocalUserArrived)
{
	CheckErrors(SpaceConnection);
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->AwaitLocalUser(LocalUserArrived);
}

UPARAM(DisplayName = "Disposable") UCavrnusBinding* UCavrnusFunctionLibrary::BindSpaceUsers(FCavrnusSpaceConnection SpaceConnection, FCavrnusSpaceUserEvent UserAdded, FCavrnusSpaceUserEvent UserRemoved)
{
	CavrnusSpaceUserEvent addedCallback = [UserAdded](const FCavrnusUser& user)
	{
		UserAdded.ExecuteIfBound(user);
	};
	CavrnusSpaceUserEvent removedCallback = [UserRemoved](const FCavrnusUser& user)
	{
		UserRemoved.ExecuteIfBound(user);
	};
	return BindSpaceUsers(SpaceConnection, addedCallback, removedCallback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindSpaceUsers(FCavrnusSpaceConnection SpaceConnection, CavrnusSpaceUserEvent UserAdded, CavrnusSpaceUserEvent UserRemoved)
{
	CheckErrors(SpaceConnection);
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->BindSpaceUsers(UserAdded, UserRemoved);
}

UPARAM(DisplayName = "Disposable") UCavrnusBinding* UCavrnusFunctionLibrary::BindUserVideoFrames(FCavrnusSpaceConnection SpaceConnection, const FCavrnusUser& User, FCavrnusUserVideoFrameEvent OnVideoFrameUpdate)
{
	VideoFrameUpdateFunction VideoFrameUpdateCallback = [OnVideoFrameUpdate](UTexture2D* VideoTexture)
	{
		OnVideoFrameUpdate.ExecuteIfBound(VideoTexture);
	};

	return BindUserVideoFrames(SpaceConnection, User, VideoFrameUpdateCallback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindUserVideoFrames(FCavrnusSpaceConnection SpaceConnection, const FCavrnusUser& User, const VideoFrameUpdateFunction& OnVideoFrameUpdate)
{
	CheckErrors(SpaceConnection);
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection)->BindUserVideoTexture(User, OnVideoFrameUpdate);
}

#pragma endregion

#pragma region Voice and Video

// ============================================
// Voice and Video
// ============================================

void UCavrnusFunctionLibrary::SetLocalUserMutedState(FCavrnusSpaceConnection SpaceConnection, bool bIsMuted)
{
	CheckErrors(SpaceConnection);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildSetLocalUserMuted(SpaceConnection, bIsMuted));
}

void UCavrnusFunctionLibrary::SetLocalUserStreamingState(FCavrnusSpaceConnection SpaceConnection, bool bIsStreaming)
{
	CheckErrors(SpaceConnection);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildSetLocalUserStreaming(SpaceConnection, bIsStreaming));
}

void UCavrnusFunctionLibrary::FetchSavedAudioInput(FCavrnusSavedInputDevice OnReceiveDevice)
{
	const CavrnusSavedInputDevice callback = [OnReceiveDevice](const FCavrnusInputDevice& device)
	{
		OnReceiveDevice.ExecuteIfBound(device);
	};
	FetchSavedAudioInput(callback);
}

void UCavrnusFunctionLibrary::FetchSavedAudioInput(CavrnusSavedInputDevice OnReceiveDevice)
{
	const CavrnusAvailableInputDevices callback = [OnReceiveDevice](const TArray<FCavrnusInputDevice>& devices)
	{
		FString savedDeviceId;
		FPlatformMisc::GetStoredValue(TEXT("Cavrnus"), TEXT("UE"), TEXT("AudioInput"), savedDeviceId);

		for (int i = 0; i < devices.Num(); i++) 
		{
			if (devices[i].DeviceId.Equals(savedDeviceId))
			{
				OnReceiveDevice(devices[i]);
				return;
			}
		}

		if (devices.Num() > 0)
			OnReceiveDevice(devices[0]);
	};
	FetchAudioInputs(callback);
}

void UCavrnusFunctionLibrary::FetchAudioInputs(FCavrnusAvailableInputDevices OnReceiveDevices)
{
	const CavrnusAvailableInputDevices callback = [OnReceiveDevices](const TArray<FCavrnusInputDevice>& devices)
	{
		OnReceiveDevices.ExecuteIfBound(devices);
	};
	FetchAudioInputs(callback);
}

void UCavrnusFunctionLibrary::FetchAudioInputs(CavrnusAvailableInputDevices OnReceiveDevices)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterFetchAudioInputs(OnReceiveDevices);

	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRequestAudioInputs(RequestId));
}

void UCavrnusFunctionLibrary::UpdateAudioInput(FCavrnusInputDevice Device)
{
	FPlatformMisc::SetStoredValue(TEXT("Cavrnus"), TEXT("UE"), TEXT("AudioInput"), Device.DeviceId);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildSetAudioInput(Device));
}
void UCavrnusFunctionLibrary::UpdateAudioInputWithCallback(FCavrnusInputDevice Device, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure)
{
	CavrnusSuccess successCallback = [OnSuccess]()
		{
			OnSuccess.ExecuteIfBound();
		};
	CavrnusError errorCallback = [OnFailure](const FString& val)
		{
			OnFailure.ExecuteIfBound(val);
		};

	UpdateAudioInput(Device, successCallback, errorCallback);
}

void UCavrnusFunctionLibrary::UpdateAudioInput(FCavrnusInputDevice Device, CavrnusSuccess OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterGenericCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildSetAudioInput(Device, RequestId));
}

void UCavrnusFunctionLibrary::FetchSavedAudioOutput(FCavrnusSavedOutputDevice OnReceiveDevice)
{
	const CavrnusSavedOutputDevice callback = [OnReceiveDevice](const FCavrnusOutputDevice& device)
	{
		OnReceiveDevice.ExecuteIfBound(device);
	};
	FetchSavedAudioOutput(callback);
}

void UCavrnusFunctionLibrary::FetchSavedAudioOutput(CavrnusSavedOutputDevice OnReceiveDevice)
{
	const CavrnusAvailableOutputDevices callback = [OnReceiveDevice](const TArray<FCavrnusOutputDevice>& devices)
	{
		FString savedDeviceId;
		FPlatformMisc::GetStoredValue(TEXT("Cavrnus"), TEXT("UE"), TEXT("AudioOutput"), savedDeviceId);

		for (int i = 0; i < devices.Num(); i++) {
			if (devices[i].DeviceId.Equals(savedDeviceId))
			{
				OnReceiveDevice(devices[i]);
				return;
			}
		}

		if (devices.Num() > 0)
			OnReceiveDevice(devices[0]);
	};
	FetchAudioOutputs(callback);
}

void UCavrnusFunctionLibrary::FetchAudioOutputs(FCavrnusAvailableOutputDevices OnReceiveDevices)
{
	const CavrnusAvailableOutputDevices callback = [OnReceiveDevices](const TArray<FCavrnusOutputDevice>& devices)
	{
		OnReceiveDevices.ExecuteIfBound(devices);
	};
	FetchAudioOutputs(callback);
}

void UCavrnusFunctionLibrary::FetchAudioOutputs(CavrnusAvailableOutputDevices OnReceiveDevices)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterFetchAudioOutputs(OnReceiveDevices);

	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRequestAudioOutputs(RequestId));
}

void UCavrnusFunctionLibrary::UpdateAudioOutput(FCavrnusOutputDevice Device)
{
	FPlatformMisc::SetStoredValue(TEXT("Cavrnus"), TEXT("UE"), TEXT("AudioOutput"), Device.DeviceId);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildSetAudioOutput(Device));
}
void UCavrnusFunctionLibrary::UpdateAudioOutputWithCallback(FCavrnusOutputDevice Device, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure)
{
	CavrnusSuccess successCallback = [OnSuccess]()
		{
			OnSuccess.ExecuteIfBound();
		};
	CavrnusError errorCallback = [OnFailure](const FString& val)
		{
			OnFailure.ExecuteIfBound(val);
		};

	UpdateAudioOutput(Device, successCallback, errorCallback);
}

void UCavrnusFunctionLibrary::UpdateAudioOutput(FCavrnusOutputDevice Device, CavrnusSuccess OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterGenericCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildSetAudioOutput(Device, RequestId));
}

void UCavrnusFunctionLibrary::FetchVideoInputs(FCavrnusAvailableVideoInputDevices OnReceiveDevices)
{
	const CavrnusAvailableVideoInputDevices callback = [OnReceiveDevices](const TArray<FCavrnusVideoInputDevice>& devices)
	{
		OnReceiveDevices.ExecuteIfBound(devices);
	};
	FetchVideoInputs(callback);
}

void UCavrnusFunctionLibrary::FetchVideoInputs(CavrnusAvailableVideoInputDevices OnReceiveDevices)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterFetchVideoInputs(OnReceiveDevices);

	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRequestVideoInputs(RequestId));
}

void UCavrnusFunctionLibrary::UpdateVideoInput(FCavrnusVideoInputDevice Device)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildSetVideoInput(Device));
}
void UCavrnusFunctionLibrary::UpdateVideoInputWithCallback(FCavrnusVideoInputDevice Device, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure)
{
	CavrnusSuccess successCallback = [OnSuccess]()
		{
			OnSuccess.ExecuteIfBound();
		};
	CavrnusError errorCallback = [OnFailure](const FString& val)
		{
			OnFailure.ExecuteIfBound(val);
		};

	UpdateVideoInput(Device, successCallback, errorCallback);
}

void UCavrnusFunctionLibrary::UpdateVideoInput(FCavrnusVideoInputDevice Device, CavrnusSuccess OnSuccess, CavrnusError OnFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterGenericCallback(OnSuccess, OnFailure);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildSetVideoInput(Device, RequestId));
}

void UCavrnusFunctionLibrary::FetchFileInfoById(FString ContentId, FCavrnusRemoteContentInfoFunction OnRecvContentInfo)
{
	CavrnusRemoteContentInfoFunction completeCallback = [OnRecvContentInfo](const FCavrnusRemoteContent& content)
		{
			OnRecvContentInfo.ExecuteIfBound(content);
		};

	FetchFileInfoById(ContentId, completeCallback);
}

void UCavrnusFunctionLibrary::FetchFileInfoById(FString ContentId, const CavrnusRemoteContentInfoFunction& OnRecvContentInfo)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterFetchRemoteContentInfo(OnRecvContentInfo);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRequestFileInfoById(RequestId, ContentId));
}

void UCavrnusFunctionLibrary::FetchFileById(FString ContentId, FCavrnusContentProgressFunction OnProgress, CavrnusContentFunctionLargeFile OnContentLoaded, FCavrnusError OnFailure)
{
	CavrnusContentProgressFunction progressCallback = [OnProgress](const float prog, const FString& step)
	{
		OnProgress.ExecuteIfBound(prog, step);
	};

	CavrnusContentFunction completeCallback = [OnContentLoaded](const TArray64<uint8>& bytes, const FString& fileName)
	{
		if (OnContentLoaded)
			OnContentLoaded(bytes);
	};

	CavrnusError errCallback = [OnFailure](const FString& exception)
	{
			OnFailure.ExecuteIfBound(exception);
	};

	FetchFileById(ContentId, progressCallback, completeCallback, errCallback);
}

void UCavrnusFunctionLibrary::FetchFileById(FString ContentId, const CavrnusContentProgressFunction& OnProgress, const CavrnusContentFunction& OnContentLoaded, const CavrnusError& OnFailure)
{
	//Sending this multiple times shouldn't hurt anything...
	if (!Cavrnus::CavrnusRelayModel::GetDataModel()->ContentModel.RegisterContentCallbacks(ContentId, OnProgress, OnContentLoaded, OnFailure))
	{
		Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRequestFileById(ContentId));
	}
}

void UCavrnusFunctionLibrary::FetchFileByIdToDisk(FString ContentId, FString FolderDestination, FCavrnusContentProgressFunction OnProgress, FCavrnusContentFileFunction OnContentLoaded, FCavrnusError OnFailure)
{
	CavrnusContentProgressFunction progressCallback = [OnProgress](const float prog, const FString& step)
	{
		OnProgress.ExecuteIfBound(prog, step);
	};

	const TFunction<void(FString)>& completeCallback = [OnContentLoaded](const FString& path)
	{
		OnContentLoaded.ExecuteIfBound(path);
	};

	CavrnusError errCallback = [OnFailure](const FString& exception)
	{
		OnFailure.ExecuteIfBound(exception);
	};

	FetchFileByIdToDisk(ContentId, FolderDestination, progressCallback, completeCallback, errCallback);
}

void UCavrnusFunctionLibrary::FetchFileByIdToDisk(FString ContentId, FString FolderDestination, const CavrnusContentProgressFunction& OnProgress, const TFunction<void(FString)>& OnContentLoaded, const CavrnusError& OnFailure)
{
	CavrnusFileContentFunction callback = [FolderDestination, OnContentLoaded](const FString& fullPath, const FString& fileName)
	{
		FString fullDestPath = FolderDestination + "/" + fileName;
		IFileManager::Get().Copy(*fullDestPath, *fullPath);
		OnContentLoaded(fullDestPath);
	};

	if (!Cavrnus::CavrnusRelayModel::GetDataModel()->ContentModel.RegisterFileContentCallbacks(ContentId, OnProgress, callback, OnFailure))
	{
		Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRequestFileById(ContentId));
	}
}

void UCavrnusFunctionLibrary::FetchAllUploadedContent(FCavrnusRemoteContentFunction OnAvailableContentFetched)
{
	const CavrnusRemoteContentFunction remoteContentCallback = [OnAvailableContentFetched](const TArray<FCavrnusRemoteContent>& allContent)
	{
		OnAvailableContentFetched.ExecuteIfBound(allContent);
	};

	FetchAllUploadedContent(remoteContentCallback);
}

void UCavrnusFunctionLibrary::FetchAllUploadedContent(const CavrnusRemoteContentFunction& OnAvailableContentFetched)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterFetchAllAvailableContent(OnAvailableContentFetched);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRequestAllUploadedContent(RequestId));
}

const TMap<FString, FCavrnusRemoteContent>& UCavrnusFunctionLibrary::GetRemoteContent()
{
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->CurrRemoteContent;
}

void UCavrnusFunctionLibrary::AwaitRemoteContentByPredicate(TFunction<bool(const FCavrnusRemoteContent& content)> predicate, TFunction<void(const FCavrnusRemoteContent& content)> onContentArrived)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->AwaitContentByPredicate(predicate, onContentArrived);
}

void UCavrnusFunctionLibrary::UploadContent(FString FilePath, FCavrnusUploadCompleteFunction OnUploadComplete)
{
	CavrnusUploadCompleteFunction callback = [OnUploadComplete](const FCavrnusRemoteContent& uploadedContent)
		{
			OnUploadComplete.ExecuteIfBound(uploadedContent);
		};

	UploadContent(FilePath, callback);
}

void UCavrnusFunctionLibrary::UploadContent(FString FilePath, const CavrnusUploadCompleteFunction& OnUploadComplete)
{
	UploadContentWithTags(FilePath, TMap<FString, FString>(), OnUploadComplete, [](FString err) {}, [](FString step, float prog) {});
}

void UCavrnusFunctionLibrary::UploadContentWithTags(FString FilePath, TMap<FString, FString> Tags, FCavrnusUploadCompleteFunction OnUploadComplete, FCavrnusError OnError, FCavrnusUploadProgressFunction OnProgress)
{
	CavrnusUploadCompleteFunction callback = [OnUploadComplete](const FCavrnusRemoteContent& uploadedContent)
		{
			OnUploadComplete.ExecuteIfBound(uploadedContent);
		};
	CavrnusError errCallback = [OnError](const FString& exception)
		{
			OnError.ExecuteIfBound(exception);
		};
	CavrnusUploadProgressFunction progCallback = [OnProgress](const FString& step, float prog)
		{
			OnProgress.ExecuteIfBound(step, prog);
		};

	UploadContentWithTags(FilePath, Tags, callback, errCallback, progCallback);
}

void UCavrnusFunctionLibrary::UploadContentWithTags(FString FilePath, TMap<FString, FString> Tags, const CavrnusUploadCompleteFunction& OnUploadComplete, const CavrnusError& OnError, const CavrnusUploadProgressFunction& OnProgress)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterUploadContent(OnUploadComplete, OnError, OnProgress);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildUploadContent(RequestId, FilePath, Tags));
}

void UCavrnusFunctionLibrary::RequestContentDestinationFolder(FString FolderName, FCavrnusFolderCallback OnRecvFullFolderPath)
{
	TFunction<void(FString)> callback = [OnRecvFullFolderPath](const FString& fullFolderPath)
		{
			OnRecvFullFolderPath.ExecuteIfBound(fullFolderPath);
		};

	RequestContentDestinationFolder(FolderName, callback);
}

void UCavrnusFunctionLibrary::RequestContentDestinationFolder(FString FolderName, const TFunction<void(FString)>& OnRecvFullFolderPath)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterFolderReq(OnRecvFullFolderPath);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildFolderReq(RequestId, FolderName));
}

UPARAM(DisplayName = "Disposable")UCavrnusBinding* UCavrnusFunctionLibrary::BindChatMessages(const FCavrnusSpaceConnection& spaceConn, const FCavrnusChatFunction& ChatAdded, const FCavrnusChatFunction& ChatUpdated, const FCavrnusChatRemovedFunction& ChatRemoved)
{
	CavrnusChatFunction addedCallback = [ChatAdded](const FChatEntry& v)
	{
		ChatAdded.ExecuteIfBound(v);
	};
	CavrnusChatFunction updatedCallback = [ChatUpdated](const FChatEntry& v)
	{
		ChatUpdated.ExecuteIfBound(v);
	};
	CavrnusChatRemovedFunction removedCallback = [ChatRemoved](const FString& v)
	{
		ChatRemoved.ExecuteIfBound(v);
	};

	return BindChatMessages(spaceConn, addedCallback, updatedCallback, removedCallback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindChatMessages(const FCavrnusSpaceConnection& spaceConn, const CavrnusChatFunction& ChatAdded, const CavrnusChatFunction& ChatUpdated, const CavrnusChatRemovedFunction& ChatRemoved)
{
	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(spaceConn)->ChatModel->BindChatEvents(ChatAdded, ChatUpdated, ChatRemoved);
}

void UCavrnusFunctionLibrary::PostChatMessage(const FCavrnusSpaceConnection& spaceConn, const FString& Chat)
{
	CheckErrors(spaceConn);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildPostChatEntry(spaceConn, Chat));
}

FString UCavrnusFunctionLibrary::CreateBindingId(Cavrnus::CavrnusUnbind bindingCallback)
{
	auto bindingId = Cavrnus::CavrnusBindingModel::GetBindingModel()->RegisterBinding(bindingCallback);
	return bindingId;
}

void UCavrnusFunctionLibrary::UnbindWithId(FString bindingId)
{
	Cavrnus::CavrnusBindingModel::GetBindingModel()->UnbindBinding(bindingId);
}

FDelegateHandle UCavrnusFunctionLibrary::CavrnusShutdownHandle;
bool UCavrnusFunctionLibrary::ShutdownHooked;
void UCavrnusFunctionLibrary::HookCavrnusShutdown()
{
	if (ShutdownHooked)
	{
		return;
	}
	UiFlowTeardownHandle = FWorldDelegates::OnWorldCleanup.AddStatic(&UCavrnusFunctionLibrary::ShutdownCavrnusSystems);
	ShutdownHooked = true;
}

#pragma endregion

bool UCavrnusFunctionLibrary::CheckErrors(FCavrnusSpaceConnection SpaceConnection)
{
	// If the SpaceConnectionId is -1, it indicates an invalid or uninitialized connection.
	if (SpaceConnection.SpaceConnectionId == -1)
	{
		// Log an error message indicating that no valid CavrnusSpaceConnection was provided.
		UE_LOG(LogCavrnusConnector, Error, TEXT("No CavrnusSpaceConnection provided, function will not execute!"));
		return false;			// Return false to indicate that the SpaceConnectionId is invalid.
	}
	return true;				// If the SpaceConnectionId is valid, return true.
}

void UCavrnusFunctionLibrary::ShutdownCavrnusSystems(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (World && World->IsGameWorld())
	{
		UiFlowTeardownHandle.Reset();
		ShutdownHooked = false;
		HooksSetUp = false;
		bSpaceJoinBound = false;

		SpawnObjectHelpers::Kill();
		FCavrnusRestApiClient::CancelPendingRequests();
		Cavrnus::CavrnusRelayModel::KillDataModel();
	}
}

void UCavrnusFunctionLibrary::BindSpaceJoin()
{
	AwaitAnySpaceConnection([](const FCavrnusSpaceConnection& spaceConn) { SetupJoinedSpace(spaceConn); });
}

void UCavrnusFunctionLibrary::SetupJoinedSpace(const FCavrnusSpaceConnection& spaceConn)
{
	CavrnusSpaceUserEvent userAdded = [](const FCavrnusUser& user) {
		UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
		if (Subsystem && Subsystem->IsRuntimeContextReady())
		{
			if (UCavrnusPawnManager* PawnManager = Subsystem->RuntimeContext->Get<UCavrnusPawnManager>())
			{
				PawnManager->RegisterUser(user);
			}
		}
	};
	CavrnusSpaceUserEvent userRemoved = [](const FCavrnusUser& user) {
		UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
		if (Subsystem && Subsystem->IsRuntimeContextReady())
		{
			if (UCavrnusPawnManager* PawnManager = Subsystem->RuntimeContext->Get<UCavrnusPawnManager>())
			{
				PawnManager->UnregisterUser(user);
			}
		}
	};

	auto bnd = BindSpaceUsers(spaceConn, userAdded, userRemoved);
	CavrnusGCManager::GetGCManager()->TrackItem(bnd);

	AwaitAnySpaceExited([bnd]
		{
			CavrnusGCManager::GetGCManager()->UntrackItem(bnd);
		
			UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
			if (Subsystem && Subsystem->IsRuntimeContextReady())
			{
				if (UCavrnusPawnManager* PawnManager = Subsystem->RuntimeContext->Get<UCavrnusPawnManager>())
				{
					PawnManager->Clear();
				}
				if (USpawnedObjectsManager* SpawnedObjectsManager = Subsystem->RuntimeContext->Get<USpawnedObjectsManager>())
				{
					SpawnedObjectsManager->Clear();
				}
			}
		
			BindSpaceJoin();
		});
}

#pragma region ExposeOnSpawn Helpers

void UCavrnusFunctionLibrary::SetExposeOnSpawnBoolProperty(AActor* Actor, const FString& PropertyName, bool Value)
{
	if (!Actor || PropertyName.IsEmpty())
	{
		return;
	}

	UClass* ActorClass = Actor->GetClass();
	FProperty* Prop = FindFProperty<FProperty>(ActorClass, *PropertyName);
	if (!Prop)
	{
		return;
	}

	void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Actor);
	if (!ValuePtr)
	{
		return;
	}

	Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::BoolPropValue(Value);
	FCavrnusSpawnPropertyHelpers::CavrnusValueToProperty(CavrnusValue, Prop, ValuePtr);
}

void UCavrnusFunctionLibrary::SetExposeOnSpawnFloatProperty(AActor* Actor, const FString& PropertyName, float Value)
{
	if (!Actor || PropertyName.IsEmpty())
	{
		return;
	}

	UClass* ActorClass = Actor->GetClass();
	FProperty* Prop = FindFProperty<FProperty>(ActorClass, *PropertyName);
	if (!Prop)
	{
		return;
	}

	void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Actor);
	if (!ValuePtr)
	{
		return;
	}

	Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::FloatPropValue(Value);
	FCavrnusSpawnPropertyHelpers::CavrnusValueToProperty(CavrnusValue, Prop, ValuePtr);
}

void UCavrnusFunctionLibrary::SetExposeOnSpawnStringProperty(AActor* Actor, const FString& PropertyName, const FString& Value)
{
	if (!Actor || PropertyName.IsEmpty())
	{
		return;
	}

	UClass* ActorClass = Actor->GetClass();
	FProperty* Prop = FindFProperty<FProperty>(ActorClass, *PropertyName);
	if (!Prop)
	{
		return;
	}

	void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Actor);
	if (!ValuePtr)
	{
		return;
	}

	Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::StringPropValue(Value);
	FCavrnusSpawnPropertyHelpers::CavrnusValueToProperty(CavrnusValue, Prop, ValuePtr);
}

void UCavrnusFunctionLibrary::SetExposeOnSpawnVectorProperty(AActor* Actor, const FString& PropertyName, FVector4 Value)
{
	if (!Actor || PropertyName.IsEmpty())
	{
		return;
	}

	UClass* ActorClass = Actor->GetClass();
	FProperty* Prop = FindFProperty<FProperty>(ActorClass, *PropertyName);
	if (!Prop)
	{
		return;
	}

	void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Actor);
	if (!ValuePtr)
	{
		return;
	}

	Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::VectorPropValue(Value);
	FCavrnusSpawnPropertyHelpers::CavrnusValueToProperty(CavrnusValue, Prop, ValuePtr);
}

void UCavrnusFunctionLibrary::SetExposeOnSpawnTransformProperty(AActor* Actor, const FString& PropertyName, FTransform Value)
{
	if (!Actor || PropertyName.IsEmpty())
	{
		return;
	}

	UClass* ActorClass = Actor->GetClass();
	FProperty* Prop = FindFProperty<FProperty>(ActorClass, *PropertyName);
	if (!Prop)
	{
		return;
	}

	void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Actor);
	if (!ValuePtr)
	{
		return;
	}

	Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::TransformPropValue(Value);
	FCavrnusSpawnPropertyHelpers::CavrnusValueToProperty(CavrnusValue, Prop, ValuePtr);
}

void UCavrnusFunctionLibrary::SetExposeOnSpawnColorProperty(AActor* Actor, const FString& PropertyName, FLinearColor Value)
{
	if (!Actor || PropertyName.IsEmpty())
	{
		return;
	}

	UClass* ActorClass = Actor->GetClass();
	FProperty* Prop = FindFProperty<FProperty>(ActorClass, *PropertyName);
	if (!Prop)
	{
		return;
	}

	void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Actor);
	if (!ValuePtr)
	{
		return;
	}

	Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::ColorPropValue(Value);
	FCavrnusSpawnPropertyHelpers::CavrnusValueToProperty(CavrnusValue, Prop, ValuePtr);
}

#pragma endregion

