// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/Login/CavrnusLoginConfig.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusConnectorSettings.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Managers/CavrnusEditorAuthenticationManager.h"

FCavrnusLoginConfig FCavrnusLoginConfig::FromPluginSettings()
{
	auto* Settings = UCavrnusConnectorSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[FCavrnusLoginConfig::FromPluginSettings] CavrnusConnectorSettings is null!"));
		return FCavrnusLoginConfig();
	}

	FCavrnusLoginConfig Config;
	Config.bIsPieUserLogin = false;
	Config.Server = Settings->ServerDomain;
	Config.AuthMethod = Settings->AuthMethod;

	switch (Settings->AuthMethod)
	{
	case ECavrnusAuthMethod::JoinAsMember:
		Config.MemberLoginEmail = Settings->MemberLoginEmail;
		Config.MemberLoginPassword = Settings->MemberLoginPassword;
		Config.MemberLoginMethod = Settings->MemberLoginMethod;
		break;
	case ECavrnusAuthMethod::JoinAsGuest:
		Config.GuestName = Settings->GuestName;
		Config.GuestLoginMethod = Settings->GuestLoginMethod;
		break;
	case ECavrnusAuthMethod::AllowBoth:
		Config.MemberLoginEmail = Settings->MemberLoginEmail;
		Config.MemberLoginPassword = Settings->MemberLoginPassword;
		Config.MemberLoginMethod = Settings->MemberLoginMethod;
		Config.GuestName = Settings->GuestName;
		Config.GuestLoginMethod = Settings->GuestLoginMethod;
		break;
	}

	Config.SpaceJoinMethod = Settings->SpaceJoinMethod;
	Config.SpaceJoinId = Settings->JoinId;

	return Config;
}

FCavrnusLoginConfig FCavrnusLoginConfig::ForMember(const FString& InServer, const FString& Email, const FString& Password, const FString& InSpaceJoinId)
{
	FCavrnusLoginConfig Config;
	Config.AuthMethod = ECavrnusAuthMethod::JoinAsMember;
	Config.Server = InServer;
	Config.MemberLoginEmail = Email;
	Config.MemberLoginPassword = Password;
	Config.SpaceJoinId = InSpaceJoinId;
	if (!InSpaceJoinId.IsEmpty())
		Config.SpaceJoinMethod = ECavrnusSpaceJoinMethod::EnterJoinId;
	Config.DeriveEnumsFromData();
	return Config;
}

FCavrnusLoginConfig FCavrnusLoginConfig::ForGuest(const FString& InServer, const FString& InGuestName, const FString& InSpaceJoinId)
{
	FCavrnusLoginConfig Config;
	Config.AuthMethod = ECavrnusAuthMethod::JoinAsGuest;
	Config.Server = InServer;
	Config.GuestName = InGuestName;
	Config.SpaceJoinId = InSpaceJoinId;
	if (!InSpaceJoinId.IsEmpty())
		Config.SpaceJoinMethod = ECavrnusSpaceJoinMethod::EnterJoinId;
	Config.DeriveEnumsFromData();
	return Config;
}

FCavrnusLoginConfig FCavrnusLoginConfig::ForAllowBoth(const FString& InServer, const FString& InSpaceJoinId, const FString& InGuestName, const FString& InMemberEmail, const FString& InMemberPassword)
{
	FCavrnusLoginConfig Config;
	Config.AuthMethod = ECavrnusAuthMethod::AllowBoth;
	Config.Server = InServer;
	Config.SpaceJoinId = InSpaceJoinId;
	Config.GuestName = InGuestName;
	Config.MemberLoginEmail = InMemberEmail;
	Config.MemberLoginPassword = InMemberPassword;
	if (!InSpaceJoinId.IsEmpty())
		Config.SpaceJoinMethod = ECavrnusSpaceJoinMethod::EnterJoinId;
	Config.DeriveEnumsFromData();
	return Config;
}

