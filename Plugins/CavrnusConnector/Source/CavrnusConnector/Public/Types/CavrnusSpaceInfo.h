// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusSpaceInfo.h
 * @brief This file defines the FCavrnusSpaceInfo structure used for storing live information about a given space.
 *
 * The FCavrnusSpaceInfo struct holds the most up-to-date information about a space. This can be used to create a
 * list of spaces, similar to the default Spaces List provided in Cavrnus. The structure contains fields for the
 * space's name, unique ID, and thumbnail link. These fields are primarily used for building space selection UIs.
 * To get a list of available CavrnusSpaceInfos, you can call FetchJoinableSpaces or BindJoinableSpaces.
 */

#pragma once

#include "CoreMinimal.h"
#include "CavrnusUserAccount.h"
#include "CavrnusSpaceMember.h"
#include "CavrnusSpaceInfo.generated.h"		// Always last

/**
 * @brief Structure to hold information about a Cavrnus space.
 *
 * The FCavrnusSpaceInfo structure contains the current name of the space, its unique ID, and a link to its thumbnail.
 * This information is used for building space selection UIs.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusSpaceInfo
{
	GENERATED_BODY()

	/** @brief The unique ID of the space. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spaces")
	FString SpaceId = "";

	/** @brief The current name of the space. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spaces")
	FString SpaceName = "";

	/** @brief The URL of the space's thumbnail image. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spaces")
	FString SpaceThumbnail = "";

	/** @brief The user id of the space's owner. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spaces")
	FString OwnerId = "";

	/** @brief The last access time of the space. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spaces")
	FDateTime LastAccess = FDateTime::MinValue();

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spaces")
	TArray<FString> Keywords;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spaces")
	TArray<FCavrnusSpaceMember> SpaceMembers;

	FCavrnusSpaceInfo(const FString& SpaceId, const FString& SpaceName, const FString& SpaceThumbnail, const FString& OwnerId, const FDateTime& LastAccess, const TArray<FString>& keywords, const TArray<FCavrnusSpaceMember>& members)
		: SpaceId(SpaceId), SpaceName(SpaceName), SpaceThumbnail(SpaceThumbnail), OwnerId(OwnerId), LastAccess(LastAccess), Keywords(keywords), SpaceMembers(members)
	{
	}

	FCavrnusSpaceInfo() = default;

	/**
	 * @brief Equality operator to compare two FCavrnusSpaceInfo objects.
	 *
	 * @param other The other FCavrnusSpaceInfo object to compare with.
	 * @return true if all fields are equal, false otherwise.
	 */
	bool operator==(const FCavrnusSpaceInfo& other) const
	{
		return SpaceId == other.SpaceId && SpaceName == other.SpaceName && SpaceThumbnail == other.SpaceThumbnail && LastAccess == other.LastAccess;
	}
};
