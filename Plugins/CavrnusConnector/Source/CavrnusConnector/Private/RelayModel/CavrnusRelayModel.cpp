// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayModel/CavrnusRelayModel.h"
#include "CavrnusConnectorSettings.h"
#include "RelayModel/SpacePropertyModel.h"
#include "RelayModel/SpacePermissionsModel.h"
#include "Interop/CavrnusInteropLayer.h"
#include "Interop/CavrnusInteropLayerNet.h"
#include "Interop/CavrnusInteropLayerNative.h"
#include "RelayModel/RelayCallbackModel.h"
#include "RelayModel/DataState.h"
#include "RelayModel/CavrnusDataCache.h"
#include <HAL/PlatformTime.h>
#include "CoreMinimal.h"
#include "Types/CavrnusSpawnedObject.h"
#include "CavrnusSubsystem.h"

namespace Cavrnus
{
	CavrnusRelayModel* CavrnusRelayModel::Instance = nullptr;

	CavrnusRelayModel::CavrnusRelayModel()
	{
		// Get settings class
		const UCavrnusConnectorSettings* settings = UCavrnusSubsystem::Get()->GetSettings();
		if (!settings)
		{
			UE_LOG(LogCavrnusConnector, Log, TEXT("[CavrnusRelayModel::CavrnusRelayModel()] Could not get UCavrnusConnectorSettings class."));
			return;
		}

		// Now get our network settings
		FString RelayNetIPAddress = "127.0.0.1";

		//interopLayer = new CavrnusInteropLayerNet(RelayNetIPAddress, RelayNetPort);
		interopLayer = new CavrnusInteropLayerNative();

		dataState = new DataState();
		callbackModel = new RelayCallbackModel(this);
		dataCache = new CavrnusDataCache();

		interopLayer->Start();
	}

	CavrnusRelayModel::~CavrnusRelayModel()
	{
		delete interopLayer;
		delete callbackModel;

		for (const auto& kvp : spacePermissionsModelLookup)
		{
			delete kvp.Value;
		}
		spacePermissionsModelLookup.Empty();

		for (const auto& kvp : spacePropertyModelLookup)
		{
			delete kvp.Value;
		}
		spacePropertyModelLookup.Empty();
	}

	CavrnusRelayModel* CavrnusRelayModel::GetDataModel()
	{
		if (Instance == nullptr)
			Instance = new Cavrnus::CavrnusRelayModel();
		return Instance;
	}

	void CavrnusRelayModel::KillDataModel()
	{
		if (Instance != nullptr)
		{
			delete Instance;
			Instance = nullptr;
		}
	}

	bool CavrnusRelayModel::IsTickableInEditor() const
	{
		return true;
	}

	void CavrnusRelayModel::Tick(float DeltaTime)
	{
		double CurrentTime = FPlatformTime::Seconds();

		auto msg = Cavrnus::CavrnusProtoTranslation::BuildUpdateTimeMsg(CurrentTime);

		interopLayer->SendMessage(msg);

		interopLayer->DoTick();

		interopLayer->ReceiveMessages(this);

		/*auto receivedMessages = interopLayer->GetReceivedMessages();
		for (int i = 0; i < receivedMessages.Num(); i++)
		{
			HandleServerMsg(receivedMessages[i]);
		}*/
	}

	void CavrnusRelayModel::ReceiveMessage(const ServerData::RelayRemoteMessage& msg)
	{
		HandleServerMsg(msg);
	}

