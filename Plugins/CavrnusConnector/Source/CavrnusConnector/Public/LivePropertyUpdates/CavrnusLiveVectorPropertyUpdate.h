// Copyright (c) 2025 Cavrnus. All rights reserved.
/**
 * @file CavrnusLiveVectorPropertyUpdate.h
 * @brief This file defines the UCavrnusLiveVectorPropertyUpdate class used for live vector property updates in Cavrnus.
 *
 * The UCavrnusLiveVectorPropertyUpdate class manages temporary and final updates to vector properties in the Cavrnus system.
 * It provides methods to initialize, update, and finalize property values, ensuring synchronization with the server.
 */
#pragma once

#include "LivePropertyUpdates/CavrnusLivePropertyUpdate.h"

#include "CavrnusLiveVectorPropertyUpdate.generated.h"		// Always last

/**
 * @brief UCavrnusLiveVectorPropertyUpdate class for handling live vector property updates in Cavrnus.
 *
 * This class provides functionality to initialize, update, and finalize vector property values within the Cavrnus system.
 * It ensures that the temporary values are sent to the server and can be finalized to persist the changes.
 */
UCLASS(BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusLiveVectorPropertyUpdate : public UCavrnusLivePropertyUpdate
{
	GENERATED_BODY()

public:
	// Default Constructor
	UCavrnusLiveVectorPropertyUpdate();

	// Destructor
	virtual ~UCavrnusLiveVectorPropertyUpdate();

	/**
	 * @brief Initializes the live vector property update with the necessary parameters.
	 *
	 * @param relayModel Pointer to the Cavrnus relay model.
	 * @param spaceConn Connection information for the space.
	 * @param propertyId Identifier for the property to be updated.
	 * @param value Initial vector value for the property.
	 */
	void Initialize(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, FVector value);

	/**
	 * @brief Updates the temporary vector value sent to the server.
	 *
	 * @param PropertyValue The new vector value to be updated.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Updates the temporary value sent to the server", ShortToolTip = "Updates the temporary value sent to the server"))
	void UpdateWithNewData(FVector PropertyValue);

	/**
	 * @brief Posts and finalizes the vector property value to the server.
	 *
	 * Once this method is called, the object will no longer be valid for further updates.
	 *
	 * @param PropertyValue The final vector value to be posted to the server.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Posts and finalizes the data to the server.  This object will no longer be valid to use afterwards.", ShortToolTip = "Posts and finalizes the data to the server"))
	void Finalize(FVector PropertyValue);
};
