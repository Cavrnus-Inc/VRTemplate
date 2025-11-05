// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/Login/LoginFlows/CavrnusRuntimeLoginFlow.h"
#include "CavrnusFunctionLibrary.h"
#include "CavrnusSubsystem.h"

void UCavrnusRuntimeLoginFlow::DoLogin(const FCavrnusLoginConfig& InLoginConfig)
{
	Super::DoLogin(InLoginConfig);
	
	AwaitValidServer([this, InLoginConfig]
	   {
	       if (InLoginConfig.AuthMethod == ECavrnusAuthMethod::JoinAsGuest)
		       HandleGuestFlow();
	       else if (InLoginConfig.AuthMethod == ECavrnusAuthMethod::JoinAsMember)
		       HandleMemberFlow();
	   });
}	

void UCavrnusRuntimeLoginFlow::HandleMemberFlow()
{
	UCavrnusFunctionLibrary::AwaitAuthentication([this](const FCavrnusAuthentication&)
	{
		if (!LoginConfig.SpaceJoinId.IsEmpty())
		{
			TryJoinSpace([]
			{
					
			}, [this](const FString&)
			{
				ShowSpaceListWidget();
			});
		} else
			ShowSpaceListWidget();
	});
	
	if (!LoginConfig.MemberLoginEmail.IsEmpty() && !LoginConfig.MemberLoginPassword.IsEmpty())
	{
		TryMemberAuthWithPassword([this]
		{
			
		},
		[this](const FString&)
		{
			PromptMemberLogin();
		});
	} else
		PromptMemberLogin();
}

void UCavrnusRuntimeLoginFlow::HandleGuestFlow()
{
	UCavrnusFunctionLibrary::AwaitAuthentication([this](const FCavrnusAuthentication&)
	{
		if (!LoginConfig.SpaceJoinId.IsEmpty())
		{
			TryJoinSpace([]
			{
					
			}, [this](const FString&)
			{
				ShowJoinIdWidget();
			});
		} else
			ShowJoinIdWidget();
	});

	if (LoginConfig.GuestName.IsEmpty())
		ShowGuestLoginWidget();
	else
		TryGuestAuth();
}
