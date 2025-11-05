// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Containers/Map.h>
#include "Types/CavrnusBinding.h"
#include "Types/CavrnusCallbackTypes.h"
#include "Types/CavrnusSpaceConnection.h"
#include "Types/CavrnusSpaceConnectionInfo.h"

namespace Cavrnus
{
	class DataState
	{
	public:
		DataState();
		virtual ~DataState();

		FString CurrentServer = "";

		FCavrnusAuthentication* CurrentAuthentication = nullptr;

		TArray<FCavrnusSpaceConnectionInfo>& GetCurrentSpaceConnections();

		void AddSpaceConnection(const FCavrnusSpaceConnectionInfo& spaceConnection);
		void RemoveSpaceConnection(int spaceConnId);

		void AwaitAnySpaceConnection(const CavrnusSpaceConnected& onConnected);
		void AwaitAnySpaceExited(const CavrnusSpaceExited& onConnected);

		void AddRemoteContent(const FCavrnusRemoteContent& content);
		void RemoveRemoteContent(const FString& id);
		TMap<FString, FCavrnusRemoteContent> CurrRemoteContent;

		typedef TFunction<bool(const FCavrnusRemoteContent& content)> ContentPredicate;
		typedef TFunction<void(const FCavrnusRemoteContent& content)> ContentArrived;
		void AwaitContentByPredicate(ContentPredicate predicate, ContentArrived onArrived);

		void AddJoinableSpace(FCavrnusSpaceInfo space);
		void UpdateJoinableSpace(FCavrnusSpaceInfo space);
		void RemoveJoinableSpace(FCavrnusSpaceInfo space);

		UCavrnusBinding* BindJoinableSpaces(CavrnusSpaceInfoEvent spaceAdded, CavrnusSpaceInfoEvent spaceUpdated, CavrnusSpaceInfoEvent spaceRemoved);

	private:
		TArray<FCavrnusSpaceConnectionInfo> CurrentSpaceConnections;

		TArray<CavrnusSpaceConnected*> spaceConnectionBindings;
		TArray<CavrnusSpaceExited*> spaceExitedBindings;

		TArray<FCavrnusSpaceInfo> CurrJoinableSpaces;

		TArray<CavrnusSpaceInfoEvent*> JoinableSpaceAddedBindings;
		TArray<CavrnusSpaceInfoEvent*> JoinableSpaceUpdatedBindings;
		TArray<CavrnusSpaceInfoEvent*> JoinableSpaceRemovedBindings;

		
		TMap<ContentPredicate*, ContentArrived*> ContentAwaitPredicates;

	};
} // namespace Cavrnus
