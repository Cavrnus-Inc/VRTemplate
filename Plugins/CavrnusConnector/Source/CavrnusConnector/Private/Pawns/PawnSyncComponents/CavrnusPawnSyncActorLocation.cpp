// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/PawnSyncComponents/CavrnusPawnSyncActorLocation.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "Pawns/CavrnusPawnComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/SceneComponent.h"
#include "LivePropertyUpdates/CavrnusLiveTransformPropertyUpdate.h"
#include "Helpers/CavrnusMathHelpers.h"

UCavrnusPawnSyncActorLocation::UCavrnusPawnSyncActorLocation()
	: TransformPropertyName("Location")
	, MaxTimeToLocationSeconds(5.0f)
	, ReachThresholdDistance(100.0f)
	, MovementUpdateThreshold(50.0f)
	, bIsMovingToLocation(false)
{
}

void UCavrnusPawnSyncActorLocation::Initialize(
	UCavrnusPawnComponent* PawnSetupComponent, 
	const FString& InTransformPropertyName,
	float InMaxTimeToLocationSeconds,
	float InReachThresholdDistance,
	float InMovementUpdateThreshold)
{
	TransformPropertyName = InTransformPropertyName;
	MaxTimeToLocationSeconds = InMaxTimeToLocationSeconds;
	ReachThresholdDistance = InReachThresholdDistance;
	MovementUpdateThreshold = InMovementUpdateThreshold;
	InitializePawnSetupComponent(PawnSetupComponent);
}

void UCavrnusPawnSyncActorLocation::Teardown()
{
	if (bTornDown)
		return;

	// Clear timeout timer
	if (TargetPawn && TargetPawn->GetWorld())
	{
		TargetPawn->GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}

	// Unbind from root component TransformUpdated delegate
	if (RootComponentProxy && RootComponentTransformDelegateHandle.IsValid())
	{
		RootComponentProxy->TransformUpdated.Remove(RootComponentTransformDelegateHandle);
		RootComponentTransformDelegateHandle.Reset();
	}

	// Cancel and clear TransformUpdater
	if (IsValid(TransformUpdater))
	{
		TransformUpdater->Cancel();
		TransformUpdater = nullptr;
	}

	AIController = nullptr;
	TargetPawn = nullptr;
	bIsMovingToLocation = false;
	RootComponentProxy = nullptr;
	LastSentTransform = FTransform();

	Super::Teardown();
}

void UCavrnusPawnSyncActorLocation::HandleLocalSync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& ContainerName,
	const FCavrnusUser& CavrnusUser)
{
	Super::HandleLocalSync(SpaceConnection, ContainerName, CavrnusUser);
	
	CachedSpaceConnection = SpaceConnection;
	CachedContainerName = ContainerName;
	
	// Only work on local pawns
	if (!PawnSetupComp)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] PawnSetupComp is null! Cannot sync location."));
		return;
	}
	
	// Get the pawn from the component's owner
	TargetPawn = Cast<APawn>(PawnSetupComp->GetOwner());
	if (!TargetPawn)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] Owner is not a Pawn! Owner: %s"), 
			PawnSetupComp->GetOwner() ? *PawnSetupComp->GetOwner()->GetName() : TEXT("null"));
		return;
	}
	
	// Get the root component as a proxy to detect actor movement
	RootComponentProxy = Cast<USceneComponent>(TargetPawn->GetRootComponent());
	if (!RootComponentProxy)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] Pawn %s does not have a valid root SceneComponent! Cannot sync actor transform."), 
			*TargetPawn->GetName());
		return;
	}
	
	// Initialize TransformUpdater with current actor transform
	FTransform InitialActorTransform = TargetPawn->GetActorTransform();
	TransformUpdater = UCavrnusFunctionLibrary::BeginTransientTransformPropertyUpdate(
		SpaceConnection,
		ContainerName,
		TransformPropertyName,
		InitialActorTransform,
		FPropertyPostOptions());
	
	if (!TransformUpdater)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] Failed to create TransformUpdater for pawn %s"), *TargetPawn->GetName());
		return;
	}
	
	// Store initial transform
	LastSentTransform = InitialActorTransform;
	
	// Bind to root component's TransformUpdated delegate as a proxy
	// When root component moves, we'll send the Actor's transform instead
	RootComponentTransformDelegateHandle = RootComponentProxy->TransformUpdated.AddLambda([this](USceneComponent* SceneComponent, EUpdateTransformFlags UpdateFlags, ETeleportType TeleportType)
		{
			OnRootComponentTransformUpdated(SceneComponent, UpdateFlags, TeleportType);
		});
	
	UE_LOG(LogCavrnusConnector, Log, TEXT("[CavrnusPawnSyncActorLocation] Started local sync for pawn %s using root component as proxy"), *TargetPawn->GetName());
}

