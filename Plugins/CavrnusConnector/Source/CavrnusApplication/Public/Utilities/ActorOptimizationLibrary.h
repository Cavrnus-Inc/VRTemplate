// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file ActorOptimizationLibrary.h
 * @brief Runtime utility for optimizing actors in the world.
 * Provides options for LOD forcing and VR performance optimizations that can be used at runtime or in Editor Utility Widgets.
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "ActorOptimizationLibrary.generated.h"

/**
 * @brief Configuration options for runtime actor optimization.
 */
USTRUCT(BlueprintType)
struct FActorOptimizationOptions
{
	GENERATED_BODY()

	/** Force LOD on static mesh components */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | Optimization")
	bool bForceLOD = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | Optimization")
	int LODIndex = 1;  // Default to 1 to always use highest detail

	/** Force minimum LOD on static mesh components (skips lower LODs, e.g., skip LOD 0) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | Optimization")
	bool bForceMinimumLOD = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | Optimization", meta = (EditCondition = "bForceMinimumLOD"))
	int MinimumLODIndex = 1;  // Default to 1 to skip LOD 0

	// VR Optimization Options (A, C-D)
	/** A: Disable shadow casting on static mesh components */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | VR Optimizations", meta = (DisplayName = "A: Disable Shadow Casting"))
	bool bDisableShadowCasting = true;  // Default: true

	/** C: Disable collision on decorative meshes (requires heuristics or manual tagging) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | VR Optimizations", meta = (DisplayName = "C: Disable Collision on Decorative"))
	bool bDisableCollisionOnDecorative = false;  // Default: false

	/** D: Optimize materials (log warnings about complex materials) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | VR Optimizations", meta = (DisplayName = "D: Optimize Materials"))
	bool bOptimizeMaterials = true;  // Default: true
};

/**
 * @brief Runtime utility library for optimizing actors in the world.
 * 
 * This class provides runtime-safe functions to optimize actors and their hierarchies:
 * - Force LOD on static mesh components
 * - Apply VR performance optimizations (shadow casting, collision on components, materials)
 * 
 * These functions modify component properties at runtime and can also be used in Editor Utility Widgets.
 * For editor-only optimizations that modify assets, use UActorOptimizationEditorLibrary instead.
 */
UCLASS(Abstract)
class CAVRNUSAPPLICATION_API UActorOptimizationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Optimizes an actor based on the provided runtime-safe configuration options.
	 * 
	 * Recursively processes the actor and all its subchildren, applying runtime-safe optimizations
	 * that modify component properties. Can be used at runtime or in Editor Utility Widgets.
	 * 
	 * @param TargetActor The root actor to optimize. If null, returns 0.
	 * @param Options Configuration options for which runtime optimizations to apply.
	 * @return The number of components that were processed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Actor Optimization",
		meta = (ToolTip = "Optimizes an actor with runtime-safe options for LOD, shadow casting, collision, and materials. Can be used at runtime or in EUW."))
	static int32 OptimizeActor(AActor* TargetActor, const FActorOptimizationOptions& Options);

	/**
	 * @brief Recursively collects all actors in the hierarchy starting from a root actor.
	 * 
	 * Uses breadth-first traversal with a queue and visited set to avoid duplicates
	 * and handle circular references. Includes the root actor and all attached child actors.
	 * 
	 * @param RootActor The root actor to start traversal from. If null, OutActors remains empty.
	 * @param OutActors Output array populated with all actors in the hierarchy.
	 */
	static void GetAllActorsRecursive(AActor* RootActor, TArray<AActor*>& OutActors);

private:
	/**
	 * @brief Forces LOD on all static mesh components in the actor hierarchy.
	 * 
	 * @param TargetActor The root actor to process.
	 * @param LODIndex The LOD index to force (0 means no forced LOD, 1 is highest detail).
	 * @return The number of static mesh components processed.
	 */
	static int32 ForceLODOnStaticMeshes(AActor* TargetActor, int LODIndex);

	/**
	 * @brief Forces minimum LOD on all static mesh components in the actor hierarchy.
	 * 
	 * Sets the minimum LOD that can be used, effectively skipping lower LODs (e.g., skip LOD 0).
	 * 
	 * @param TargetActor The root actor to process.
	 * @param MinimumLODIndex The minimum LOD index to use (1 means skip LOD 0, use LOD 1+).
	 * @return The number of static mesh components processed.
	 */
	static int32 ForceMinimumLODOnStaticMeshes(AActor* TargetActor, int MinimumLODIndex);

	/**
	 * @brief VR Optimization A: Disables shadow casting on static mesh components.
	 * 
	 * @param TargetActor The root actor to process.
	 * @return The number of components modified.
	 */
	static int32 DisableShadowCasting(AActor* TargetActor);

	/**
	 * @brief VR Optimization C: Disables collision on decorative meshes.
	 * 
	 * Uses heuristics to identify decorative meshes (small size, no physics, etc.).
	 * 
	 * @param TargetActor The root actor to process.
	 * @return The number of components modified.
	 */
	static int32 DisableCollisionOnDecorative(AActor* TargetActor);

	/**
	 * @brief VR Optimization D: Logs warnings about complex materials.
	 * 
	 * @param TargetActor The root actor to process.
	 * @return The number of materials checked.
	 */
	static int32 OptimizeMaterials(AActor* TargetActor);
};
