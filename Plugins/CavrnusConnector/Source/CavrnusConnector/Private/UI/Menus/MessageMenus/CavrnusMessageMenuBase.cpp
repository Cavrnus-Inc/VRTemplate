// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/MessageMenus/CavrnusMessageMenuBase.h"

#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "Types/CavrnusCallbackTypes.h"
#include "TimerManager.h"

void UCavrnusMessageMenuBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ChatScrollBox)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Null ParentContainer!"));
		return;
	}

	if (!ChatEntryWidget)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Null ChatEntryWidget!"));
		return;
	}

	if (!ChatScrollBox->IsValidLowLevel())
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("ChatScrollBox is not valid!"));
		return;
	}
	
	ChatScrollBox->OnUserScrolled.AddDynamic(this, &UCavrnusMessageMenuBase::OnScrollBoxScrolled);
	ResetPositionButton->SetVisibility(ESlateVisibility::Hidden);

	HookChatBindings();
	ScrollToEnd();
}

void UCavrnusMessageMenuBase::NativeDestruct()
{
	Super::NativeDestruct();

	if (ChatScrollBox)
		ChatScrollBox->OnUserScrolled.RemoveDynamic(this, &UCavrnusMessageMenuBase::OnScrollBoxScrolled);

	if (ChatEventsBinding)
		ChatEventsBinding->Unbind();

	if (GetWorld())
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	SortedChatWidgets.Empty();
	AllWidgetsMap.Empty();
}

void UCavrnusMessageMenuBase::ResetPositionButtonClicked()
{
	ScrollToEnd();
}

void UCavrnusMessageMenuBase::OnScrollBoxScrolled(float CurrentOffset)
{
	const float ContentHeight = ChatScrollBox->GetScrollOffsetOfEnd();
	const bool IsAtBottom = (FMath::Abs(ContentHeight - CurrentOffset) < 0.01f);
	ResetPositionButton->SetVisibility(IsAtBottom ? ESlateVisibility::Hidden : ESlateVisibility::Visible);

	AutoScrollToEnd = IsAtBottom;
}

void UCavrnusMessageMenuBase::ScrollToEnd()
{
	if (GetWorld())
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UCavrnusMessageMenuBase::ScrollToBottomAfterDelay);
}

void UCavrnusMessageMenuBase::ScrollToBottomAfterDelay()
{
	AutoScrollToEnd = true;;
	ChatScrollBox->SetScrollOffset(FLT_MAX);

	ResetPositionButton->SetVisibility(ESlateVisibility::Hidden);
}

void UCavrnusMessageMenuBase::LocalUserArrived() {}

void UCavrnusMessageMenuBase::HookChatBindings()
{
	const CavrnusSpaceConnected OnConnected = [this](const FCavrnusSpaceConnection& SpaceConn)
	{
		SpaceConnection = SpaceConn;
		
		ChatEventsBinding = UCavrnusFunctionLibrary::BindChatMessages(SpaceConn, [this](const FChatEntry& ChatAdded)
		{
			HandleChatAdded(ChatAdded);
		}, [this](FChatEntry ChatUpdated)
		{
			HandleChatUpdated(ChatUpdated);
		},
		[this](FString ChatRemoved)
		{
			HandleChatRemoved(ChatRemoved);
		});

		const CavrnusSpaceUserEvent OnLocalUserJoined = [this](const FCavrnusUser& Lu)
		{
			LocalUserArrived();
		};
		
		UCavrnusFunctionLibrary::AwaitLocalUser(SpaceConn,OnLocalUserJoined);
	};
	
	UCavrnusFunctionLibrary::AwaitAnySpaceConnection(OnConnected);
}

void UCavrnusMessageMenuBase::HandleChatAdded(FChatEntry ChatAdded)
{
	if (UCavrnusChatEntry* ChatWidget = CreateWidget<UCavrnusChatEntry>(this, ChatEntryWidget))
	{
		ChatWidget->Setup(ChatAdded);
		ChatWidget->SetVisibility(ESlateVisibility::Visible);

		AllWidgetsMap.Add(ChatAdded.ChatId, ChatWidget);

		//Sort Chats
		SortedChatWidgets.Add(ChatWidget);
		SortedChatWidgets.Sort([](const UCavrnusChatEntry& A, const UCavrnusChatEntry& B)
		{
			return A.ChatData.CreatedTime < B.ChatData.CreatedTime;
		});

		RebuildChildren();
	}

	if (AutoScrollToEnd)
		ScrollToEnd();
}

void UCavrnusMessageMenuBase::HandleChatUpdated(FChatEntry ChatUpdated)
{
	// UE_LOG(LogCavrnusConnector, Display, TEXT("%s Updated!"), *ChatUpdated.ChatId);

	if (AllWidgetsMap.Contains(ChatUpdated.ChatId))
		AllWidgetsMap[ChatUpdated.ChatId]->Setup(ChatUpdated);
}

void UCavrnusMessageMenuBase::HandleChatRemoved(const FString RemovalId)
{
	const int32 FoundIndex = SortedChatWidgets.IndexOfByPredicate([&] (UCavrnusChatEntry* Entry)
	{
		return Entry->ChatData.ChatId.Equals(RemovalId, ESearchCase::CaseSensitive);
	});

	if (FoundIndex != INDEX_NONE)
	{
		SortedChatWidgets.RemoveAt(FoundIndex);

		if (UCavrnusChatEntry** FoundWidget = AllWidgetsMap.Find(RemovalId))
		{
			(*FoundWidget)->RemoveFromParent();
			AllWidgetsMap.Remove(RemovalId);

			// UE_LOG(LogCavrnusConnector, Display, TEXT("%s Removed!"), *RemovalId);
		}
		else
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("Chat %s not found in CreatedWidgetsMap!"), *RemovalId);
		}
	} else
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Failure to remove Chat: %s"), *RemovalId);
	}
}

void UCavrnusMessageMenuBase::ForceChildrenRebuild()
{
	Super::ForceChildrenRebuild();

	RebuildChildren();
}

void UCavrnusMessageMenuBase::RebuildChildren()
{
	//Because InsertAtChildren + rebuilding layout doesn't work (UE issue), this is the next best option...
	ChatScrollBox->ClearChildren();
	for (UCavrnusChatEntry* Chat : SortedChatWidgets)
	{	
		ChatScrollBox->AddChild(Chat);
		Chat->SetVisibility(ESlateVisibility::Visible);
	}
}
