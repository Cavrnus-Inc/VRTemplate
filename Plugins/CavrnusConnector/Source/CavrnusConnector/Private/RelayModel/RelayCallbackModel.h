// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Containers/Map.h>
#include "Types/CavrnusBinding.h"
#include "Types/CavrnusCallbackTypes.h"
#include "Translation/CavrnusProtoTranslation.h"
#include "DataState.h"

namespace ServerData
{
	class RelayRemoteMessage;
}

namespace Cavrnus
{
	class CavrnusRelayModel;

	class RelayCallbackModel
	{
	public:
		RelayCallbackModel(CavrnusRelayModel* model);
		virtual ~RelayCallbackModel();

		void HandleServerCallback(int callbackId, const ServerData::RelayRemoteMessage& msg);

		void HandleServerMessage(ServerData::ServerMessage msg);
		void HandleSpaceStatus(ServerData::SpaceConnectionStatus msg);

		void RegisterGotDataCache(TFunction<void()> onSuccess);
		void HandleDataCache(const ServerData::RelayDataCache& dataCache);

		int RegisterGenericCallback(CavrnusSuccess onSuccess, CavrnusError onFailure);

		int RegisterAuthenticationCallback(CavrnusAuthRecv onSuccess, CavrnusError onFailure);

		int RegisterDeviceCodeBeginCallback(CavrnusDeviceCodeRecv onSuccess, CavrnusError onFailure);
		int RegisterConstructApiKeyCallback(CavrnusApiKeyRecv onSuccess, CavrnusError onFailure);

		int RegisterServerStatusCallback(CavrnusServerStatusRecv onStatus);

		void RegisterServerSetCallback(CavrnusServerSet onServerSet);
		void SetServer(FString server);

		void RegisterSpaceStatusCallback(int spaceid, CavrnusSpaceStatusChanged onSpaceStatus);
		void RegisterServerMessageCallback(int spaceid, CavrnusServerMessageEvent onServerMessage);

		void RegisterAuthCallback(CavrnusAuthRecv onAuth);

		void RegisterBeginLoadingSpaceCallback(CavrnusSpaceBeginLoading onBeginLoading);
		void HandleSpaceBeginLoading(FString spaceId);

		void RegisterEndLoadingSpaceCallback(CavrnusSpaceEndLoading onEndLoading);

		int RegisterFetchCavrnusUserAccounts(CavrnusUserAccountsFetched onFetchComplete);

		int RegisterCreateSpaceCallback(CavrnusSpaceCreated onCreated, CavrnusError onFailure);

		int RegisterJoinSpaceCallback(CavrnusSpaceConnected onConnected, CavrnusError onFailure);

		int RegisterFetchAvailableSpacesCallback(CavrnusAllSpacesInfoEvent onAllSpacesArrived);
		int RegisterFetchSpaceInfoCallback(CavrnusSpaceInfoEvent onSpaceInfoFetched);

		int RegisterFetchAudioInputs(CavrnusAvailableInputDevices onRecvDevices);
		int RegisterFetchAudioOutputs(CavrnusAvailableOutputDevices onRecvDevices);
		int RegisterFetchVideoInputs(CavrnusAvailableVideoInputDevices onRecvDevices);

		int RegisterFetchRemoteContentInfo(CavrnusRemoteContentInfoFunction onfetchedContentInfo);
		int RegisterFetchAllAvailableContent(CavrnusRemoteContentFunction onfetchedContent);

		int RegisterUploadContent(CavrnusUploadCompleteFunction onUploadComplete, CavrnusError onFailure, CavrnusUploadProgressFunction onProgress);

		int RegisterFolderReq(const TFunction<void(FString)>& onRecvFullFolderPath);

		int RegisterAiReq(const TFunction<void(FString)>& onSuccess, CavrnusError onFailure);

		void ReleaseErrorHandler(int requestId);

	private:
		CavrnusRelayModel* relayModel;

		int currReqId = 0;

		TArray<TFunction<void()>*> DataCacheCallbacks;
		ServerData::RelayDataCache savedDataCache;
		bool gotDataCache = false;

		TArray<CavrnusServerSet*> ServerSetCallbacks;

		TArray<CavrnusAuthRecv*> AuthCallbacks;
		void HandleAuthRecv(FCavrnusAuthentication auth);

