// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/Login/CavrnusLoginManager.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "Core/Contexts/CavrnusEditorContext.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Managers/CavrnusEditorAuthenticationManager.h"
#include "Managers/Login/CavrnusLoginConfig.h"
#include "Managers/Login/LoginFlows/CavrnusUnifiedLoginFlow.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "UI/CavrnusUI.h"
#include "UI/CavrnusUISystems.h"
#include "Containers/Ticker.h"

void UCavrnusLoginManager::Dispose()
{
	if (ViewportReadyTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ViewportReadyTickerHandle);
		ViewportReadyTickerHandle.Reset();
	}
	Super::Dispose();
}

void UCavrnusLoginManager::Initialize()
{
	Super::Initialize();
}

void UCavrnusLoginManager::DoLogin(FCavrnusLoginConfig InConfig)
{
	if (bLoginInitiated)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[LoginManager] Login already initiated -- blocking duplicate call."
											" Check Cavrnus Connector Plugin settings and set [ConnectOnStart] to false if using Blueprint login."))
		return;
	}

	LoginFlow = NewObject<UCavrnusUnifiedLoginFlow>();

	ApplyCommandLineArgs(&InConfig);
	InConfig.DeriveEnumsFromData();
	InConfig.Validate();

	UCavrnusFunctionLibrary::SetupCavrnusEventHooks();

	ResolveServer(InConfig.Server);

	UE_LOG(LogCavrnusConnector, Log, TEXT("[LoginManager] DoLogin | Config: %s"), *InConfig.ToDebugString());

	BindUIIsReadyWhenViewportReady(InConfig);
}

void UCavrnusLoginManager::BindUIIsReadyWhenViewportReady(FCavrnusLoginConfig InConfig)
{
	TWeakObjectPtr<UCavrnusLoginManager> WeakThis(this);
	FCavrnusLoginConfig ConfigCopy = InConfig;

	auto TryBind = [WeakThis, ConfigCopy]() -> bool
	{
		if (!WeakThis.IsValid())
			return false;
		if (UCavrnusUISystems* UI = UCavrnusUI::Get())
		{
			UI->UIIsReadySetting->Bind(WeakThis.Get(), [WeakThis, ConfigCopy](const bool& IsInit)
			{
				if (IsInit && WeakThis.IsValid() && WeakThis->LoginFlow)
					WeakThis->LoginFlow->DoLogin(ConfigCopy);
			});
			WeakThis->bLoginInitiated = true;
			return false;
		}
		return true;
	};

	if (!TryBind())
		return;

	ViewportReadyTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([WeakThis, TryBind](float) -> bool
		{
			bool keepTicking = TryBind();
			if (!keepTicking && WeakThis.IsValid() && WeakThis->ViewportReadyTickerHandle.IsValid())
			{
				FTSTicker::GetCoreTicker().RemoveTicker(WeakThis->ViewportReadyTickerHandle);
				WeakThis->ViewportReadyTickerHandle.Reset();
			}
			return keepTicking;
		}), 0.0f);
}

void UCavrnusLoginManager::DoPluginSettingsLogin()
{
	FCavrnusLoginConfig LoginConfig = FCavrnusLoginConfig::FromPluginSettings();
	if (LoginConfig.Server.IsEmpty() && !UCavrnusConnectorSettings::Get())
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusLoginManager::DoPluginSettingsLogin] Unable to login! CavrnusConnectorSettings is null!"));
		return;
	}

	DoLogin(LoginConfig);
}

void UCavrnusLoginManager::DoPieLogin()
{
	auto* Sub = UCavrnusSubsystem::Get();
	if (!Sub || !Sub->EditorContext)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusLoginManager::DoPieLogin] Unable to login! Subsystem or EditorContext is null!"));
		return;
	}
	auto* AuthManager = Sub->EditorContext->Get<UCavrnusEditorAuthenticationManager>();
	if (AuthManager == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusLoginManager::DoPieLogin] Unable to login! AuthManager is null!"));
		return;
	}

	FCavrnusEditorLoginInfo EditorLogin;
	FString ApiKey;
	FString ApiToken;

	if (AuthManager->GetCurrentAuthMethod() == ECavrnusAuthMethodForPIE::JoinAsPIE)
	{
		if (AuthManager->TryGetEditorLoginInfo(EditorLogin))
		{
			ApiKey = EditorLogin.AccessKey;
			ApiToken = EditorLogin.AccessToken;
		}
	}

	FCavrnusLoginConfig LoginConfig = FCavrnusLoginConfig::ForPIE(
		AuthManager->GetPIEAuthedServer(),
		AuthManager->GetCurrentAuthMethod(),
		ApiKey,
		ApiToken);

	DoLogin(LoginConfig);
}

void UCavrnusLoginManager::ResetLoginState()
{
	bLoginInitiated = false;
	LoginFlow = nullptr;
}

void UCavrnusLoginManager::OnEndPIE(bool bIsSimulating)
{
	Super::OnEndPIE(bIsSimulating);
	bLoginInitiated = false;
}

void UCavrnusLoginManager::OnAppShutdown()
{
	Super::OnAppShutdown();
	bLoginInitiated = false;
}

bool UCavrnusLoginManager::ApplyCommandLineArgs(FCavrnusLoginConfig* InConfig)
{
	auto Overridden = false;
	bool HasGuestArg = false;
	bool HasMemberArg = false;

	FString Server;
	if (FParse::Value(FCommandLine::Get(), TEXT("Server="), Server))
	{
		InConfig->Server = Server;
		Overridden = true;
	}

	FString GuestName;
	if (FParse::Value(FCommandLine::Get(), TEXT("GuestName="), GuestName))
	{
		HasGuestArg = true;
		InConfig->GuestName = GuestName;
		InConfig->GuestLoginMethod = ECavrnusGuestLoginMethod::EnterNameBelow;
		Overridden = true;
	}

	FString UserEmail;
	if (FParse::Value(FCommandLine::Get(), TEXT("UserEmail="), UserEmail))
	{
		HasMemberArg = true;
		InConfig->MemberLoginEmail = UserEmail;
		InConfig->MemberLoginMethod = ECavrnusMemberLoginMethod::EnterMemberCredentials;
		Overridden = true;
	}
	FString UserPassword;
	if (FParse::Value(FCommandLine::Get(), TEXT("UserPassword="), UserPassword))
	{
		HasMemberArg = true;
		InConfig->MemberLoginPassword = UserPassword;
		InConfig->MemberLoginMethod = ECavrnusMemberLoginMethod::EnterMemberCredentials;
		Overridden = true;
	}

	// Conflict detection: if both guest and member args are present, member wins
	if (HasGuestArg && HasMemberArg)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[ApplyCommandLineArgs] Both -GuestName= and -UserEmail= provided -- member login takes priority"));
		InConfig->AuthMethod = ECavrnusAuthMethod::JoinAsMember;
	}
	else if (HasMemberArg)
	{
		InConfig->AuthMethod = ECavrnusAuthMethod::JoinAsMember;
	}
	else if (HasGuestArg)
	{
		InConfig->AuthMethod = ECavrnusAuthMethod::JoinAsGuest;
	}

	FString SpaceJoinId;
	if (FParse::Value(FCommandLine::Get(), TEXT("SpaceJoinId="), SpaceJoinId))
	{
		InConfig->SpaceJoinId = SpaceJoinId;
		InConfig->SpaceJoinMethod = ECavrnusSpaceJoinMethod::EnterJoinId;
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
