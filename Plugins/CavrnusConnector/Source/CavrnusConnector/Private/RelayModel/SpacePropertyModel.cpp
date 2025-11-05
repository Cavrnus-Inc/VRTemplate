// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayModel/SpacePropertyModel.h"
#include <TextureResource.h>
#include "RelayModel/CavrnusBindingModel.h"
#include "CavrnusGCManager.h"

namespace Cavrnus
{
	SpacePropertyModel::SpacePropertyModel()
	{
		ChatModel = new SpaceChatModel();
	}

	SpacePropertyModel::~SpacePropertyModel()
	{
		for (auto& arrived : LocalUserArrivedCallbacks)
		{
			delete arrived;
		}
		LocalUserArrivedCallbacks.Empty();

		for (auto& added : UserAddedBindings)
		{
			delete added;
		}
		UserAddedBindings.Empty();

		for (auto& removed : UserRemovedBindings)
		{
			delete removed;
		}
		UserRemovedBindings.Empty();
	}

	void SpacePropertyModel::UpdateServerPropVal(const FAbsolutePropertyId& fullPropertyId, FPropertyValue value)
	{
		if (!CurrServerPropValues.Contains(fullPropertyId))
		{
			CurrServerPropValues.FindOrAdd(fullPropertyId);
		}

		CurrServerPropValues[fullPropertyId] = value;

		if (!CurrLocalPropValues.Contains(fullPropertyId))
			TryExecPropBindings(fullPropertyId);
	}

	void SpacePropertyModel::UpdateServerPropPriority(const FAbsolutePropertyId& fullPropertyId, int32 priority)
	{
		if (!CurrServerPropPriorities.Contains(fullPropertyId))
			CurrServerPropPriorities.Add(fullPropertyId);

		CurrServerPropPriorities[fullPropertyId] = priority;
	}

	int SpacePropertyModel::SetLocalPropVal(const FAbsolutePropertyId& fullPropertyId, FPropertyValue value, int32 priority)
	{
		// We treat this like a has, but use a dictionary to handle string comparisons
		if (CurrPropReadonlyMetadata.Contains(fullPropertyId))
		{
			return -1;
		}

		//If the server's value exceeds our priority, don't set a local override
		if (CurrServerPropPriorities.Contains(fullPropertyId) && CurrServerPropPriorities[fullPropertyId] > priority)
		{
			return -1;
		}

		if (!CurrLocalPropValues.Contains(fullPropertyId))
		{
			CurrLocalPropValues.FindOrAdd(fullPropertyId);
			LocalPropValidationIds.FindOrAdd(fullPropertyId);
		}

		CurrLocalPropValues[fullPropertyId] = value;
		TryExecPropBindings(fullPropertyId);

		validationIdIncrementer++;
		LocalPropValidationIds[fullPropertyId] = validationIdIncrementer;
		return validationIdIncrementer;
	}

	FPropertyValue SpacePropertyModel::GetCurrentPropValue(const FAbsolutePropertyId& fullPropertyId)
	{
		FPropertyValue* CurrServerValue = CurrServerPropValues.Find(fullPropertyId);
		FPropertyValue* CurrLocalValue = CurrLocalPropValues.Find(fullPropertyId);

		if (CurrServerValue && CurrLocalValue)
			return (CurrServerValue->Priority > CurrLocalValue->Priority) ? *CurrServerValue : *CurrLocalValue;

		if (CurrServerValue)
			return *CurrServerValue;

		if (CurrLocalValue)
			return *CurrLocalValue;

		//Returns a default value
		return FPropertyValue();
	}
	
	void SpacePropertyModel::InvalidateLocalPropValue(const FAbsolutePropertyId& fullPropertyId, int propValidationId)
	{
		if (LocalPropValidationIds.Contains(fullPropertyId) && LocalPropValidationIds[fullPropertyId] == propValidationId)
		{
			LocalPropValidationIds.Remove(fullPropertyId);

			const FPropertyValue& oldPropVal = GetCurrentPropValue(fullPropertyId);

			CurrLocalPropValues.Remove(fullPropertyId);

			const FPropertyValue& newPropVal = GetCurrentPropValue(fullPropertyId);

			bool changed = !(newPropVal == oldPropVal);
			if (changed)
				TryExecPropBindings(fullPropertyId);
		}
	}

