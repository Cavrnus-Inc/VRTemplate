// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/Login/LoginFlows/CavrnusLoginBaseFlow.h"

#include "CavrnusConnectorModule.h"
#include "CavrnusConnectorSettings.h"
#include "CavrnusFunctionLibrary.h"
#include "Core/Contexts/CavrnusEditorContext.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Managers/CavrnusEditorAuthenticationManager.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "RelayModel/DataState.h"
#include "RelayModel/RelayCallbackModel.h"
#include "Translation/CavrnusProtoTranslation.h"
#include "UI/CavrnusUI.h"
#include "UI/CavrnusUISystems.h"
#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"
#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageWidget.h"
#include "UI/Systems/RawWidgetHost/CavrnusRawWidgetHost.h"

void UCavrnusLoginBaseFlow::DoLogin(const FCavrnusLoginConfig& InInitialLoginConfig)
{
	LoginConfig = InInitialLoginConfig;

	// Use weak pointer — these callbacks can fire after the login flow UObject is GC'd
	TWeakObjectPtr<UCavrnusLoginBaseFlow> WeakThis(this);
	UCavrnusFunctionLibrary::AwaitAnySpaceBeginLoading([WeakThis](const FString&) { if (WeakThis.IsValid()) WeakThis->ShowLoadingProgressWidget(true); });
	UCavrnusFunctionLibrary::AwaitAnySpaceEndLoading([WeakThis] { if (WeakThis.IsValid()) WeakThis->ShowLoadingProgressWidget(false); });
	UCavrnusFunctionLibrary::AwaitAuthentication([WeakThis](const FCavrnusAuthentication& ) { if (WeakThis.IsValid()) WeakThis->ShowAuthenticationProgressWidget(false); });
	UCavrnusFunctionLibrary::AwaitAnySpaceConnection([WeakThis](const FCavrnusSpaceConnection& Sc)
	{
		if (WeakThis.IsValid())
			WeakThis->HandleConnectedSpace(Sc);
	});
}

void UCavrnusLoginBaseFlow::AwaitValidServer(const TFunction<void()>& OnSuccess, const TFunction<void(const FString&)>&)
{
	// Guard against use-after-free: relay callbacks outlive the UObject, so use weak pointer
	TWeakObjectPtr<UCavrnusLoginBaseFlow> WeakThis(this);
	auto BindServerWidget = [WeakThis, OnSuccess]()
	{
		if (!WeakThis.IsValid()) return;

		UCavrnusFunctionLibrary::AwaitServerSet([WeakThis, OnSuccess](const FString& Server)
		{
			if (!WeakThis.IsValid()) return;

			UCavrnusFunctionLibrary::CheckServerStatus(Server,[WeakThis, OnSuccess, Server](const FCavrnusServerStatus& Status)
			{
				if (!WeakThis.IsValid()) return;

				if (Status.Live)
				{
					const FString DomainName = Status.OrganizationInfo.Domain;
					WeakThis->LoginConfig.Server = Server;
					UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
						->SetPrimaryText("Server Domain Accepted")
						->SetSecondaryText(FString::Printf(TEXT("The server domain '%s' has been successfully validated."), *DomainName))
						->SetType(ECavrnusInfoToastMessageEnum::Success);

					Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->CurrentServer = Server;

					OnSuccess();
				}
				else
				{
					UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
						->SetPrimaryText("Something Went Wrong")
						->SetSecondaryText(Status.FailReason)
						->SetType(ECavrnusInfoToastMessageEnum::Error);
				}
			});
		});

		if (WeakThis.IsValid())
			WeakThis->ShowServerSelectionWidget();
	};

	if (LoginConfig.Server.IsEmpty())
	{
		// If SaveUserAuthToken is enabled, try to recover the server from the saved runtime state
		if (UCavrnusConnectorSettings::Get()->SaveUserAuthToken)
		{
			auto* Sub = UCavrnusSubsystem::Get();
			if (Sub && Sub->EditorContext)
			{
				if (auto* AuthMgr = Sub->EditorContext->Get<UCavrnusEditorAuthenticationManager>())
				{
					const FString SavedServer = AuthMgr->GetRuntimeServer();
					if (!SavedServer.IsEmpty())
					{
						UE_LOG(LogCavrnusConnector, Log, TEXT("[AwaitValidServer] Server is empty but found saved runtime server: %s -- using it"), *SavedServer);
						LoginConfig.Server = SavedServer;
						Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->CurrentServer = SavedServer;
						OnSuccess();
						return;
					}
				}
			}
		}

		UE_LOG(LogCavrnusConnector, Log, TEXT("[AwaitValidServer] Server is empty -- showing server selection widget"));
		BindServerWidget();
	}
	else
	{
		UE_LOG(LogCavrnusConnector, Log, TEXT("[AwaitValidServer] Server pre-populated: %s -- proceeding"), *LoginConfig.Server);
		Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->CurrentServer = LoginConfig.Server;
		OnSuccess();
	}
}

