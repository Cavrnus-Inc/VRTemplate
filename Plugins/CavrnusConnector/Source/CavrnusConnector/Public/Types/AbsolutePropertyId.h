// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Containers/Map.h>
#include "Types/CavrnusBinding.h"
#include "Types/PropertiesContainer.h"

#include "AbsolutePropertyId.generated.h"

/**
 * @brief Structure representing the ID of a property within the Cavrnus system.
 *
 * The FFullPropertyId structure is used to uniquely identify properties within a Cavrnus container.
 */
USTRUCT()
struct CAVRNUSCONNECTOR_API FAbsolutePropertyId
{
	GENERATED_BODY()

public:
	/** The name of the container that holds the property. */
	FString ContainerName = "";
	/** The ID of the property value. */
	FString PropValueId = "";

	FName CombinedName = "";
	/**
	 * @brief Default constructor for FFullPropertyId.
	 *
	 * Initializes the ContainerName and PropValueId to empty strings.
	 */
	FAbsolutePropertyId()
	{
		PropValueId = "";
		ContainerName = "";
		CombinedName = "";
	}
	/**
	 * @brief Constructor to initialize FFullPropertyId from a full name string.
	 *
	 * @param fullName The full name string containing both container and property IDs.
	 */
	FAbsolutePropertyId(const FString& fullName)
	{
		AssignFullName(fullName);
	}

	/**
	 * @brief Constructor to initialize FFullPropertyId with container name and property ID.
	 *
	 * @param containerName The name of the container.
	 * @param propId The ID of the property value.
	 */
	FAbsolutePropertyId(const FString& containerName, const FString& propId)
	{
		PropValueId = propId;
		ContainerName = containerName;

		//Clean up & standardize the ContainerName
		if (ContainerName.StartsWith("/"))
			ContainerName.RemoveFromStart("/");
		if (ContainerName.EndsWith("/"))
			ContainerName.RemoveFromEnd("/");

		FString StrCombinedName = ContainerName + "/" + PropValueId;
		CombinedName = *StrCombinedName;
	}

	/**
	 * @brief Constructor to initialize FFullPropertyId with container name and property ID.
	 *
	 * @param containerName The name of the container.
	 * @param propId The ID of the property value.
	 */
	FAbsolutePropertyId(const FPropertiesContainer& containerName, const FString& propId)
	{
		PropValueId = propId;
		ContainerName = containerName.ContainerName;

		//Clean up & standardize the ContainerName
		if (ContainerName.StartsWith("/"))
			ContainerName.RemoveFromStart("/");
		if (ContainerName.EndsWith("/"))
			ContainerName.RemoveFromEnd("/");

		FString StrCombinedName = ContainerName + "/" + PropValueId;
		CombinedName = *StrCombinedName;
	}

	FAbsolutePropertyId(const FAbsolutePropertyId& OtherAbsolutePropertyId)
	{
		ContainerName = OtherAbsolutePropertyId.ContainerName;
		PropValueId = OtherAbsolutePropertyId.PropValueId;
		CombinedName = OtherAbsolutePropertyId.CombinedName;
	}

	/** Default Destructor for FFullPropertyId. */
	~FAbsolutePropertyId(){}

	/**
	 * @brief Combines the container name and property ID into a single string.
	 *
	 * @param pId The property ID structure.
	 * @return The combined name string.
	 */
	static const FString GetCombinedName(const FAbsolutePropertyId& pId)
	{
		return pId.CombinedName.ToString();
	}

	/**
	 * @brief Gets the hash value of the property ID.
	 *
	 * @param propertyId The property ID structure.
	 * @return The hash value.
	 */
	friend uint32 GetTypeHash(const FAbsolutePropertyId& propertyId)
	{
		return GetTypeHash(propertyId.CombinedName);
	}

	/**
	 * @brief Equality operator for FFullPropertyId.
	 *
	 * @param other The other property ID to compare with.
	 * @return True if the property IDs are equal, false otherwise.
	 */
	bool operator==(const FAbsolutePropertyId& other) const
	{
		return CombinedName.IsEqual(other.CombinedName, ENameCase::CaseSensitive);
	}

	/**
	 * @brief Convenience function to return FString type without calling static function GetCombinedName
	 *
	 * @return The Absolute Property Id as an FString
	 */
	FString ToString() const
	{
		return(FAbsolutePropertyId::GetCombinedName(*this));
	}

private:
	void AssignFullName(FString FullName)
	{
		if (FullName.StartsWith("/"))
			FullName.RemoveFromStart("/");
		if (FullName.EndsWith("/"))
			FullName.RemoveFromEnd("/");

		CombinedName = *FullName;

		PropValueId = "";
		for (int i = FullName.Len() - 1; i >= 0; i--)
		{
			if (FullName[i] == '/')
			{
				PropValueId = PropValueId.Reverse();
				break;
			}
			PropValueId.AppendChar(FullName[i]);
		}

		ContainerName = FullName;
		ContainerName.RemoveFromEnd("/" + PropValueId);
	}
};
