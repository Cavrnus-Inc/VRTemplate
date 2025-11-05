// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayModel/RelayCallbackModel.h"
#include "CoreMinimal.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "CavrnusConnectorModule.h"
#include "Types\CavrnusRemoteContent.h"
#include "Types\CavrnusConnectionStatusEnum.h"

namespace Cavrnus
{
	RelayCallbackModel::RelayCallbackModel(CavrnusRelayModel* model)
	{
		relayModel = model;
	}

	RelayCallbackModel::~RelayCallbackModel()
	{
	}

	void RelayCallbackModel::HandleServerCallback(int callbackId, const ServerData::RelayRemoteMessage& msg)
	{
		switch (msg.Msg_case())
		{
		case ServerData::RelayRemoteMessage::kAuthenticateResp:
			HandleAuthenticationResponse(callbackId, msg.authenticateresp());
			break;
		case ServerData::RelayRemoteMessage::kAuthenticateDeviceCodeBeginResp:
			HandleDeviceCodeResponse(callbackId, msg.authenticatedevicecodebeginresp());
			break;
		case ServerData::RelayRemoteMessage::kConstructApiKeyResp:
			HandleConstructApiKeyResponse(callbackId, msg.constructapikeyresp());
			break;
		case ServerData::RelayRemoteMessage::kCheckServerStatusResp:
			HandleServerStatusResponse(callbackId, msg.checkserverstatusresp());
			break;
		case ServerData::RelayRemoteMessage::kCreateSpaceResp:
			HandleCreateSpaceResponse(callbackId, msg.createspaceresp());
			break;
		case ServerData::RelayRemoteMessage::kGenericResponse:
			HandleGenericResponse(callbackId, msg.genericresponse());
			break;
		case ServerData::RelayRemoteMessage::kJoinSpaceFromIdResp:
			HandleJoinSpaceResponse(callbackId, msg.joinspacefromidresp());
			break;
		case ServerData::RelayRemoteMessage::kGetAudioInputDevicesResp:
			HandleAudioInputsResponse(callbackId, msg.getaudioinputdevicesresp());
			break;
		case ServerData::RelayRemoteMessage::kGetAudioOutputDevicesResp:
			HandleAudioOutputsResponse(callbackId, msg.getaudiooutputdevicesresp());
			break;
		case ServerData::RelayRemoteMessage::kGetVideoInputDevicesResp:
			HandleVideoInputsResponse(callbackId, msg.getvideoinputdevicesresp());
			break;
		case ServerData::RelayRemoteMessage::kAllJoinableSpacesResp:
			HandleJoinableSpacesRecv(callbackId, msg.alljoinablespacesresp());
			break;
		case ServerData::RelayRemoteMessage::kFetchAllUploadedContentResp:
			HandleAllRemoteContentRecv(callbackId, msg.fetchalluploadedcontentresp());
			break;
		case ServerData::RelayRemoteMessage::kUploadLocalFileResp:
			HandleUploadComplete(callbackId, msg.uploadlocalfileresp());
			break;
		case ServerData::RelayRemoteMessage::kUploadLocalFileProgressResp:
			HandleUploadProgress(callbackId, msg.uploadlocalfileprogressresp());
			break;
		case ServerData::RelayRemoteMessage::kFetchRemoteContentInfoResp:
			HandleRemoteContentInfoComplete(callbackId, msg.fetchremotecontentinforesp());
			break;
		case ServerData::RelayRemoteMessage::kContentDestinationFolderResp:
			HandleFolderResp(callbackId, msg.contentdestinationfolderresp());
			break;
		case ServerData::RelayRemoteMessage::kFetchAllUserAccountsResp:
			HandleFetchAllUserAccountsComplete(callbackId, msg.fetchalluseraccountsresp());
			break;
		case ServerData::RelayRemoteMessage::kGetSpaceInfoResp:
			HandleFetchSpaceInfoComplete(callbackId, msg.getspaceinforesp());
			break;
		case ServerData::RelayRemoteMessage::kQueryAiResp:
			HandleAiResp(callbackId, msg.queryairesp());
			break;
		default:
			UE_LOG(LogCavrnusConnector, Warning, TEXT("Unhandled server callback, message type: %d"), static_cast<int>(msg.Msg_case()));
			break;
		}
		ReleaseErrorHandler(callbackId); // Release error handlers generically, because most calls register an error callback
	}

