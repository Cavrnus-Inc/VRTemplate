// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusSpaceTagsFunctionLibrary.h
 * @brief Blueprint function library for Cavrnus Space Tag operations (fetch/set/delete).
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/CavrnusSpaceTagMap.h"
#include "Types/CavrnusApiNotifyAction.h"

#include "CavrnusSpaceTagsFunctionLibrary.generated.h"		// Always last

// ============================================
// Delegate Declarations
// ============================================

DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusSpaceTagsReceived, FCavrnusSpaceTagMap, Tags);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusSpaceTagsError, FString, Error);
DECLARE_DYNAMIC_DELEGATE(FCavrnusSpaceTagsDeleted);

// ============================================
// C++ Callback Types
// ============================================

typedef TFunction<void(const FCavrnusSpaceTagMap&)> CavrnusSpaceTagsCallback;
typedef TFunction<void(const FString&)> CavrnusSpaceTagsErrorCallback;
typedef TFunction<void()> CavrnusSpaceTagsDeletedCallback;

// ============================================
// Class Definition
// ============================================

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusSpaceTagsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// ============================================
	// Fetch Space Tags
	// ============================================

	/**
	 * @brief Fetch all tags for a space.
	 * @param SpaceId The space ID to fetch tags for
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|SpaceTags|Advanced",
		meta = (ToolTip = "Fetch all tags for a space"))
	static void FetchSpaceTags(const FString& SpaceId, FCavrnusSpaceTagsReceived OnSuccess, FCavrnusSpaceTagsError OnFailure);

	static void FetchSpaceTags(const FString& SpaceId, CavrnusSpaceTagsCallback OnSuccess, CavrnusSpaceTagsErrorCallback OnFailure);

	// ============================================
	// Fetch Space Tag (single)
	// ============================================

	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusSpaceTagValueReceived, FString, Value);

	/**
	 * @brief Fetch a single tag value by key from a space.
	 * @param SpaceId The space ID to fetch the tag from
	 * @param Key The tag key to look up
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|SpaceTags|Advanced",
		meta = (ToolTip = "Fetch a single tag value by key from a space"))
	static void FetchSpaceTag(const FString& SpaceId, const FString& Key, FCavrnusSpaceTagValueReceived OnSuccess, FCavrnusSpaceTagsError OnFailure);

	static void FetchSpaceTag(const FString& SpaceId, const FString& Key, TFunction<void(const FString&)> OnSuccess, CavrnusSpaceTagsErrorCallback OnFailure);

	// ============================================
	// Add Space Tag
	// ============================================

	/**
	 * @brief Add or update a single tag on a space without affecting other tags.
	 * @param SpaceId The space ID to add the tag to
	 * @param Key The tag key
	 * @param Value The tag value
	 * @param NotifyAction Optionally show a toast or log the result
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|SpaceTags|Advanced",
		meta = (ToolTip = "Add or update a single tag on a space"))
	static void AddSpaceTag(const FString& SpaceId, const FString& Key, const FString& Value, FCavrnusSpaceTagsReceived OnSuccess, FCavrnusSpaceTagsError OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);

	static void AddSpaceTag(const FString& SpaceId, const FString& Key, const FString& Value, CavrnusSpaceTagsCallback OnSuccess, CavrnusSpaceTagsErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);

	// ============================================
	// Set Space Tags
	// ============================================

	/**
	 * @brief Set tags on a space. If bReplaceAll is true, all existing tags are replaced.
	 * @param SpaceId The space ID to set tags on
	 * @param Tags The tags to set (key-value pairs)
	 * @param bReplaceAll If true, replaces all existing tags; if false, merges with existing
	 * @param NotifyAction Optionally show a toast or log the result
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|SpaceTags|Advanced",
		meta = (ToolTip = "Set tags on a space"))
	static void SetSpaceTags(const FString& SpaceId, FCavrnusSpaceTagMap Tags, bool bReplaceAll, FCavrnusSpaceTagsReceived OnSuccess, FCavrnusSpaceTagsError OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);

	static void SetSpaceTags(const FString& SpaceId, const FCavrnusSpaceTagMap& Tags, bool bReplaceAll, CavrnusSpaceTagsCallback OnSuccess, CavrnusSpaceTagsErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);

	// ============================================
	// Delete Space Tags
	// ============================================

	/**
	 * @brief Delete specific tags from a space by their keys.
	 * @param SpaceId The space ID to delete tags from
	 * @param TagKeys The tag keys to delete
	 * @param NotifyAction Optionally show a toast or log the result
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|SpaceTags|Advanced",
		meta = (ToolTip = "Delete specific tags from a space by key"))
	static void DeleteSpaceTags(const FString& SpaceId, const TArray<FString>& TagKeys, FCavrnusSpaceTagsDeleted OnSuccess, FCavrnusSpaceTagsError OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);

	static void DeleteSpaceTags(const FString& SpaceId, const TArray<FString>& TagKeys, CavrnusSpaceTagsDeletedCallback OnSuccess, CavrnusSpaceTagsErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);
};
