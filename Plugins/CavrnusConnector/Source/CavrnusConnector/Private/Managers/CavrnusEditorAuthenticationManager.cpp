// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/CavrnusEditorAuthenticationManager.h"

#include "CavrnusConnectorModule.h"
#include "CavrnusConnectorSettings.h"
#include "CavrnusEnumHelpers.h"
#include "CavrnusFunctionLibrary.h"
#include "CavrnusKeyValueStore.h"
#include "CavrnusSubsystem.h"
#include "RelayModel/CavrnusRelayModel.h"

FString UCavrnusEditorAuthenticationManager::EditorAuthMethod = "Cavrnus_EditorAuthMethod";
FString UCavrnusEditorAuthenticationManager::RuntimeToken = "Cavrnus_RuntimeToken";
FString UCavrnusEditorAuthenticationManager::ServerKey = "Cavrnus_EditorServerKey";
FString UCavrnusEditorAuthenticationManager::ApiEditorKey = "Cavrnus_ApiEditorKey";
FString UCavrnusEditorAuthenticationManager::ApiEditorToken = "Cavrnus_ApiEditorToken";

void UCavrnusEditorAuthenticationManager::Initialize()
{
	Super::Initialize();
	// Wiping data on each restart. Force user to sign in each time.
	ClearEditorAuthData();
	UpdateConnectedState(Disconnected);
}

void UCavrnusEditorAuthenticationManager::Deinitialize()
{
	Super::Deinitialize();
	// Wiping data on each restart. Force user to sign in each time.
	Teardown();
}

void UCavrnusEditorAuthenticationManager::Teardown()
{
}

ECavrnusAuthMethodForPIE UCavrnusEditorAuthenticationManager::GetCurrentAuthMethod()
{
	if (FCavrnusKeyValueStore::KeyEntryExists(EditorAuthMethod))
	{
		const FString FoundAuthValue = FCavrnusKeyValueStore::GetStoredStringValue(EditorAuthMethod);

		ECavrnusAuthMethodForPIE FoundEnum;
		if (FCavrnusEnumHelpers::StringToEnum<ECavrnusAuthMethodForPIE>(FoundAuthValue, FoundEnum))
			return FoundEnum;
	}

	UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusAuthenticationManager::GetCurrentAuthMethod] Unable to find desired key! Defaulting to JoinAsMember!"));

	SetLoginAuthMethod(ECavrnusAuthMethodForPIE::JoinAsMember);
	return ECavrnusAuthMethodForPIE::JoinAsMember;
}

void UCavrnusEditorAuthenticationManager::SetLoginAuthMethod(const ECavrnusAuthMethodForPIE& AuthMethod)
{
	FString OutString;
	FCavrnusEnumHelpers::EnumToString<ECavrnusAuthMethodForPIE>(AuthMethod, OutString);
	FCavrnusKeyValueStore::SetStoredStringValue(EditorAuthMethod, OutString);
	
	CurrentEditorAuthMethod = AuthMethod;
		
	if (OnEditorAuthMethodChanged.IsBound())
		OnEditorAuthMethodChanged.Broadcast(AuthMethod);
}

void UCavrnusEditorAuthenticationManager::SetRuntimeToken(const FString& InToken)
{
	FCavrnusKeyValueStore::SetStoredStringValue(RuntimeToken, InToken);
}

FString UCavrnusEditorAuthenticationManager::GetRuntimeToken()
{
	return FCavrnusKeyValueStore::GetStoredStringValue(RuntimeToken);
}

FString UCavrnusEditorAuthenticationManager::GetPIEAuthedServer()
{
	return FCavrnusKeyValueStore::GetStoredStringValue(ServerKey);
}

