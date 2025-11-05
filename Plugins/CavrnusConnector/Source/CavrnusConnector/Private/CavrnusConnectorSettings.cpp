// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusConnectorSettings.h"
#include "Engine/Engine.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"

//=====================================================================
UCavrnusConnectorSettings::UCavrnusConnectorSettings(const FObjectInitializer& obj)
{
	ConnectOnStart = true;
	SaveUserAuthToken = false;
	ServerDomain = "";

	AuthMethod = ECavrnusAuthMethod::JoinAsMember;
	GuestLoginMethod = ECavrnusGuestLoginMethod::PromptToEnterName;
	MemberLoginMethod = ECavrnusMemberLoginMethod::EnterMemberCredentials;
	GuestName = "";
	MemberLoginEmail = "";
	MemberLoginPassword = "";
	SpaceJoinMethod = ECavrnusSpaceJoinMethod::SpacesListMenu;
	JoinId = "";

	StorageRootPath = FApp::GetProjectName();

	VerboseOutputLogging = true;
	LogOutputToFile = true;
	EnableRTC = true;

	// Initialize UObject pointers to nullptr — don't load assets here
	ServerSelectionMenu = nullptr;
	GuestJoinMenu = nullptr;
	MemberLoginMenu = nullptr;
	JoinIdMenu = nullptr;
	SpacesListMenu = nullptr;
	LoadingWidgetMenu = nullptr;
	AuthenticationWidgetMenu = nullptr;
	WidgetsToLoad.Empty();
}

//=====================================================================
void UCavrnusConnectorSettings::PostInitProperties()
{
	Super::PostInitProperties();
	UE_LOG(LogTemp, Warning, TEXT("XXX Config name: %s"), *GetClass()->GetConfigName());

	if (!ServerSelectionMenu)
		ServerSelectionMenu = FCavrnusWidgetFactory::GetDefaultBlueprint(TEXT("/CavrnusConnector/UI/Menus/ServerMenu/WBP_ServerSelectionMenu.WBP_ServerSelectionMenu_C"), UUserWidget::StaticClass());

	if (!GuestJoinMenu)
		GuestJoinMenu = FCavrnusWidgetFactory::GetDefaultBlueprint(TEXT("/CavrnusConnector/UI/Menus/LoginMenus/WBP_GuestLogin.WBP_GuestLogin_C"), UUserWidget::StaticClass());

	if (!MemberLoginMenu)
		MemberLoginMenu = FCavrnusWidgetFactory::GetDefaultBlueprint(TEXT("/CavrnusConnector/UI/Menus/LoginMenus/WBP_MemberLogin.WBP_MemberLogin_C"), UUserWidget::StaticClass());

	if (!JoinIdMenu)
		JoinIdMenu = FCavrnusWidgetFactory::GetDefaultBlueprint(TEXT("/CavrnusConnector/UI/Menus/LoginMenus/WBP_JoinIdLogin.WBP_JoinIdLogin_C"), UUserWidget::StaticClass());

	if (!SpacesListMenu)
		SpacesListMenu = FCavrnusWidgetFactory::GetDefaultBlueprint(TEXT("/CavrnusConnector/UI/Menus/SpaceListMenu/WBP_Cavrnus_SpaceSelectionWidget.WBP_Cavrnus_SpaceSelectionWidget_C"), UUserWidget::StaticClass());

	if (!LoadingWidgetMenu)
		LoadingWidgetMenu = FCavrnusWidgetFactory::GetDefaultBlueprint(TEXT("/CavrnusConnector/UI/Menus/LoadingMenu/WBP_LoadingWidget.WBP_LoadingWidget_C"), UUserWidget::StaticClass());

	if (!AuthenticationWidgetMenu)
		AuthenticationWidgetMenu = FCavrnusWidgetFactory::GetDefaultBlueprint(TEXT("/CavrnusConnector/UI/Menus/LoadingMenu/WBP_AuthenticationWidget.WBP_AuthenticationWidget_C"), UUserWidget::StaticClass());

	if (WidgetsToLoad.Num() == 0)
		WidgetsToLoad.Add(FCavrnusWidgetFactory::GetDefaultBlueprint(TEXT("/CavrnusConnector/UI/Menus/MinimalUI/WBP_Cavrnus_MinimalUI.WBP_Cavrnus_MinimalUI_C"), UUserWidget::StaticClass()));

	UE_LOG(LogTemp, Warning, TEXT("UCavrnusConnectorSettings initialized (Config name: %s)"), *GetClass()->GetConfigName());
}

#if WITH_EDITOR
//=====================================================================
void UCavrnusConnectorSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
		SaveConfig(CPF_Config, *GetClass()->GetConfigName());
}
#endif

FString UCavrnusConnectorSettings::GetRelayNetOptionalParameters() const
{
	FString result;

	if (VerboseOutputLogging)
	{
		result = "-v";
	}

	if (LogOutputToFile)
	{
		result.Append(result.IsEmpty() ? "-f" : " -f");
	}

	if (EnableRTC)
	{
		result.Append(result.IsEmpty() ? "-d" : " -d");
	}

	return result;
}
