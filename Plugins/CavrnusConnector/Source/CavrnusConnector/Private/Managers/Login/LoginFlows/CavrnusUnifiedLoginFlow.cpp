// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/Login/LoginFlows/CavrnusUnifiedLoginFlow.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusConnectorSettings.h"
#include "CavrnusFunctionLibrary.h"
#include "Core/Contexts/CavrnusEditorContext.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Managers/CavrnusEditorAuthenticationManager.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "RelayModel/DataState.h"
#include "UI/CavrnusUI.h"
#include "UI/CavrnusUISystems.h"
#include "UI/Systems/RawWidgetHost/CavrnusRawWidgetHost.h"

void UCavrnusUnifiedLoginFlow::DoLogin(const FCavrnusLoginConfig& InLoginConfig)
{
	Super::DoLogin(InLoginConfig);

	LogFlowStep(TEXT("DoLogin"), TEXT("Starting unified login flow"));

	AwaitValidServer([this]
	{
		HandleAuthDispatch();
	});
}

void UCavrnusUnifiedLoginFlow::HandleAuthDispatch()
{
	// Transition: Server phase complete → Auth phase
	CloseCurrentFlowWidget();

	if (LoginConfig.bIsPieUserLogin)
	{
		LogFlowStep(TEXT("AuthDispatch"), TEXT("PIE login detected"));
		HandlePieAuth();
	}
	else if (LoginConfig.AuthMethod == ECavrnusAuthMethod::JoinAsMember)
	{
		LogFlowStep(TEXT("AuthDispatch"), TEXT("Member login"));
		HandleMemberAuth();
	}
	else if (LoginConfig.AuthMethod == ECavrnusAuthMethod::JoinAsGuest)
	{
		LogFlowStep(TEXT("AuthDispatch"), TEXT("Guest login"));
		HandleGuestAuth();
	}
	else if (LoginConfig.AuthMethod == ECavrnusAuthMethod::AllowBoth)
	{
		LogFlowStep(TEXT("AuthDispatch"), TEXT("AllowBoth login"));
		HandleAllowBothAuth();
	}
}

void UCavrnusUnifiedLoginFlow::HandlePieAuth()
{
	TryApiKeyAuth([this]
	{
		LogFlowStep(TEXT("PieAuth"), TEXT("API key auth succeeded"));
		ShowSpaceListWidget();
	}, [this](const FString& Error)
	{
		LogFlowStep(TEXT("PieAuth"), FString::Printf(TEXT("API key auth failed (%s) -- falling back to member prompt"), *Error));
		PromptMemberLogin([this]
		{
			ShowSpaceListWidget();
		});
	});
}

void UCavrnusUnifiedLoginFlow::HandleMemberAuth()
{
	// Register space join handler before auth so it fires when auth completes
	UCavrnusFunctionLibrary::AwaitAuthentication([this](const FCavrnusAuthentication&)
	{
		HandleSpaceJoinForMember();
	});

	// Clear saved token if SaveUserAuthToken is disabled
	if (!UCavrnusConnectorSettings::Get()->SaveUserAuthToken)
	{
		auto* Sub = UCavrnusSubsystem::Get();
		if (Sub && Sub->EditorContext)
		{
			if (auto* AuthMgr = Sub->EditorContext->Get<UCavrnusEditorAuthenticationManager>())
				AuthMgr->SetRuntimeToken("");
		}
	}

	if (LoginConfig.MemberLoginMethod == ECavrnusMemberLoginMethod::EnterMemberCredentials)
	{
		if (UCavrnusConnectorSettings::Get()->SaveUserAuthToken)
		{
			LogFlowStep(TEXT("MemberAuth"), TEXT("EnterMemberCredentials + SaveUserAuthToken -- trying runtime token"));
			TryMemberAuthWithRuntimeToken([] {  }, [this](const FString& Error)
			{
				LogFlowStep(TEXT("MemberAuth"), FString::Printf(TEXT("Runtime token failed (%s) -- prompting member login and saving token"), *Error));
				PromptMemberLoginAndSaveToken();
			});
		}
		else if (!LoginConfig.MemberLoginEmail.IsEmpty() && !LoginConfig.MemberLoginPassword.IsEmpty())
		{
			LogFlowStep(TEXT("MemberAuth"), TEXT("EnterMemberCredentials + has email+pw -- trying password auth"));
			TryMemberAuthWithPassword([] {  }, [this](const FString& Error)
			{
				LogFlowStep(TEXT("MemberAuth"), FString::Printf(TEXT("Password auth failed (%s) -- prompting member login"), *Error));
				PromptMemberLogin();
			});
		}
		else
		{
			LogFlowStep(TEXT("MemberAuth"), TEXT("EnterMemberCredentials but no credentials -- prompting member login"));
			PromptMemberLogin();
		}
	}
	else if (LoginConfig.MemberLoginMethod == ECavrnusMemberLoginMethod::PromptMemberToLogin)
	{
		LogFlowStep(TEXT("MemberAuth"), TEXT("PromptMemberToLogin -- showing member login widget"));
		PromptMemberLogin();
	}
}

