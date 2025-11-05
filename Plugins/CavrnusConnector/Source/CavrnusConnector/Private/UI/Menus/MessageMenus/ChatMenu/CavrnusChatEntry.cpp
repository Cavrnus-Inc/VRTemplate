// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/MessageMenus/ChatMenu/CavrnusChatEntry.h"
#include "CavrnusFunctionLibrary.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UCavrnusChatEntry::Setup(const FChatEntry& InChatEntry)
{
	ChatData = InChatEntry;
	
	const FString UserName = InChatEntry.ChatCreatorIsLocalUser ? "You" : InChatEntry.ChatCreatorName;

	CreatorName->SetText(FText::FromString(UserName));
	CreationTime->SetText(FText::FromString(FormatTime(InChatEntry)));
	Message->SetText(FText::FromString(InChatEntry.ChatDisplayText));

	if (InChatEntry.ChatCreatorIsLocalUser)
		HandleLocalUserChat();

	SetupComplete(InChatEntry);
}

void UCavrnusChatEntry::HandleLocalUserChat()
{
	ChatBubbleBackground->SetBrushColor(LocalUserColor);
}

FString UCavrnusChatEntry::FormatTime(const FChatEntry& ChatEntry)
{
	const FDateTime LocalNow = FDateTime::Now();
	const FDateTime UtcNow = FDateTime::UtcNow();
	const FTimespan TimeZoneOffset = LocalNow - UtcNow;

	const FDateTime LocalTime = ChatEntry.CreatedTime + TimeZoneOffset;
	
	const int32 Hour24 = LocalTime.GetHour();
	const int32 Minute = LocalTime.GetMinute();
    
	const FString AmPm = Hour24 < 12 ? TEXT("am") : TEXT("pm");

	// Convert hour to 12-hour format
	int32 Hour12 = Hour24 % 12;
	if (Hour12 == 0)
		Hour12 = 12; // 12 AM or 12 PM

	return FString::Printf(TEXT("%d:%02d %s"), Hour12, Minute, *AmPm);
}
