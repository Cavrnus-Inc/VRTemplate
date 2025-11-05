// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayModel/SpaceChatModel.h"
#include "CavrnusConnectorModule.h"
#include "RelayModel/CavrnusBindingModel.h"

namespace Cavrnus
{
	SpaceChatModel::SpaceChatModel()
	{
	}

	SpaceChatModel::~SpaceChatModel()
	{

	}

	void SpaceChatModel::AddChat(FChatEntry chat)
	{
		CurrChatEntries.Add(FAbsolutePropertyId(chat.ChatId), chat);

		for (int i = 0; i < ChatAddedBindings.Num(); i++)
			(*ChatAddedBindings[i])(chat);
	}

	void SpaceChatModel::UpdateChat(FChatEntry chat)
	{
		if (!CurrChatEntries.Contains(FAbsolutePropertyId(chat.ChatId))) 
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("CHAT UPDATE ID NOT FOUND IN DICTIONARY!"));
			return;
		}

		CurrChatEntries[FAbsolutePropertyId(chat.ChatId)] = chat;

		for (int i = 0; i < ChatUpdatedBindings.Num(); i++)
			(*ChatUpdatedBindings[i])(chat);
	}

	void SpaceChatModel::RemoveChat(FString chatId)
	{
		if (!CurrChatEntries.Contains(FAbsolutePropertyId(chatId)))
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("CHAT REMOVAL ID NOT FOUND IN DICTIONARY!"));
			return;
		}

		CurrChatEntries.Remove(FAbsolutePropertyId(chatId));
		for (int i = 0; i < ChatRemovedBindings.Num(); i++)
			(*ChatRemovedBindings[i])(chatId);
	}

	UCavrnusBinding* SpaceChatModel::BindChatEvents(CavrnusChatFunction chatAdded, CavrnusChatFunction chatUpdated, CavrnusChatRemovedFunction chatRemoved)
	{
		for (const TPair<FAbsolutePropertyId, FChatEntry>& pair : CurrChatEntries) 
		{ 
			chatAdded(pair.Value);
		}

		CavrnusChatFunction* addedPtr = new CavrnusChatFunction(chatAdded);
		ChatAddedBindings.Add(addedPtr);

		CavrnusChatFunction* updatedPtr = new CavrnusChatFunction(chatUpdated);
		ChatUpdatedBindings.Add(updatedPtr);

		CavrnusChatRemovedFunction* removedPtr = new CavrnusChatRemovedFunction(chatRemoved);
		ChatRemovedBindings.Add(removedPtr);

		auto bindingId = Cavrnus::CavrnusBindingModel::GetBindingModel()->RegisterBinding([this, addedPtr, updatedPtr, removedPtr]()
			{
				ChatAddedBindings.Remove(addedPtr);
				ChatUpdatedBindings.Remove(updatedPtr);
				ChatRemovedBindings.Remove(removedPtr);
			});


		UCavrnusBinding* binding;
		binding = NewObject<UCavrnusBinding>();
		binding->Setup(bindingId);

		return binding;
	}
} // namespace Cavrnus