void UCavrnusUnifiedLoginFlow::HandleGuestAuth()
{
	// Register space join handler before auth so it fires when auth completes
	UCavrnusFunctionLibrary::AwaitAuthentication([this](const FCavrnusAuthentication&)
	{
		HandleSpaceJoinForGuest();
	});

	if (LoginConfig.GuestLoginMethod == ECavrnusGuestLoginMethod::EnterNameBelow)
	{
		if (!LoginConfig.GuestName.IsEmpty())
		{
			LogFlowStep(TEXT("GuestAuth"), TEXT("EnterNameBelow + has name -- trying guest auth"));
			TryGuestAuth([] {  }, [this](const FString& Error)
			{
				LogFlowStep(TEXT("GuestAuth"), FString::Printf(TEXT("Guest auth failed (%s) -- prompting guest login"), *Error));
				PromptGuestLogin();
			});
		}
		else
		{
			LogFlowStep(TEXT("GuestAuth"), TEXT("EnterNameBelow but no name -- prompting guest login"));
			PromptGuestLogin();
		}
	}
	else if (LoginConfig.GuestLoginMethod == ECavrnusGuestLoginMethod::PromptToEnterName)
	{
		LogFlowStep(TEXT("GuestAuth"), TEXT("PromptToEnterName -- showing guest login widget"));
		PromptGuestLogin();
	}
}

void UCavrnusUnifiedLoginFlow::HandleAllowBothAuth()
{
	bool bSaveToken = UCavrnusConnectorSettings::Get()->SaveUserAuthToken;

	// Try saved token first (highest priority -- returns instantly for returning users)
	if (bSaveToken)
	{
		LogFlowStep(TEXT("AllowBothAuth"), TEXT("SaveUserAuthToken enabled -- trying runtime token"));
		TryMemberAuthWithRuntimeToken([this]
		{
			LogFlowStep(TEXT("AllowBothAuth"), TEXT("Runtime token succeeded -- routing as member"));
			HandleSpaceJoinForMember();
		}, [this](const FString& Error)
		{
			LogFlowStep(TEXT("AllowBothAuth"), FString::Printf(TEXT("Runtime token failed (%s) -- showing combined widget"), *Error));
			HandleAllowBothShowCombinedWidget();
		});
		return;
	}

	HandleAllowBothShowCombinedWidget();
}

void UCavrnusUnifiedLoginFlow::HandleAllowBothShowCombinedWidget()
{
	// AllowBoth always shows the combined widget so the user can choose member or guest
	LogFlowStep(TEXT("AllowBothAuth"), TEXT("Showing combined login widget"));
	ShowCombinedLoginWidget();

	UCavrnusFunctionLibrary::AwaitAuthentication([this](const FCavrnusAuthentication&)
	{
		bool bIsGuest = Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->bAuthenticatedAsGuest;
		if (bIsGuest)
		{
			LogFlowStep(TEXT("AllowBothAuth"), TEXT("Authenticated as guest -- routing to guest space join"));
			HandleSpaceJoinForGuest();
		}
		else
		{
			LogFlowStep(TEXT("AllowBothAuth"), TEXT("Authenticated as member -- routing to member space join"));
			HandleSpaceJoinForMember();
		}
	});
}

void UCavrnusUnifiedLoginFlow::HandleSpaceJoinForMember()
{
	// Transition: Auth phase complete → Space join phase
	CloseCurrentFlowWidget();

	if (LoginConfig.SpaceJoinMethod == ECavrnusSpaceJoinMethod::EnterJoinId && !LoginConfig.SpaceJoinId.IsEmpty())
	{
		LogFlowStep(TEXT("SpaceJoin"), TEXT("Member + EnterJoinId + has ID -- trying to join space"));
		TryJoinSpace([] {  }, [this](const FString& Error)
		{
			LogFlowStep(TEXT("SpaceJoin"), FString::Printf(TEXT("Join space failed (%s) -- showing space list"), *Error));
			ShowSpaceListWidget();
		});
	}
	else if (LoginConfig.SpaceJoinMethod == ECavrnusSpaceJoinMethod::SpacesListMenu)
	{
		LogFlowStep(TEXT("SpaceJoin"), TEXT("Member + SpacesListMenu -- showing space list widget"));
		ShowSpaceListWidget();
	}
	else if (LoginConfig.SpaceJoinMethod == ECavrnusSpaceJoinMethod::PromptUserForJoinId)
	{
		LogFlowStep(TEXT("SpaceJoin"), TEXT("Member + PromptUserForJoinId -- showing join ID widget"));
		ShowJoinIdWidget();
	}
	else
	{
		// EnterJoinId but no ID provided -- fall back to space list
		LogFlowStep(TEXT("SpaceJoin"), TEXT("Member + EnterJoinId but no ID -- falling back to space list"));
		ShowSpaceListWidget();
	}
}

void UCavrnusUnifiedLoginFlow::HandleSpaceJoinForGuest()
{
	// Transition: Auth phase complete → Space join phase
	CloseCurrentFlowWidget();

	if (LoginConfig.SpaceJoinMethod == ECavrnusSpaceJoinMethod::EnterJoinId && !LoginConfig.SpaceJoinId.IsEmpty())
	{
		LogFlowStep(TEXT("SpaceJoin"), TEXT("Guest + EnterJoinId + has ID -- trying to join space"));
		TryJoinSpace([] {  }, [this](const FString& Error)
		{
			LogFlowStep(TEXT("SpaceJoin"), FString::Printf(TEXT("Join space failed (%s) -- showing join ID widget"), *Error));
			ShowJoinIdWidget();
		});
	}
	else
	{
		// Guest always falls back to ShowJoinIdWidget (SpacesListMenu was already corrected to PromptUserForJoinId by Validate())
		LogFlowStep(TEXT("SpaceJoin"), TEXT("Guest -- showing join ID widget"));
		ShowJoinIdWidget();
	}
}

void UCavrnusUnifiedLoginFlow::LogFlowStep(const FString& StepName, const FString& Reason) const
{
	UE_LOG(LogCavrnusConnector, Log, TEXT("[LoginFlow] %s -- %s | Config: %s"), *StepName, *Reason, *LoginConfig.ToDebugString());
}