	void SpacePropertyModel::UpdatePropMetadata(const FAbsolutePropertyId& fullPropertyId, bool isReadonly)
	{
		if (!isReadonly && CurrPropReadonlyMetadata.Contains(fullPropertyId))
		{
			CurrPropReadonlyMetadata.Remove(fullPropertyId);
		}
		else if (isReadonly && !CurrPropReadonlyMetadata.Contains(fullPropertyId))
		{
			CurrPropReadonlyMetadata.Add(fullPropertyId, true);
		}

		if (isReadonly && LocalPropValidationIds.Contains(fullPropertyId))
		{
			LocalPropValidationIds.Remove(fullPropertyId);

			const FPropertyValue& oldPropVal = GetCurrentPropValue(fullPropertyId);

			CurrLocalPropValues.Remove(fullPropertyId);

			const FPropertyValue& newPropVal = GetCurrentPropValue(fullPropertyId);

			bool changed = !(newPropVal == oldPropVal);
			if (changed)
				TryExecPropBindings(fullPropertyId);
		}
	}

	void SpacePropertyModel::TryExecPropBindings(const FAbsolutePropertyId& fullPropertyId)
	{
		const FPropertyValue& activePropVal = GetCurrentPropValue(fullPropertyId);

		if (PropBindings.Contains(fullPropertyId))
		{
			auto bindings = PropBindings[fullPropertyId];
			for (int i = 0; i < bindings.Num(); i++)
				(*bindings[i])(activePropVal, fullPropertyId.ContainerName, fullPropertyId.PropValueId);
		}
	}

	bool SpacePropertyModel::PropValueExists(const FAbsolutePropertyId& fullPropertyId)
	{
		return CurrLocalPropValues.Contains(fullPropertyId) || CurrServerPropValues.Contains(fullPropertyId);
	}

	UCavrnusBinding* SpacePropertyModel::BindProperty(const FAbsolutePropertyId& fullPropertyId, CavrnusPropertyFunction callback)
	{
		CavrnusPropertyFunction* cb = new CavrnusPropertyFunction(callback);
		PropBindings.FindOrAdd(fullPropertyId);
		PropBindings[fullPropertyId].Add(cb);

		callback(GetCurrentPropValue(fullPropertyId), fullPropertyId.ContainerName, fullPropertyId.PropValueId);

		auto bindingId = Cavrnus::CavrnusBindingModel::GetBindingModel()->RegisterBinding([this, fullPropertyId, cb]()
			{
				if (!PropBindings.Contains(fullPropertyId))
					return;
				PropBindings[fullPropertyId].Remove(cb);
				if (PropBindings[fullPropertyId].IsEmpty())
					PropBindings.Remove(fullPropertyId);
			});


		UCavrnusBinding* binding;
		binding = NewObject<UCavrnusBinding>();
		binding->Setup(bindingId);

		return binding;
	}

	UCavrnusBinding* SpacePropertyModel::BindUserVideoTexture(const FCavrnusUser& User, VideoFrameUpdateFunction Callback)
	{
		FString UserConnectionId = User.UserConnectionId;

		if (CurrSpaceUsersVideoTextures.Contains(UserConnectionId))
			Callback(CurrSpaceUsersVideoTextures[UserConnectionId]);

		VideoFrameUpdateFunction* cb = new VideoFrameUpdateFunction(Callback);
		UserVideoFrameBindings.FindOrAdd(UserConnectionId);
		UserVideoFrameBindings[UserConnectionId].Add(cb);

		auto bindingId = Cavrnus::CavrnusBindingModel::GetBindingModel()->RegisterBinding([this, UserConnectionId, cb]()
			{
				if (!UserVideoFrameBindings.Contains(UserConnectionId))
					return;
				UserVideoFrameBindings[UserConnectionId].Remove(cb);
				if (UserVideoFrameBindings[UserConnectionId].IsEmpty())
					UserVideoFrameBindings.Remove(UserConnectionId);
			});


		UCavrnusBinding* binding;
		binding = NewObject<UCavrnusBinding>();
		binding->Setup(bindingId);

		return binding;
	}

	Cavrnus::FPropertyValue SpacePropertyModel::GetPropValue(const FAbsolutePropertyId& fullPropertyId)
	{
		return GetCurrentPropValue(fullPropertyId);
	}

