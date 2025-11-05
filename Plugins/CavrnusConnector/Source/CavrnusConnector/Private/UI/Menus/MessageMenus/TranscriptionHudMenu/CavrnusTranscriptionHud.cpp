// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/MessageMenus/TranscriptionHudMenu/CavrnusTranscriptionHud.h"
#include "CavrnusFunctionLibrary.h"
#include "UI/Menus/MessageMenus/TranscriptionHudMenu/CavrnusTranscriptionHudEntry.h"

void UCavrnusTranscriptionHud::NativeConstruct()
{
	Super::NativeConstruct();

	HookChatBindings();
}

void UCavrnusTranscriptionHud::NativeDestruct()
{
	Super::NativeDestruct();

	if (ChatEventsBinding != nullptr)
		ChatEventsBinding->Unbind();
}

void UCavrnusTranscriptionHud::HookChatBindings()
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
	};
	
	UCavrnusFunctionLibrary::AwaitAnySpaceConnection(OnConnected);
}

void UCavrnusTranscriptionHud::HandleChatAdded(FChatEntry ChatAdded)
{
	if (!ChatAdded.IsTranscription)
		return;
	
	if (UCavrnusTranscriptionHudEntry* CW = CreateWidget<UCavrnusTranscriptionHudEntry>(this, TranscriptionHudEntryWidget))
	{
		CW->Setup(ChatAdded, DurationVisible);
		CW->SetVisibility(ESlateVisibility::Visible);

		// TODO: Perf analysis
		CW->OnKilled.BindLambda([this](UCavrnusTranscriptionHudEntry* UserWidget)
		{
			// UE_LOG(LogCavrnusConnector, Log, TEXT("OnKilled"));
			if (UserWidget)
			{
				// UE_LOG(LogCavrnusConnector, Log, TEXT("Removing %s"), *UserWidget->ChatEntry.ChatDisplayText);

				UserWidget->RemoveFromParent();
				VisibleChatWidgets.Remove(UserWidget);
				
				SortVisibleWidgets();
				RebuildChildren();
			}
		});

		AllWidgetsMap.Add(ChatAdded.ChatId, CW);
		VisibleChatWidgets.Add(CW);
		
		SortVisibleWidgets();
		RebuildChildren();
	}
}

void UCavrnusTranscriptionHud::HandleChatUpdated(FChatEntry ChatUpdated)
{
	
}

void UCavrnusTranscriptionHud::HandleChatRemoved(FString ChatRemoved)
{

}

void UCavrnusTranscriptionHud::RebuildChildren()
{
	ScrollBox->ClearChildren();

	const int32 NumToShow = FMath::Min(MaxEntriesVisible, VisibleChatWidgets.Num());

	for (int i = 0; i < NumToShow; i++)
	{
		UUserWidget* Widget = VisibleChatWidgets[i];
		ScrollBox->AddChild(Widget);
		Widget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UCavrnusTranscriptionHud::SortVisibleWidgets()
{
	VisibleChatWidgets.Sort([](const UCavrnusTranscriptionHudEntry& A, const UCavrnusTranscriptionHudEntry& B)
	{
		return A.ChatEntry.CreatedTime < B.ChatEntry.CreatedTime;
	});
}