void UCavrnusLoginBaseFlow::TryJoinSpace(const TFunction<void()>& OnSuccess, const TFunction<void(const FString&)>& OnFail)
{
	UCavrnusFunctionLibrary::JoinSpace(
		LoginConfig.SpaceJoinId,
		[OnSuccess](const FCavrnusSpaceConnection& Sc)
		{
			if (OnSuccess)
				OnSuccess();
		},[OnFail, this](const FString& Err)
		{
			if (OnFail)
				OnFail(Err);

			UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
				->SetPrimaryText("Failed to Join Space")
				->SetSecondaryText(Err)
				->SetType(ECavrnusInfoToastMessageEnum::Error);
			
			UE_LOG(LogCavrnusConnector, Warning, TEXT("[UCavrnusDefaultLoginFlow::HandleEditorFlow] Join space failed!"))
		});
}

void UCavrnusLoginBaseFlow::TryGuestAuth(const TFunction<void()>& OnSuccess, const TFunction<void(const FString&)>& OnFail)
{
	ShowAuthenticationProgressWidget(true);
	
	UCavrnusFunctionLibrary::AuthenticateAsGuest
	(
		LoginConfig.Server,
		LoginConfig.GuestName,
		[this, OnSuccess](const FCavrnusAuthentication&)
		{
			if (OnSuccess)
				OnSuccess();
		},OnFail);
}

void UCavrnusLoginBaseFlow::TryApiKeyAuth(const TFunction<void()>& OnSuccess, const TFunction<void(const FString&)>& OnFail)
{
	ShowAuthenticationProgressWidget(true);
	
	UCavrnusFunctionLibrary::AuthenticateWithApiKey(
		LoginConfig.Server,
		LoginConfig.ApiKey,
		LoginConfig.ApiToken,
		[this, OnSuccess](FCavrnusAuthentication)
		{
			if (OnSuccess)
				OnSuccess();
			
		},OnFail);
}

void UCavrnusLoginBaseFlow::PromptMemberLogin(const TFunction<void()>& OnSuccess)
{
	ShowMemberLoginWidget(); 
	UCavrnusFunctionLibrary::AwaitAuthentication([this, OnSuccess](const FCavrnusAuthentication&) 
	{
		if (OnSuccess)
			OnSuccess();
	});
}

void UCavrnusLoginBaseFlow::PromptMemberLoginAndSaveToken(const TFunction<void()>& OnSuccess)
{
	ShowMemberLoginWidget(); 
	UCavrnusFunctionLibrary::AwaitAuthentication([this, OnSuccess](const FCavrnusAuthentication& Auth) 
	{
		UCavrnusSubsystem::Get()->EditorContext->Get<UCavrnusEditorAuthenticationManager>()->SetRuntimeToken(Auth.Token);
		
		if (OnSuccess)
			OnSuccess();
	});
}

void UCavrnusLoginBaseFlow::PromptGuestLogin(const TFunction<void()>& OnSuccess)
{
	ShowGuestLoginWidget(); 
	UCavrnusFunctionLibrary::AwaitAuthentication([this, OnSuccess](const FCavrnusAuthentication&) 
	{
		if (OnSuccess)
			OnSuccess();
	});
}