		TMap<int, CavrnusAuthRecv*> LoginAuthenticationCallbacks;
		void HandleAuthenticationResponse(int callbackId, ServerData::AuthenticateResp resp);

		TMap<int, CavrnusDeviceCodeRecv*> DeviceCodeCallbacks;
		void HandleDeviceCodeResponse(int callbackId, ServerData::AuthenticateDeviceCodeBeginResp resp);

		TMap<int, CavrnusApiKeyRecv*> ConstructApiKeyCallbacks;
		void HandleConstructApiKeyResponse(int callbackId, ServerData::ConstructApiKeyResp resp);

		TMap<int, CavrnusServerStatusRecv*> ServerStatusCallbacks;
		void HandleServerStatusResponse(int callbackId, ServerData::CheckServerStatusResp resp);
		
		TArray<CavrnusSpaceBeginLoading*> BeginLoadingSpaceCallbacks;
		TArray<CavrnusSpaceEndLoading*> EndLoadingSpaceCallbacks;

		TMap<int, CavrnusSuccess*> GenericSuccessCallbacks;
		TMap<int, CavrnusError*> GenericErrorCallbacks;
		void HandleGenericResponse(int callbackId, ServerData::GenericResp resp);

		TMap<int, CavrnusSpaceCreated*> CreateSpaceSuccessCallbacks;
		void HandleCreateSpaceResponse(int callbackId, ServerData::CreateSpaceResp resp);

		TMap<int, CavrnusUserAccountsFetched*> CavrnusUserAccountsFetchedCallbacks;
		void HandleFetchAllUserAccountsComplete(int callbackId, ServerData::FetchAllUserAccountsResp resp);
		
		TMap<int, CavrnusSpaceConnected*> JoinSpaceSuccessCallbacks;
		void HandleJoinSpaceResponse(int callbackId, ServerData::JoinSpaceFromIdResp resp);

		TMap<int, CavrnusAllSpacesInfoEvent*> AllSpacesInfoCallbacks;
		void HandleJoinableSpacesRecv(int callbackId, ServerData::AllJoinableSpacesResp resp);

		TMap<int, CavrnusAvailableInputDevices*> FetchAudioInputsCallbacks;
		void HandleAudioInputsResponse(int callbackId, ServerData::GetAudioInputDevicesResp resp);

		TMap<int, CavrnusAvailableOutputDevices*> FetchAudioOutputsCallbacks;
		void HandleAudioOutputsResponse(int callbackId, ServerData::GetAudioOutputDevicesResp resp);

		TMap<int, CavrnusAvailableVideoInputDevices*> FetchVideoInputsCallbacks;
		void HandleVideoInputsResponse(int callbackId, ServerData::GetVideoInputDevicesResp resp);

		TMap<int, CavrnusRemoteContentFunction*> AllRemoteContentCallbacks;
		void HandleAllRemoteContentRecv(int callbackId, ServerData::FetchAllUploadedContentResp resp);

		TMap<int, CavrnusRemoteContentInfoFunction*> AllContentInfoCallbacks;
		void HandleRemoteContentInfoComplete(int callbackId, ServerData::FetchRemoteContentInfoResp resp);

		TMap<int, CavrnusUploadCompleteFunction*> AllUploadContentSuccessCallbacks;
		TMap<int, CavrnusUploadProgressFunction*> AllUploadContentProgressCallbacks;
		void HandleUploadProgress(int callbackId, ServerData::UploadLocalFileProgressResp resp);
		void HandleUploadComplete(int callbackId, ServerData::UploadLocalFileResp resp);

		TMap<int, CavrnusSpaceInfoEvent*> FetchSpaceInfoCallbacks;
		void HandleFetchSpaceInfoComplete(int callbackId, ServerData::GetSpaceInfoResp resp);

		TMap<int, TFunction<void(FString)>*> AllFolderReqCallbacks;
		void HandleFolderResp(int callbackId, ServerData::ContentDestinationFolderResp resp);

		TMap<int, TFunction<void(FString)>*> AllAiReqCallbacks;
		void HandleAiResp(int callbackId, ServerData::QueryAiResp resp);

		TMap<int, TArray<CavrnusServerMessageEvent*>> ServerMessageCallbacks;

		TMap<int, TArray<CavrnusSpaceStatusChanged*>> SpaceStatusCallbacks;

	};
} // namespace Cavrnus
