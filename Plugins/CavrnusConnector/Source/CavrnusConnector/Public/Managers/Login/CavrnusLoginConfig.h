// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once
#include "CavrnusConnectorSettings.h"

struct FCavrnusLoginConfig
{
	bool bIsPieUserLogin = false;
	
	ECavrnusAuthMethod AuthMethod = ECavrnusAuthMethod::JoinAsMember;
	
	ECavrnusMemberLoginMethod MemberLoginMethod = ECavrnusMemberLoginMethod::PromptMemberToLogin;
	ECavrnusGuestLoginMethod GuestLoginMethod = ECavrnusGuestLoginMethod::PromptToEnterName;
	ECavrnusSpaceJoinMethod SpaceJoinMethod = ECavrnusSpaceJoinMethod::SpacesListMenu;
	
	FString Server = "";
	FString SpaceJoinId = "";

	FString ApiToken = "";
	FString ApiKey = "";
	 
	FString GuestName = "";
	
	FString MemberLoginEmail = "";
	FString MemberLoginPassword = "";
};