	int RelayCallbackModel::RegisterGenericCallback(CavrnusSuccess onSuccess, CavrnusError onFailure)
	{
		int reqId = ++currReqId;

		CavrnusSuccess* CallbackPtr = new CavrnusSuccess(onSuccess);
		CavrnusError* ErrorPtr = new CavrnusError(onFailure);

		GenericSuccessCallbacks.Add(reqId, CallbackPtr);
		GenericErrorCallbacks.Add(reqId, ErrorPtr);

		return reqId;
	}
	void RelayCallbackModel::HandleGenericResponse(int callbackId, ServerData::GenericResp resp)
	{
		if (resp.success())
		{
			//UE_LOG(LogCavrnusConnector, Log, TEXT("[GENERIC SUCCESS]"));
			if (GenericSuccessCallbacks.Contains(callbackId))
				(*GenericSuccessCallbacks[callbackId])();
		}
		else
		{
			FString error = UTF8_TO_TCHAR(resp.error().c_str());
			UE_LOG(LogCavrnusConnector, Log, TEXT("[GENERIC FAILURE]: %s"), *error);
			if (GenericErrorCallbacks.Contains(callbackId))
				(*GenericErrorCallbacks[callbackId])(error);
		}

		GenericSuccessCallbacks.Remove(callbackId);
		GenericErrorCallbacks.Remove(callbackId);
	}

	void RelayCallbackModel::RegisterGotDataCache(TFunction<void()> gotCache)
	{
		if (gotDataCache)
		{
			gotCache();
		}
		else
		{
			TFunction<void()>* callback = new TFunction<void()>(gotCache);
			DataCacheCallbacks.Add(callback);
		}
	}

	void RelayCallbackModel::HandleDataCache(const ServerData::RelayDataCache& dataCache)
	{
		savedDataCache = dataCache;
		gotDataCache = true;
		for (int i = 0; i < DataCacheCallbacks.Num(); i++)
		{
			(*DataCacheCallbacks[i])();
		}
		DataCacheCallbacks.Empty();
	}

	void RelayCallbackModel::SetServer(FString server)
	{
		relayModel->GetDataState()->CurrentServer = server;

		for (int i = 0; i < ServerSetCallbacks.Num(); i++)
		{
			(*ServerSetCallbacks[i])(server);
		}
		ServerSetCallbacks.Empty();
	}

	void RelayCallbackModel::HandleAuthRecv(FCavrnusAuthentication auth)
	{
		for (int i = 0; i < AuthCallbacks.Num(); i++) 
		{
			(*AuthCallbacks[i])(auth);
		}
		AuthCallbacks.Empty();
	}

	int RelayCallbackModel::RegisterAuthenticationCallback(CavrnusAuthRecv onSuccess, CavrnusError onFailure)
	{
		int reqId = ++currReqId;

		CavrnusAuthRecv* callback = new CavrnusAuthRecv(onSuccess);
		LoginAuthenticationCallbacks.Add(reqId, callback);

		CavrnusError* errorCallback = new CavrnusError(onFailure);
		GenericErrorCallbacks.Add(reqId, errorCallback);

		return reqId;
	}

	void RelayCallbackModel::HandleAuthenticationResponse(int callbackId, ServerData::AuthenticateResp resp)
	{
		if (resp.has_auth())
		{
			FString token = UTF8_TO_TCHAR(resp.auth().token().c_str());
			FString server = UTF8_TO_TCHAR(resp.auth().server().c_str());
			FString userdisplayname = UTF8_TO_TCHAR(resp.auth().displayname().c_str());
			FString orgname = UTF8_TO_TCHAR(resp.auth().orgname().c_str());

			FCavrnusAuthentication* auth = new FCavrnusAuthentication(token, server, userdisplayname, orgname);
			HandleAuthRecv(*auth);

			relayModel->GetDataState()->CurrentAuthentication = auth;

			UE_LOG(LogCavrnusConnector, Log, TEXT("[AUTH SUCCESS]"));

			if (LoginAuthenticationCallbacks.Contains(callbackId))
				(*LoginAuthenticationCallbacks[callbackId])(*auth);
		}

		LoginAuthenticationCallbacks.Remove(callbackId);

	}

