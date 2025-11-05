// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/Login/CavrnusLoginManager.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "CavrnusSubsystem.h"
#include "Managers/CavrnusEditorAuthenticationManager.h"
#include "Managers/Login/CavrnusLoginConfig.h"
#include "Managers/Login/LoginFlows/CavrnusPIELoginFlow.h"
#include "Managers/Login/LoginFlows/CavrnusRuntimeLoginFlow.h"
#include "Managers/Login/LoginFlows/CavrnusPluginSettingsLoginFlow.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "UI/CavrnusUI.h"

void UCavrnusLoginManager::Initialize()
{
	Super::Initialize();
}

void UCavrnusLoginManager::DoLogin(const FString& InLoginFlowType, FCavrnusLoginConfig InConfig)
{
	if (HasAttemptedToLoginAlready)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("Multiple login attempts detected before authentication completed."
											" Check Cavrnus Connector Plugin settings and set [ConnectOnStart] to false"))

		return;
	}
	
	const auto LoginFlowType = InLoginFlowType.TrimStartAndEnd().ToLower();

	if (LoginFlowType == TEXT("runtime"))
		LoginFlow = NewObject<UCavrnusRuntimeLoginFlow>();
	else if (LoginFlowType == TEXT("settings"))
		LoginFlow = NewObject<UCavrnusPluginSettingsLoginFlow>();
	else if (LoginFlowType == TEXT("pie"))
		LoginFlow = NewObject<UCavrnusPIELoginFlow>();
	else
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("Unrecognized login flow: [%s] -- defaulting to DefaultLogin!"), *InLoginFlowType);
		LoginFlow = NewObject<UCavrnusRuntimeLoginFlow>();
	}
	
	ApplyCommandLineArgs(&InConfig);
	UCavrnusFunctionLibrary::SetupCavrnusEventHooks();

	ResolveServer(InConfig.Server);
	UCavrnusUI::Get()->AwaitUIInit([this, InConfig]
	{
		LoginFlow->DoLogin(InConfig);
	});

	HasAttemptedToLoginAlready = true;
}

void UCavrnusLoginManager::DoPluginSettingsLogin()
{
	auto* Settings = UCavrnusSubsystem::Get()->GetSettings();

	if (Settings == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusLoginManager::DoPluginSettingsLogin] Unable to login! CavrnusConnectorSettings is null!"));
		return;
	}

	FCavrnusLoginConfig LoginConfig;
	LoginConfig.bIsPieUserLogin = false;
	LoginConfig.Server = Settings->ServerDomain;
	
	switch (Settings->AuthMethod)
	{
	case ECavrnusAuthMethod::JoinAsMember:
		LoginConfig.AuthMethod = ECavrnusAuthMethod::JoinAsMember;
		LoginConfig.MemberLoginEmail = Settings->MemberLoginEmail;
		LoginConfig.MemberLoginPassword = Settings->MemberLoginPassword;
		LoginConfig.MemberLoginMethod = Settings->MemberLoginMethod;
		LoginConfig.SpaceJoinMethod = Settings->SpaceJoinMethod;
		LoginConfig.SpaceJoinId = Settings->JoinId;
		break;
	case ECavrnusAuthMethod::JoinAsGuest:
		LoginConfig.AuthMethod = ECavrnusAuthMethod::JoinAsGuest;
		LoginConfig.GuestName = Settings->GuestName;
		LoginConfig.GuestLoginMethod = Settings->GuestLoginMethod;
		LoginConfig.SpaceJoinMethod = Settings->SpaceJoinMethod;
		LoginConfig.SpaceJoinId = Settings->JoinId;
		break;
	}

	DoLogin("settings", LoginConfig);
}

void UCavrnusLoginManager::DoPieLogin()
{
	auto* Settings = UCavrnusSubsystem::Get()->GetSettings();

	if (Settings == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusLoginManager::DoPieLogin] Unable to login! CavrnusConnectorSettings is null!"));
		return;
	}

	auto* AuthManager = UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>();
	if (AuthManager == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusLoginManager::DoPieLogin] Unable to login! AuthManager is null!"));
		return;
	}
	
	FCavrnusLoginConfig LoginConfig;
	LoginConfig.Server = AuthManager->GetPIEAuthedServer();

	FCavrnusEditorLoginInfo EditorLogin;
	
	switch (AuthManager->GetCurrentAuthMethod())
	{
	case ECavrnusAuthMethodForPIE::JoinAsPIE:
		LoginConfig.bIsPieUserLogin = true;
		if (UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>()->TryGetEditorLoginInfo(EditorLogin))
		{
			LoginConfig.ApiKey = EditorLogin.AccessKey;
			LoginConfig.ApiToken = EditorLogin.AccessToken;
		}
		break;
	case ECavrnusAuthMethodForPIE::JoinAsMember:
		LoginConfig.AuthMethod = ECavrnusAuthMethod::JoinAsMember;
		break;
	case ECavrnusAuthMethodForPIE::JoinAsGuest:
		LoginConfig.AuthMethod = ECavrnusAuthMethod::JoinAsGuest;
		break;
	}
	
	DoLogin("pie", LoginConfig);
}

void UCavrnusLoginManager::OnEndPIE(bool bIsSimulating)
{
	Super::OnEndPIE(bIsSimulating);
	HasAttemptedToLoginAlready = false;
}

void UCavrnusLoginManager::OnAppShutdown()
{
	Super::OnAppShutdown();
	HasAttemptedToLoginAlready = false;
}

bool UCavrnusLoginManager::ApplyCommandLineArgs(FCavrnusLoginConfig* InConfig)
{
	auto Overridden = false;
	
	FString Server;
	if (FParse::Value(FCommandLine::Get(), TEXT("Server="), Server))
	{
		InConfig->Server = Server;
		Overridden = true;
	}

	FString GuestName;
	if (FParse::Value(FCommandLine::Get(), TEXT("GuestName="), GuestName))
	{
		InConfig->AuthMethod = ECavrnusAuthMethod::JoinAsGuest;
		InConfig->GuestName = GuestName;
		Overridden = true;
	}

	FString UserEmail;
	if (FParse::Value(FCommandLine::Get(), TEXT("UserEmail="), UserEmail))
	{
		InConfig->AuthMethod = ECavrnusAuthMethod::JoinAsMember;
		InConfig->MemberLoginEmail = UserEmail;
		Overridden = true;
	}
	FString UserPassword;
	if (FParse::Value(FCommandLine::Get(), TEXT("UserPassword="), UserPassword))
	{
		InConfig->AuthMethod = ECavrnusAuthMethod::JoinAsMember;
		InConfig->MemberLoginPassword = UserPassword;
		Overridden = true;
	}

	FString SpaceJoinId;
	if (FParse::Value(FCommandLine::Get(), TEXT("SpaceJoinId="), SpaceJoinId))
	{
		InConfig->SpaceJoinId = SpaceJoinId;
		Overridden = true;
	}

	return Overridden;
}

void UCavrnusLoginManager::ResolveServer(FString& Server)
{
	Server.TrimStartAndEndInline();
	Server.ToLowerInline();

	// Leave empty so the login flow will show server prompt
	if (Server.IsEmpty())
		return;

	if (Server.StartsWith(TEXT("http")))
		return;

	if (!Server.EndsWith(TEXT(".cavrn.us")))
		Server += TEXT(".cavrn.us");
}
