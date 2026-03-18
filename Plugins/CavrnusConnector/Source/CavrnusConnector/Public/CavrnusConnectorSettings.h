// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISettingsModule.h"
#include "UObject/Object.h"
#include "Blueprint/UserWidget.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "Misc/App.h"
#include "CavrnusConnectorSettings.generated.h"

UENUM(BlueprintType)
enum class ECavrnusAuthMethod : uint8
{
	JoinAsMember,
	JoinAsGuest,
	AllowBoth UMETA(DisplayName = "Allow Member or Guest"),
};

UENUM(BlueprintType)
enum class ECavrnusAuthMethodForPIE : uint8
{
	JoinAsPIE UMETA(DisplayName = "JoinAsPIE"),
	JoinAsMember UMETA(DisplayName = "JoinAsMember"),
	JoinAsGuest  UMETA(DisplayName = "JoinAsGuest")
};

UENUM(BlueprintType)
enum class ECavrnusMemberLoginMethod : uint8
{
	EnterMemberCredentials,
	PromptMemberToLogin
};

UENUM(BlueprintType)
enum class ECavrnusGuestLoginMethod : uint8
{
	EnterNameBelow,
	PromptToEnterName
};

UENUM(BlueprintType)
enum class ECavrnusPreferredLoginTab : uint8
{
	Member,
	Guest,
};

UENUM(BlueprintType)
enum class ECavrnusSpaceJoinMethod : uint8
{
	EnterJoinId,
	SpacesListMenu,
	PromptUserForJoinId,
};

UCLASS(config=Cavrnus, defaultconfig)
class CAVRNUSCONNECTOR_API UCavrnusConnectorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UCavrnusConnectorSettings* Get()
	{
#if WITH_EDITOR
		// Editor build: allow mutation
		if (UCavrnusConnectorSettings* FoundSettings = GetMutableDefault<UCavrnusConnectorSettings>())
		{
			return FoundSettings;
		}
#else
		// Runtime build: read-only access
		if (const UCavrnusConnectorSettings* FoundSettings = GetDefault<UCavrnusConnectorSettings>())
		{
			return const_cast<UCavrnusConnectorSettings*>(FoundSettings); // if caller expects non-const
		}
#endif
		UE_LOG(LogTemp, Error, TEXT("Unabled to find CavrnusSettings!"));
		return nullptr;
	}
	
	static void Show()
	{
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
			SettingsModule->ShowViewer("Project", "Plugins", "CavrnusSpatialConnector");
	}
	
#if WITH_EDITOR
	virtual FName GetCategoryName() const override
	{
		return TEXT("Plugins"); // Top-level category
	}
	virtual FName GetSectionName() const override
	{
		return TEXT("CavrnusSpatialConnector"); // Subsection label
	}
#endif

	UCavrnusConnectorSettings(const FObjectInitializer& obj);
	
	virtual void PostInitProperties() override;

	FString GetRelayNetOptionalParameters() const;