	int RelayCallbackModel::RegisterDeviceCodeBeginCallback(CavrnusDeviceCodeRecv onSuccess, CavrnusError onFailure)
	{
		int reqId = ++currReqId;

		DeviceCodeCallbacks.Add(reqId, new CavrnusDeviceCodeRecv(onSuccess));
		GenericErrorCallbacks.Add(reqId, new CavrnusError(onFailure));

		return reqId;
	}

	int RelayCallbackModel::RegisterConstructApiKeyCallback(CavrnusApiKeyRecv onSuccess, CavrnusError onFailure)
	{
		int reqId = ++currReqId;

		ConstructApiKeyCallbacks.Add(reqId, new CavrnusApiKeyRecv(onSuccess));
		GenericErrorCallbacks.Add(reqId, new CavrnusError(onFailure));

		return reqId;
	}

	int RelayCallbackModel::RegisterServerStatusCallback(CavrnusServerStatusRecv onStatus)
	{
		int reqId = ++currReqId;

		CavrnusServerStatusRecv* callback = new CavrnusServerStatusRecv(onStatus);
		ServerStatusCallbacks.Add(reqId, callback);

		return reqId;
	}


	void RelayCallbackModel::RegisterSpaceStatusCallback(int spaceId, CavrnusSpaceStatusChanged onSpaceStatus)
	{
		CavrnusSpaceStatusChanged* callback = new CavrnusSpaceStatusChanged(onSpaceStatus);
		SpaceStatusCallbacks.FindOrAdd(spaceId);
		SpaceStatusCallbacks[spaceId].Add(callback);
	}
	void RelayCallbackModel::RegisterServerMessageCallback(int spaceId, CavrnusServerMessageEvent onServerMessage)
	{
		CavrnusServerMessageEvent* callback = new CavrnusServerMessageEvent(onServerMessage);
		ServerMessageCallbacks.FindOrAdd(spaceId);
		ServerMessageCallbacks[spaceId].Add(callback);
	}

	void RelayCallbackModel::HandleServerMessage(ServerData::ServerMessage msg)
	{
		if (ServerMessageCallbacks.Contains(msg.messageforconnection().spaceconnectionid()))
		{
			EServerMessageCategory cat = static_cast<EServerMessageCategory>(msg.category());
			FCavrnusServerMessage csmsg = FCavrnusServerMessage(cat, UTF8_TO_TCHAR(msg.message().c_str()));
			for (int i = 0; i < ServerMessageCallbacks[msg.messageforconnection().spaceconnectionid()].Num(); i++)
			{
				(*ServerMessageCallbacks[msg.messageforconnection().spaceconnectionid()][i])(csmsg);
			}
		}
	}

	void RelayCallbackModel::HandleSpaceStatus(ServerData::SpaceConnectionStatus msg)
	{
		if (SpaceStatusCallbacks.Contains(msg.statusforconnection().spaceconnectionid()))
		{
			ECavrnusConnectionStatusEnum statusenum = static_cast<ECavrnusConnectionStatusEnum>(msg.status());
			FCavrnusConnectionStatus connStatus = FCavrnusConnectionStatus(statusenum, UTF8_TO_TCHAR(msg.lasterror().c_str()));
			for (int i = 0; i < SpaceStatusCallbacks[msg.statusforconnection().spaceconnectionid()].Num(); i++)
			{
				(*SpaceStatusCallbacks[msg.statusforconnection().spaceconnectionid()][i])(connStatus);
			}
		}
	}



	void RelayCallbackModel::RegisterServerSetCallback(CavrnusServerSet onServerSet)
	{
		if (relayModel->GetDataState()->CurrentServer != "")
		{
			onServerSet(*relayModel->GetDataState()->CurrentServer);
		}
		else
		{
			CavrnusServerSet* callback = new CavrnusServerSet(onServerSet);
			ServerSetCallbacks.Add(callback);
		}
	}

	void RelayCallbackModel::RegisterAuthCallback(CavrnusAuthRecv onAuth)
	{
		if (relayModel->GetDataState()->CurrentAuthentication != nullptr)
		{
			onAuth(*relayModel->GetDataState()->CurrentAuthentication);
		}
		else 
		{
			CavrnusAuthRecv* callback = new CavrnusAuthRecv(onAuth);
			AuthCallbacks.Add(callback);
		}		
	}

