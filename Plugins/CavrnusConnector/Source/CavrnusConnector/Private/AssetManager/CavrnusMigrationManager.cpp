// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "AssetManager/CavrnusMigrationManager.h"
#include "CavrnusConnectorModule.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "UObject/Package.h"

static const FString MigrationSection = TEXT("CavrnusMigrations");
static const FString MigrationKey = TEXT("CompletedVersion");

void UCavrnusMigrationManager::RunPendingMigrations()
{
	const int32 Completed = GetCompletedMigrationVersion();
	if (Completed >= LatestMigrationVersion)
	{
		return;
	}

	UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusMigrationManager: Running migrations from v%d to v%d"), Completed, LatestMigrationVersion);

	// Dispatch table: sequential numbered migrations
	using MigrationFunc = bool (UCavrnusMigrationManager::*)();
	struct FMigrationEntry
	{
		int32 Version;
		MigrationFunc Func;
	};

	const FMigrationEntry Migrations[] =
	{
		{ 1, &UCavrnusMigrationManager::RunMigration_001 },
		{ 2, &UCavrnusMigrationManager::RunMigration_002 },
	};

	for (const FMigrationEntry& Entry : Migrations)
	{
		if (Entry.Version <= Completed)
		{
			continue;
		}

		UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusMigrationManager: Running migration %03d..."), Entry.Version);
		const bool bSuccess = (this->*Entry.Func)();
		if (!bSuccess)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusMigrationManager: Migration %03d FAILED. Will retry next launch."), Entry.Version);
			return; // Halt — retry next launch
		}

		SetCompletedMigrationVersion(Entry.Version);
		UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusMigrationManager: Migration %03d completed successfully."), Entry.Version);
	}
}

int32 UCavrnusMigrationManager::GetCompletedMigrationVersion() const
{
	int32 Version = 0;
	GConfig->GetInt(*MigrationSection, *MigrationKey, Version, GEditorPerProjectIni);
	return Version;
}

void UCavrnusMigrationManager::SetCompletedMigrationVersion(int32 Version)
{
	GConfig->SetInt(*MigrationSection, *MigrationKey, Version, GEditorPerProjectIni);
	GConfig->Flush(false, GEditorPerProjectIni);
}

// Migration_001: Delete RuntimeRegistry.uasset + legacy CavrnusSpawnableActorsDataAsset.uasset
bool UCavrnusMigrationManager::RunMigration_001()
{
#if WITH_EDITOR
	const FString DataAssetsDir = FPaths::ProjectContentDir() / TEXT("Cavrnus/DataAssets");

	auto DeletePackageWithSidecars = [](const FString& BaseFilename)
	{
		IFileManager::Get().Delete(*BaseFilename, /*RequireExists=*/false);
		IFileManager::Get().Delete(*FPaths::ChangeExtension(BaseFilename, TEXT("uexp")), false);
		IFileManager::Get().Delete(*FPaths::ChangeExtension(BaseFilename, TEXT("ubulk")), false);
	};

	auto ClearGhostPackage = [](const FString& LongPackageName)
	{
		if (UPackage* OldPkg = FindPackage(nullptr, *LongPackageName))
		{
			OldPkg->ClearFlags(RF_Public | RF_Standalone);
			OldPkg->MarkAsGarbage();
		}
	};

	// Delete RuntimeRegistry
	{
		const FString RegistryFile = DataAssetsDir / TEXT("RuntimeRegistry.uasset");
		if (FPaths::FileExists(RegistryFile))
		{
			UE_LOG(LogCavrnusConnector, Log, TEXT("Migration_001: Deleting RuntimeRegistry"));
			DeletePackageWithSidecars(RegistryFile);
			ClearGhostPackage(TEXT("/Game/Cavrnus/DataAssets/RuntimeRegistry"));
		}
	}

	// Delete legacy CavrnusSpawnableActorsDataAsset
	{
		const FString LegacyFile = DataAssetsDir / TEXT("CavrnusSpawnableActorsDataAsset.uasset");
		if (FPaths::FileExists(LegacyFile))
		{
			UE_LOG(LogCavrnusConnector, Log, TEXT("Migration_001: Deleting legacy CavrnusSpawnableActorsDataAsset"));
			DeletePackageWithSidecars(LegacyFile);
			ClearGhostPackage(TEXT("/Game/Cavrnus/DataAssets/CavrnusSpawnableActorsDataAsset"));
		}
	}
#endif
	return true;
}

// Migration_002: Ensure Enhanced Input is enabled on UE 5.0
bool UCavrnusMigrationManager::RunMigration_002()
{
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

		UE_LOG(LogCavrnusConnector, Log, TEXT("Migration_002: Enhanced Input override applied to DefaultInput.ini"));
		FText Message = FText::FromString(TEXT("Enhanced Input has been enabled. Please restart the Editor to apply changes to DefaultPlayerInputClass."));
		FMessageDialog::Open(EAppMsgType::Ok, Message);
	}
	else
	{
		UE_LOG(LogCavrnusConnector, Log, TEXT("Migration_002: Enhanced Input already present in DefaultInput.ini"));
	}
#endif
	return true;
}
