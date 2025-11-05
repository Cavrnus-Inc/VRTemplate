// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusRemoteContent.h
 * @brief This file defines the FCavrnusRemoteContent structure used for representing remote content information in Cavrnus.
 *
 * The FCavrnusRemoteContent struct is used to hold metadata about content stored remotely in the Cavrnus system.
 * This includes information such as the content ID, name, file name, thumbnail URL, and associated tags.
 */

#pragma once

#include "CoreMinimal.h"

#include "CavrnusRemoteContent.generated.h"		// Always last

/**
* @brief Structure to hold information about remote content in Cavrnus.
*
* The FCavrnusRemoteContent structure contains fields representing metadata about content stored on the Cavrnus server.
* This includes the content ID, name, file name, thumbnail URL, and a map of associated tags.
*/
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusRemoteContent
{
	GENERATED_BODY()

	/** The unique identifier for the remote content. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Content")
	FString Id = "";

	/** The name of the remote content. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Content")
	FString Name = "";

	/** The file name of the remote content. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Content")
	FString FileName = "";

	/** The URL for the thumbnail image of the remote content. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Content")
	FString ThumbnailUrl = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Content")
	int64 Size = 0L;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Content")
	FString SizeString = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Content")
	bool CachedOnDisk = false;

	/** A map of tags associated with the remote content. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Content")
	TMap<FString, FString> Tags = TMap<FString, FString>();

	/** Default constructor for FCavrnusRemoteContent. */
	FCavrnusRemoteContent() {}

	/**
	 * @brief Constructor to initialize the FCavrnusRemoteContent with given values.
	 *
	 * @param id The unique identifier for the remote content.
	 * @param name The name of the remote content.
	 * @param fileName The file name of the remote content.
	 * @param thumbnailUrl The URL for the thumbnail image of the remote content.
	 * @param tags A map of tags associated with the remote content.
	 */
	FCavrnusRemoteContent(FString id, FString name, FString fileName, FString thumbnailUrl, long size, FString sizeStr, bool cachedOnDisk, TMap<FString, FString> tags) 
	{
		Id = id;
		Name = name;
		FileName = fileName;
		ThumbnailUrl = thumbnailUrl;
		Size = size;
		SizeString = sizeStr,
		CachedOnDisk = cachedOnDisk,
		Tags = tags;
	}
};
