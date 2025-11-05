// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Containers/Map.h>
#include "Types/AbsolutePropertyId.h"
#include "Types/CavrnusBinding.h"
#include "Types/CavrnusCallbackTypes.h"
#include "Types/ChatEntry.h"

namespace Cavrnus
{
	class SpaceChatModel
	{
	public:
		SpaceChatModel();
		virtual ~SpaceChatModel();

		void AddChat(FChatEntry chat);
		void UpdateChat(FChatEntry chat);
		void RemoveChat(FString chatId);

		UCavrnusBinding* BindChatEvents(CavrnusChatFunction chatAdded, CavrnusChatFunction chatUpdated, CavrnusChatRemovedFunction chatRemoved);

	private:
		TMap<FAbsolutePropertyId, FChatEntry> CurrChatEntries;

		TArray<CavrnusChatFunction*> ChatAddedBindings;
		TArray<CavrnusChatFunction*> ChatUpdatedBindings;
		TArray<CavrnusChatRemovedFunction*> ChatRemovedBindings;
	};
} // namespace Cavrnus
