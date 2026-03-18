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

		bool bAuthenticatedAsGuest = false;

		TArray<FCavrnusSpaceConnectionInfo>& GetCurrentSpaceConnections();

		void AddSpaceConnection(const FCavrnusSpaceConnectionInfo& spaceConnection);
		void RemoveSpaceConnection(int spaceConnId);
		bool IsSpaceConnectionActive(int spaceConnId) const;

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
		const FCavrnusSpaceInfo* GetJoinableSpaceById(const FString& spaceId) const;

		UCavrnusBinding* BindJoinableSpaces(CavrnusSpaceInfoEvent spaceAdded, CavrnusSpaceInfoEvent spaceUpdated, CavrnusSpaceInfoEvent spaceRemoved);

		UCavrnusBinding* BindSpaceInfoChanged(ESpaceInfoChangeFlags changeMask, CavrnusSpaceInfoChangedEvent onChanged);

		static ESpaceInfoChangeFlags DetectSpaceInfoChanges(const FCavrnusSpaceInfo& oldInfo, const FCavrnusSpaceInfo& newInfo);

	private:
		struct SpaceInfoChangedBinding
		{
			ESpaceInfoChangeFlags Mask;
			CavrnusSpaceInfoChangedEvent* Callback;
		};
		TArray<FCavrnusSpaceConnectionInfo> CurrentSpaceConnections;

		TArray<CavrnusSpaceConnected*> spaceConnectionBindings;
		TArray<CavrnusSpaceExited*> spaceExitedBindings;

		TArray<FCavrnusSpaceInfo> CurrJoinableSpaces;

		TArray<CavrnusSpaceInfoEvent*> JoinableSpaceAddedBindings;
		TArray<CavrnusSpaceInfoEvent*> JoinableSpaceUpdatedBindings;
		TArray<CavrnusSpaceInfoEvent*> JoinableSpaceRemovedBindings;

		TArray<SpaceInfoChangedBinding> SpaceInfoChangedBindings;

		TMap<ContentPredicate*, ContentArrived*> ContentAwaitPredicates;

	};
} // namespace Cavrnus
