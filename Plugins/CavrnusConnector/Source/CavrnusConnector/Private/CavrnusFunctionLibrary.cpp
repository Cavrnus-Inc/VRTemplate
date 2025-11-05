// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusFunctionLibrary.h"
#include "CavrnusConnectorModule.h"
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

#include "RelayModel/CavrnusBindingModel.h"
#include "SpawnObjectHelpers.h"
#include "SpawnedObjectsManager.h"
#include "CavrnusGCManager.h"
#include "CavrnusSubsystem.h"
#include "Managers/Login/CavrnusLoginConfig.h"
#include "Managers/Login/CavrnusLoginManager.h"
#include "Pawns/CavrnusPawnManager.h"
#include "UI/CavrnusUI.h"

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

bool UCavrnusFunctionLibrary::HasLiveUiFlowManager = false;
FDelegateHandle UCavrnusFunctionLibrary::UiFlowTeardownHandle;


void UCavrnusFunctionLibrary::SetupUiFlowManager(const FCavrnusLoginConfig& LoginConfig)
{
	if (HasLiveUiFlowManager)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("A UI Flow Manager is already running."))
		return;
	}

	HasLiveUiFlowManager = true;
	UCavrnusSubsystem::Get()->Services->Get<UCavrnusLoginManager>()->DoLogin("runtime", LoginConfig);
}

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
	HasLiveUiFlowManager = true;
	UCavrnusSubsystem::Get()->Services->Get<UCavrnusLoginManager>()->DoPluginSettingsLogin();
}

bool UCavrnusFunctionLibrary::HooksSetUp = false;

void UCavrnusFunctionLibrary::SetupCavrnusEventHooks()
{
	if (HooksSetUp)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("Event hooks have already been established for Cavrnus.  This only needs to happen once."));
		return;
	}
	HooksSetUp = true;

	TFunction<AActor* (FCavrnusSpawnedObject, FString)> onObjectCreation = [](const FCavrnusSpawnedObject& ob, FString uniqueId)
		{
			return SpawnedObjectsManager::GetSpawnedObjectsManager()->RegisterSpawnedObject(ob, uniqueId, GEngine->GameViewport->GetWorld());
		};
	Cavrnus::CavrnusRelayModel::GetDataModel()->RegisterObjectCreationCallback(onObjectCreation);

	TFunction<void(FCavrnusSpawnedObject)> onObjectDestruction = [](const FCavrnusSpawnedObject& ob)
		{
			SpawnedObjectsManager::GetSpawnedObjectsManager()->UnregisterSpawnedObject(ob, GEngine->GameViewport->GetWorld());
		};
	Cavrnus::CavrnusRelayModel::GetDataModel()->RegisterObjectDestructionCallback(onObjectDestruction);

	BindSpaceJoin();

	UCavrnusFunctionLibrary::HookCavrnusShutdown();
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

void UCavrnusFunctionLibrary::ExitSpace(FCavrnusSpaceConnection SpaceConnection)
{
	CheckErrors(SpaceConnection);
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildExitSpaceMsg(SpaceConnection));
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

#pragma region Permissions

UPARAM(DisplayName = "Disposable")UCavrnusBinding* UCavrnusFunctionLibrary::BindGlobalPolicy(const FString& Policy, FCavrnusPolicyUpdated OnPolicyUpdated)
{
	CavrnusPolicyUpdated callback = [OnPolicyUpdated](const FString& policy, bool allowed)
	{
		OnPolicyUpdated.ExecuteIfBound(policy, allowed);
	};

	return BindGlobalPolicy(Policy, callback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindGlobalPolicy(FString Policy, CavrnusPolicyUpdated OnPolicyUpdated)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRequestGlobalPermission(Policy));

	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetGlobalPermissionsModel()->BindPolicyAllowed(Policy, OnPolicyUpdated);
}

UPARAM(DisplayName = "Disposable")UCavrnusBinding* UCavrnusFunctionLibrary::BindSpacePolicy(FCavrnusSpaceConnection SpaceConnection, const FString& Policy, FCavrnusPolicyUpdated OnPolicyUpdated)
{
	CavrnusPolicyUpdated callback = [OnPolicyUpdated](const FString& policy, bool allowed)
	{
		OnPolicyUpdated.ExecuteIfBound(policy, allowed);
	};

	return BindSpacePolicy(SpaceConnection, Policy, callback);
}

UCavrnusBinding* UCavrnusFunctionLibrary::BindSpacePolicy(FCavrnusSpaceConnection SpaceConnection, const FString& Policy, CavrnusPolicyUpdated OnPolicyUpdated)
{
	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildRequestSpacePermission(SpaceConnection, Policy));

	return Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePermissionsModel(SpaceConnection)->BindPolicyAllowed(Policy, OnPolicyUpdated);
}

