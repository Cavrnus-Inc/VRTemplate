// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CavrnusMigrationManager.generated.h"

/**
 * Manages one-time structural migrations for the Cavrnus plugin.
 * Migration state is persisted in Saved/Config/<Platform>/CavrnusMigrations.ini.
 * Each migration runs exactly once; on failure it halts and retries next launch.
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusMigrationManager : public UObject
{
	GENERATED_BODY()

public:
	void RunPendingMigrations();

private:
	static constexpr int32 LatestMigrationVersion = 2;

	int32 GetCompletedMigrationVersion() const;
	void SetCompletedMigrationVersion(int32 Version);

	bool RunMigration_001();
	bool RunMigration_002();
};