	void RelayCallbackModel::HandleDeviceCodeResponse(int callbackId, ServerData::AuthenticateDeviceCodeBeginResp resp)
	{
		if (DeviceCodeCallbacks.Contains(callbackId))
		{
			FString devicecode = UTF8_TO_TCHAR(resp.devicecode().c_str());
			FString usercode = UTF8_TO_TCHAR(resp.usercode().c_str());
			FString verificationUrl = UTF8_TO_TCHAR(resp.verificationurl().c_str());

			(*DeviceCodeCallbacks[callbackId])(FCavrnusDeviceCodeData(devicecode, usercode, verificationUrl));
			DeviceCodeCallbacks.Remove(callbackId);
		}
	}
	void RelayCallbackModel::HandleConstructApiKeyResponse(int callbackId, ServerData::ConstructApiKeyResp resp)
	{
		if (ConstructApiKeyCallbacks.Contains(callbackId))
		{
			FString accesskey = UTF8_TO_TCHAR(resp.accesskey().c_str());
			FString accesstoken = UTF8_TO_TCHAR(resp.accesstoken().c_str());

			(*ConstructApiKeyCallbacks[callbackId])(FCavrnusApiKeyData(accesskey, accesstoken));
			ConstructApiKeyCallbacks.Remove(callbackId);
		}
	}

	void RelayCallbackModel::HandleServerStatusResponse(int callbackId, ServerData::CheckServerStatusResp resp)
	{
		FCavrnusServerStatus servStatus = FCavrnusServerStatus(resp.live(), CavrnusProtoTranslation::ToCavrnusOrganizationInfo(resp.info()), UTF8_TO_TCHAR(resp.failreason().c_str()));
		if (ServerStatusCallbacks.Contains(callbackId))
		{
			(*ServerStatusCallbacks[callbackId])(servStatus);
			ServerStatusCallbacks.Remove(callbackId);
		}
	}

	void RelayCallbackModel::RegisterBeginLoadingSpaceCallback(CavrnusSpaceBeginLoading onBeginLoading)
	{
		CavrnusSpaceBeginLoading* CallbackPtr = new CavrnusSpaceBeginLoading(onBeginLoading);
		BeginLoadingSpaceCallbacks.Add(CallbackPtr);
	}

	void RelayCallbackModel::HandleSpaceBeginLoading(FString spaceId)
	{
		for (int i = 0; i < BeginLoadingSpaceCallbacks.Num(); i++) 
		{
			(*BeginLoadingSpaceCallbacks[i])(spaceId);
		}
		BeginLoadingSpaceCallbacks.Empty();
	}

	void RelayCallbackModel::RegisterEndLoadingSpaceCallback(CavrnusSpaceEndLoading onEndLoading)
	{
		CavrnusSpaceEndLoading* CallbackPtr = new CavrnusSpaceEndLoading(onEndLoading);
		EndLoadingSpaceCallbacks.Add(CallbackPtr);
	}

	int RelayCallbackModel::RegisterFetchCavrnusUserAccounts(CavrnusUserAccountsFetched onFetchComplete)
	{
		int reqId = ++currReqId;

		CavrnusUserAccountsFetched* CallbackPtr = new CavrnusUserAccountsFetched(onFetchComplete);

		CavrnusUserAccountsFetchedCallbacks.Add(reqId, CallbackPtr);

		return reqId;
	}

	void RelayCallbackModel::HandleFetchAllUserAccountsComplete(int callbackId, ServerData::FetchAllUserAccountsResp resp)
	{
		TArray<FCavrnusUserAccount> UserAccounts;

		for (int i = 0; i < resp.alluseraccounts().size(); i++)
		{
			UserAccounts.Add(CavrnusProtoTranslation::ToCavrnusUserAccount(resp.alluseraccounts()[i]));
		}

		if (CavrnusUserAccountsFetchedCallbacks.Contains(callbackId))
			(*CavrnusUserAccountsFetchedCallbacks[callbackId])(UserAccounts);

		CavrnusUserAccountsFetchedCallbacks.Remove(callbackId);
	}

