// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/MessageMenus/TranscriptionHudMenu/CavrnusTranscriptionHudEntry.h"

void UCavrnusTranscriptionHudEntry::Setup(const FChatEntry& InChatEntry, const float InDisplayDuration)
{
	ChatEntry = InChatEntry;
	DisplayDuration = InDisplayDuration;
	
	const FString UserName = ChatEntry.ChatCreatorIsLocalUser ? "You" : ChatEntry.ChatCreatorName;

	if (CreatorName)
		CreatorName->SetText(FText::FromString(UserName));

	if (CreationTime)
		CreationTime->SetText(FText::FromString(FormatTime(ChatEntry)));

	if (Message)
		Message->SetText(FText::FromString(ChatEntry.ChatDisplayText));

	if (ChatEntry.Complete)
	{
		CanTick = false;
		TranscriptionComplete();
	}
	else
	{
		TranscriptionInProgress();
		CanTick = true;
	}

	SetupComplete(ChatEntry);
}

void UCavrnusTranscriptionHudEntry::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (CanTick)
	{
		if (ChatEntry.Complete)
		{
			CanTick = false;
			TranscriptionComplete();
		}
	}
}

void UCavrnusTranscriptionHudEntry::DurationExpired()
{
	// UE_LOG(LogCavrnusConnector, Log, TEXT("Trying to kill: %s"), *ChatEntry.ChatDisplayText);

	if (OnKilled.IsBound())
		OnKilled.Execute(this);
}

FString UCavrnusTranscriptionHudEntry::FormatTime(const FChatEntry& InChatEntry)
{
	const FDateTime LocalNow = FDateTime::Now();
	const FDateTime UtcNow = FDateTime::UtcNow();
	const FTimespan TimeZoneOffset = LocalNow - UtcNow;

	const FDateTime LocalTime = InChatEntry.CreatedTime + TimeZoneOffset;
	
	const int32 Hour24 = LocalTime.GetHour();
	const int32 Minute = LocalTime.GetMinute();
    
	const FString AmPm = Hour24 < 12 ? TEXT("am") : TEXT("pm");

	// Convert hour to 12-hour format
	int32 Hour12 = Hour24 % 12;
	if (Hour12 == 0)
		Hour12 = 12; // 12 AM or 12 PM

	return FString::Printf(TEXT("%d:%02d %s"), Hour12, Minute, *AmPm);
}
