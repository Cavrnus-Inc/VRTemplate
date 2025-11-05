// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Helpers/CavrnusUserHelpers.h"

#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "Helpers/CavrnusMathHelpers.h"
#include "LivePropertyUpdates/CavrnusLiveColorPropertyUpdate.h"

FString FCavrnusUserHelpers::GetUserEmail(const FCavrnusUser& InUser)
{
	return UCavrnusFunctionLibrary::GetStringPropertyValue(InUser.SpaceConn, InUser.PropertiesContainerName, "email");
}

FString FCavrnusUserHelpers::GetUserProfilePhoto(const FCavrnusUser& InUser)
{
	return UCavrnusFunctionLibrary::GetStringPropertyValue(InUser.SpaceConn, InUser.PropertiesContainerName, "profilePicture");
}

FString FCavrnusUserHelpers::GetPreferredUserDisplayName(const FCavrnusUser& InUser)
{
	auto Name = GetUserName(InUser);

	if (Name.IsEmpty())
		Name = GetUserEmail(InUser);
	
	if (InUser.IsLocalUser)
		Name += " (You)";

	return Name;
}

FString FCavrnusUserHelpers::GetPreferredUserDisplayName(const FCavrnusUserAccount& InUser, const bool LastNameFirst)
{
	if (InUser.UserFirstName.IsEmpty() && InUser.UserLastName.IsEmpty())
		return InUser.UserEmail;

	return GetFullUserName(InUser, LastNameFirst);
}

FString FCavrnusUserHelpers::GetFullUserName(const FCavrnusUserAccount& InUser, const bool LastNameFirst)
{
	if (LastNameFirst)
		return FString::Printf(TEXT("%s, %s"), *InUser.UserLastName, *InUser.UserFirstName);
	else
		return FString::Printf(TEXT("%s %s"), *InUser.UserFirstName, *InUser.UserLastName);
}

UCavrnusBinding* FCavrnusUserHelpers::BindUserName(
	const FCavrnusUser& InUser,
	TFunction<void(const FString&)> OnNameUpdated)
{
	return UCavrnusFunctionLibrary::BindStringPropertyValue(
		InUser.SpaceConn,
		InUser.PropertiesContainerName,
		"name",
		[OnNameUpdated](const FString& Val, const FString&, const FString&)
	{
		if (OnNameUpdated)
			OnNameUpdated(Val);
	});
}

FString FCavrnusUserHelpers::GetUserName(const FCavrnusUser& InUser)
{
	return UCavrnusFunctionLibrary::GetStringPropertyValue(InUser.SpaceConn, InUser.PropertiesContainerName, "name");
}

UCavrnusBinding* FCavrnusUserHelpers::BindUserPrimaryColor(const FCavrnusUser& InUser, TFunction<void(const FLinearColor&)> OnColorUpdated)
{
	return UCavrnusFunctionLibrary::BindColorPropertyValue(
	InUser.SpaceConn,
	InUser.PropertiesContainerName, "primaryColor",
	[OnColorUpdated](const FLinearColor& Val, const FString&, const FString&)
	{
		if (OnColorUpdated)
			OnColorUpdated(Val);
	});
}

FLinearColor FCavrnusUserHelpers::GetUserSessionPrimaryColor(const FCavrnusUser& InUser)
{
	return UCavrnusFunctionLibrary::GetColorPropertyValue(InUser.SpaceConn, InUser.PropertiesContainerName, "primaryColor");
}

bool FCavrnusUserHelpers::GetUserMuted(const FCavrnusUser& InUser)
{
	return UCavrnusFunctionLibrary::GetBoolPropertyValue(InUser.SpaceConn, InUser.PropertiesContainerName, "muted");
}

UCavrnusBinding* FCavrnusUserHelpers::BindUserMuted(const FCavrnusUser& InUser, const TFunction<void(bool)>& OnMutedUpdated)
{
	return UCavrnusFunctionLibrary::BindBooleanPropertyValue(
		InUser.SpaceConn,
		InUser.PropertiesContainerName, "muted",
		[OnMutedUpdated](const bool& Val, const FString&, const FString&)
		{
			if (OnMutedUpdated)
				OnMutedUpdated(Val);
		});
}