	int RelayCallbackModel::RegisterCreateSpaceCallback(CavrnusSpaceCreated onCreated, CavrnusError onFailure)
	{
		int reqId = ++currReqId;

		CavrnusSpaceCreated* CallbackPtr = new CavrnusSpaceCreated(onCreated);
		CavrnusError* ErrorPtr = new CavrnusError(onFailure);

		CreateSpaceSuccessCallbacks.Add(reqId, CallbackPtr);
		GenericErrorCallbacks.Add(reqId, ErrorPtr);

		return reqId;
	}

	int RelayCallbackModel::RegisterJoinSpaceCallback(CavrnusSpaceConnected onConnected, CavrnusError onFailure)
	{
		int reqId = ++currReqId;

		CavrnusSpaceConnected* CallbackPtr = new CavrnusSpaceConnected(onConnected);
		CavrnusError* ErrorPtr = new CavrnusError(onFailure);

		JoinSpaceSuccessCallbacks.Add(reqId, CallbackPtr);
		GenericErrorCallbacks.Add(reqId, ErrorPtr);

		return reqId;
	}

	int RelayCallbackModel::RegisterFetchAvailableSpacesCallback(CavrnusAllSpacesInfoEvent onAllSpacesArrived)
	{
		int reqId = ++currReqId;

		CavrnusAllSpacesInfoEvent* callback = new CavrnusAllSpacesInfoEvent(onAllSpacesArrived);
		AllSpacesInfoCallbacks.Add(reqId, callback);

		return reqId;
	}

	void RelayCallbackModel::HandleJoinableSpacesRecv(int callbackId, ServerData::AllJoinableSpacesResp resp)
	{
		TArray<FCavrnusSpaceInfo> AvailableSpaces;

		for (int i = 0; i < resp.availablespaces().size(); i++)
		{
			AvailableSpaces.Add(CavrnusProtoTranslation::ToSpaceInfo(resp.availablespaces()[i]));
		}

		if (AllSpacesInfoCallbacks.Contains(callbackId))
			(*AllSpacesInfoCallbacks[callbackId])(AvailableSpaces);

		AllSpacesInfoCallbacks.Remove(callbackId);
	}

	int RelayCallbackModel::RegisterFetchSpaceInfoCallback(CavrnusSpaceInfoEvent onSpaceInfoFetched)
	{
		int reqId = ++currReqId;

		CavrnusSpaceInfoEvent* callback = new CavrnusSpaceInfoEvent(onSpaceInfoFetched);
		FetchSpaceInfoCallbacks.Add(reqId, callback);

		return reqId;
	}

	void RelayCallbackModel::HandleFetchSpaceInfoComplete(int callbackId, ServerData::GetSpaceInfoResp resp)
	{
		FCavrnusSpaceInfo info = CavrnusProtoTranslation::ToSpaceInfo(resp.info());

		if (FetchSpaceInfoCallbacks.Contains(callbackId))
			(*FetchSpaceInfoCallbacks[callbackId])(info);

		FetchSpaceInfoCallbacks.Remove(callbackId);
	}

	void RelayCallbackModel::HandleCreateSpaceResponse(int callbackId, ServerData::CreateSpaceResp resp)
	{
		UE_LOG(LogCavrnusConnector, Log, TEXT("[CREATE SPACE SUCCESS]"));
		if (CreateSpaceSuccessCallbacks.Contains(callbackId))
			(*CreateSpaceSuccessCallbacks[callbackId])(CavrnusProtoTranslation::ToSpaceInfo(resp.newspaceinfo()));

		CreateSpaceSuccessCallbacks.Remove(callbackId);
	}

