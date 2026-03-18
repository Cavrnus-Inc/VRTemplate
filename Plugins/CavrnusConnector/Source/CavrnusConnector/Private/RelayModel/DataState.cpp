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
		// Clean up heap-allocated callbacks to prevent leaks
		for (auto* cb : spaceConnectionBindings)
			delete cb;
		spaceConnectionBindings.Empty();

		for (auto* cb : spaceExitedBindings)
			delete cb;
		spaceExitedBindings.Empty();
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

		// Guard against null/empty callbacks — crash at 0x0 observed here
		for (int i = 0; i < spaceConnectionBindings.Num(); i++)
		{
			if (spaceConnectionBindings[i])
			{
				(*spaceConnectionBindings[i])(FCavrnusSpaceConnection(spaceConnection.SpaceConnectionId));
			}
			delete spaceConnectionBindings[i];
		}
		spaceConnectionBindings.Empty();
	}

	void DataState::RemoveSpaceConnection(int spaceConnId)
	{
		CurrentSpaceConnections.RemoveAll([spaceConnId](const FCavrnusSpaceConnectionInfo& Element)
		{
			return Element.SpaceConnectionId == spaceConnId;
		});

		// Take ownership, then clear — same null guard pattern as AddSpaceConnection
		auto cb = spaceExitedBindings;
		spaceExitedBindings.Empty();
		for (int i = 0; i < cb.Num(); i++)
		{
			if (CurrentSpaceConnections.Num() == 0 && cb[i])
			{
				(*cb[i])();
			}
			delete cb[i];
		}
	}

	bool DataState::IsSpaceConnectionActive(int spaceConnId) const
	{
		for (const auto& Conn : CurrentSpaceConnections)
		{
			if (Conn.SpaceConnectionId == spaceConnId)
				return true;
		}
		return false;
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

		CurrRemoteContent.Add(content.ContentId, content);
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
		// Find the existing space and detect changes before updating
		FCavrnusSpaceInfo oldSpace;
		int indexToRem = -1;
		for (int i = 0; i < CurrJoinableSpaces.Num(); i++) {
			if (CurrJoinableSpaces[i].SpaceId == space.SpaceId)
			{
				oldSpace = CurrJoinableSpaces[i];
				indexToRem = i;
				break;
			}
		}

		// Preserve tags from cached space if update has none (server often omits tags in updates)
		if (indexToRem != -1 && space.Tags.Num() == 0 && oldSpace.Tags.Num() > 0)
		{
			space.Tags = oldSpace.Tags;
		}

		// Detect what changed
		ESpaceInfoChangeFlags changedFlags = ESpaceInfoChangeFlags::None;
		if (indexToRem != -1)
		{
			changedFlags = DetectSpaceInfoChanges(oldSpace, space);
			CurrJoinableSpaces.RemoveAt(indexToRem);
		}
		CurrJoinableSpaces.Add(space);

		// Fire legacy update callbacks
		for (int i = 0; i < JoinableSpaceUpdatedBindings.Num(); i++)
			(*JoinableSpaceUpdatedBindings[i])(space);

		// Fire change-specific callbacks if any changes detected
		if (changedFlags != ESpaceInfoChangeFlags::None)
		{
			for (const SpaceInfoChangedBinding& binding : SpaceInfoChangedBindings)
			{
				// Check if any of the masked flags changed
				ESpaceInfoChangeFlags relevantChanges = changedFlags & binding.Mask;
				if (relevantChanges != ESpaceInfoChangeFlags::None)
				{
					(*binding.Callback)(space, relevantChanges);
				}
			}
		}
	}

	void DataState::RemoveJoinableSpace(FCavrnusSpaceInfo space)
	{
		CurrJoinableSpaces.Remove(space);

		for (int i = 0; i < JoinableSpaceRemovedBindings.Num(); i++)
			(*JoinableSpaceRemovedBindings[i])(space);
	}

	const FCavrnusSpaceInfo* DataState::GetJoinableSpaceById(const FString& spaceId) const
	{
		for (const FCavrnusSpaceInfo& space : CurrJoinableSpaces)
		{
			if (space.SpaceId == spaceId)
			{
				return &space;
			}
		}
		return nullptr;
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

	ESpaceInfoChangeFlags DataState::DetectSpaceInfoChanges(const FCavrnusSpaceInfo& oldInfo, const FCavrnusSpaceInfo& newInfo)
	{
		ESpaceInfoChangeFlags changes = ESpaceInfoChangeFlags::None;

		if (oldInfo.SpaceName != newInfo.SpaceName)
		{
			changes |= ESpaceInfoChangeFlags::Name;
		}

		if (oldInfo.SpaceThumbnail != newInfo.SpaceThumbnail)
		{
			changes |= ESpaceInfoChangeFlags::Thumbnail;
		}

		if (oldInfo.OwnerId != newInfo.OwnerId)
		{
			changes |= ESpaceInfoChangeFlags::Owner;
		}

		if (oldInfo.LastAccess != newInfo.LastAccess)
		{
			changes |= ESpaceInfoChangeFlags::LastAccess;
		}

		if (oldInfo.Keywords != newInfo.Keywords)
		{
			changes |= ESpaceInfoChangeFlags::Keywords;
		}

		// Compare members by count and content
		if (oldInfo.SpaceMembers.Num() != newInfo.SpaceMembers.Num())
		{
			changes |= ESpaceInfoChangeFlags::Members;
		}
		else
		{
			for (int i = 0; i < oldInfo.SpaceMembers.Num(); i++)
			{
				if (!(oldInfo.SpaceMembers[i] == newInfo.SpaceMembers[i]))
				{
					changes |= ESpaceInfoChangeFlags::Members;
					break;
				}
			}
		}

		// Compare tags
		if (oldInfo.Tags.Num() != newInfo.Tags.Num())
		{
			changes |= ESpaceInfoChangeFlags::Tags;
		}
		else
		{
			for (const auto& pair : oldInfo.Tags)
			{
				const FString* newValue = newInfo.Tags.Find(pair.Key);
				if (!newValue || *newValue != pair.Value)
				{
					changes |= ESpaceInfoChangeFlags::Tags;
					break;
				}
			}
		}

		return changes;
	}

	UCavrnusBinding* DataState::BindSpaceInfoChanged(ESpaceInfoChangeFlags changeMask, CavrnusSpaceInfoChangedEvent onChanged)
	{
		CavrnusSpaceInfoChangedEvent* callback = new CavrnusSpaceInfoChangedEvent(onChanged);

		SpaceInfoChangedBinding binding;
		binding.Mask = changeMask;
		binding.Callback = callback;
		SpaceInfoChangedBindings.Add(binding);

		auto bindingId = Cavrnus::CavrnusBindingModel::GetBindingModel()->RegisterBinding([this, callback]()
			{
				SpaceInfoChangedBindings.RemoveAll([callback](const SpaceInfoChangedBinding& b)
					{
						return b.Callback == callback;
					});
				delete callback;
			});

		UCavrnusBinding* bindingObj = NewObject<UCavrnusBinding>();
		bindingObj->Setup(bindingId);

		return bindingObj;
	}
} // namespace Cavrnus