	TStatId CavrnusRelayModel::GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(CavrnusRelayModel, STATGROUP_Tickables);
	}

	SpacePermissionsModel* CavrnusRelayModel::GetGlobalPermissionsModel()
	{
		if (globalPermissionsModel == nullptr)
			globalPermissionsModel = new SpacePermissionsModel();

		return globalPermissionsModel;
	}

	SpacePermissionsModel* CavrnusRelayModel::GetSpacePermissionsModel(FCavrnusSpaceConnection spaceConn)
	{
		if (!spacePermissionsModelLookup.Contains(spaceConn.SpaceConnectionId))
			spacePermissionsModelLookup.Add(spaceConn.SpaceConnectionId, new SpacePermissionsModel());

		return spacePermissionsModelLookup[spaceConn.SpaceConnectionId];
	}

	SpacePropertyModel* CavrnusRelayModel::GetSpacePropertyModel(FCavrnusSpaceConnection spaceConn)
	{
		if (!spacePropertyModelLookup.Contains(spaceConn.SpaceConnectionId))
			spacePropertyModelLookup.Add(spaceConn.SpaceConnectionId, new SpacePropertyModel());

		return spacePropertyModelLookup[spaceConn.SpaceConnectionId];
	}

	RelayCallbackModel* CavrnusRelayModel::GetCallbackModel()
	{
		return callbackModel;
	}

	DataState* CavrnusRelayModel::GetDataState()
	{
		return dataState;
	}

	CavrnusDataCache* CavrnusRelayModel::GetDataCache()
	{
		return dataCache;
	}


	void CavrnusRelayModel::SendMessage(const ServerData::RelayClientMessage& msg)
	{
		interopLayer->SendMessage(msg);
	}

	void CavrnusRelayModel::RegisterObjectCreationCallback(TFunction<AActor* (FCavrnusSpawnedObject, FString)> cb)
	{
		ObjectCreationCallback = new TFunction<AActor* (FCavrnusSpawnedObject, FString)>(cb);
	}

	void CavrnusRelayModel::RegisterObjectDestructionCallback(TFunction<void(FCavrnusSpawnedObject)> cb)
	{
		ObjectDestructionCallback = new TFunction<void(FCavrnusSpawnedObject)>(cb);
	}

	void CavrnusRelayModel::HandleServerMsg(const ServerData::RelayRemoteMessage& msg)
	{
		/*bool shouldPrint = true;

		if (msg.Msg_case() == ServerData::RelayRemoteMessage::kPropertyValueStatus) {
			std::string propId = msg.propertyvaluestatus().propertypath().id();
			if (propId._Starts_with("/users") || propId._Starts_with("/date") || propId._Starts_with("/time"))
				shouldPrint = false;
		}

		if (msg.Msg_case() == ServerData::RelayRemoteMessage::kUserVideoFrame)
			shouldPrint = false;



		if (shouldPrint)
		{
			FString output = msg.DebugString().c_str();

			UE_LOG(LogCavrnusConnector, Log, TEXT("CLIENT RECV: %s"), *output);
		}*/

		switch (msg.Msg_case())
		{
		case ServerData::RelayRemoteMessage::kRelayDataCache:
			HandleInitialDataCache(msg.relaydatacache());
			break;
		case ServerData::RelayRemoteMessage::kPropertyValueStatus:
			HandleServerPropertyUpdate(msg.propertyvaluestatus());
			break;
		case ServerData::RelayRemoteMessage::kAuthenticateResp:
			callbackModel->HandleServerCallback(msg.authenticateresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kAuthenticateDeviceCodeBeginResp:
			callbackModel->HandleServerCallback(msg.authenticatedevicecodebeginresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kConstructApiKeyResp:
			callbackModel->HandleServerCallback(msg.constructapikeyresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kCheckServerStatusResp:
			callbackModel->HandleServerCallback(msg.checkserverstatusresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kJoinSpaceFromIdResp:
			callbackModel->HandleServerCallback(msg.joinspacefromidresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kConnectionStatusResp:
			callbackModel->HandleSpaceStatus(msg.connectionstatusresp());
			break;
		case ServerData::RelayRemoteMessage::kServerMessageResp:
			callbackModel->HandleServerMessage(msg.servermessageresp());
			break;
		case ServerData::RelayRemoteMessage::kGenericResponse:
			callbackModel->HandleServerCallback(msg.genericresponse().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kShutdownSpaceConnectionResp:
			dataState->RemoveSpaceConnection(msg.shutdownspaceconnectionresp().spaceconn().spaceconnectionid());
			break;
		case ServerData::RelayRemoteMessage::kGetAudioInputDevicesResp:
			callbackModel->HandleServerCallback(msg.getaudioinputdevicesresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kGetAudioOutputDevicesResp:
			callbackModel->HandleServerCallback(msg.getaudiooutputdevicesresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kGetVideoInputDevicesResp:
			callbackModel->HandleServerCallback(msg.getvideoinputdevicesresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kContentDestinationFolderResp:
			callbackModel->HandleServerCallback(msg.contentdestinationfolderresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kGetSpaceInfoResp:
			callbackModel->HandleServerCallback(msg.getspaceinforesp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kMessage:
			HandleLogging(msg.message());
			break;
		case ServerData::RelayRemoteMessage::kSpaceAdded:
			dataState->AddJoinableSpace(CavrnusProtoTranslation::ToSpaceInfo(msg.spaceadded()));
			break;
		case ServerData::RelayRemoteMessage::kSpaceUpdated:
			dataState->UpdateJoinableSpace(CavrnusProtoTranslation::ToSpaceInfo(msg.spaceupdated()));
			break;
		case ServerData::RelayRemoteMessage::kSpaceRemoved:
			dataState->RemoveJoinableSpace(CavrnusProtoTranslation::ToSpaceInfo(msg.spaceremoved()));
			break;
		case ServerData::RelayRemoteMessage::kRemoteContentAdded:
			dataState->AddRemoteContent(CavrnusProtoTranslation::ToRemoteContent(msg.remotecontentadded().remotecontent()));
			break;
		case ServerData::RelayRemoteMessage::kRemoteContentRemoved:
			dataState->RemoveRemoteContent(UTF8_TO_TCHAR(msg.remotecontentremoved().id().c_str()));
			break;
		case ServerData::RelayRemoteMessage::kUserAdded:
			HandleSpaceUserAdded(msg.useradded().spaceconn(), msg.useradded().user());
			break;
		case ServerData::RelayRemoteMessage::kUserRemoved:
			HandleSpaceUserRemoved(msg.userremoved().spaceconn(), msg.userremoved().userid());
			break;
		case ServerData::RelayRemoteMessage::kUserVideoFrame:
			HandleSpaceUserVideoFrame(msg.uservideoframe());
			break;
		case ServerData::RelayRemoteMessage::kObjectAdded:
			HandleSpaceObjectAdded(msg.objectadded());
			break;
		case ServerData::RelayRemoteMessage::kObjectRemoved:
			HandleSpaceObjectRemoved(msg.objectremoved());
			break;
		case ServerData::RelayRemoteMessage::kPermissionStatus:
			HandlePermissionStatus(msg.permissionstatus());
			break;
		case ServerData::RelayRemoteMessage::kAllJoinableSpacesResp:
			callbackModel->HandleServerCallback(msg.alljoinablespacesresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kLocalPropertyHandledResp:
			HandleLocalPropHandledResp(msg.localpropertyhandledresp());
			break;
		case ServerData::RelayRemoteMessage::kPropMetadataStatus:
			HandlePropMetadataStatus(msg.propmetadatastatus());
			break;
		case ServerData::RelayRemoteMessage::kFetchFileByIdProgressResp:
			ContentModel.HandleProgressCallback(UTF8_TO_TCHAR(msg.fetchfilebyidprogressresp().contentid().c_str()), msg.fetchfilebyidprogressresp().progress(), UTF8_TO_TCHAR(msg.fetchfilebyidprogressresp().progressstep().c_str()));
			break;
		case ServerData::RelayRemoteMessage::kFetchFileByIdCompletedResp:
			ContentModel.HandleCompletionCallback(UTF8_TO_TCHAR(msg.fetchfilebyidcompletedresp().contentid().c_str()), UTF8_TO_TCHAR(msg.fetchfilebyidcompletedresp().filepath().c_str()), UTF8_TO_TCHAR(msg.fetchfilebyidcompletedresp().finalfilenamewithextension().c_str()));
			break;
		case ServerData::RelayRemoteMessage::kFetchFileByIdFailureResp:
			ContentModel.HandleFailureCallback(UTF8_TO_TCHAR(msg.fetchfilebyidfailureresp().contentid().c_str()), UTF8_TO_TCHAR(msg.fetchfilebyidfailureresp().errormsg().c_str()));
			break;
		case ServerData::RelayRemoteMessage::kFetchRemoteContentInfoResp:
			callbackModel->HandleServerCallback(msg.fetchremotecontentinforesp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kFetchAllUploadedContentResp:
			callbackModel->HandleServerCallback(msg.fetchalluploadedcontentresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kUploadLocalFileResp:
			callbackModel->HandleServerCallback(msg.uploadlocalfileresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kUploadLocalFileProgressResp:
			callbackModel->HandleServerCallback(msg.uploadlocalfileprogressresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kCreateSpaceResp:
			callbackModel->HandleServerCallback(msg.createspaceresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kFetchAllUserAccountsResp:
			callbackModel->HandleServerCallback(msg.fetchalluseraccountsresp().reqid(), msg);
			break;
		case ServerData::RelayRemoteMessage::kChatAdded:
			HandleChatAdded(msg.chatadded());
			break;
		case ServerData::RelayRemoteMessage::kChatUpdated:
			HandleChatUpdated(msg.chatupdated());
			break;
		case ServerData::RelayRemoteMessage::kChatRemoved:
			HandleChatRemoved(msg.chatremoved());
			break;
		case ServerData::RelayRemoteMessage::kQueryAiResp:
			callbackModel->HandleServerCallback(msg.queryairesp().reqid(), msg);
			break;
		default:
			UE_LOG(LogCavrnusConnector, Warning, TEXT("Unhandled server message, message type: %d"), static_cast<int>(msg.Msg_case()));
			break;
		}
	}

	void CavrnusRelayModel::HandleLogging(const ServerData::StatusMessage& message)
	{
		switch (message.Message_case())
		{
		case ServerData::StatusMessage::kLog:
			UE_LOG(LogCavrnusConnector, Log, TEXT("%s"), UTF8_TO_TCHAR(message.log().c_str()));
			break;
		case ServerData::StatusMessage::kWarning:
			UE_LOG(LogCavrnusConnector, Warning, TEXT("%s"), UTF8_TO_TCHAR(message.warning().c_str()));
			break;
		case ServerData::StatusMessage::kError:
			UE_LOG(LogCavrnusConnector, Error, TEXT("%s"), UTF8_TO_TCHAR(message.error().c_str()));
			break;
		default:
			break;
		}
	}

	void CavrnusRelayModel::HandleInitialDataCache(const ServerData::RelayDataCache& cache)
	{
		TArray<FString> keys;
		TArray<FString> vals;
		for (int i = 0; i < cache.stringkeys_size(); i++)
		{
			keys.Add(UTF8_TO_TCHAR(cache.stringkeys()[i].c_str()));
			vals.Add(UTF8_TO_TCHAR(cache.stringvalues()[i].c_str()));
		}
		GetDataCache()->Setup(keys, vals);

		callbackModel->HandleDataCache(cache);
	}

	void CavrnusRelayModel::HandleSpaceUserAdded(ServerData::CavrnusSpaceConnection spaceConn, ServerData::CavrnusUser user)
	{
		if (!spacePropertyModelLookup.Contains(spaceConn.spaceconnectionid()))
			spacePropertyModelLookup.Add(spaceConn.spaceconnectionid(), new SpacePropertyModel());

		spacePropertyModelLookup[spaceConn.spaceconnectionid()]->AddSpaceUser(CavrnusProtoTranslation::ToCavrnusUser(user, CavrnusProtoTranslation::SpaceConnectionFromPb(spaceConn)));
	}

	void CavrnusRelayModel::HandleSpaceUserRemoved(ServerData::CavrnusSpaceConnection spaceConn, std::string userId)
	{
		if (!spacePropertyModelLookup.Contains(spaceConn.spaceconnectionid()))
			spacePropertyModelLookup.Add(spaceConn.spaceconnectionid(), new SpacePropertyModel());

		auto spm = spacePropertyModelLookup[spaceConn.spaceconnectionid()];
		FString fUserId = UTF8_TO_TCHAR(userId.c_str());
		spm->RemoveSpaceUser(fUserId);
	}

	void CavrnusRelayModel::HandleSpaceUserVideoFrame(const ServerData::UserVideoFrame& VideoFrame)
	{
		if (spacePropertyModelLookup.Contains(VideoFrame.spaceconn().spaceconnectionid()))
		{
			TArray<uint8> StreamArray((uint8*)VideoFrame.stream().c_str(), VideoFrame.stream().size());
			spacePropertyModelLookup[VideoFrame.spaceconn().spaceconnectionid()]->UpdateUserVideoTexture(UTF8_TO_TCHAR(VideoFrame.userid().c_str()), VideoFrame.resx(), VideoFrame.resy(), StreamArray);
		}
	}

	void CavrnusRelayModel::HandleSpaceObjectAdded(const ServerData::ObjectAdded& ObjectAdded)
	{
		FCavrnusSpaceConnection spaceConn = Cavrnus::CavrnusProtoTranslation::SpaceConnectionFromPb(ObjectAdded.spaceconn());
		if (!spacePropertyModelLookup.Contains(spaceConn.SpaceConnectionId))
			return;

		FString propsContainerName = FString(UTF8_TO_TCHAR(ObjectAdded.propertiescontainer().c_str()));

		if (spacePropertyModelLookup[spaceConn.SpaceConnectionId]->SpawnedObjects.Contains(propsContainerName))
			return;

		FCavrnusSpawnedObject SpawnedObject;
		SpawnedObject.SpaceConnection = spaceConn;
		SpawnedObject.PropertiesContainerName = propsContainerName;

		AActor* spawnedInstance = (*ObjectCreationCallback)(SpawnedObject, UTF8_TO_TCHAR(ObjectAdded.objectcreated().c_str()));

		SpawnedObject.SpawnedActorInstance = spawnedInstance;

		spacePropertyModelLookup[spaceConn.SpaceConnectionId]->SpawnedObjects.Add(propsContainerName, SpawnedObject);
	}

	void CavrnusRelayModel::HandleSpaceObjectRemoved(const ServerData::ObjectRemoved& ObjectRemoved)
	{
		FCavrnusSpaceConnection spaceConn = Cavrnus::CavrnusProtoTranslation::SpaceConnectionFromPb(ObjectRemoved.spaceconn());
		if (!spacePropertyModelLookup.Contains(spaceConn.SpaceConnectionId))
			return;

		FString propsContainerName = FString(UTF8_TO_TCHAR(ObjectRemoved.propertiescontainer().c_str()));

		if (!spacePropertyModelLookup[spaceConn.SpaceConnectionId]->SpawnedObjects.Contains(propsContainerName))
			return;

		FCavrnusSpawnedObject SpawnedObject;
		SpawnedObject.SpaceConnection = spaceConn;
		SpawnedObject.PropertiesContainerName = propsContainerName;

		(*ObjectDestructionCallback)(SpawnedObject);

		spacePropertyModelLookup[ObjectRemoved.spaceconn().spaceconnectionid()]->SpawnedObjects.Remove(propsContainerName);
	}

	void CavrnusRelayModel::HandlePermissionStatus(const ServerData::PermissionStatus& PermissionStatus)
	{
		if (PermissionStatus.has_spaceconn())
		{
			int spaceConnId = PermissionStatus.spaceconn().spaceconnectionid();

			if (!spacePermissionsModelLookup.Contains(spaceConnId))
				spacePermissionsModelLookup[spaceConnId] = new SpacePermissionsModel();

			spacePermissionsModelLookup[spaceConnId]->UpdatePolicyAllowed(UTF8_TO_TCHAR(PermissionStatus.permission().c_str()), PermissionStatus.value());
		}
		else
		{
			GetGlobalPermissionsModel()->UpdatePolicyAllowed(UTF8_TO_TCHAR(PermissionStatus.permission().c_str()), PermissionStatus.value());
		}
	}

	void CavrnusRelayModel::HandleLocalPropHandledResp(const ServerData::LocalPropertyHandledResp& localPropHandled)
	{
		int spaceConnId = localPropHandled.spaceconn().spaceconnectionid();

		if (!spacePropertyModelLookup.Contains(spaceConnId))
			spacePropertyModelLookup.Add(spaceConnId, new SpacePropertyModel());

		spacePropertyModelLookup[spaceConnId]->InvalidateLocalPropValue(FAbsolutePropertyId(UTF8_TO_TCHAR(localPropHandled.propertypath().id().c_str())), localPropHandled.localpropchangeid());
	}

	void CavrnusRelayModel::HandlePropMetadataStatus(const ServerData::PropMetadataStatus& metadataStatus)
	{
	}

	void CavrnusRelayModel::HandleChatAdded(const ServerData::ChatAdded& chatAdded)
	{
		int spaceConnId = chatAdded.spaceconn().spaceconnectionid();

		if (!spacePropertyModelLookup.Contains(spaceConnId))
			spacePropertyModelLookup.Add(spaceConnId, new SpacePropertyModel());

		spacePropertyModelLookup[spaceConnId]->ChatModel->AddChat(CavrnusProtoTranslation::ToFChatEntry(UTF8_TO_TCHAR(chatAdded.chatpropertyid().c_str()), chatAdded.chatdata()));
	}

	void CavrnusRelayModel::HandleChatUpdated(const ServerData::ChatUpdated& chatUpdated)
	{
		int spaceConnId = chatUpdated.spaceconn().spaceconnectionid();

		if (!spacePropertyModelLookup.Contains(spaceConnId))
			spacePropertyModelLookup.Add(spaceConnId, new SpacePropertyModel());

		spacePropertyModelLookup[spaceConnId]->ChatModel->UpdateChat(CavrnusProtoTranslation::ToFChatEntry(UTF8_TO_TCHAR(chatUpdated.chatpropertyid().c_str()), chatUpdated.chatdata()));
	}

	void CavrnusRelayModel::HandleChatRemoved(const ServerData::ChatRemoved& chatRemoved)
	{
		int spaceConnId = chatRemoved.spaceconn().spaceconnectionid();

		if (!spacePropertyModelLookup.Contains(spaceConnId))
			spacePropertyModelLookup.Add(spaceConnId, new SpacePropertyModel());

		spacePropertyModelLookup[spaceConnId]->ChatModel->RemoveChat(UTF8_TO_TCHAR(chatRemoved.chatpropertyid().c_str()));
	}

	void CavrnusRelayModel::HandleServerPropertyUpdate(const ServerData::PropertyValueStatus& propStatus)
	{
		int spaceConnId = propStatus.spaceconn().spaceconnectionid();

		if (!spacePropertyModelLookup.Contains(spaceConnId))
			spacePropertyModelLookup.Add(spaceConnId, new SpacePropertyModel());

		const FString& propId = UTF8_TO_TCHAR(propStatus.propertypath().id().c_str());

		FPropertyValue val;

		switch (propStatus.propertyvalue().Value_case())
		{
		case ServerData::PropertyValue::kColorVal:
			val = FPropertyValue::ColorPropValue(Cavrnus::CavrnusProtoTranslation::ToFLinearColor(propStatus.propertyvalue().colorval()));
			break;
		case ServerData::PropertyValue::kBoolVal:
			val = FPropertyValue::BoolPropValue(propStatus.propertyvalue().boolval());
			break;
		case ServerData::PropertyValue::kScalarVal:
			val = FPropertyValue::FloatPropValue(propStatus.propertyvalue().scalarval());
			break;
		case ServerData::PropertyValue::kStringVal:
			val = FPropertyValue::StringPropValue(UTF8_TO_TCHAR(propStatus.propertyvalue().stringval().c_str()));
			break;
		case ServerData::PropertyValue::kVectorVal:
			val = FPropertyValue::VectorPropValue(Cavrnus::CavrnusProtoTranslation::ToFVector4(propStatus.propertyvalue().vectorval()));
			break;
		case ServerData::PropertyValue::kTransformVal:
			val = FPropertyValue::TransformPropValue(Cavrnus::CavrnusProtoTranslation::ToFTransform(propStatus.propertyvalue().transformval()));
			break;
		default:
			break;
		}

		spacePropertyModelLookup[spaceConnId]->UpdateServerPropVal(propId, val);
		spacePropertyModelLookup[spaceConnId]->UpdateServerPropPriority(propId, propStatus.priority());
	}
} // namespace Cavrnus