void UCavrnusPawnSyncActorLocation::OnRootComponentTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateFlags, ETeleportType TeleportType)
{
	if (!IsValid(SceneComponent))
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[CavrnusPawnSyncActorLocation] SceneComponent is null in OnRootComponentTransformUpdated!"));
		return;
	}
	
	if (!IsValid(TransformUpdater))
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[CavrnusPawnSyncActorLocation] TransformUpdater is null!"));
		return;
	}
	
	if (!TargetPawn)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[CavrnusPawnSyncActorLocation] TargetPawn is null!"));
		return;
	}
	
	// Get the Actor's transform (not the component transform)
	const FTransform ActorTransform = TargetPawn->GetActorTransform();
	
	// Compare with last sent transform
	if (!FCavrnusMathHelpers::AreTransformsApproximatelyEqual(ActorTransform, LastSentTransform))
	{
		// Send the Actor Transform (not component transform)
		TransformUpdater->UpdateWithNewData(ActorTransform);
		LastSentTransform = ActorTransform;
	}
}

void UCavrnusPawnSyncActorLocation::HandleRemoteSync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& ContainerName,
	const FCavrnusUser& CavrnusUser)
{
	Super::HandleRemoteSync(SpaceConnection, ContainerName, CavrnusUser);
	
	CachedSpaceConnection = SpaceConnection;
	CachedContainerName = ContainerName;
	
	// Only work on remote pawns
	if (!PawnSetupComp)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] PawnSetupComp is null! Cannot sync location."));
		return;
	}
	
	// Get the pawn from the component's owner
	TargetPawn = Cast<APawn>(PawnSetupComp->GetOwner());
	if (!TargetPawn)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] Owner is not a Pawn! Owner: %s"), 
			PawnSetupComp->GetOwner() ? *PawnSetupComp->GetOwner()->GetName() : TEXT("null"));
		return;
	}
	
	// Validate all requirements before proceeding
	if (!ValidateRequirements())
	{
		return;
	}
	
	// Ensure we have an AI Controller
	EnsureAIController();
	
	if (!AIController)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] Failed to get or create AI Controller for pawn %s"), *TargetPawn->GetName());
		return;
	}
	
	// Bind to the Transform property and extract location from it
	BindingIds.Add(UCavrnusFunctionLibrary::BindTransformPropertyValue(
		SpaceConnection,
		ContainerName,
		TransformPropertyName,
		[this](const UE::Math::TTransform<double>& Transform, const FString&, const FString&)
		{
			// Extract location from Transform
			FVector TargetLocation = Transform.GetTranslation();
			
			// Check if we should update movement based on distance threshold
			if (bIsMovingToLocation)
			{
				const float DistanceToCurrentTarget = FVector::Dist(CurrentTargetLocation, TargetLocation);
				if (DistanceToCurrentTarget < MovementUpdateThreshold)
				{
					// New location is too close to current target, ignore to avoid stuttering
					return;
				}
			}
			
			// Update movement to new location
			MoveToLocation(TargetLocation);
		})->BindingId);
	
	// Bind to AvatarVis property - when it becomes visible, teleport to Transform location
	BindingIds.Add(UCavrnusFunctionLibrary::BindBooleanPropertyValue(
		SpaceConnection,
		ContainerName,
		"AvatarVis",
		[this](const bool bIsVisible, const FString&, const FString&)
		{
			HandleAvatarVisChanged(bIsVisible);
		})->BindingId);
}

bool UCavrnusPawnSyncActorLocation::ValidateRequirements()
{
	if (!TargetPawn)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] TargetPawn is null!"));
		return false;
	}
	
	// Check for Navigation System
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(TargetPawn->GetWorld());
	if (!NavSys)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] Navigation system is not available! Pawn: %s. Ensure NavMeshBoundsVolume exists in the level."), 
			*TargetPawn->GetName());
		return false;
	}
	
	// Check for Movement Component
	UPawnMovementComponent* MovementComp = TargetPawn->GetMovementComponent();
	if (!MovementComp)
	{
		// Try to get CharacterMovementComponent as well
		if (ACharacter* Character = Cast<ACharacter>(TargetPawn))
		{
			MovementComp = Character->GetCharacterMovement();
		}
		
		if (!MovementComp)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] Pawn %s does not have a movement component! AI movement requires a UPawnMovementComponent or UCharacterMovementComponent."), 
				*TargetPawn->GetName());
			return false;
		}
	}
	
	return true;
}

