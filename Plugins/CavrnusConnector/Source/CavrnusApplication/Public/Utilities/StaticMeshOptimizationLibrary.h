// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file StaticMeshOptimizationLibrary.h
 * @brief Standalone utility class for optimizing static mesh components by setting LOD settings
 * and remapping empty material slots. This class has no dependencies on Cavrnus-specific code
 * and can be moved to other Unreal Engine projects.
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Actor.h"
#include "Misc/Paths.h"
#include "Containers/Set.h"

#include "StaticMeshOptimizationLibrary.generated.h"

// Forward declaration
class ADatasmithRuntimeActor;

/**
 * @brief Delegate fired when Datasmith loading is complete.
 * @param bSuccess True if loading completed successfully, false if timed out or actor was destroyed.
 * @param DatasmithActor The actor that was being monitored (may be null if destroyed).
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnDatasmithLoadComplete, bool, bSuccess, ADatasmithRuntimeActor*, DatasmithActor);

/**
 * @brief Standalone utility library for optimizing static mesh components.
 * 
 * This class provides functions to:
 * - Set MinLOD to 1 on static mesh assets
 * - Set forced LOD model on static mesh components
 * - Remap empty material slots to slots with matching slot names
 * - Track processed actors to avoid duplicate processing
 * 
 * This class has no dependencies on Cavrnus-specific code and can be moved
 * to any Unreal Engine project.
 */
UCLASS(Abstract)
class CAVRNUSAPPLICATION_API UStaticMeshOptimizationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Optimizes all static mesh components in the actor hierarchy.
	 * 
	 * Recursively finds all static mesh components in the given actor and its children,
	 * then for each component:
	 * - Sets MinLOD to 1 on the static mesh asset (if valid)
	 * - Sets forced LOD model to 1 on the component
	 * - Remaps empty material slots to slots with matching slot names that have valid materials
	 * 
	 * @param TargetActor The root actor to process. If null, returns 0.
	 * @return The number of static mesh components that were processed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Mesh Optimization",
		meta = (ToolTip = "Optimizes all static mesh components in the actor hierarchy by setting MinLOD to 1 and remapping empty material slots",
			CallInEditor = "true"))
	static int32 OptimizeStaticMeshesForActor(AActor* TargetActor);

	/**
	 * @brief Optimizes all static mesh components in the actor hierarchy with tracking.
	 * 
	 * Same as OptimizeStaticMeshesForActor, but tracks processed actors to avoid duplicate processing.
	 * Use this version when calling repeatedly as child actors become available.
	 * 
	 * @param TargetActor The root actor to process. If null, returns 0.
	 * @param InOutProcessedActors Input/output set of already-processed actors. Actors in this set will be skipped.
	 * @return The number of newly processed static mesh components (excluding already-processed actors).
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Mesh Optimization",
		meta = (ToolTip = "Optimizes static mesh components with tracking to avoid processing the same actors multiple times. Pass the same ProcessedActors set across multiple calls.",
			CallInEditor = "true"))
	static int32 OptimizeStaticMeshesForActorWithTracking(
		AActor* TargetActor,
		UPARAM(ref) TSet<AActor*>& InOutProcessedActors);

	/**
	 * @brief Clears the processed actors tracking set.
	 * 
	 * Use this when starting a new batch of processing to reset the tracking.
	 * 
	 * @param ProcessedActors The set of processed actors to clear.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Mesh Optimization",
		meta = (ToolTip = "Clears the processed actors tracking set. Use this when starting a new batch of processing.",
			CallInEditor = "true"))
	static void ClearProcessedActors(UPARAM(ref) TSet<AActor*>& ProcessedActors);

	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Mesh Optimization",
		meta = (ToolTip = "Clears the processed actors tracking array. Use this when starting a new batch of processing.",
			CallInEditor = "true"))
	static void ClearProcessedActorsArray(UPARAM(ref) TArray<AActor*>& ProcessedActors);
	/**
	 * @brief Optimizes static mesh components with validation and retry tracking.
	 * 
	 * Same as OptimizeStaticMeshesForActorWithTracking, but validates component readiness before processing.
	 * Actors with unready components are marked for retry instead of being processed.
	 * 
	 * @param TargetActor The root actor to process. If null, returns 0.
	 * @param InOutProcessedActors Input/output array of already-processed actors. Actors in this array will be skipped.
	 * @param OutUnreadyActors Output array of actors that had unready components (for retry later).
	 * @param OutWarnings Output array of warning messages describing what wasn't ready.
	 * @return The number of newly processed static mesh components (excluding already-processed and unready actors).
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Mesh Optimization",
		meta = (ToolTip = "Optimizes static mesh components with validation and retry tracking. Returns actors/components that weren't ready.",
			CallInEditor = "true"))
	static int32 OptimizeStaticMeshesForActorWithValidation(
		AActor* TargetActor,
		UPARAM(ref) TArray<AActor*>& InOutProcessedActors,
		UPARAM(ref) TArray<AActor*>& OutUnreadyActors,
		UPARAM(ref) TArray<FString>& OutWarnings,
		bool bErrorCheck);

	/**
	 * @brief Finds .udatasmith files in the project.
	 * 
	 * Searches for .udatasmith files in the Content directory and subdirectories.
	 * 
	 * @param OutFilePaths Array to populate with found .udatasmith file paths.
	 * @param SearchDirectory Optional directory to search in. If empty, searches in Content directory.
	 * @return The number of .udatasmith files found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Datasmith",
		meta = (ToolTip = "Finds all .udatasmith files in the project Content directory",
			CallInEditor = "true"))
	static int32 FindDatasmithFiles(UPARAM(ref) TArray<FString>& OutFilePaths, const FString& SearchDirectory = TEXT(""));

	/**
	 * @brief Counts actors in a .udatasmith XML file.
	 * 
	 * Parses the .udatasmith XML file and counts all Actor and ActorMesh nodes.
	 * 
	 * @param FilePath The path to the .udatasmith file.
	 * @return The number of actors found in the file, or -1 if the file could not be parsed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Datasmith",
		meta = (ToolTip = "Counts all actors in a .udatasmith XML file by parsing Actor and ActorMesh nodes",
			CallInEditor = "true"))
	static int32 CountActorsInDatasmithFile(const FString& FilePath);

	/**
	 * @brief Monitors a DatasmithRuntimeActor's loading state and fires a delegate when loading is complete.
	 * 
	 * Uses the same polling pattern as DatasmithFileImporter: waits for one of the two booleans
	 * (bBuilding or IsReceiving) to become true (loading started), then waits for both to be false (loading complete).
	 * 
	 * @param DatasmithActor The Datasmith runtime actor to monitor. If null, delegate fires immediately with bSuccess=false.
	 * @param PollInterval Polling frequency in seconds (default: 0.1f to match DatasmithFileImporter).
	 * @param TimeoutSeconds Maximum time to wait before giving up (default: 60.0f, 0 = no timeout).
	 * @param OnComplete Delegate to fire when loading is complete or timeout occurs.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Datasmith",
		meta = (ToolTip = "Monitors Datasmith actor loading state and fires delegate when complete. Wait for one boolean to become true, then both to be false.",
			CallInEditor = "false"))
	static void MonitorDatasmithLoadCompletion(
		ADatasmithRuntimeActor* DatasmithActor,
		float PollInterval,
		float TimeoutSeconds,
		const FOnDatasmithLoadComplete& OnComplete);
};

