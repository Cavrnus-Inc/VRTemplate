// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusLivePropertyUpdate.h
 * @brief This file defines the UCavrnusLivePropertyUpdate class used for handling live property updates in Cavrnus.
 *
 * The UCavrnusLivePropertyUpdate class provides functionality for managing temporary and transient property updates
 * in the Cavrnus system. It offers methods to cancel updates and track the last update time.
 */
#pragma once

#include "CoreMinimal.h"
#include "Types/CavrnusSpaceConnection.h"
#include "Types/CavrnusPropertyValue.h"
#include "Types/PropertyPostOptions.h"

#include "CavrnusLivePropertyUpdate.generated.h"		// Always last

struct FAbsolutePropertyId;

namespace Cavrnus
{
	class CavrnusVirtualPropertyUpdate;
	class CavrnusRelayModel;
}

/**
 * @brief UCavrnusLivePropertyUpdate class for managing live property updates in Cavrnus.
 *
 * This abstract class serves as a base for specific property update classes, providing common functionality such as
 * canceling updates and tracking the last updated time.
 */
UCLASS(Abstract, BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusLivePropertyUpdate : public UObject
{
	GENERATED_BODY()

public:
	// Default Constructor
	UCavrnusLivePropertyUpdate();

	// Destructor
	virtual ~UCavrnusLivePropertyUpdate();

	void InitializeGeneric(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, Cavrnus::FPropertyValue value, const FPropertyPostOptions& options = FPropertyPostOptions());

	/**
	 * @brief Cancels transient operations sent to the server.
	 *
	 * This method cancels any temporary or transient property updates that have been sent to the server.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Cancels transient operations sent to the server.", ShortToolTip = "Cancels transient operations sent to the server"))
	void Cancel();

	/**
	 * @brief Gets the last updated time in seconds.
	 *
	 * This method returns the time in seconds since the last property update was made.
	 *
	 * @return The time in seconds since the last update.
	 */
	double GetLastUpdatedTimeSeconds();

	void UpdateWithNewDataGeneric(const Cavrnus::FPropertyValue& propVal);

	void FinalizeGeneric(const Cavrnus::FPropertyValue& propVal);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Posts and finalizes the most recent data to the server.  This object will no longer be valid to use afterwards.", ShortToolTip = "Posts and finalizes the most recent data to the server"))
	void FinalizeCurrentValue();

protected:

	/** Pointer to the live property update instance */
	Cavrnus::CavrnusVirtualPropertyUpdate* livePropertyUpdate = nullptr;

	Cavrnus::FPropertyValue lastSentPropValue;
};
