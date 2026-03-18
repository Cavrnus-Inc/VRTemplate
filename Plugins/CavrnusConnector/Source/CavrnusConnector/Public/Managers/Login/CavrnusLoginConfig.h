// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once
#include "CavrnusConnectorSettings.h"

struct CAVRNUSCONNECTOR_API FCavrnusLoginConfig
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

	// Static builders
	static FCavrnusLoginConfig FromPluginSettings();
	static FCavrnusLoginConfig ForMember(const FString& Server, const FString& Email = "", const FString& Password = "", const FString& SpaceJoinId = "");
	static FCavrnusLoginConfig ForGuest(const FString& Server, const FString& GuestName = "", const FString& SpaceJoinId = "");
	static FCavrnusLoginConfig ForAllowBoth(const FString& Server, const FString& SpaceJoinId = "", const FString& GuestName = "", const FString& MemberEmail = "", const FString& MemberPassword = "");
	static FCavrnusLoginConfig ForPIE(const FString& Server, ECavrnusAuthMethodForPIE AuthMethodForPIE, const FString& ApiKey = "", const FString& ApiToken = "");

	// Sets enum values to match provided data (e.g. if email+pw provided -> EnterMemberCredentials)
	void DeriveEnumsFromData();

	// Validates config, logs warnings for conflicts, fixes Guest+SpacesListMenu
	void Validate();

	// Returns a one-line summary for logging
	FString ToDebugString() const;
};