	void RelayCallbackModel::HandleJoinSpaceResponse(int callbackId, ServerData::JoinSpaceFromIdResp resp)
	{
		//Always trigger this
		for (int i = 0; i < EndLoadingSpaceCallbacks.Num(); i++)
		{
			(*EndLoadingSpaceCallbacks[i])();
		}
		EndLoadingSpaceCallbacks.Empty();

		if (resp.Resp_case() == ServerData::JoinSpaceFromIdResp::kSpaceConn)
		{
			FCavrnusSpaceConnectionInfo spaceConnInfo = Cavrnus::CavrnusProtoTranslation::SpaceConnectionInfoFromPb(resp.spaceconn());

			relayModel->GetDataState()->AddSpaceConnection(spaceConnInfo);

			UE_LOG(LogCavrnusConnector, Log, TEXT("[JOIN SPACE SUCCESS]"));
			if (JoinSpaceSuccessCallbacks.Contains(callbackId))
				(*JoinSpaceSuccessCallbacks[callbackId])(FCavrnusSpaceConnection(spaceConnInfo.SpaceConnectionId));
		}
		else
		{
			FString error = UTF8_TO_TCHAR(resp.error().c_str());
			UE_LOG(LogCavrnusConnector, Log, TEXT("[JOIN SPACE FAILURE]: %s"), *error);
			if (GenericErrorCallbacks.Contains(callbackId))
				(*GenericErrorCallbacks[callbackId])(error);
		}

		JoinSpaceSuccessCallbacks.Remove(callbackId);
		GenericErrorCallbacks.Remove(callbackId);
	}

	int RelayCallbackModel::RegisterFetchAudioInputs(CavrnusAvailableInputDevices onRecvDevices)
	{
		int reqId = ++currReqId;

		CavrnusAvailableInputDevices* callback = new CavrnusAvailableInputDevices(onRecvDevices);
		FetchAudioInputsCallbacks.Add(reqId, callback);

		return reqId;
	}

	void RelayCallbackModel::HandleAudioInputsResponse(int callbackId, ServerData::GetAudioInputDevicesResp resp)
	{
		TArray<FCavrnusInputDevice> devices;

		const auto& devices_field = resp.devices();
		for (int i = 0; i < devices_field.devices_size(); ++i)
		{
			const ServerData::RtcAudioInputDevice& device = devices_field.devices(i);
			FCavrnusInputDevice item = FCavrnusInputDevice(UTF8_TO_TCHAR(device.name().c_str()), UTF8_TO_TCHAR(device.id().c_str()));

			devices.Add(item);
		}

		if (FetchAudioInputsCallbacks.Contains(callbackId))
			(*FetchAudioInputsCallbacks[callbackId])(devices);
	}

	int RelayCallbackModel::RegisterFetchAudioOutputs(CavrnusAvailableOutputDevices onRecvDevices)
	{
		int reqId = ++currReqId;

		CavrnusAvailableOutputDevices* callback = new CavrnusAvailableOutputDevices(onRecvDevices);
		FetchAudioOutputsCallbacks.Add(reqId, callback);

		return reqId;
	}

	void RelayCallbackModel::HandleAudioOutputsResponse(int callbackId, ServerData::GetAudioOutputDevicesResp resp)
	{
		TArray<FCavrnusOutputDevice> devices;

		const auto& devices_field = resp.devices();
		for (int i = 0; i < devices_field.devices_size(); ++i)
		{
			const ServerData::RtcAudioOutputDevice& device = devices_field.devices(i);
			FCavrnusOutputDevice item = FCavrnusOutputDevice(UTF8_TO_TCHAR(device.name().c_str()), UTF8_TO_TCHAR(device.id().c_str()));

			devices.Add(item);
		}

		if (FetchAudioOutputsCallbacks.Contains(callbackId))
			(*FetchAudioOutputsCallbacks[callbackId])(devices);
	}

	int RelayCallbackModel::RegisterFetchVideoInputs(CavrnusAvailableVideoInputDevices onRecvDevices)
	{
		int reqId = ++currReqId;

		CavrnusAvailableVideoInputDevices* callback = new CavrnusAvailableVideoInputDevices(onRecvDevices);
		FetchVideoInputsCallbacks.Add(reqId, callback);

		return reqId;
	}

	void RelayCallbackModel::HandleVideoInputsResponse(int callbackId, ServerData::GetVideoInputDevicesResp resp)
	{
		TArray<FCavrnusVideoInputDevice> devices;

		const auto& devices_field = resp.devices();
		for (int i = 0; i < devices_field.devices_size(); ++i)
		{
			const ServerData::RtcVideoInputDevice& device = devices_field.devices(i);
			FCavrnusVideoInputDevice item = FCavrnusVideoInputDevice(UTF8_TO_TCHAR(device.name().c_str()), UTF8_TO_TCHAR(device.id().c_str()));

			devices.Add(item);
		}

		if (FetchVideoInputsCallbacks.Contains(callbackId))
			(*FetchVideoInputsCallbacks[callbackId])(devices);
	}

