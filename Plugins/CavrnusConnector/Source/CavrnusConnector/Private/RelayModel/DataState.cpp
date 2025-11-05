// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayModel/DataState.h"
#include "RelayModel/CavrnusBindingModel.h"

namespace Cavrnus
{
	DataState::DataState()
	{
	}

	DataState::~DataState()
	{

	}

	TArray<FCavrnusSpaceConnectionInfo>& DataState::GetCurrentSpaceConnections()
	{
		return CurrentSpaceConnections;
	}

	void DataState::AddSpaceConnection(const FCavrnusSpaceConnectionInfo& spaceConnection)
	{
		if (spaceConnection.SpaceConnectionId == -1)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("Got an invalid space connection id!!!"));
			return;
		}

		CurrentSpaceConnections.Add(spaceConnection);

		for (int i = 0; i < spaceConnectionBindings.Num(); i++)
		{
			(*spaceConnectionBindings[i])(FCavrnusSpaceConnection(spaceConnection.SpaceConnectionId));
		}
		spaceConnectionBindings.Empty();
	}

	void DataState::RemoveSpaceConnection(int spaceConnId)
	{
		CurrentSpaceConnections.RemoveAll([spaceConnId](const FCavrnusSpaceConnectionInfo& Element)
		{
			return Element.SpaceConnectionId == spaceConnId;
		});

		auto cb = spaceExitedBindings;
		spaceExitedBindings.Empty();
		if (CurrentSpaceConnections.Num() == 0) 
		{
			for (int i = 0; i < cb.Num(); i++)
			{
				(*cb[i])();
			}
		}
	}

	void DataState::AwaitAnySpaceConnection(const CavrnusSpaceConnected& onConnected)
	{
		if (CurrentSpaceConnections.Num() > 0)
		{
			onConnected(FCavrnusSpaceConnection(CurrentSpaceConnections[0].SpaceConnectionId));
			return;
		}

		CavrnusSpaceConnected* CallbackPtr = new CavrnusSpaceConnected(onConnected);
		spaceConnectionBindings.Add(CallbackPtr);
	}

	void DataState::AwaitAnySpaceExited(const CavrnusSpaceExited& onConnected)
	{
		CavrnusSpaceExited* CallbackPtr = new CavrnusSpaceExited(onConnected);
		spaceExitedBindings.Add(CallbackPtr);
	}

	void DataState::AddRemoteContent(const FCavrnusRemoteContent& content)
	{
		TArray<ContentPredicate*> callbacksToRemove;
		for (auto predicate : ContentAwaitPredicates)
		{
			if ((*predicate.Key)(content))
			{
				(*predicate.Value)(content);
				callbacksToRemove.Add(predicate.Key);
			}
		}
		for (auto cb : callbacksToRemove)
			ContentAwaitPredicates.Remove(cb);

		CurrRemoteContent.Add(content.Id, content);
	}

	void DataState::RemoveRemoteContent(const FString& id)
	{
		CurrRemoteContent.Remove(id);
	}

	void DataState::AwaitContentByPredicate(ContentPredicate predicate, ContentArrived onArrived)
	{
		for (auto content : CurrRemoteContent)
		{
			if (predicate(content.Value))
			{
				onArrived(content.Value);
				return;
			}
		}

		ContentPredicate* PredicatePtr = new ContentPredicate(predicate);
		ContentArrived* CallbackPtr = new ContentArrived(onArrived);

		ContentAwaitPredicates.Add(PredicatePtr, CallbackPtr);
	}

	void DataState::AddJoinableSpace(FCavrnusSpaceInfo space)
	{
		CurrJoinableSpaces.Add(space);

		for (int i = 0; i < JoinableSpaceAddedBindings.Num(); i++)
			(*JoinableSpaceAddedBindings[i])(space);
	}

	void DataState::UpdateJoinableSpace(FCavrnusSpaceInfo space)
	{
		int indexToRem = -1;
		for (int i = 0; i < CurrJoinableSpaces.Num(); i++) {
			if (CurrJoinableSpaces[i].SpaceId == space.SpaceId)
			{
				indexToRem = i;
				break;
			}
		}
		if (indexToRem != -1)
		{
			CurrJoinableSpaces.RemoveAt(indexToRem);
		}
		CurrJoinableSpaces.Add(space);

		for (int i = 0; i < JoinableSpaceUpdatedBindings.Num(); i++)
			(*JoinableSpaceUpdatedBindings[i])(space);
	}

	void DataState::RemoveJoinableSpace(FCavrnusSpaceInfo space)
	{
		CurrJoinableSpaces.Remove(space);

		for (int i = 0; i < JoinableSpaceRemovedBindings.Num(); i++)
			(*JoinableSpaceRemovedBindings[i])(space);
	}

	UCavrnusBinding* DataState::BindJoinableSpaces(CavrnusSpaceInfoEvent spaceAdded, CavrnusSpaceInfoEvent spaceUpdated, CavrnusSpaceInfoEvent spaceRemoved)
	{
		CavrnusSpaceInfoEvent* added = new CavrnusSpaceInfoEvent(spaceAdded);
		CavrnusSpaceInfoEvent* updated = new CavrnusSpaceInfoEvent(spaceUpdated);
		CavrnusSpaceInfoEvent* removed = new CavrnusSpaceInfoEvent(spaceRemoved);

		JoinableSpaceAddedBindings.Add(added);
		JoinableSpaceUpdatedBindings.Add(updated);
		JoinableSpaceRemovedBindings.Add(removed);

		for (int i = 0; i < CurrJoinableSpaces.Num(); i++)
			spaceAdded(CurrJoinableSpaces[i]);

		auto bindingId = Cavrnus::CavrnusBindingModel::GetBindingModel()->RegisterBinding([this, added, updated, removed]()
			{
				JoinableSpaceAddedBindings.Remove(added);
				JoinableSpaceUpdatedBindings.Remove(updated);
				JoinableSpaceRemovedBindings.Remove(removed);
			});

		UCavrnusBinding* binding;
		binding = NewObject<UCavrnusBinding>();
		binding->Setup(bindingId);

		return binding;
	}
} // namespace Cavrnus
