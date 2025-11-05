// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusSubsystem.h"
#include "CavrnusConnectorSettings.h"
#include "AssetManager/CavrnusDataAssetManager.h"

#include "Managers/CavrnusEditorAuthenticationManager.h"
#include "Managers/Login/CavrnusLoginManager.h"
#include "Helpers/CavrnusPixelStreamingManager.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Blueprint.h"
#include "Modes/CavrnusModeManager.h"
#include "Pawns/CavrnusPawnManager.h"
#include "PropertySyncers/CavrnusPropertySyncManager.h"
#if WITH_EDITOR && ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
#include "Misc/MessageDialog.h"
#endif

#if WITH_EDITOR
#include "Editor.h"
#include "Editor/EditorEngine.h"
#endif

UCavrnusSubsystem* UCavrnusSubsystem::Get()
{
	if (GEngine)
		return GEngine->GetEngineSubsystem<UCavrnusSubsystem>();
	
	return nullptr;
}

void UCavrnusSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("CavrnusSubsystem initialized"));

#if WITH_EDITOR && ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
	const FString Section = TEXT("/Script/Engine.InputSettings");
	const FString Key1 = TEXT("DefaultPlayerInputClass");
	const FString Key2 = TEXT("DefaultInputComponentClass");

	const FString Value1 = TEXT("/Script/EnhancedInput.EnhancedPlayerInput");
	const FString Value2 = TEXT("/Script/EnhancedInput.EnhancedInputComponent");

	FString ExistingValue;
	if (!GConfig->GetString(*Section, *Key1, ExistingValue, GInputIni) || ExistingValue != Value1)
	{
		GConfig->SetString(*Section, *Key1, *Value1, GInputIni);
		GConfig->SetString(*Section, *Key2, *Value2, GInputIni);
		GConfig->Flush(false, GInputIni);

		UE_LOG(LogTemp, Log, TEXT("Enhanced Input override applied to DefaultInput.ini"));
		FText Message = FText::FromString("Enhanced Input has been enabled. Please restart the Editor to apply changes to DefaultPlayerInputClass.");
		FMessageDialog::Open(EAppMsgType::Ok, Message);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Enhanced Input override already present in DefaultInput.ini"));
	}
#endif
	Services = NewObject<UCavrnusServiceLocator>(this);
	
	Services->RegisterService<UCavrnusEditorAuthenticationManager>();
	Services->RegisterService<UCavrnusPawnManager>();
	Services->RegisterService<UCavrnusPropertySyncManager>();
	Services->RegisterService<UCavrnusDataAssetManager>();
	
	Services->RegisterService<UCavrnusLoginManager>();
	Services->RegisterService<UCavrnusConnectorSettings>();
	Services->RegisterService<UCavrnusModeManager>();
	Services->RegisterService<UCavrnusPixelStreamingManager>();

	InitializeLoginBehavior();
}

void UCavrnusSubsystem::Deinitialize()
{
	Services->Teardown();

	FWorldDelegates::OnPostWorldInitialization.Remove(WorldHandle);
	WorldHandle.Reset();

	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("CavrnusSubsystem deinitialized"));
}

void UCavrnusSubsystem::InitializeLoginBehavior()
{

#if WITH_EDITOR
	FEditorDelegates::BeginPIE.AddLambda([this](bool)
	{
		UE_LOG(LogTemp, Log, TEXT("CavrnusSubsystem: BeginPIE called!"));
		if (Services->Get<UCavrnusEditorAuthenticationManager>()->HasEditorAuthenticated())
			Services->Get<UCavrnusLoginManager>()->DoPieLogin();
		else
		{
			if (GetSettings()->ConnectOnStart)
				Services->Get<UCavrnusLoginManager>()->DoPluginSettingsLogin();
		}
	});
#else
	WorldHandle =  FWorldDelegates::OnPostWorldInitialization.AddLambda(
[this](const UWorld* World, const UWorld::InitializationValues&)
{
	// This World check doesn't solve the double fire problem
	UE_LOG(LogTemp, Log, TEXT("OnPostWorldInitialization called for CavrnusSubsystem!"));
	if (!World || World->WorldType != EWorldType::Game || !World->IsGameWorld())
	{
		UE_LOG(LogTemp, Log, TEXT("CavrnusSubsystem:Initialize ignoring non-game world.  Should run again"));
		return;
	}
	// OnPostWorldInitialization gets called twice in builds.
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("No Game Instance.  Returning"));
		return;
	}
	if (const auto* Settings = GetSettings())
	{
		UE_LOG(LogTemp, Log, TEXT("Settings : ConnectOnStart: %s, AuthMethod: %s"),
			Settings->ConnectOnStart ? TEXT("true") : TEXT("false"),
			*UEnum::GetValueAsString(Settings->AuthMethod));
		if (Settings->ConnectOnStart)
			Services->Get<UCavrnusLoginManager>()->DoPluginSettingsLogin();
	}
});
#endif
}
