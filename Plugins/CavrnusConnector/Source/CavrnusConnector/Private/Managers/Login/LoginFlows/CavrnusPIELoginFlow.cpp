// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/Login/LoginFlows/CavrnusPIELoginFlow.h"

void UCavrnusPIELoginFlow::DoLogin(const FCavrnusLoginConfig& InInitialLoginConfig)
{
	Super::DoLogin(InInitialLoginConfig);

	AwaitValidServer([this]
	{
		HandlePieLogin();
	});
}

void UCavrnusPIELoginFlow::HandlePieLogin()
{
	if (LoginConfig.AuthMethod == ECavrnusAuthMethod::JoinAsMember)
	{
		PromptMemberLogin([this]
		{
			ShowSpaceListWidget();
		});
	}
	else if (LoginConfig.AuthMethod == ECavrnusAuthMethod::JoinAsGuest)
	{
		PromptGuestLogin([this]
		{
			ShowJoinIdWidget();
		});
	}
	else
	{
		TryApiKeyAuth([this]
		{
			ShowSpaceListWidget();
		}, [this](const FString& )
		{
			PromptMemberLogin([this]
			{
				ShowSpaceListWidget();
			});
		});
	}
}