FCavrnusLoginConfig FCavrnusLoginConfig::ForPIE(const FString& InServer, ECavrnusAuthMethodForPIE AuthMethodForPIE, const FString& InApiKey, const FString& InApiToken)
{
	FCavrnusLoginConfig Config;
	Config.Server = InServer;

	switch (AuthMethodForPIE)
	{
	case ECavrnusAuthMethodForPIE::JoinAsPIE:
		Config.bIsPieUserLogin = true;
		Config.ApiKey = InApiKey;
		Config.ApiToken = InApiToken;
		break;
	case ECavrnusAuthMethodForPIE::JoinAsMember:
		Config.AuthMethod = ECavrnusAuthMethod::JoinAsMember;
		break;
	case ECavrnusAuthMethodForPIE::JoinAsGuest:
		Config.AuthMethod = ECavrnusAuthMethod::JoinAsGuest;
		break;
	}

	return Config;
}

void FCavrnusLoginConfig::DeriveEnumsFromData()
{
	if (AuthMethod == ECavrnusAuthMethod::JoinAsMember || AuthMethod == ECavrnusAuthMethod::AllowBoth)
	{
		if (!MemberLoginEmail.IsEmpty() && !MemberLoginPassword.IsEmpty())
			MemberLoginMethod = ECavrnusMemberLoginMethod::EnterMemberCredentials;
	}

	if (AuthMethod == ECavrnusAuthMethod::JoinAsGuest || AuthMethod == ECavrnusAuthMethod::AllowBoth)
	{
		if (!GuestName.IsEmpty())
			GuestLoginMethod = ECavrnusGuestLoginMethod::EnterNameBelow;
	}

	// Note: SpaceJoinMethod is NOT derived here. The explicit setting (plugin settings,
	// command-line, or API builder) takes priority. A non-empty SpaceJoinId with
	// SpacesListMenu means the user wants the list — the ID field may be leftover.
}

void FCavrnusLoginConfig::Validate()
{
	// Check guest+member conflict (data vs enum mismatch) -- skip for AllowBoth since both are expected
	if (AuthMethod == ECavrnusAuthMethod::JoinAsGuest && (!MemberLoginEmail.IsEmpty() || !MemberLoginPassword.IsEmpty()))
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[LoginConfig::Validate] AuthMethod is Guest but member credentials are provided -- they will be ignored"));
	}
	if (AuthMethod == ECavrnusAuthMethod::JoinAsMember && !GuestName.IsEmpty())
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[LoginConfig::Validate] AuthMethod is Member but guest name is provided -- it will be ignored"));
	}

	// Guest + SpacesListMenu is not supported (Bug #6) -- fall back to PromptUserForJoinId
	if (AuthMethod == ECavrnusAuthMethod::JoinAsGuest && SpaceJoinMethod == ECavrnusSpaceJoinMethod::SpacesListMenu)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[LoginConfig::Validate] Guest + SpacesListMenu is not supported -- falling back to PromptUserForJoinId"));
		SpaceJoinMethod = ECavrnusSpaceJoinMethod::PromptUserForJoinId;
	}
}

FString FCavrnusLoginConfig::ToDebugString() const
{
	return FString::Printf(TEXT("Auth=%s PIE=%s Server=%s Guest=%s Email=%s SpaceJoin=%s SpaceId=%s"),
		AuthMethod == ECavrnusAuthMethod::JoinAsMember ? TEXT("Member") :
			AuthMethod == ECavrnusAuthMethod::AllowBoth ? TEXT("AllowBoth") : TEXT("Guest"),
		bIsPieUserLogin ? TEXT("Y") : TEXT("N"),
		Server.IsEmpty() ? TEXT("(empty)") : *Server,
		GuestName.IsEmpty() ? TEXT("(empty)") : *GuestName,
		MemberLoginEmail.IsEmpty() ? TEXT("(empty)") : *MemberLoginEmail,
		SpaceJoinMethod == ECavrnusSpaceJoinMethod::EnterJoinId ? TEXT("EnterJoinId") :
			SpaceJoinMethod == ECavrnusSpaceJoinMethod::SpacesListMenu ? TEXT("SpacesListMenu") : TEXT("PromptForJoinId"),
		SpaceJoinId.IsEmpty() ? TEXT("(empty)") : *SpaceJoinId);
}
