// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Bookmarks/CavrnusBookmarkManager.h"
#include "CavrnusCVT.h"
#include "Misc/Guid.h"

void UCavrnusBookmarkManager::Initialize()
{
    Super::Initialize();
    Bookmarks.Empty();
    UE_LOG(LogCavrnusCVT, Log, TEXT("BookmarkManager initialized"));
}

void UCavrnusBookmarkManager::Dispose()
{
    ClearAllBookmarks();
    MetadataCollector.Unbind();
    BookmarkSelectedCallback.Unbind();
    Super::Dispose();
}

void UCavrnusBookmarkManager::RegisterMetadataCollector(FOnCollectBookmarkMetadata InDelegate)
{
    MetadataCollector = InDelegate;
}

void UCavrnusBookmarkManager::RegisterBookmarkSelectedCallback(FOnBookmarkSelected InDelegate)
{
    BookmarkSelectedCallback = InDelegate;
}

FCavrnusBookmarkData UCavrnusBookmarkManager::CreateBookmark(const FString& BookmarkName)
{
    if (BookmarkName.IsEmpty())
    {
        UE_LOG(LogCavrnusCVT, Warning, TEXT("Cannot create bookmark with empty name"));
        return FCavrnusBookmarkData();
    }

    // Generate unique ID
    const FString BookmarkId = GenerateBookmarkId();

    // Collect metadata via callback if registered
    TMap<FString, FString> Metadata;
    if (MetadataCollector.IsBound())
    {
        Metadata = MetadataCollector.Execute(BookmarkName);
    }

    // Create bookmark data
    FCavrnusBookmarkData NewBookmark(BookmarkName, BookmarkId);
    NewBookmark.Metadata = Metadata;
    NewBookmark.CreatedTimestamp = FDateTime::Now();

    // Add to storage
    Bookmarks.Add(NewBookmark);

    UE_LOG(LogCavrnusCVT, Log, TEXT("Created bookmark: %s (ID: %s)"), *BookmarkName, *BookmarkId);

    return NewBookmark;
}

TArray<FCavrnusBookmarkData> UCavrnusBookmarkManager::GetAllBookmarks() const
{
    return Bookmarks;
}

FCavrnusBookmarkData* UCavrnusBookmarkManager::GetBookmarkById(const FString& BookmarkId)
{
    return Bookmarks.FindByPredicate([BookmarkId](const FCavrnusBookmarkData& Bookmark)
    {
        return Bookmark.BookmarkId == BookmarkId;
    });
}

bool UCavrnusBookmarkManager::DeleteBookmark(const FString& BookmarkId)
{
    const int32 RemovedCount = Bookmarks.RemoveAll([BookmarkId](const FCavrnusBookmarkData& Bookmark)
    {
        return Bookmark.BookmarkId == BookmarkId;
    });

    if (RemovedCount > 0)
    {
        UE_LOG(LogCavrnusCVT, Log, TEXT("Deleted bookmark with ID: %s"), *BookmarkId);
        return true;
    }

    UE_LOG(LogCavrnusCVT, Warning, TEXT("Bookmark not found for deletion: %s"), *BookmarkId);
    return false;
}

void UCavrnusBookmarkManager::ClearAllBookmarks()
{
    const int32 Count = Bookmarks.Num();
    Bookmarks.Empty();
    UE_LOG(LogCavrnusCVT, Log, TEXT("Cleared all bookmarks (%d removed)"), Count);
}

void UCavrnusBookmarkManager::TriggerBookmarkSelected(const FCavrnusBookmarkData& BookmarkData)
{
    if (BookmarkSelectedCallback.IsBound())
    {
        BookmarkSelectedCallback.Execute(BookmarkData);
    }
}

FString UCavrnusBookmarkManager::GenerateBookmarkId() const
{
    return FGuid::NewGuid().ToString();
}

