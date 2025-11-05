// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/Login/LoginFlows/CavrnusPluginSettingsLoginFlow.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "CavrnusSubsystem.h"
#include "Managers/CavrnusEditorAuthenticationManager.h"

void UCavrnusPluginSettingsLoginFlow::DoLogin(const FCavrnusLoginConfig& InLoginConfig)
{
	Super::DoLogin(InLoginConfig);
	AwaitValidServer([this, InLoginConfig]
	{
		if (InLoginConfig.AuthMethod == ECavrnusAuthMethod::JoinAsGuest)
			HandleGuestFlow();
		if (InLoginConfig.AuthMethod == ECavrnusAuthMethod::JoinAsMember)
			HandleMemberFlow();
	});
}

void UCavrnusPluginSettingsLoginFlow::HandleMemberFlow()
{
	UCavrnusFunctionLibrary::AwaitAuthentication([this](const FCavrnusAuthentication&)
	{
		if (LoginConfig.SpaceJoinMethod == ECavrnusSpaceJoinMethod::EnterJoinId && !LoginConfig.SpaceJoinId.IsEmpty())
		{
			TryJoinSpace([]
			{
					
			}, [this](const FString&)
			{
				ShowSpaceListWidget();
			});
		}
		else if (LoginConfig.SpaceJoinMethod == ECavrnusSpaceJoinMethod::SpacesListMenu)
			ShowSpaceListWidget();
		else if (LoginConfig.SpaceJoinMethod == ECavrnusSpaceJoinMethod::PromptUserForJoinId)
			ShowJoinIdWidget();
	});

	if (!UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->SaveUserAuthToken)
		UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>()->SetRuntimeToken("");

	if (LoginConfig.MemberLoginMethod == ECavrnusMemberLoginMethod::EnterMemberCredentials)
	{
		if (UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->SaveUserAuthToken)
			TryMemberAuthWithRuntimeToken([] {  }, [this](const FString& ) { PromptMemberLoginAndSaveToken(); });
		else if (!LoginConfig.MemberLoginEmail.IsEmpty() && !LoginConfig.MemberLoginPassword.IsEmpty())
			TryMemberAuthWithPassword([] {  }, [this](const FString& ) { PromptMemberLogin(); });
		else
			PromptMemberLogin(); 
	}
	else if (LoginConfig.MemberLoginMethod == ECavrnusMemberLoginMethod::PromptMemberToLogin)
		PromptMemberLogin();
}

void UCavrnusPluginSettingsLoginFlow::HandleGuestFlow()
{
	UCavrnusFunctionLibrary::AwaitAuthentication([this](const FCavrnusAuthentication&)
	{
		if (LoginConfig.SpaceJoinMethod == ECavrnusSpaceJoinMethod::PromptUserForJoinId)
			ShowJoinIdWidget();
		else if (LoginConfig.SpaceJoinMethod == ECavrnusSpaceJoinMethod::EnterJoinId && !LoginConfig.SpaceJoinId.IsEmpty())
		{
			TryJoinSpace([]
			{
					
			}, [this](const FString&)
			{
				ShowJoinIdWidget();
			});
		}
		else
			ShowJoinIdWidget();
	});

	if (LoginConfig.GuestLoginMethod == ECavrnusGuestLoginMethod::PromptToEnterName)
		PromptGuestLogin();
	else if (LoginConfig.GuestLoginMethod == ECavrnusGuestLoginMethod::EnterNameBelow)
	{
		if (LoginConfig.GuestName.IsEmpty())
			PromptGuestLogin();
		else
		{
			TryGuestAuth([] {  }, [this](const FString&)
			{
				PromptGuestLogin();
			});
		}
	}
}
