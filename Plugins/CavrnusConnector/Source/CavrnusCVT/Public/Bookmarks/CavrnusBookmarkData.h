// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusBookmarkData.generated.h"

/**
 * @brief Structure to hold bookmark data including name, ID, timestamp, and flexible metadata.
 * 
 * The FCavrnusBookmarkData structure contains all information needed to represent a bookmark,
 * including a unique identifier, display name, creation timestamp, and a flexible metadata map
 * that can be populated via callback functions.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCVT_API FCavrnusBookmarkData
{
    GENERATED_BODY()

    /** Unique identifier for the bookmark */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Bookmark")
    FString BookmarkId = "";

    /** Display name of the bookmark */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Bookmark")
    FString Name = "";

    /** Timestamp when bookmark was created */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Bookmark")
    FDateTime CreatedTimestamp = FDateTime::Now();

    /** Flexible metadata map for custom data (collected via callback) */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Bookmark")
    TMap<FString, FString> Metadata = TMap<FString, FString>();

    /** Default constructor */
    FCavrnusBookmarkData() = default;
    
    /** Constructor with name and ID */
    FCavrnusBookmarkData(const FString& InName, const FString& InBookmarkId)
        : BookmarkId(InBookmarkId), Name(InName), CreatedTimestamp(FDateTime::Now())
    {
    }

    /** Equality operator for comparison */
    bool operator==(const FCavrnusBookmarkData& Other) const
    {
        return BookmarkId == Other.BookmarkId;
    }
};