bool FCavrnusUserHelpers::GetUserStreaming(const FCavrnusUser& InUser)
{
	return UCavrnusFunctionLibrary::GetBoolPropertyValue(InUser.SpaceConn,InUser.PropertiesContainerName,"streaming");
}

void FCavrnusUserHelpers::SetUserMuted(const FCavrnusUser& InUser, const bool InMuted)
{
	if (InUser.IsLocalUser)
		UCavrnusFunctionLibrary::SetLocalUserMutedState(InUser.SpaceConn, InMuted);
	else
		UE_LOG(LogCavrnusConnector, Error, TEXT("Unable to mute remote user! Only allows muting local FCavrnusUser."))
}

UCavrnusBinding* FCavrnusUserHelpers::BindUserStreaming(const FCavrnusUser& InUser, TFunction<void(const bool)> OnStreamingUpdated)
{
	return UCavrnusFunctionLibrary::BindBooleanPropertyValue(
	InUser.SpaceConn, InUser.PropertiesContainerName, "streaming",
	[OnStreamingUpdated](const bool Val, const FString&, const FString&)
	{
		if (OnStreamingUpdated)
			OnStreamingUpdated(Val);
	});
}

UCavrnusBinding* FCavrnusUserHelpers::BindUserVideoFrames(const FCavrnusUser& InUser, const TFunction<void(UTexture2D*, const FVector2D& AspectRatio)>& OnStreamingUpdated)
{
	const VideoFrameUpdateFunction UserVideoFrameUpdate = [OnStreamingUpdated](UTexture2D* InTexture) {
		if (!InTexture)
		{
			return;
		}
		
		const int32 TextureWidth = InTexture->GetSizeX();
		const int32 TextureHeight = InTexture->GetSizeY();

		if (TextureWidth <= 0 || TextureHeight <= 0)
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("InTexture has invalid dimensions: %d x %d"), TextureWidth, TextureHeight);
			return;
		}

		FVector2D OutAspectRatio = FVector2D(1,1);

		if (TextureHeight != 0)
		{
			// Trying to ensure that the texture ratio is something like 16:9, for instance
			const int32 GCD = FCavrnusMathHelpers::GetGCD(TextureWidth, TextureHeight);
			const int32 RatioNumerator = TextureWidth / GCD;
			const int32 RatioDenominator = TextureHeight / GCD;

			OutAspectRatio.X = RatioNumerator;
			OutAspectRatio.Y = RatioDenominator;
		}
		
		if (OnStreamingUpdated)
			OnStreamingUpdated(InTexture, OutAspectRatio);
	};
	
	return UCavrnusFunctionLibrary::BindUserVideoFrames(InUser.SpaceConn, InUser, UserVideoFrameUpdate);
}

void FCavrnusUserHelpers::SetLocalUserStreaming(const FCavrnusUser& InUser, const bool InStreaming)
{
	if (InUser.IsLocalUser)
		UCavrnusFunctionLibrary::SetLocalUserStreamingState(InUser.SpaceConn, InStreaming);
	else
		UE_LOG(LogCavrnusConnector, Error, TEXT("Unable to set stCavrnusConnectorstatus of remote user! Only allows muting local FCavrnusUser."))
}

UCavrnusBinding* FCavrnusUserHelpers::BindUserSpeaking(const FCavrnusUser& InUser, TFunction<void(bool)> OnSpeakingUpdated)
{
	return UCavrnusFunctionLibrary::BindBooleanPropertyValue(
		InUser.SpaceConn,
		InUser.PropertiesContainerName, "speaking",
		[OnSpeakingUpdated](const bool& Val, const FString&, const FString&)
		{
			if (OnSpeakingUpdated)
				OnSpeakingUpdated(Val);
		});
}