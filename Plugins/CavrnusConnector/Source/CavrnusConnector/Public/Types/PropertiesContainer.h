// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Containers/Map.h>
#include "Types/CavrnusBinding.h"

#include "PropertiesContainer.generated.h"

/**
 * @brief Structure representing the ID of a property within the Cavrnus system.
 *
 * The FFullPropertyId structure is used to uniquely identify properties within a Cavrnus container.
 */
USTRUCT()
struct CAVRNUSCONNECTOR_API FPropertiesContainer
{
	GENERATED_BODY()

public:
	/** The name of the container that holds the property. */
	FString ContainerName = "";

	/**
	 * @brief Default constructor for FPropertiesContainer.
	 *
	 * Initializes the ContainerName to empty string.
	 */
	FPropertiesContainer()
	{
		ContainerName = "";
	}

	/**
	 * @brief Constructor to initialize FPropertiesContainer with container name
	 *
	 * @param containerName The name of the container.
	 */
	FPropertiesContainer(FString containerName)
	{
		ContainerName = containerName;

		//Clean up & standardize the ContainerName
		if (!ContainerName.StartsWith("/"))
			ContainerName = "/" + ContainerName;
		if (!ContainerName.EndsWith("/"))
			ContainerName = ContainerName + "/";
	}

	/** Default Destructor for FFullPropertyId. */
	~FPropertiesContainer(){}

	/**
	 * @brief Gets the hash value of the property ID.
	 *
	 * @param propertyId The property ID structure.
	 * @return The hash value.
	 */
	friend uint32 GetTypeHash(const FPropertiesContainer& propertyId)
	{
		return GetTypeHash(propertyId.ContainerName);
	}

	/**
	 * @brief Equality operator for FFullPropertyId.
	 *
	 * @param other The other property ID to compare with.
	 * @return True if the property IDs are equal, false otherwise.
	 */
	bool operator==(const FPropertiesContainer& other) const
	{
		return ContainerName.Equals(other.ContainerName, ESearchCase::CaseSensitive);
	}

	/**
	 * @brief Convenience function to return FString type without accessing member variable
	 *
	 * @return The container as an FString
	 */
	FString ToString() const
	{
		return ContainerName;
	}

	/**
	 * @brief Add subcontainer to existing container
	 *
	 * @param Subcontainer FString to be added under the existing container.
	 * @return New Container as FString
	 */
	FString Push(FString Subcontainer)
	{
		if (!ContainerName.EndsWith("/"))
			ContainerName = ContainerName + "/";
		if (Subcontainer.StartsWith("/"))
			ContainerName = ContainerName + Subcontainer.RightChop(1);
		else
			ContainerName = ContainerName + Subcontainer;
		if (!ContainerName.EndsWith("/"))
			ContainerName = ContainerName + "/";
		return ContainerName;
	}

	/**
	 * @brief Remove lowest subcontainer level from Container
	 *
	 * @return The subcontainer that was removed as an FString
	 */
	FString Pop()
	{
		if (ContainerName.Len() <= 1)
			return FString("");
		int32 Position = ContainerName.Find(TEXT("/"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, ContainerName.Len() - 1);
		FString popVal = ContainerName.RightChop(Position);
		ContainerName = ContainerName.LeftChop(ContainerName.Len()-Position);
		return popVal;
	}
};
