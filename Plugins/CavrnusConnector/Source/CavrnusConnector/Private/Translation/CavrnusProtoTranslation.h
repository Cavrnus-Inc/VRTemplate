// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "Comm/relay.pb.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/CavrnusSpaceConnection.h"
#include "Types/CavrnusSpaceConnectionInfo.h"
#include "Types/CavrnusSpaceInfo.h"
#include "Types/CavrnusSpaceMember.h"
#include "Types/CavrnusInputDevice.h"
#include "Types/CavrnusOutputDevice.h"
#include "Types/CavrnusVideoInputDevice.h"
#include "Types/CavrnusUser.h"
#include "Types/AbsolutePropertyId.h"
#include "Types/CavrnusPropertyValue.h"
#include "Types/PropertyPostOptions.h"
#include "Types/CavrnusOrganizationInfo.h"

namespace Cavrnus
{
	class CavrnusProtoTranslation
	{
	public:
		CavrnusProtoTranslation();
		virtual ~CavrnusProtoTranslation();

#pragma region Type Conversions
		static Common::Color4 ToProtoColor(const FLinearColor& color);
		static FLinearColor ToFLinearColor(const Common::Color4& color);
		static ServerData::TransformPropertyValue ToProtoTransform(const FTransform& transform);
		static FTransform ToFTransform(const ServerData::TransformPropertyValue& transform);
		static Common::Float4 ToProtoVector(const FVector4& vector);
		static FVector4 ToFVector4(const  Common::Float4& vector);

		static FCavrnusSpaceConnection SpaceConnectionFromPb(ServerData::CavrnusSpaceConnection InSpaceConnection);
		static ServerData::CavrnusSpaceConnection SpaceConnectionToPb(FCavrnusSpaceConnection InSpaceConnection);
		static FCavrnusSpaceConnectionInfo SpaceConnectionInfoFromPb(ServerData::CavrnusSpaceConnectionInfo InSpaceConnectionInfo);
		static FCavrnusRemoteContent ToRemoteContent(ServerData::CavrnusRemoteContent content);
		static FCavrnusSpaceInfo ToSpaceInfo(ServerData::SpaceInfo space);
		static FCavrnusSpaceMember ToSpaceMember(ServerData::CavrnusRoomMember member);
		static FCavrnusUser ToCavrnusUser(ServerData::CavrnusUser user, const FCavrnusSpaceConnection& spaceConn);
		static FCavrnusUserAccount ToCavrnusUserAccount(ServerData::CavrnusUserAccount user);
		static FCavrnusOrganizationInfo ToCavrnusOrganizationInfo(const ServerData::OrganizationInformation& orgInfo);

		static FChatEntry ToFChatEntry(FString id, ServerData::ChatBase chat);
		static const ServerData::RelayClientMessage BuildPostChatEntry(const FCavrnusSpaceConnection& spaceConn, const FString& chat);
#pragma endregion


#pragma region Message Builders
		static const ServerData::RelayClientMessage BuildKeepAliveMsg();
		static const ServerData::RelayClientMessage BuildUpdateTimeMsg(double currTime);
		static const ServerData::RelayClientMessage BuildSetForceKeepAlive();
		static const ServerData::RelayClientMessage BuildEndForceKeepAlive();
		static const ServerData::RelayClientMessage BuildPostDataCacheUpdate(FString key, FString val);

		static const ServerData::RelayClientMessage BuildAuthenticateGuest(int callbackId, const FString& server, const FString& screenName);
		static const ServerData::RelayClientMessage BuildAuthenticateWithPassword(int callbackId, const FString& server, const FString& email, const FString& password);
		static const ServerData::RelayClientMessage BuildAuthenticateToken(int callbackId, const FString& server, const FString& token);
		static const ServerData::RelayClientMessage BuildAuthenticateWithApiKey(int callbackId, const FString& server, const FString& accessKey, const FString& accessToken);
		static const ServerData::RelayClientMessage BuildConstructApiKey(int callbackId, const FString& name);
		static const ServerData::RelayClientMessage BuildAuthenticateWithDeviceCodeBegin(int callbackId, const FString& server, const FString& source, const FString& customActivatedMessage, bool autoOpenUrl);
		static const ServerData::RelayClientMessage BuildAuthenticateWithDeviceCodeConclude(int callbackId, const FString& server, const FString& deviceCode, const FString& userCode);

		static const ServerData::RelayClientMessage BuildCheckServerStatus(int callbackId, const FString& server);

		static const ServerData::RelayClientMessage BuildFetchAvailableSpaces(int callbackId);
		static const ServerData::RelayClientMessage BuildFetchSpaceInfo(int callbackId, const FString& spaceId);
		static const ServerData::RelayClientMessage BuildCreateSpaceMsg(int callbackId, const FString& spaceName, const TArray<FString>& keywords);
		static const ServerData::RelayClientMessage BuildDeleteSpaceMsg(int callbackId, const FString& spaceId);
		static const ServerData::RelayClientMessage BuildRenameSpaceMsg(int callbackId, const FString& spaceId, const FString& newName);
		static const ServerData::RelayClientMessage BuildUploadThumbnailSpaceMsg(int callbackId, const FString& spaceId, const FString& thumbnailFilename);
		static const ServerData::RelayClientMessage BuildJoinSpaceWithId(int callbackId, const FString& spaceId);
		static const ServerData::RelayClientMessage BuildExitSpaceMsg(const FCavrnusSpaceConnection& spaceConn);

		static const ServerData::RelayClientMessage BuildFetchUserAccounts(int callbackId);
		static const ServerData::RelayClientMessage BuildInviteUserToSpace(int callbackId, FString spaceId, FString userEmail, FString roleId);
		static const ServerData::RelayClientMessage BuildRemoveUserFromSpace(int callbackId, FString spaceId, FString userAccountId);