#if WITH_EDITOR
	// Called when an edit is made to the settings
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Automatically connect on Start.  Disable if you manually connect via Blueprint or C++
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options")
	bool ConnectOnStart = true;

	// Locally cache the User Authentication Token after successful login
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options")
	bool SaveUserAuthToken = false;

	// Hard-code the Server Domain. If blank, User will be prompted to enter it
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options")
	FString ServerDomain = "";

	// Select User Authentication method
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options", meta = (DisplayName = "Authentication Method"))
	ECavrnusAuthMethod AuthMethod = ECavrnusAuthMethod::AllowBoth;

	// Which tab to show first when using AllowBoth
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options", meta = (DisplayName = "Default Login Tab", EditCondition = "AuthMethod == ECavrnusAuthMethod::AllowBoth", EditConditionHides))
	ECavrnusPreferredLoginTab PreferredLoginTab = ECavrnusPreferredLoginTab::Member;

	// Select the method for defining the Guest User name
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options|Guest", meta = (EditCondition = "AuthMethod != ECavrnusAuthMethod::JoinAsMember", EditConditionHides))
	ECavrnusGuestLoginMethod GuestLoginMethod = ECavrnusGuestLoginMethod::PromptToEnterName;

	// Hard-code the Name that a Guest User will have
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options|Guest", meta = (EditCondition = "AuthMethod != ECavrnusAuthMethod::JoinAsMember && GuestLoginMethod == ECavrnusGuestLoginMethod::EnterNameBelow", EditConditionHides))
	FString GuestName = "";

	// Select the method for obtaining the Team Member authentication credentials
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options|Member", meta = (EditCondition = "AuthMethod != ECavrnusAuthMethod::JoinAsGuest", EditConditionHides))
	ECavrnusMemberLoginMethod MemberLoginMethod = ECavrnusMemberLoginMethod::EnterMemberCredentials;

	// Hard-code the Team Member Email address
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options|Member", meta = (EditCondition = "AuthMethod != ECavrnusAuthMethod::JoinAsGuest && MemberLoginMethod == ECavrnusMemberLoginMethod::EnterMemberCredentials", EditConditionHides))
	FString MemberLoginEmail = "";

	// Hard-code the Team Member Password
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options|Member", meta = (EditCondition = "AuthMethod != ECavrnusAuthMethod::JoinAsGuest && MemberLoginMethod == ECavrnusMemberLoginMethod::EnterMemberCredentials", EditConditionHides, PasswordField = true))
	FString MemberLoginPassword = "";

	// Select the method for choosing which Space to join
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options|Space")
	ECavrnusSpaceJoinMethod SpaceJoinMethod = ECavrnusSpaceJoinMethod::SpacesListMenu;
	
	// Hard-code the Join ID for a Space
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Sign-in Options|Space", meta = (EditCondition = "SpaceJoinMethod == ECavrnusSpaceJoinMethod::EnterJoinId", EditConditionHides))
	FString JoinId = "";

	// Select the Widget Blueprint for the Server Domain entry
	UPROPERTY(Config, EditAnywhere, Category = "Menu Widgets")
	TSubclassOf<UUserWidget> ServerSelectionMenu;

	// Select the Widget Blueprint for the Guest Name entry
	UPROPERTY(Config, EditAnywhere, Category = "Menu Widgets")
	TSubclassOf<UUserWidget> GuestJoinMenu;

	// Select the Widget Blueprint for the Team Member login
	UPROPERTY(Config, EditAnywhere, Category = "Menu Widgets")
	TSubclassOf<UUserWidget> MemberLoginMenu;

	// Select the Widget Blueprint for the Combined (Member + Guest) login
	UPROPERTY(Config, EditAnywhere, Category = "Menu Widgets")
	TSubclassOf<UUserWidget> CombinedLoginMenu;

	// Select the Widget Blueprint for the Space List Menu
	UPROPERTY(Config, EditAnywhere, Category = "Menu Widgets")
	TSubclassOf<UUserWidget> SpacesListMenu;

	// Select the Widget Blueprint for entering the Join ID for a Space
	UPROPERTY(Config, EditAnywhere, Category = "Menu Widgets")
	TSubclassOf<UUserWidget> JoinIdMenu;

	UPROPERTY(Config, EditAnywhere, Category = "Menu Widgets")
	TSubclassOf<UUserWidget> AuthenticationWidgetMenu;

	UPROPERTY(Config, EditAnywhere, Category = "Menu Widgets")
	TSubclassOf<UUserWidget> LoadingWidgetMenu;

	UPROPERTY(Config, EditAnywhere, Category = "Menu Widgets")
	TArray<TSubclassOf<UUserWidget>> WidgetsToLoad;

	UPROPERTY(Config, EditAnywhere, Category = "Pawn Options")
	bool bAutoSetupLocalPawn = true;
	UPROPERTY(Config, EditAnywhere, Category = "Pawn Options")
	bool bAutoSetupRemotePawns = true;

	// When enabled, exiting a space automatically loads the specified level, destroying all actors and ensuring clean state
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Space Exit Options")
	bool bLoadLevelOnSpaceExit = false;

	// Level to load when exiting a space
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Space Exit Options",
		meta = (EditCondition = "bLoadLevelOnSpaceExit", EditConditionHides))
	TSoftObjectPtr<UWorld> SpaceExitLevel;

	// When enabled, deauthenticating automatically loads the specified level
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Space Exit Options")
	bool bLoadLevelOnDeauthenticate = false;

	// Level to load when deauthenticating
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Space Exit Options",
		meta = (EditCondition = "bLoadLevelOnDeauthenticate", EditConditionHides))
	TSoftObjectPtr<UWorld> DeauthenticateLevel;

	UPROPERTY(Config, EditAnywhere, Category = "Datasmith Import Options")
	bool bIgnoreVersionCheck = false;

	UPROPERTY(Config, EditAnywhere, Category = "Datasmith Import Options")
	bool bFixTwinmotionLODs = false;
	
	UPROPERTY(Config, EditAnywhere, Category = "Datasmith Import Options")
	bool bEnableDatasmithMaterialRepair = true;

	// Relative to AppData/Local; root directory for runtime data, logs, and system files
	UPROPERTY(Config, EditAnywhere, Category = "Debug Configuration")
	FString StorageRootPath = FApp::GetProjectName();
	
	// Enables verbose debug logging
	UPROPERTY(Config, EditAnywhere, Category = "Debug Configuration")
	bool VerboseOutputLogging = false;

	// Enable writing logs to disk for persistent debugging
	UPROPERTY(Config, EditAnywhere, Category = "Debug Configuration")
	bool LogOutputToFile = false;

	// Disable real-time communication (voice and video) entirely
	UPROPERTY(Config, EditAnywhere, Category = "Debug Configuration")
	bool EnableRTC = true;
};