	int RelayCallbackModel::RegisterFetchRemoteContentInfo(CavrnusRemoteContentInfoFunction onfetchedContentInfo)
	{
		int reqId = ++currReqId;

		CavrnusRemoteContentInfoFunction* CallbackPtr = new CavrnusRemoteContentInfoFunction(onfetchedContentInfo);

		AllContentInfoCallbacks.Add(reqId, CallbackPtr);

		return reqId;
	}

	void RelayCallbackModel::HandleRemoteContentInfoComplete(int callbackId, ServerData::FetchRemoteContentInfoResp resp)
	{
		TMap<FString, FString> tags;
		for (int j = 0; j < resp.uploadedcontent().tagkeys_size(); j++)
		{
			tags.Add(UTF8_TO_TCHAR(resp.uploadedcontent().tagkeys()[j].c_str()), UTF8_TO_TCHAR(resp.uploadedcontent().tagvalues()[j].c_str()));
		}

		FCavrnusRemoteContent remoteContent = FCavrnusRemoteContent(UTF8_TO_TCHAR(resp.uploadedcontent().id().c_str()), UTF8_TO_TCHAR(resp.uploadedcontent().name().c_str()), UTF8_TO_TCHAR(resp.uploadedcontent().filename().c_str()), UTF8_TO_TCHAR(resp.uploadedcontent().thumbnailurl().c_str()), resp.uploadedcontent().filesize(), UTF8_TO_TCHAR(resp.uploadedcontent().filesizestring().c_str()), resp.uploadedcontent().iscachedondisk(), tags);

		if (AllContentInfoCallbacks.Contains(callbackId))
		{
			(*AllContentInfoCallbacks[callbackId])(remoteContent);
			AllContentInfoCallbacks.Remove(callbackId);
		}
	}

	int RelayCallbackModel::RegisterFetchAllAvailableContent(CavrnusRemoteContentFunction onfetchedContent)
	{
		int reqId = ++currReqId;

		CavrnusRemoteContentFunction* CallbackPtr = new CavrnusRemoteContentFunction(onfetchedContent);

		AllRemoteContentCallbacks.Add(reqId, CallbackPtr);

		return reqId;
	}

	void RelayCallbackModel::HandleAllRemoteContentRecv(int callbackId, ServerData::FetchAllUploadedContentResp resp)
	{
		TArray<FCavrnusRemoteContent> remoteContent;
		for (int i = 0; i < resp.availablecontent().size(); i++) 
		{
			TMap<FString, FString> tags;
			for (int j = 0; j < resp.availablecontent()[i].tagkeys_size(); j++) 
			{
				tags.Add(UTF8_TO_TCHAR(resp.availablecontent()[i].tagkeys()[j].c_str()), UTF8_TO_TCHAR(resp.availablecontent()[i].tagvalues()[j].c_str()));
			}

			remoteContent.Add(FCavrnusRemoteContent(UTF8_TO_TCHAR(resp.availablecontent()[i].id().c_str()), UTF8_TO_TCHAR(resp.availablecontent()[i].name().c_str()), UTF8_TO_TCHAR(resp.availablecontent()[i].filename().c_str()), UTF8_TO_TCHAR(resp.availablecontent()[i].thumbnailurl().c_str()), resp.availablecontent()[i].filesize(), UTF8_TO_TCHAR(resp.availablecontent()[i].filesizestring().c_str()), resp.availablecontent()[i].iscachedondisk(), tags));
		}

		if (AllRemoteContentCallbacks.Contains(callbackId))
		{
			(*AllRemoteContentCallbacks[callbackId])(remoteContent);
			AllRemoteContentCallbacks.Remove(callbackId);
		}
	}

	int RelayCallbackModel::RegisterUploadContent(CavrnusUploadCompleteFunction onUploadComplete, CavrnusError onFailure, CavrnusUploadProgressFunction onProgress)
	{
		int reqId = ++currReqId;

		CavrnusUploadCompleteFunction* CallbackPtr = new CavrnusUploadCompleteFunction(onUploadComplete);
		AllUploadContentSuccessCallbacks.Add(reqId, CallbackPtr);

		CavrnusError* ErrorCallbackPtr = new CavrnusError(onFailure);
		GenericErrorCallbacks.Add(reqId, ErrorCallbackPtr);

		CavrnusUploadProgressFunction* ProgressCallbackPtr = new CavrnusUploadProgressFunction(onProgress);
		AllUploadContentProgressCallbacks.Add(reqId, ProgressCallbackPtr);

		return reqId;
	}