		static const ServerData::RelayClientMessage BuildDefinePropMsg(const FCavrnusSpaceConnection& spaceConn, const FAbsolutePropertyId& propertyId, const FPropertyValue& value, const int localChangeId);
		static const ServerData::RelayClientMessage BuildUpdatePropMsg(const FCavrnusSpaceConnection& spaceConn, const FAbsolutePropertyId& propertyId, const FPropertyValue& value, const int localChangeId, const FPropertyPostOptions& options);
		static const ServerData::RelayClientMessage BuildBeginLivePropertyUpdateMsg(const FCavrnusSpaceConnection& spaceConn, const FString& liveUpdaterId, const FAbsolutePropertyId& propertyId, const FPropertyValue& value, const int localChangeId, const FPropertyPostOptions& options);
		static const ServerData::RelayClientMessage BuildContinueLivePropertyUpdateMsg(const FCavrnusSpaceConnection& spaceConn, const FString& liveUpdaterId, const FAbsolutePropertyId& propertyId, const FPropertyValue& value, const int localChangeId, const FPropertyPostOptions& options);
		static const ServerData::RelayClientMessage BuildFinalizeLivePropertyUpdateMsg(const FCavrnusSpaceConnection& spaceConn, const FString& liveUpdaterId, const FAbsolutePropertyId& propertyId, const FPropertyValue& value, const int localChangeId, const FPropertyPostOptions& options);
		static const ServerData::RelayClientMessage BuildCancelLiveUpdateMsg(const FCavrnusSpaceConnection& spaceConn, const FString& liveUpdaterId);

		static const ServerData::RelayClientMessage BuildSetLocalUserMuted(FCavrnusSpaceConnection spaceConn, bool muted);
		static const ServerData::RelayClientMessage BuildSetLocalUserStreaming(FCavrnusSpaceConnection spaceConn, bool streaming);

		static const ServerData::RelayClientMessage BuildRequestAudioInputs(int callbackId);
		static const ServerData::RelayClientMessage BuildRequestAudioOutputs(int callbackId);
		static const ServerData::RelayClientMessage BuildRequestVideoInputs(int callbackId);

		static const ServerData::RelayClientMessage BuildSetAudioInput(const FCavrnusInputDevice& device, int reqId = -1);
		static const ServerData::RelayClientMessage BuildSetAudioOutput(const FCavrnusOutputDevice& device, int reqId = -1);
		static const ServerData::RelayClientMessage BuildSetVideoInput(const FCavrnusVideoInputDevice& device, int reqId = -1);

		static const ServerData::RelayClientMessage BuildCreateOp(const FCavrnusSpaceConnection& spaceConn, const FString& uniqueObjectId, const FString& instanceId);
		static const ServerData::ObjectAdded BuildObjectAdded(const FCavrnusSpaceConnection& spaceConn, const FString& uniqueObjectId, const FString& instanceId);
		static const ServerData::RelayClientMessage BuildDestroyOp(const FCavrnusSpaceConnection& spaceConn, const FString& containerName);
		static const ServerData::ObjectRemoved BuildObjectRemoved(const FCavrnusSpaceConnection& spaceConn, const FString& instanceId);

		static const ServerData::RelayClientMessage BuildRequestGlobalPermission(const FString& permission);
		static const ServerData::RelayClientMessage BuildRequestSpacePermission(const FCavrnusSpaceConnection& spaceConn, const FString& permission);

		static const ServerData::RelayClientMessage BuildRequestFileInfoById(int callbackId, const FString& contentId);
		static const ServerData::RelayClientMessage BuildRequestFileById(const FString& contentId);
		static const ServerData::RelayClientMessage BuildRequestAllUploadedContent(int callbackId);
		static const ServerData::RelayClientMessage BuildUploadContent(int callbackId, const FString& filePath, const TMap<FString, FString>& tags);
		static const ServerData::RelayClientMessage BuildFolderReq(int callbackId, const FString& folderName);

		static const ServerData::RelayClientMessage BuildAiRequest(const FCavrnusSpaceConnection& spaceConn, int callbackId, const FString& question, const FString& apiKey);
		
#pragma endregion

		static FString CreateTransientId()
		{
			//Copied from: https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c

			size_t length = 8;
			auto randchar = []() -> char
			{
				const char charset[] =
					"0123456789"
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					"abcdefghijklmnopqrstuvwxyz";
				const size_t max_index = (sizeof(charset) - 1);
				return charset[rand() % max_index];
			};
			std::string str(length, 0);
			std::generate_n(str.begin(), length, randchar);
			return str.c_str();
		}

	private:

		static ServerData::PropertyValue GeneratePropVal(const FPropertyValue& value)
		{
			ServerData::PropertyValue propVal;

			if (value.PropType == FPropertyValue::PropertyType::Bool)
				propVal.set_boolval(value.BoolValue);
			if (value.PropType == FPropertyValue::PropertyType::Float)
				propVal.set_scalarval(value.FloatValue);
			if (value.PropType == FPropertyValue::PropertyType::String)
				propVal.set_stringval(TCHAR_TO_UTF8(*value.StringValue));
			if (value.PropType == FPropertyValue::PropertyType::Color)
				propVal.mutable_colorval()->CopyFrom(ToProtoColor(value.ColorValue));
			if (value.PropType == FPropertyValue::PropertyType::Vector)
				propVal.mutable_vectorval()->CopyFrom(ToProtoVector(value.VectorValue));
			if (value.PropType == FPropertyValue::PropertyType::Transform)
				propVal.mutable_transformval()->CopyFrom(ToProtoTransform(value.TransformValue));
			
			return propVal;
		}
	};
} // namespace Cavrnus