void UCavrnusEditorAuthenticationManager::DoConnect()
{
	UpdateConnectedState(Connecting);
	
	UCavrnusFunctionLibrary::AuthenticateWithDeviceCodeBegin(
		"cavrnus.cavrn.us",
		"unreal-editor",
		"Your Unreal Editor is logged in and ready! Return to your Unreal Editor and get building!",
		false,
		[this](const FCavrnusDeviceCodeData& CavrnusDeviceCodeData)
		{
			if (OnDeviceCodeDataReceived.IsBound())
				OnDeviceCodeDataReceived.Broadcast(CavrnusDeviceCodeData);

			FPlatformProcess::LaunchURL(*CavrnusDeviceCodeData.VerificationUrl, nullptr, nullptr);

			UCavrnusFunctionLibrary::AuthenticateWithDeviceCodeConclude(
				"cavrnus.cavrn.us",
				CavrnusDeviceCodeData.DeviceCode,
				CavrnusDeviceCodeData.UserCode,
				[this](const FCavrnusAuthentication& CavrnusAuthentication)
				{
					if (OnAuthCompleted.IsBound())
						OnAuthCompleted.Broadcast(CavrnusAuthentication);

					UpdateConnectedState(Connected);
					StoreAuthData(CavrnusAuthentication, []
					{
						Cavrnus::CavrnusRelayModel::KillDataModel(); // Immediately kill Relay once keys are created!
					}, [] (const FString&)
					{
						
					});
				},
				[](const FString& String)
				{
					UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusEditorConnectionManager::AuthenticateWithDeviceCodeConclude] Unable to AuthenticateWithDeviceCodeConclude! %s"), *String);
				});
		},
		[](const FString& String)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusEditorConnectionManager::AuthenticateWithDeviceCodeBegin] Unable to AuthWithDevice! %s"), *String);
		});
}

bool UCavrnusEditorAuthenticationManager::TryGetEditorLoginInfo(FCavrnusEditorLoginInfo& OutInfo)
{
	if (FCavrnusKeyValueStore::KeyEntryExists(ApiEditorKey))
	{
		OutInfo.Server = FCavrnusKeyValueStore::GetStoredStringValue(ServerKey);
		OutInfo.AccessKey = FCavrnusKeyValueStore::GetStoredStringValue(ApiEditorKey);
		OutInfo.AccessToken = FCavrnusKeyValueStore::GetStoredStringValue(ApiEditorToken);
		
		return true;
	}
	
	return false;
}

FDelegateHandle UCavrnusEditorAuthenticationManager::BindConnectedState(const TFunction<void(const ECavrnusEditorConnectedStateEnum&)>& OnConnCallback)
{
	OnConnCallback(CurrentState);
	return OnConnStateChanged.AddLambda(OnConnCallback);
}

void UCavrnusEditorAuthenticationManager::UpdateConnectedState(const ECavrnusEditorConnectedStateEnum& NewState)
{
	CurrentState = NewState;
	if (OnConnStateChanged.IsBound())
		OnConnStateChanged.Broadcast(NewState);
}

void UCavrnusEditorAuthenticationManager::StoreAuthData(const FCavrnusAuthentication& Auth, const TFunction<void()>& OnFinished, const TFunction<void(const FString&)>& OnFailed)
{
#if WITH_EDITOR
	FCavrnusKeyValueStore::SetStoredStringValue(ServerKey, Auth.Server);
	UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->ServerDomain = Auth.Server;

	// Default to join as editor!
	if (!FCavrnusKeyValueStore::KeyEntryExists(EditorAuthMethod))
		SetLoginAuthMethod(ECavrnusAuthMethodForPIE::JoinAsPIE);
	
	UCavrnusFunctionLibrary::CreateApiKey(
		ApiEditorKey,
		[OnFinished](const FCavrnusApiKeyData& CavrnusApiKeyData)
		{
			FCavrnusKeyValueStore::SetStoredStringValue(ApiEditorKey, CavrnusApiKeyData.AccessKey);
			FCavrnusKeyValueStore::SetStoredStringValue(ApiEditorToken, CavrnusApiKeyData.AccessToken);
			
			if (OnFinished)
				OnFinished();
		},
		[OnFailed](const FString& Err)
		{
			UE_LOG(LogCavrnusConnector, Log, TEXT("[UCavrnusEditorConnectionManager::StoreAuthData--CreateApiKey] %s"), *Err);

			if (OnFailed)
				OnFailed(Err);
		});

	HasAuthenticated = true;
#endif
}

void UCavrnusEditorAuthenticationManager::ClearEditorAuthData()
{
	FCavrnusKeyValueStore::DeleteKey(ServerKey);
	FCavrnusKeyValueStore::DeleteKey(ApiEditorKey);
	FCavrnusKeyValueStore::DeleteKey(ApiEditorToken);
	FCavrnusKeyValueStore::DeleteKey(EditorAuthMethod);
}