void UCavrnusLoginBaseFlow::HandleConnectedSpace(const FCavrnusSpaceConnection& SpaceConn)
{
	UE_LOG(LogCavrnusConnector, Log, TEXT("Connected to space!"));

	const auto SpaceName = UCavrnusFunctionLibrary::GetCavrnusSpaceConnectionInfo(SpaceConn)->SpaceInfo.SpaceName;
	UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
		->SetPrimaryText("Joined Space Successfully")
		->SetSecondaryText(FString::Printf(TEXT("You are now connected to the space \"%s\"."), *SpaceName))
		->SetType(ECavrnusInfoToastMessageEnum::Success);

	for (auto Widget : UCavrnusConnectorSettings::Get()->WidgetsToLoad)
		UCavrnusUI::Get()->GenericWidgetDisplayer()->Show(Widget);

	// Same weak guard — space exit can fire after login flow is destroyed
	TWeakObjectPtr<UCavrnusLoginBaseFlow> WeakThis(this);
	UCavrnusFunctionLibrary::AwaitAnySpaceExited([WeakThis]
	{
		if (WeakThis.IsValid())
			UCavrnusUI::Get()->GenericWidgetDisplayer()->CloseAll();
	});
}

void UCavrnusLoginBaseFlow::TryMemberAuthWithRuntimeToken(const TFunction<void()>& OnSuccess, const TFunction<void(const FString&)>& OnFail)
{
	const auto RuntimeToken = UCavrnusSubsystem::Get()->EditorContext->Get<UCavrnusEditorAuthenticationManager>()->GetRuntimeToken();
	if (RuntimeToken.IsEmpty())
	{
		if (OnFail)
			OnFail("Member token is empty! Login with credentials to refresh token.");
		
		return;
	}

	if (UCavrnusConnectorSettings::Get()->SaveUserAuthToken)
	{
		ShowAuthenticationProgressWidget(true); 
		
		int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterAuthenticationCallback(
			[this, OnSuccess](const FCavrnusAuthentication& Auth)
			{
				auto* AuthMgr = UCavrnusSubsystem::Get()->EditorContext->Get<UCavrnusEditorAuthenticationManager>();
				AuthMgr->SetRuntimeToken(Auth.Token);
				AuthMgr->SetRuntimeServer(LoginConfig.Server);
				if (OnSuccess)
					OnSuccess();
			},
			[this, OnFail](const FString& Error)
			{
				ShowAuthenticationProgressWidget(false);
				auto* AuthMgr = UCavrnusSubsystem::Get()->EditorContext->Get<UCavrnusEditorAuthenticationManager>();
				AuthMgr->SetRuntimeToken("");
				AuthMgr->SetRuntimeServer("");
				UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
					->SetPrimaryText("Invalid Member Token")
					->SetSecondaryText(Error)
					->SetType(ECavrnusInfoToastMessageEnum::Error);

				UE_LOG(LogCavrnusConnector, Warning, TEXT("Token Callback! : [%s]"), *Error);

				OnFail(Error);
			});
			
		Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildAuthenticateToken(RequestId, LoginConfig.Server, RuntimeToken));
		UE_LOG(LogCavrnusConnector, Log, TEXT("Token Request!"));
	}
	else
	{
		PromptMemberLogin(OnSuccess);
	}
}

void UCavrnusLoginBaseFlow::TryMemberAuthWithPassword(const TFunction<void()>& OnSuccess, const TFunction<void(const FString&)>& OnFail)
{
	ShowAuthenticationProgressWidget(true); 
	UCavrnusFunctionLibrary::AuthenticateWithPassword
	(
		LoginConfig.Server,
		LoginConfig.MemberLoginEmail,
		LoginConfig.MemberLoginPassword,
		[this, OnSuccess](const FCavrnusAuthentication& Auth)
		{
			auto* AuthMgr = UCavrnusSubsystem::Get()->EditorContext->Get<UCavrnusEditorAuthenticationManager>();
			AuthMgr->SetRuntimeToken(Auth.Token);
			AuthMgr->SetRuntimeServer(LoginConfig.Server);

			if (OnSuccess)
				OnSuccess();
		}, [OnFail](const FString& Error)
		{
			UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
				->SetPrimaryText("Member login failed")
				->SetSecondaryText(Error)
				->SetType(ECavrnusInfoToastMessageEnum::Error);

			if (OnFail)
				OnFail(Error);
		}
	);
}

