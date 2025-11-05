// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusLiveStringPropertyUpdate.h
 * @brief This file defines the UCavrnusLiveStringPropertyUpdate class used for live string property updates in Cavrnus.
 *
 * The UCavrnusLiveStringPropertyUpdate class manages temporary and final updates to string properties in the Cavrnus system.
 * It provides methods to initialize, update, and finalize property values, ensuring synchronization with the server.
 */
#pragma once

#include "LivePropertyUpdates/CavrnusLivePropertyUpdate.h"

#include "CavrnusLiveStringPropertyUpdate.generated.h"		// Always last

/**
 * @brief UCavrnusLiveStringPropertyUpdate class for handling live string property updates in Cavrnus.
 *
 * This class provides functionality to initialize, update, and finalize string property values within the Cavrnus system.
 * It ensures that the temporary values are sent to the server and can be finalized to persist the changes.
 */
UCLASS(BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusLiveStringPropertyUpdate : public UCavrnusLivePropertyUpdate
{
	GENERATED_BODY()

public:
	// Default Constructor
	UCavrnusLiveStringPropertyUpdate();

	// Destructor
	virtual ~UCavrnusLiveStringPropertyUpdate();

	/**
	 * @brief Initializes the live string property update with the necessary parameters.
	 *
	 * @param relayModel Pointer to the Cavrnus relay model.
	 * @param spaceConn Connection information for the space.
	 * @param propertyId Identifier for the property to be updated.
	 * @param value Initial string value for the property.
	 */
	void Initialize(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, FString value);

	/**
	 * @brief Updates the temporary string value sent to the server.
	 *
	 * @param PropertyValue The new string value to be updated.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Updates the temporary value sent to the server", ShortToolTip = "Updates the temporary value sent to the server"))
	void UpdateWithNewData(FString PropertyValue);

	/**
	 * @brief Posts and finalizes the string property value to the server.
	 *
	 * Once this method is called, the object will no longer be valid for further updates.
	 *
	 * @param PropertyValue The final string value to be posted to the server.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Posts and finalizes the data to the server.  This object will no longer be valid to use afterwards.", ShortToolTip = "Posts and finalizes the data to the server"))
	void Finalize(FString PropertyValue);
};
