// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusLiveTransformPropertyUpdate.h
 * @brief This file defines the UCavrnusLiveTransformPropertyUpdate class used for live transform property updates in Cavrnus.
 *
 * The UCavrnusLiveTransformPropertyUpdate class manages temporary and final updates to transform properties in the Cavrnus system.
 * It provides methods to initialize, update, and finalize property values, ensuring synchronization with the server.
 */

#pragma once

#include "LivePropertyUpdates/CavrnusLivePropertyUpdate.h"
#include "Types/PropertyPostOptions.h"

#include "CavrnusLiveTransformPropertyUpdate.generated.h"		// Always last

/**
 * @brief UCavrnusLiveTransformPropertyUpdate class for handling live transform property updates in Cavrnus.
 *
 * This class provides functionality to initialize, update, and finalize transform property values within the Cavrnus system.
 * It ensures that the temporary values are sent to the server and can be finalized to persist the changes.
 */
UCLASS(BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusLiveTransformPropertyUpdate : public UCavrnusLivePropertyUpdate
{
	GENERATED_BODY()

public:
	// Constructor
	UCavrnusLiveTransformPropertyUpdate();

	// Destructor
	virtual ~UCavrnusLiveTransformPropertyUpdate();

	/**
	 * @brief Initializes the live transform property update with the necessary parameters.
	 *
	 * @param relayModel Pointer to the Cavrnus relay model.
	 * @param spaceConn Connection information for the space.
	 * @param propertyId Identifier for the property to be updated.
	 * @param value Initial transform value for the property.
	 */
	void Initialize(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, FTransform value, const FPropertyPostOptions& options = FPropertyPostOptions());

	/**
	 * @brief Updates the temporary transform value sent to the server.
	 *
	 * @param PropertyValue The new transform value to be updated.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Updates the temporary value sent to the server", ShortToolTip = "Updates the temporary value sent to the server"))
	void UpdateWithNewData(FTransform PropertyValue);

	/**
	 * @brief Posts and finalizes the transform property value to the server.
	 *
	 * Once this method is called, the object will no longer be valid for further updates.
	 *
	 * @param PropertyValue The final transform value to be posted to the server.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Posts and finalizes the data to the server.  This object will no longer be valid to use afterwards.", ShortToolTip = "Posts and finalizes the data to the server"))
	void Finalize(FTransform PropertyValue);
};
