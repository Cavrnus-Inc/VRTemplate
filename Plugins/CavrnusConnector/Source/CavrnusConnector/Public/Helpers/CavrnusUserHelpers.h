// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"

struct FCavrnusUserAccount;
class UCavrnusBinding;
struct FCavrnusUser;

class CAVRNUSCONNECTOR_API FCavrnusUserHelpers
{
public:
	// Display Info
	static UCavrnusBinding* BindUserName(const FCavrnusUser& InUser, TFunction<void(const FString&)> OnNameUpdated);
	static FString GetUserName(const FCavrnusUser& InUser);
	static FString GetUserEmail(const FCavrnusUser& InUser);
	static FString GetUserProfilePhoto(const FCavrnusUser& InUser);
	
	static FString GetPreferredUserDisplayName(const FCavrnusUser& InUser);
	static FString GetPreferredUserDisplayName(const FCavrnusUserAccount& InUser, bool LastNameFirst = false);
	static FString GetFullUserName(const FCavrnusUserAccount& InUser, bool LastNameFirst = false);
	
	// Colors
	static UCavrnusBinding* BindUserPrimaryColor(const FCavrnusUser& InUser, TFunction<void(const FLinearColor&)> OnColorUpdated);
	static FLinearColor GetUserSessionPrimaryColor(const FCavrnusUser& InUser);
	
	// Mute
	static bool GetUserMuted(const FCavrnusUser& InUser);
	static void SetUserMuted(const FCavrnusUser& InUser, const bool InMuted);
	static UCavrnusBinding* BindUserMuted(const FCavrnusUser& InUser, const TFunction<void(bool)>& OnMutedUpdated);
	
	// Stream
	static bool GetUserStreaming(const FCavrnusUser& InUser);
	static void SetLocalUserStreaming(const FCavrnusUser& InUser, const bool InStreaming);
	static UCavrnusBinding* BindUserStreaming(const FCavrnusUser& InUser, TFunction<void(const bool)> OnStreamingUpdated);
	static UCavrnusBinding* BindUserVideoFrames(const FCavrnusUser& InUser, const TFunction<void(UTexture2D*, const FVector2D& AspectRatio)>& OnStreamingUpdated);
	
	// Speaking
	static UCavrnusBinding* BindUserSpeaking(const FCavrnusUser& InUser, TFunction<void(bool)> OnSpeakingUpdated);
};