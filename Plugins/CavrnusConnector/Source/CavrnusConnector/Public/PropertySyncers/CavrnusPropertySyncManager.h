// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPropertySyncer.h"
#include "Managers/CavrnusService.h"
#include "CavrnusPropertySyncManager.generated.h"

/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPropertySyncManager : public UCavrnusService
{
	GENERATED_BODY()
public:
	virtual void Initialize() override;
	void Register(const FGuid& Id, UCavrnusPropertySyncer* PropertySync);
	void Unregister(const FGuid& Id);

	virtual void Dispose() override;
private:
	UPROPERTY()
	TMap<FGuid, UCavrnusPropertySyncer*> Tracked;
};
