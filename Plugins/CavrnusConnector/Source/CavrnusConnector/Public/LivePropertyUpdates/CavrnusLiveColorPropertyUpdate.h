// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusLiveColorPropertyUpdate.h
 * @brief This file defines the UCavrnusLiveColorPropertyUpdate class used for handling live color property updates in Cavrnus.
 *
 * The UCavrnusLiveColorPropertyUpdate class provides functionality for managing temporary and transient color property updates
 * in the Cavrnus system. It offers methods to initialize the update, update with new data, and finalize the update.
 */
#pragma once

#include "LivePropertyUpdates/CavrnusLivePropertyUpdate.h"

#include "CavrnusLiveColorPropertyUpdate.generated.h"		// Always last

/**
 * @brief UCavrnusLiveColorPropertyUpdate class for managing live color property updates in Cavrnus.
 *
 * This class provides functionality to handle the initialization, updating, and finalization of live color property updates.
 */
UCLASS(BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusLiveColorPropertyUpdate : public UCavrnusLivePropertyUpdate
{
	GENERATED_BODY()

public:
	// Default Constructor
	UCavrnusLiveColorPropertyUpdate();

	// Destructor
	virtual ~UCavrnusLiveColorPropertyUpdate();

	/**
	 * @brief Initializes the color property update.
	 *
	 * This method initializes the live color property update with the given relay model, space connection, property ID, and initial value.
	 *
	 * @param relayModel The relay model to use for the update.
	 * @param spaceConn The space connection associated with the update.
	 * @param propertyId The ID of the property being updated.
	 * @param value The initial value of the property.
	 */
	void Initialize(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, FLinearColor value);

	/**
	 * @brief Updates the temporary value sent to the server.
	 *
	 * This method updates the temporary color value sent to the server.
	 *
	 * @param PropertyValue The new color value to update.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Updates the temporary value sent to the server", ShortToolTip = "Updates the temporary value sent to the server"))
	void UpdateWithNewData(FLinearColor PropertyValue);

	/**
	 * @brief Posts and finalizes the data to the server.
	 *
	 * This method posts the final color value to the server and finalizes the update. The object will no longer be valid to use afterwards.
	 *
	 * @param PropertyValue The final color value to post.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Posts and finalizes the data to the server.  This object will no longer be valid to use afterwards.", ShortToolTip = "Posts and finalizes the data to the server"))
	void Finalize(FLinearColor PropertyValue);
};