#pragma region UI Spawning

void UCavrnusLoginBaseFlow::CloseCurrentFlowWidget()
{
	if (CurrentFlowWidget.IsValid())
	{
		UCavrnusUI::Get()->GenericWidgetDisplayer()->Close(CurrentFlowWidget.Get());
		CurrentFlowWidget = nullptr;
	}
}

void UCavrnusLoginBaseFlow::ShowMemberLoginWidget()
{
	if (UCavrnusConnectorSettings::Get()->MemberLoginMenu)
	{
		UE_LOG(LogCavrnusConnector, Log, TEXT("[LoginBaseFlow] ShowMemberLoginWidget -- widget is valid, displaying"));
		CurrentFlowWidget = UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusConnectorSettings::Get()->MemberLoginMenu);
	}
	else
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[LoginBaseFlow] ShowMemberLoginWidget -- MemberLoginMenu is null! Cannot display widget."));
	}
}

void UCavrnusLoginBaseFlow::ShowGuestLoginWidget()
{
	if (UCavrnusConnectorSettings::Get()->GuestJoinMenu)
		CurrentFlowWidget = UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusConnectorSettings::Get()->GuestJoinMenu);
}

void UCavrnusLoginBaseFlow::ShowCombinedLoginWidget()
{
	if (UCavrnusConnectorSettings::Get()->CombinedLoginMenu)
	{
		UE_LOG(LogCavrnusConnector, Log, TEXT("[LoginBaseFlow] ShowCombinedLoginWidget -- widget is valid, displaying"));
		CurrentFlowWidget = UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusConnectorSettings::Get()->CombinedLoginMenu);
	}
	else
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[LoginBaseFlow] ShowCombinedLoginWidget -- CombinedLoginMenu is null! Cannot display widget."));
	}
}

void UCavrnusLoginBaseFlow::ShowJoinIdWidget()
{
	if (UCavrnusConnectorSettings::Get()->JoinIdMenu)
		CurrentFlowWidget = UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusConnectorSettings::Get()->JoinIdMenu);
}

void UCavrnusLoginBaseFlow::ShowServerSelectionWidget()
{
	if (UCavrnusConnectorSettings::Get()->ServerSelectionMenu)
		CurrentFlowWidget = UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusConnectorSettings::Get()->ServerSelectionMenu);
}

void UCavrnusLoginBaseFlow::ShowSpaceListWidget()
{
	if (UCavrnusConnectorSettings::Get()->SpacesListMenu)
		CurrentFlowWidget = UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusConnectorSettings::Get()->SpacesListMenu);
}

void UCavrnusLoginBaseFlow::ShowAuthenticationProgressWidget(const bool bShowWidget)
{
	if (bShowWidget && UCavrnusConnectorSettings::Get()->AuthenticationWidgetMenu)
		AuthLoadingWidget = UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusConnectorSettings::Get()->AuthenticationWidgetMenu);
	else
		UCavrnusUI::Get()->GenericWidgetDisplayer()->Close(AuthLoadingWidget.Get());
}

void UCavrnusLoginBaseFlow::ShowLoadingProgressWidget(bool bShowWidget)
{
	if (bShowWidget && UCavrnusConnectorSettings::Get()->LoadingWidgetMenu)
		LoadingWidget = UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusConnectorSettings::Get()->LoadingWidgetMenu);
	else
		UCavrnusUI::Get()->GenericWidgetDisplayer()->Close(LoadingWidget.Get());
}

#pragma endregion