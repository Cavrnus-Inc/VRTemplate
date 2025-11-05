// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "CavrnusPropertySyncStructs.h"
#include "LivePropertyUpdates/CavrnusLivePropertyUpdate.h"
#include "Types/CavrnusPropertyValue.h"
#include "UObject/Object.h"
#include "CavrnusPropertyPoller.generated.h"

/**
 * Polling system for properties
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPropertyPoller : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
public:
	void Initialize(
		const FCavrnusSpaceConnection& InSpaceConnection,
		const FCavrnusSyncInfo& InSyncInfo,
		const FCavrnusSyncOptions& InSyncOptions);
	
	void Teardown();
	
	UCavrnusPropertyPoller* GetLocalValueFunc(const TFunction<Cavrnus::FPropertyValue()>& GetLocalValueFunc);
	UCavrnusPropertyPoller* GetAreValuesEqualFunc(const TFunction<bool(Cavrnus::FPropertyValue LocalPropValue, Cavrnus::FPropertyValue ServerPropValue)>& InAreValuesEqualFunc);
	UCavrnusPropertyPoller* SetSendGuidCallback(const TFunction<void(FGuid)>& InSetGuidCallback);

	// FTickableGameObject overrides
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const override { return false; }
	virtual bool IsTickableWhenPaused() const override { return false; }
	
private:
	bool bShouldTick = false;
	float PollingTimeRemaining = 0.2f;
	float MaxPollingTimeBeforeSendRemaining = 1.0f;
	
	UPROPERTY()
	TObjectPtr<UCavrnusLivePropertyUpdate> LiveUpdater = nullptr;
	Cavrnus::FPropertyValue LastSentPropValue;
	
	FCavrnusSyncInfo SyncInfo = FCavrnusSyncInfo();
	FCavrnusSyncOptions SyncOptions = FCavrnusSyncOptions();
	FCavrnusSpaceConnection SpaceConn = FCavrnusSpaceConnection();
	
	TFunction<Cavrnus::FPropertyValue()> LocalValueFunc;
	TFunction<bool(Cavrnus::FPropertyValue LocalPropValue, Cavrnus::FPropertyValue ServerPropValue)> AreValuesEqualFunc;
	
	TFunction<void(FGuid LoadGuid)> SetInFlightGuid;

	bool TryFinalizeValue();
	void TrySendingNewValue();
};