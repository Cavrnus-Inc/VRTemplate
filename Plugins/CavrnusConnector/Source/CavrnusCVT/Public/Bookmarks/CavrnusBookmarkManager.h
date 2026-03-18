// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusConnector/Public/Managers/CavrnusService.h"
#include "Bookmarks/CavrnusBookmarkData.h"
#include "CavrnusBookmarkManager.generated.h"

using FBookmarkMetadataMap = TMap<FString, FString>;
/**
 * @brief Callback delegate for collecting bookmark metadata when creating a bookmark.
 * @param BookmarkName The name of the bookmark being created
 * @return Map of metadata key-value pairs to store with the bookmark
 */
DECLARE_DELEGATE_RetVal_OneParam(FBookmarkMetadataMap, FOnCollectBookmarkMetadata, const FString&);

/**
 * @brief Callback delegate for when a bookmark is selected.
 * @param BookmarkData The bookmark data that was selected
 */
DECLARE_DELEGATE_OneParam(FOnBookmarkSelected, const FCavrnusBookmarkData& /* BookmarkData */);

/**
 * @brief Manager for bookmark operations including CRUD operations and callback registration.
 * 
 * The UCavrnusBookmarkManager is registered as a service in the UCavrnusServiceLocator
 * and handles all bookmark-related operations including creation, deletion, and callback management.
 */
UCLASS()
class CAVRNUSCVT_API UCavrnusBookmarkManager : public UCavrnusService
{
    GENERATED_BODY()

public:
    virtual void Initialize() override;
    virtual void Dispose() override;

    /**
     * Register a callback function that will be called to collect metadata when creating a bookmark.
     * @param InDelegate The delegate to call when collecting metadata
     */
    void RegisterMetadataCollector(FOnCollectBookmarkMetadata InDelegate);
    
    /**
     * Register a callback function that will be called when a bookmark is selected.
     * @param InDelegate The delegate to call when a bookmark is selected
     */
    void RegisterBookmarkSelectedCallback(FOnBookmarkSelected InDelegate);

    /**
     * Create a new bookmark with the given name.
     * This will call the registered metadata collector callback if one exists.
     * @param BookmarkName The display name for the bookmark
     * @return The created bookmark data
     */
    FCavrnusBookmarkData CreateBookmark(const FString& BookmarkName);

    /**
     * Get all bookmarks.
     * @return Array of all bookmark data
     */
    TArray<FCavrnusBookmarkData> GetAllBookmarks() const;

    /**
     * Get a bookmark by its ID.
     * @param BookmarkId The unique identifier of the bookmark
     * @return Pointer to the bookmark data, or nullptr if not found
     */
    FCavrnusBookmarkData* GetBookmarkById(const FString& BookmarkId);

    /**
     * Delete a bookmark by its ID.
     * @param BookmarkId The unique identifier of the bookmark to delete
     * @return true if the bookmark was found and deleted, false otherwise
     */
    bool DeleteBookmark(const FString& BookmarkId);

    /**
     * Clear all bookmarks.
     */
    void ClearAllBookmarks();

    /**
     * Trigger the bookmark selected callback if registered.
     * This is called when a bookmark is selected in the UI.
     * @param BookmarkData The bookmark that was selected
     */
    void TriggerBookmarkSelected(const FCavrnusBookmarkData& BookmarkData);

private:
    /** In-memory storage for bookmarks */
    UPROPERTY()
    TArray<FCavrnusBookmarkData> Bookmarks;

    /** Callback for collecting metadata when creating bookmarks */
    FOnCollectBookmarkMetadata MetadataCollector;
    
    /** Callback for when a bookmark is selected */
    FOnBookmarkSelected BookmarkSelectedCallback;

    /**
     * Generate a unique bookmark ID.
     * @return A unique string identifier
     */
    FString GenerateBookmarkId() const;
};

