// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusJoinConfigFunctionLibrary.h
 * @brief Blueprint function library for Cavrnus Join Config CRUD operations (space-level).
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/CavrnusJoinConfig.h"
#include "Types/CavrnusApiNotifyAction.h"

#include "CavrnusJoinConfigFunctionLibrary.generated.h"		// Always last

// ============================================
// Delegate Declarations
// ============================================

DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusJoinConfigReceived, FCavrnusJoinConfig, JoinConfig);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusJoinConfigListReceived, FCavrnusJoinConfigListResponse, Response);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusJoinConfigError, FString, Error);
DECLARE_DYNAMIC_DELEGATE(FCavrnusJoinConfigDeleted);

// ============================================
// C++ Callback Types
// ============================================

typedef TFunction<void(const FCavrnusJoinConfig&)> CavrnusJoinConfigCallback;
typedef TFunction<void(const FCavrnusJoinConfigListResponse&)> CavrnusJoinConfigListCallback;
typedef TFunction<void(const FString&)> CavrnusJoinConfigErrorCallback;
typedef TFunction<void()> CavrnusJoinConfigDeletedCallback;

// ============================================
// Class Definition
// ============================================

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusJoinConfigFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// ============================================
	// List Join Configs
	// ============================================

	/**
	 * @brief List join configs for a space (paginated).
	 * @param SpaceId The space ID to list join configs for
	 * @param Limit Max results per page (default 10)
	 * @param Page Page number (default 1)
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|JoinConfig|Advanced",
		meta = (ToolTip = "List join configs for a space (paginated)"))
	static void ListJoinConfigs(const FString& SpaceId, int32 Limit, int32 Page, FCavrnusJoinConfigListReceived OnSuccess, FCavrnusJoinConfigError OnFailure);

	static void ListJoinConfigs(const FString& SpaceId, int32 Limit, int32 Page, CavrnusJoinConfigListCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure);

	// ============================================
	// Create Join Config
	// ============================================

	/**
	 * @brief Create a new join config. The JoinConfigId field is ignored (server assigns it).
	 * @param NotifyAction Optionally show a toast or log the result
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|JoinConfig|Advanced",
		meta = (ToolTip = "Create a new join config"))
	static void CreateJoinConfig(FCavrnusJoinConfig Config, FCavrnusJoinConfigReceived OnSuccess, FCavrnusJoinConfigError OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);

	static void CreateJoinConfig(const FCavrnusJoinConfig& Config, CavrnusJoinConfigCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);

	// ============================================
	// Fetch Join Config
	// ============================================

	/**
	 * @brief Fetch a join config by its ID.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|JoinConfig|Advanced",
		meta = (ToolTip = "Fetch a join config by ID"))
	static void FetchJoinConfig(const FString& Id, FCavrnusJoinConfigReceived OnSuccess, FCavrnusJoinConfigError OnFailure);

	static void FetchJoinConfig(const FString& Id, CavrnusJoinConfigCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure);

	// ============================================
	// Update Join Config
	// ============================================

	/**
	 * @brief Update an existing join config (POST to join-configs/{id}).
	 * @param NotifyAction Optionally show a toast or log the result
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|JoinConfig|Advanced",
		meta = (ToolTip = "Update an existing join config"))
	static void UpdateJoinConfig(FCavrnusJoinConfig Config, FCavrnusJoinConfigReceived OnSuccess, FCavrnusJoinConfigError OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);

	static void UpdateJoinConfig(const FCavrnusJoinConfig& Config, CavrnusJoinConfigCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);

	// ============================================
	// Delete Join Config
	// ============================================

	/**
	 * @brief Delete a join config by ID.
	 * @param NotifyAction Optionally show a toast or log the result
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|JoinConfig|Advanced",
		meta = (ToolTip = "Delete a join config by ID"))
	static void DeleteJoinConfig(const FString& Id, FCavrnusJoinConfigDeleted OnSuccess, FCavrnusJoinConfigError OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);

	static void DeleteJoinConfig(const FString& Id, CavrnusJoinConfigDeletedCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction = ECavrnusApiNotifyAction::None);

	// ============================================
	// Fetch Join Config by Slug
	// ============================================

	/**
	 * @brief Fetch a join config by its custom slug.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|JoinConfig|Advanced",
		meta = (ToolTip = "Fetch a join config by custom slug"))
	static void FetchJoinConfigBySlug(const FString& Slug, FCavrnusJoinConfigReceived OnSuccess, FCavrnusJoinConfigError OnFailure);

	static void FetchJoinConfigBySlug(const FString& Slug, CavrnusJoinConfigCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure);
};
