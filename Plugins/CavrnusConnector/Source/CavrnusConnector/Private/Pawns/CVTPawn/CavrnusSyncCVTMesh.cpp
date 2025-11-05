// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/CVTPawn/CavrnusSyncCVTMesh.h"

#include "CavrnusFunctionLibrary.h"
#include "Helpers/CavrnusMathHelpers.h"
#include "Pawns/CavrnusPawnFunctions.h"
#include "Pawns/CavrnusPawnSpeed.h"
#include "Pawns/CVTPawn/CavrnusCVTWalkAnim.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UCavrnusSyncCVTMesh::UCavrnusSyncCVTMesh()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCavrnusSyncCVTMesh::BeginPlay()
{
	Super::BeginPlay();

	if (Pawn)
	{
		UE_LOG(LogTemp, Log, TEXT("BeginPlay: Pawn is valid: %s"), *Pawn->GetName());

		if (USkeletalMeshComponent* Sm = Pawn->FindComponentByClass<USkeletalMeshComponent>())
		{
			UE_LOG(LogTemp, Log, TEXT("Found SkeletalMeshComponent: %s"), *Sm->GetName());

			if (auto* AnimInstance = Sm->GetAnimInstance())
			{
				UE_LOG(LogTemp, Log, TEXT("Got AnimInstance: %s"), *AnimInstance->GetClass()->GetName());

				if (auto* FoundWalkAnim = Cast<UCavrnusCVTWalkAnim>(AnimInstance))
				{
					WalkAnim = FoundWalkAnim;
					MeshArray.Add(Sm);
					
					UE_LOG(LogTemp, Log, TEXT("Successfully cast to UCavrnusCVTWalkAnim"));
				}
				else
					UE_LOG(LogTemp, Warning, TEXT("AnimInstance is NOT of type UCavrnusCVTWalkAnim (it’s %s)"), *AnimInstance->GetClass()->GetName());
			}
			else
				UE_LOG(LogTemp, Warning, TEXT("No AnimInstance found on SkeletalMeshComponent"));
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("Pawn %s has no SkeletalMeshComponent"), *Pawn->GetName());
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Pawn is null in UCavrnusSyncCVTMesh::BeginPlay"));
}

void UCavrnusSyncCVTMesh::HandleLocalTick_Implementation(float DeltaTime)
{
	Super::HandleLocalTick_Implementation(DeltaTime);

	// This handles dampening the speed which is sent to the walk anim blending
	if (IsValid(PawnSpeed) && IsValid(WalkAnim))
	{
		PawnSpeed->UpdateTick(DeltaTime);
		WalkAnim->SetSpeed(PawnSpeed->GetSpeed());
	}

	// Head Updater
	if (IsValid(HeadUpdater))
	{
		const auto HeadRotationVector = FCavrnusMathHelpers::RotatorToVector4(Pawn->GetControlRotation());
		HeadUpdater->UpdateWithNewData(HeadRotationVector);
	}
}

void UCavrnusSyncCVTMesh::HandleRemoteTick_Implementation(float DeltaTime)
{
	Super::HandleRemoteTick_Implementation(DeltaTime);

	// This handles dampening the speed which is sent to the walk anim blending
	if (IsValid(PawnSpeed) && IsValid(WalkAnim))
	{
		PawnSpeed->UpdateTick(DeltaTime);
		WalkAnim->SetSpeed(PawnSpeed->GetSpeed());
	}
}

void UCavrnusSyncCVTMesh::HandleAnyPawnSync_Implementation(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserContainerName,
	const FCavrnusUser& CavrnusUser)
{
	Super::HandleAnyPawnSync_Implementation(SpaceConnection, UserContainerName, CavrnusUser);

	if (!IsValid(WalkAnim))
		return;

	// PrimaryColor
	BindingIds.Add(UCavrnusFunctionLibrary::BindColorPropertyValue(
	SpaceConnection,
	UserContainerName,
	"primaryColor",[this](const FLinearColor& Val, const FString&, const FString&)
	{
		UCavrnusPawnFunctions::CavrnusUpdateMeshColor(MeshArray, Val, 0.25f);
	})->BindingId);
	
	PawnSpeed = NewObject<UCavrnusPawnSpeed>(GetWorld());
	PawnSpeed->Initialize(Pawn);
}

void UCavrnusSyncCVTMesh::HandleLocalSync_Implementation(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserContainerName,
	const FCavrnusUser& CavrnusUser)
{
	Super::HandleLocalSync_Implementation(SpaceConnection, UserContainerName, CavrnusUser);

	if (!IsValid(WalkAnim))
		return;

	// HeadRotation
	const auto HeadVector = FCavrnusMathHelpers::RotatorToVector4(Pawn->GetControlRotation());
	HeadUpdater = UCavrnusFunctionLibrary::BeginTransientVectorPropertyUpdate(SpaceConnection, UserContainerName, "HeadRotation", HeadVector);
	Updaters.Add(HeadUpdater);

	bDoLocalTick = true;
}

void UCavrnusSyncCVTMesh::HandleRemoteSync_Implementation(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserContainerName,
	const FCavrnusUser& CavrnusUser)
{
	Super::HandleRemoteSync_Implementation(SpaceConnection, UserContainerName, CavrnusUser);
	
	if (!IsValid(WalkAnim))
		return;

	// HeadRotation
	BindingIds.Add(UCavrnusFunctionLibrary::BindVectorPropertyValue(
		SpaceConnection,
		UserContainerName,
		"HeadRotation",
		[this](const UE::Math::TVector4<double>& Val, const FString&, const FString&)
		{
			WalkAnim->SetHeadLookAt(Val);
		})->BindingId);

	bDoRemoteTick = true;
}