	void RelayCallbackModel::HandleUploadProgress(int callbackId, ServerData::UploadLocalFileProgressResp resp)
	{
		FString progressStep = UTF8_TO_TCHAR(resp.progressstep().c_str());
		float prog = resp.progress();

		if (AllUploadContentProgressCallbacks.Contains(callbackId))
			(*AllUploadContentProgressCallbacks[callbackId])(progressStep, prog);
	}

	void RelayCallbackModel::HandleUploadComplete(int callbackId, ServerData::UploadLocalFileResp resp)
	{
		TMap<FString, FString> tags;
		for (int j = 0; j < resp.uploadedcontent().tagkeys_size(); j++)
		{
			tags.Add(UTF8_TO_TCHAR(resp.uploadedcontent().tagkeys()[j].c_str()), UTF8_TO_TCHAR(resp.uploadedcontent().tagvalues()[j].c_str()));
		}

		FCavrnusRemoteContent remoteContent = FCavrnusRemoteContent(UTF8_TO_TCHAR(resp.uploadedcontent().id().c_str()), UTF8_TO_TCHAR(resp.uploadedcontent().name().c_str()), UTF8_TO_TCHAR(resp.uploadedcontent().filename().c_str()), UTF8_TO_TCHAR(resp.uploadedcontent().thumbnailurl().c_str()), resp.uploadedcontent().filesize(), UTF8_TO_TCHAR(resp.uploadedcontent().filesizestring().c_str()), resp.uploadedcontent().iscachedondisk(), tags);

		if (AllUploadContentSuccessCallbacks.Contains(callbackId))
		{
			(*AllUploadContentSuccessCallbacks[callbackId])(remoteContent);
			AllUploadContentSuccessCallbacks.Remove(callbackId);
		}
		if (AllUploadContentProgressCallbacks.Contains(callbackId))
			AllUploadContentProgressCallbacks.Remove(callbackId);
	}

	int RelayCallbackModel::RegisterFolderReq(const TFunction<void(FString)>& onRecvFullFolderPath)
	{
		int reqId = ++currReqId;

		TFunction<void(FString)>* CallbackPtr = new TFunction<void(FString)>(onRecvFullFolderPath);

		AllFolderReqCallbacks.Add(reqId, CallbackPtr);

		return reqId;
	}

	void RelayCallbackModel::HandleFolderResp(int callbackId, ServerData::ContentDestinationFolderResp resp)
	{
		FString fullFolderName = UTF8_TO_TCHAR(resp.fullfoldername().c_str());

		if (AllFolderReqCallbacks.Contains(callbackId))
		{
			(*AllFolderReqCallbacks[callbackId])(fullFolderName);
			AllFolderReqCallbacks.Remove(callbackId);
		}
	}

	int RelayCallbackModel::RegisterAiReq(const TFunction<void(FString)>& onSuccess, CavrnusError onFailure)
	{
		int reqId = ++currReqId;

		TFunction<void(FString)>* CallbackPtr = new TFunction<void(FString)>(onSuccess);
		AllAiReqCallbacks.Add(reqId, CallbackPtr);

		CavrnusError* ErrorCallbackPtr = new CavrnusError(onFailure);
		GenericErrorCallbacks.Add(reqId, ErrorCallbackPtr);

		return reqId;
	}

	void RelayCallbackModel::HandleAiResp(int callbackId, ServerData::QueryAiResp resp)
	{
		FString respMsg = UTF8_TO_TCHAR(resp.resptext().c_str());

		if (AllAiReqCallbacks.Contains(callbackId))
		{
			(*AllAiReqCallbacks[callbackId])(respMsg);
			AllAiReqCallbacks.Remove(callbackId);
		}
	}

	void RelayCallbackModel::ReleaseErrorHandler(int requestId)
	{
		GenericErrorCallbacks.Remove(requestId);
	}
} // namespace Cavrnus