void UCavrnusPawnSyncActorLocation::EnsureAIController()
{
	if (!TargetPawn)
		return;
	
	// Check if pawn already has an AI Controller
	AIController = Cast<AAIController>(TargetPawn->GetController());
	
	if (!AIController)
	{
		// Spawn an AI Controller for the pawn
		FActorSpawnParameters SpawnParams;
		// Don't set Owner - AI Controller doesn't need an owner, it controls the pawn via Possess()
		// Setting Owner to the pawn would create a circular ownership relationship
		SpawnParams.Instigator = TargetPawn->GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		UWorld* World = TargetPawn->GetWorld();
		if (!World)
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] Cannot get World from pawn %s!"), *TargetPawn->GetName());
			return;
		}
		
		AIController = World->SpawnActor<AAIController>(AAIController::StaticClass(), SpawnParams);
		
		if (AIController)
		{
			// Possess the pawn with the AI Controller
			AIController->Possess(TargetPawn);
			UE_LOG(LogCavrnusConnector, Log, TEXT("[CavrnusPawnSyncActorLocation] Created and possessed AI Controller for pawn %s"), *TargetPawn->GetName());
		}
		else
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("[CavrnusPawnSyncActorLocation] Failed to spawn AI Controller for pawn %s"), *TargetPawn->GetName());
		}
	}
}

void UCavrnusPawnSyncActorLocation::MoveToLocation(const FVector& TargetLocation)
{
	if (!AIController || !TargetPawn)
		return;
	
	// Check if navigation system is available
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(TargetPawn->GetWorld());
	if (!NavSys)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[CavrnusPawnSyncActorLocation] Navigation system not available when trying to move!"));
		return;
	}
	
	// Check if target location is on navmesh
	FNavLocation ProjectedLocation;
	const bool bIsOnNavMesh = NavSys->ProjectPointToNavigation(TargetLocation, ProjectedLocation, FVector(500.0f, 500.0f, 500.0f));
	if (!bIsOnNavMesh)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[CavrnusPawnSyncActorLocation] Target location %s is not on navmesh! Pawn: %s"), 
			*TargetLocation.ToString(), *TargetPawn->GetName());
		// Still try to move, but it may fail
	}
	
	// Stop any existing movement
	if (bIsMovingToLocation)
	{
		AIController->StopMovement();
	}
	
	// Clear existing timeout timer
	if (TargetPawn->GetWorld())
	{
		TargetPawn->GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}
	
	// Update current target
	CurrentTargetLocation = TargetLocation;
	bIsMovingToLocation = true;
	
	// Start movement to target location
	AIController->MoveToLocation(TargetLocation);
	
	// Set timeout timer
	if (TargetPawn->GetWorld())
	{
		FTimerDelegate TimeoutDelegate;
		TimeoutDelegate.BindUObject(this, &UCavrnusPawnSyncActorLocation::OnTimeoutExpired);
		TargetPawn->GetWorld()->GetTimerManager().SetTimer(TimeoutTimerHandle, TimeoutDelegate, MaxTimeToLocationSeconds, false);
	}
}

void UCavrnusPawnSyncActorLocation::OnTimeoutExpired()
{
	if (!TargetPawn || !AIController)
		return;
	
	// Check if we've reached the target location
	const float DistanceToTarget = FVector::Dist(TargetPawn->GetActorLocation(), CurrentTargetLocation);
	
	if (DistanceToTarget > ReachThresholdDistance)
	{
		// Haven't reached target, teleport
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[CavrnusPawnSyncActorLocation] Pawn %s did not reach target location within %f seconds (distance: %f). Teleporting to target."), 
			*TargetPawn->GetName(), MaxTimeToLocationSeconds, DistanceToTarget);
		
		// Stop movement
		AIController->StopMovement();
		
		// Teleport to target location
		TargetPawn->SetActorLocation(CurrentTargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
	
	bIsMovingToLocation = false;
}

void UCavrnusPawnSyncActorLocation::HandleAvatarVisChanged(bool bIsVisible)
{
	if (bIsVisible)
	{
		// When avatar becomes visible, teleport to current Transform property location
		TeleportToTransformLocation();
	}
}

void UCavrnusPawnSyncActorLocation::TeleportToTransformLocation()
{
	if (!TargetPawn)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[CavrnusPawnSyncActorLocation] Cannot teleport: TargetPawn is null"));
		return;
	}
	
	// Get current Transform property value
	FTransform CurrentTransform = UCavrnusFunctionLibrary::GetTransformPropertyValue(
		CachedSpaceConnection,
		CachedContainerName,
		TransformPropertyName);
	
	// Extract location from transform
	FVector TransformLocation = CurrentTransform.GetLocation();
	
	// Stop any ongoing movement
	if (AIController && bIsMovingToLocation)
	{
		AIController->StopMovement();
		bIsMovingToLocation = false;
	}
	
	// Clear timeout timer
	if (TargetPawn->GetWorld())
	{
		TargetPawn->GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}
	
	// Teleport to transform location
	TargetPawn->SetActorLocation(TransformLocation, false, nullptr, ETeleportType::TeleportPhysics);
	
	// Also update rotation if needed (optional, but might be useful)
	TargetPawn->SetActorRotation(CurrentTransform.GetRotation());
	
	UE_LOG(LogCavrnusConnector, Log, TEXT("[CavrnusPawnSyncActorLocation] Teleported pawn %s to Transform property location: %s"), 
		*TargetPawn->GetName(), *TransformLocation.ToString());
}

