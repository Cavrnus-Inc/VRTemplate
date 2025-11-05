// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/Login/LoginFlows/CavrnusLoginBaseFlow.h"

#include "CavrnusConnectorModule.h"
#include "CavrnusConnectorSettings.h"
#include "CavrnusFunctionLibrary.h"
#include "CavrnusSubsystem.h"
#include "Managers/CavrnusEditorAuthenticationManager.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "RelayModel/RelayCallbackModel.h"
#include "Translation/CavrnusProtoTranslation.h"
#include "UI/CavrnusUI.h"
#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"
#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageWidget.h"
#include "UI/Systems/RawWidgetHost/CavrnusRawWidgetHost.h"

void UCavrnusLoginBaseFlow::DoLogin(const FCavrnusLoginConfig& InInitialLoginConfig)
{
	LoginConfig = InInitialLoginConfig;

	UCavrnusFunctionLibrary::AwaitAnySpaceBeginLoading([this](const FString&) { ShowLoadingProgressWidget(true); });
	UCavrnusFunctionLibrary::AwaitAnySpaceEndLoading([this] { ShowLoadingProgressWidget(false); });
	UCavrnusFunctionLibrary::AwaitAuthentication([this](const FCavrnusAuthentication& ) { ShowAuthenticationProgressWidget(false); });
	UCavrnusFunctionLibrary::AwaitAnySpaceConnection([this](const FCavrnusSpaceConnection& Sc)
	{
		HandleConnectedSpace(Sc);
	});
}

void UCavrnusLoginBaseFlow::AwaitValidServer(const TFunction<void()>& OnSuccess, const TFunction<void(const FString&)>&)
{
	UCavrnusFunctionLibrary::AwaitServerSet([this, OnSuccess](const FString& Server)
	{
		UCavrnusFunctionLibrary::CheckServerStatus(Server,[this, OnSuccess, Server](const FCavrnusServerStatus& Status)
		{
			if (Status.Live)
			{
				const FString DomainName = Status.OrganizationInfo.Domain;
				LoginConfig.Server = Server;
				UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
					->SetPrimaryText("Server Domain Accepted")
					->SetSecondaryText(FString::Printf(TEXT("The server domain '%s' has been successfully validated."), *DomainName))
					->SetType(ECavrnusInfoToastMessageEnum::Success);

				Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->CurrentServer = Server;
				
				OnSuccess();
				
			} else
			{
				UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
					->SetPrimaryText("Something Went Wrong")
					->SetSecondaryText(Status.FailReason)
					->SetType(ECavrnusInfoToastMessageEnum::Error);
			}
		});
	});
	
	if (LoginConfig.Server.IsEmpty())
		ShowServerSelectionWidget();
	else
	{
		UCavrnusFunctionLibrary::CheckServerStatus(LoginConfig.Server,[this, OnSuccess](const FCavrnusServerStatus& Status)
		{
			if (Status.Live)
			{
				OnSuccess();
				Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->CurrentServer = LoginConfig.Server;
			}
			else
			{
				ShowServerSelectionWidget();
				UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
					->SetPrimaryText("Something Went Wrong")
					->SetSecondaryText(Status.FailReason)
					->SetType(ECavrnusInfoToastMessageEnum::Error);
			}
		});
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
		UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>()->SetRuntimeToken(Auth.Token);
		
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

	for (auto Widget : UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->WidgetsToLoad)
		UCavrnusUI::Get()->GenericWidgetDisplayer()->Show(Widget);

	UCavrnusFunctionLibrary::AwaitAnySpaceExited([this]
	{
		UCavrnusUI::Get()->GenericWidgetDisplayer()->CloseAll();
	});
}

void UCavrnusLoginBaseFlow::TryMemberAuthWithRuntimeToken(const TFunction<void()>& OnSuccess, const TFunction<void(const FString&)>& OnFail)
{
	const auto RuntimeToken = UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>()->GetRuntimeToken();
	if (RuntimeToken.IsEmpty())
	{
		if (OnFail)
			OnFail("Member token is empty! Login with credentials to refresh token.");
		
		return;
	}

	if (UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->SaveUserAuthToken)
	{
		ShowAuthenticationProgressWidget(true); 
		
		int RequestId = Cavrnus::CavrnusRelayModel::GetDataModel()->GetCallbackModel()->RegisterAuthenticationCallback(
			[this, OnSuccess](const FCavrnusAuthentication& Auth)
			{
				UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>()->SetRuntimeToken(Auth.Token);
				if (OnSuccess)
					OnSuccess();
			},
			[this, OnFail](const FString& Error)
			{
				ShowAuthenticationProgressWidget(false);
				UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>()->SetRuntimeToken("");
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
			UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>()->SetRuntimeToken(Auth.Token);

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

void UCavrnusLoginBaseFlow::ShowMemberLoginWidget()
{
	if (UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->MemberLoginMenu)
		UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->MemberLoginMenu);
}

void UCavrnusLoginBaseFlow::ShowGuestLoginWidget()
{
	if (UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->GuestJoinMenu)
		UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->GuestJoinMenu);
}

void UCavrnusLoginBaseFlow::ShowJoinIdWidget()
{
	if (UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->GuestJoinMenu)
		UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->JoinIdMenu);
}

void UCavrnusLoginBaseFlow::ShowServerSelectionWidget()
{
	if (UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->ServerSelectionMenu)
		UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->ServerSelectionMenu);
}

void UCavrnusLoginBaseFlow::ShowSpaceListWidget()
{
	if (UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->SpacesListMenu)
		UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->SpacesListMenu);
}

void UCavrnusLoginBaseFlow::ShowAuthenticationProgressWidget(const bool bShowWidget)
{
	if (bShowWidget && UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->AuthenticationWidgetMenu)
		AuthLoadingWidget = UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->AuthenticationWidgetMenu);
	else
		UCavrnusUI::Get()->GenericWidgetDisplayer()->Close(AuthLoadingWidget.Get());
}

void UCavrnusLoginBaseFlow::ShowLoadingProgressWidget(bool bShowWidget)
{
	if (bShowWidget && UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->LoadingWidgetMenu)
		LoadingWidget = UCavrnusUI::Get()->GenericWidgetDisplayer()->ShowWithScrim(UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->LoadingWidgetMenu);
	else
		UCavrnusUI::Get()->GenericWidgetDisplayer()->Close(LoadingWidget.Get());
}

#pragma endregion