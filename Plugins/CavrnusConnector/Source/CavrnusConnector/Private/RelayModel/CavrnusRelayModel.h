// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "SpacePropertyModel.h"

#include <CoreMinimal.h>
#include <string>
#include <Tickable.h>
#include "CavrnusConnectorModule.h"
#include "CavrnusContentModel.h"
#include "Interop/CavrnusInteropLayer.h"
#include "Types/AbsolutePropertyId.h"
#include "Types/CavrnusSpaceConnection.h"

namespace ServerData
{
	class CavrnusUser;
	class RelayClientMessage;
	class RelayRemoteMessage;
	class StatusMessage;
	class CavrnusSpaceConnection;
	class UserVideoFrame;
	class PropertyValueStatus;
	class ObjectAdded;
	class ObjectRemoved;
	class PermissionStatus;
	class LocalPropertyHandledResp;
	class PropMetadataStatus;
	class ChatAdded;
	class ChatUpdated;
	class ChatRemoved;
	class RelayDataCache;
} // namespace ServerData

namespace Cavrnus
{
	class CavrnusInteropLayer;
	class DataState;
	class RelayCallbackModel;
	class SpacePropertyModel;
	class SpacePermissionsModel;
	class CavrnusDataCache;

	/**
	 * @class CavrnusRelayModel
	 * @brief Manages the relay communication between the Cavrnus server and the client.
	 *
	 * Handles the reception and processing of messages from the Cavrnus server,
	 * as well as sending messages to the server. Also responsible for managing 
	 * data models and invoking callbacks for object creation and destruction.
	 */
	class CAVRNUSCONNECTOR_API CavrnusRelayModel : public FTickableGameObject, private ICavrnusInteropMessageReceiver
	{
	public:
		CavrnusRelayModel();
		virtual ~CavrnusRelayModel();

		/**
		 * @brief Gets the data model instance.
		 * @return A pointer to the CavrnusRelayModel instance.
		 */
		static CavrnusRelayModel* GetDataModel();

		/**
		 * @brief Shuts down the data model instance.
		 */
		static void KillDataModel();

		/**
		 * @brief Determines if the object is tickable in the editor.
		 *
		 * @return true if tickable in the editor, false otherwise.
		 */
		bool IsTickableInEditor() const override;

		/**
		 * @brief Overrides FTickableGameObject Tick.
		 *
		 * @param DeltaTime The time elapsed since last tick.
		 */
		void Tick(float DeltaTime) override;

		/**
		 * @brief Gets the TStatId for profiling.
		 *
		 * @return The TStatId for performance metrics.
		 */
		TStatId GetStatId() const override;

		/** Accessors for various data models */
		SpacePermissionsModel* GetGlobalPermissionsModel();
		SpacePermissionsModel* GetSpacePermissionsModel(FCavrnusSpaceConnection spaceConn);
		SpacePropertyModel* GetSpacePropertyModel(FCavrnusSpaceConnection spaceConn);
		RelayCallbackModel* GetCallbackModel();
		DataState* GetDataState();
		CavrnusDataCache* GetDataCache();
		
		/**
		 * @brief Sends a message to the server.
		 *
		 * @param msg The message to send.
		 */
		void SendMessage(const ServerData::RelayClientMessage& msg);

		CavrnusContentModel ContentModel;

		/**
		 * @brief Registers a callback for object creation.
		 *
		 * @param cb The callback function to register.
		 */
		void RegisterObjectCreationCallback(TFunction<AActor* (FCavrnusSpawnedObject, FString)> cb);

		/**
		 * @brief Registers a callback for object destruction.
		 *
		 * @param cb The callback function to register.
		 */
		void RegisterObjectDestructionCallback(TFunction<void (FCavrnusSpawnedObject)> cb);

		void HandleSpaceObjectAdded(const ServerData::ObjectAdded& ObjectAdded);
		void HandleSpaceObjectRemoved(const ServerData::ObjectRemoved& ObjectRemoved);

	private:
		/**
		 * @param msg Implements ICavrnusInteropMessageReceiver to receive messages from Cavrnus Interop
		 */
		virtual void ReceiveMessage(const ServerData::RelayRemoteMessage& msg);

	private:
		static CavrnusRelayModel* Instance;

		/** Internal data members */
		CavrnusInteropLayer* interopLayer;
		RelayCallbackModel* callbackModel;
		DataState* dataState;
		CavrnusDataCache* dataCache;
		SpacePermissionsModel* globalPermissionsModel;
		TMap<int, SpacePermissionsModel*> spacePermissionsModelLookup;
		TMap<int, SpacePropertyModel*> spacePropertyModelLookup;

		/** Handles server messages */
		void HandleServerMsg(const ServerData::RelayRemoteMessage& msg);

		/** Callbacks for various server messages */
		const TFunction<AActor* (FCavrnusSpawnedObject, FString)>* ObjectCreationCallback = nullptr;
		const TFunction<void(FCavrnusSpawnedObject)>* ObjectDestructionCallback = nullptr;

		void HandleLogging(const ServerData::StatusMessage& message);
		void HandleInitialDataCache(const ServerData::RelayDataCache& cache);
		void HandleSpaceUserAdded(ServerData::CavrnusSpaceConnection spaceConn, ServerData::CavrnusUser userId);
		void HandleSpaceUserRemoved(ServerData::CavrnusSpaceConnection spaceConn, std::string userId);
		void HandleSpaceUserVideoFrame(const ServerData::UserVideoFrame& VideoFrame);
		void HandleServerPropertyUpdate(const ServerData::PropertyValueStatus& propStatus);
		void HandlePermissionStatus(const ServerData::PermissionStatus& PermissionStatus);
		void HandleLocalPropHandledResp(const ServerData::LocalPropertyHandledResp& localPropHandled);
		void HandlePropMetadataStatus(const ServerData::PropMetadataStatus& metadataStatus);

		void HandleChatAdded(const ServerData::ChatAdded& chatAdded);
		void HandleChatUpdated(const ServerData::ChatUpdated& chatUpdated);
		void HandleChatRemoved(const ServerData::ChatRemoved& chatRemoved);
	};
} // namespace Cavrnus