#pragma endregion


#pragma region Spawned Objects

const FCavrnusSpawnedObject& UCavrnusFunctionLibrary::SpawnObject(FCavrnusSpaceConnection SpaceConnection, const FString& UniqueIdentifier)
{
	CheckErrors(SpaceConnection);

	FString InstanceId = Cavrnus::CavrnusProtoTranslation::CreateTransientId();

	Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildCreateOp(SpaceConnection, UniqueIdentifier, InstanceId));

	//CavrnusSpawnedObjectFunction* CallbackPtr = new CavrnusSpawnedObjectFunction(spawnedObjectArrived);
	//Cavrnus::CavrnusRelayModel::GetDataModel()->ObjectCreationCallbacks.Add(FAbsolutePropertyId(InstanceId), CallbackPtr);

	Cavrnus::CavrnusRelayModel::GetDataModel()->HandleSpaceObjectAdded(Cavrnus::CavrnusProtoTranslation::BuildObjectAdded(SpaceConnection, UniqueIdentifier, InstanceId));
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
	SpawnedObjectsManager::GetSpawnedObjectsManager()->RegisterSpawnableObjectType(Identifier, ClassType);
}

void UCavrnusFunctionLibrary::UnregisterSpawnableObjectType(const FString& Identifier)
{
	SpawnedObjectsManager::GetSpawnedObjectsManager()->UnregisterSpawnableObjectType(Identifier);
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

void UCavrnusFunctionLibrary::QueryAiAboutCurrentSpace(const FCavrnusSpaceConnection& spaceConn, const FString& Question, const FCavrnusAiResponseFunction& onSuccess, const FCavrnusError& onFailure)
{
	TFunction<void(FString)> successCallback = [onSuccess](const FString& v)
		{
			onSuccess.ExecuteIfBound(v);
		};
	CavrnusError errorCallback = [onFailure](const FString& v)
		{
			onFailure.ExecuteIfBound(v);
		};

	QueryAiAboutCurrentSpace(spaceConn, Question, successCallback, errorCallback);
}

void UCavrnusFunctionLibrary::QueryAiAboutCurrentSpace(const FCavrnusSpaceConnection& spaceConn, const FString& Question, const TFunction<void(FString)>& onSuccess, const CavrnusError& onFailure)
{
	int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterAiReq(onSuccess, onFailure);
	if (UCavrnusSubsystem* CavrnusSubsystem = UCavrnusSubsystem::Get())
	{
		if (const UCavrnusConnectorSettings* CavrnusConnectorSettings = CavrnusSubsystem->GetSettings())
		{
			// FString OpenApiKey = CavrnusConnectorSettings->OpenAiApiKey;
			// Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildAiRequest(spaceConn, RequestId, Question, OpenApiKey));
		}
	}
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
		UE_LOG(LogCavrnusConnector, Warning, TEXT("We have already hooked and prepared for Cavrnus Shutdown."))
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
		HasLiveUiFlowManager = false;

		UCavrnusPawnManager* PawnManager = UCavrnusSubsystem::Get()->Services->Get<UCavrnusPawnManager>();
		if (PawnManager)
		{
			UE_LOG(LogTemp, Warning, TEXT("UCavrnusFunctionLibrary::ShutdownCavrnusSystems: Calling Teardown"));
			PawnManager->Teardown();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UCavrnusFunctionLibrary::ShutdownCavrnusSystems: No Pawn Manager found during shutdown."));
		}
		SpawnObjectHelpers::Kill();
		SpawnedObjectsManager::Kill();
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
		UCavrnusSubsystem::Get()->Services->Get<UCavrnusPawnManager>()->RegisterUser(user);
		};
	CavrnusSpaceUserEvent userRemoved = [](const FCavrnusUser& user) {
		UCavrnusSubsystem::Get()->Services->Get<UCavrnusPawnManager>()->UnregisterUser(user);
		};

	auto bnd = BindSpaceUsers(spaceConn, userAdded, userRemoved);
	CavrnusGCManager::GetGCManager()->TrackItem(bnd);

	AwaitAnySpaceExited([bnd]
		{
			CavrnusGCManager::GetGCManager()->UntrackItem(bnd);

			UCavrnusSubsystem::Get()->Services->Get<UCavrnusPawnManager>()->Clear();
			SpawnedObjectsManager::GetSpawnedObjectsManager()->Clear();

			BindSpaceJoin();
		});
}