	void SpacePropertyModel::SetIsDefined(const FAbsolutePropertyId& fullPropertyId)
	{
		CurrDefinedProps.Add(fullPropertyId);
	}

	void SpacePropertyModel::AddSpaceUser(const FCavrnusUser& user)
	{
		if (user.IsLocalUser)
		{
			LocalUser = user;
			hasLocalUser = true;

			for (int i = 0; i < LocalUserArrivedCallbacks.Num(); i++)
			{
				(*LocalUserArrivedCallbacks[i])(user);
				delete LocalUserArrivedCallbacks[i];
			}
			LocalUserArrivedCallbacks.Empty();
		}

		CurrSpaceUsers.Add(user.UserConnectionId, user);
		CurrSpaceUsersVideoTextures.Add(user.UserConnectionId, nullptr);
		for (int i = 0; i < UserAddedBindings.Num(); i++)
			(*UserAddedBindings[i])(user);
	}

	void SpacePropertyModel::RemoveSpaceUser(FString userId)
	{
		if (CurrSpaceUsers.Contains(userId))
		{
			FCavrnusUser removedUser = CurrSpaceUsers[userId];
			CurrSpaceUsers.Remove(userId);

			if(CurrSpaceUsersVideoTextures[userId])
				CavrnusGCManager::GetGCManager()->UntrackItem(CurrSpaceUsersVideoTextures[userId]);
			CurrSpaceUsersVideoTextures.Remove(userId);

			for (int i = 0; i < UserRemovedBindings.Num(); i++)
				(*UserRemovedBindings[i])(removedUser);
		}
	}

	void SpacePropertyModel::UpdateUserVideoTexture(FString userId, int ResX, int ResY, const TArray<uint8>& ByteArray)
	{
		if (!CurrSpaceUsersVideoTextures.Contains(userId))
			return;

		UTexture2D* Tex = CurrSpaceUsersVideoTextures[userId];

		bool TextureCreated = false;
		if (Tex == nullptr || Tex->GetSizeX() != ResX || Tex->GetSizeY() != ResY)
		{
			FName UniqueName = MakeUniqueObjectName(nullptr, UTexture2D::StaticClass(), "RTCStream");
			Tex = UTexture2D::CreateTransient(ResX, ResY, PF_B8G8R8A8, UniqueName);

			CavrnusGCManager::GetGCManager()->TrackItem(Tex);
			CurrSpaceUsersVideoTextures[userId] = Tex;

			TextureCreated = true;
		}

		void* TextureData = Tex->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		int maxBytesToCopy = FMath::Min(ByteArray.Num(), ResX * ResY * 4);
		FMemory::Memcpy(TextureData, ByteArray.GetData(), maxBytesToCopy);
		Tex->GetPlatformData()->Mips[0].BulkData.Unlock();
		Tex->UpdateResource();

		if (TextureCreated && UserVideoFrameBindings.Contains(userId))
		{
			for (auto& Binding : UserVideoFrameBindings[userId])
			{
				(*Binding)(Tex);
			}
		}
	}

	void SpacePropertyModel::AwaitLocalUser(CavrnusSpaceUserEvent localUserArrived)
	{
		if (hasLocalUser)
		{
			localUserArrived(LocalUser);
		}
		else
		{
			CavrnusSpaceUserEvent* callback = new CavrnusSpaceUserEvent(localUserArrived);
			LocalUserArrivedCallbacks.Add(callback);
		}
	}

	UCavrnusBinding* SpacePropertyModel::BindSpaceUsers(CavrnusSpaceUserEvent userAdded, CavrnusSpaceUserEvent userRemoved)
	{
		CavrnusSpaceUserEvent* added = new CavrnusSpaceUserEvent(userAdded);
		UserAddedBindings.Add(added);
		CavrnusSpaceUserEvent* removed = new CavrnusSpaceUserEvent(userRemoved);
		UserRemovedBindings.Add(removed);

		for (auto& UserElem : CurrSpaceUsers)
			userAdded(UserElem.Value);

		auto bindingId = Cavrnus::CavrnusBindingModel::GetBindingModel()->RegisterBinding([this, added, removed]()
			{
				UserAddedBindings.Remove(added);
				UserRemovedBindings.Remove(removed);
			});


		UCavrnusBinding* binding;
		binding = NewObject<UCavrnusBinding>();
		binding->Setup(bindingId);

		return binding;
	}
} // namespace Cavrnus
