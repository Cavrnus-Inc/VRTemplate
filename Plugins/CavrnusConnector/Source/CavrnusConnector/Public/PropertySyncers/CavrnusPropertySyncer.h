// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPropertyPoller.h"
#include "CavrnusPropertySyncStructs.h"
#include "LivePropertyUpdates/CavrnusLivePropertyUpdate.h"
#include "Types/CavrnusSpaceConnection.h"
#include "UObject/Object.h"
#include "CavrnusPropertySyncer.generated.h"

/**
 * Base class that syncs values using transients + finalizing values after set time.
 */
 // For receiving a value from the server
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCavrnusPropertyReceived, bool, Local);

UCLASS(BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusPropertySyncer : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(
	FProperty* InUnrealProperty,
	UObject* InContainerObject,
	const FCavrnusSyncInfo& InSyncInfo,
	const FCavrnusSyncOptions& InSyncOptions);
	
	UFUNCTION(BlueprintCallable, Category="Cavrnus|Properties")
	virtual void Teardown();

	UPROPERTY(BlueprintAssignable, Category="Cavrnus|Properties")
	FCavrnusPropertyReceived CavrnusPropertyReceived;

	void OnPropertyReceiveBroadcast(bool bLocal);
	FCavrnusUpdateTracker UpdateTracker;

	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Properties")
	bool SetSyncedStructFromText(const FString& StructText);

protected:
	FCavrnusSyncInfo SyncInfo = FCavrnusSyncInfo();
	FCavrnusSyncOptions SyncOptions = FCavrnusSyncOptions();
	FCavrnusSpaceConnection SpaceConnection = FCavrnusSpaceConnection();

private:
	FGuid Id = FGuid();
	bool bHasTornDown = false;

	UPROPERTY()
	TObjectPtr<UCavrnusPropertyPoller> Poller;

	UPROPERTY()
	UCavrnusBinding* PropertyBinding;
	FString BindingId = FString();

	ECavrnusSyncResolvedPropertyType ResolvedPropertyType;
	void* KnownValuePtr = nullptr;
	FBoolProperty* BoolProperty;
	FStrProperty* StringProperty;
	FFloatProperty* FloatProperty;
	FDoubleProperty* DoubleProperty;
	FNumericProperty* NumericProperty;
	FStructProperty* StructProperty;

	bool TryGetPropertyType(FProperty* Prop, void* InContainerObject);
	void SetupPolling();

	Cavrnus::FPropertyValue GetLocalValue() const;
	void SetLocalValue(const Cavrnus::FPropertyValue& PropertyValue) const;

	UFUNCTION(BlueprintCallable, Category="Cavrnus|Properties")
	void SendCurrentValue(bool Transient=false);

	template<typename TSyncOption>
	bool TryGetSyncOptionType(const FCavrnusSyncOptions& BaseOptions, TSyncOption& Out)
	{
		const UScriptStruct* BaseStruct = BaseOptions.StaticStruct();
		const UScriptStruct* TargetStruct = TSyncOption::StaticStruct();

		if (BaseStruct == TargetStruct)
		{
			Out = *reinterpret_cast<const TSyncOption*>(&BaseOptions);
			return true;
		}

		return false;
	}

};
