// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/PawnSyncComponents/CavrnusPawnSyncTransform.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "Engine/Texture2D.h"
#include "Helpers/CavrnusMathHelpers.h"

UCavrnusPawnSyncTransform::UCavrnusPawnSyncTransform()
{
}

void UCavrnusPawnSyncTransform::Initialize(
	UCavrnusPawnComponent* PawnSetupComponent,
	bool bRelativeTransform,
	const FString& PropertyName,
	USceneComponent* Target)
{
	FTransformSyncTarget Obj = FTransformSyncTarget();
	Obj.PropertyName = PropertyName;
	Obj.TargetComponent = Target;

	bIsRelativeTransform = bRelativeTransform;
	SyncTargets.Add(Obj);
	InitializePawnSetupComponent(PawnSetupComponent);
}

void UCavrnusPawnSyncTransform::InitializeMany(
	UCavrnusPawnComponent* PawnSetupComponent,
	const TArray<FTransformSyncTarget>& Targets)
{
	SyncTargets = Targets;
	InitializePawnSetupComponent(PawnSetupComponent);
}

void UCavrnusPawnSyncTransform::Teardown()
{
	if (bTornDown)
		return;

	for (const auto Target : SyncTargets)
	{
		if (IsValid(Target.TargetComponent))
			Target.TargetComponent->TransformUpdated.Clear();
	}

	if (IsValid(TransformUpdater))
	{
		TransformUpdater->Cancel();
		TransformUpdater = nullptr;
	}

	Super::Teardown();
}

void UCavrnusPawnSyncTransform::HandleLocalSync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& ContainerName,
	const FCavrnusUser& CavrnusUser)
{
	Super::HandleLocalSync(SpaceConnection, ContainerName, CavrnusUser);

	for (int32 Idx = 0; Idx < SyncTargets.Num(); ++Idx)
	{
		FTransformSyncTarget& Target = SyncTargets[Idx];

		if (!IsValid(Target.TargetComponent))
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("[HandleLocalSync] SyncTarget[%d] for property '%s' has invalid TargetComponent -- skipping"), Idx, *Target.PropertyName);
			continue;
		}

		TransformUpdater = UCavrnusFunctionLibrary::BeginTransientTransformPropertyUpdate(
			SpaceConnection,
			ContainerName,
			Target.PropertyName,
			Target.TargetComponent->GetComponentTransform(),
			FPropertyPostOptions());

		Target.TargetComponent->TransformUpdated.AddLambda([this, Idx](USceneComponent* SceneComponent, EUpdateTransformFlags, ETeleportType)
			{
				if (!IsValid(SceneComponent))
				{
					UE_LOG(LogCavrnusConnector, Warning, TEXT("SceneComponent is null!"));
					return;
				}

				if (!IsValid(TransformUpdater))
				{
					UE_LOG(LogCavrnusConnector, Warning, TEXT("TransformUpdater is null!"));
					return;
				}

				if (!SyncTargets.IsValidIndex(Idx))
					return;

				FTransformSyncTarget& SyncTarget = SyncTargets[Idx];

				const FTransform NewTransform = bIsRelativeTransform
				                                ? SceneComponent->GetRelativeTransform()
				                                : SceneComponent->GetComponentTransform();

				const FTransform& LastTransform = bIsRelativeTransform
				   ? SyncTarget.LastRelativeTransform
				   : SyncTarget.LastWorldTransform;

				if (!FCavrnusMathHelpers::AreTransformsApproximatelyEqual(NewTransform, LastTransform))
				{
					TransformUpdater->UpdateWithNewData(NewTransform);

					if (bIsRelativeTransform)
						SyncTarget.LastRelativeTransform = NewTransform;
					else
						SyncTarget.LastWorldTransform = NewTransform;
				}
			});
	}
}

void UCavrnusPawnSyncTransform::HandleRemoteSync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& ContainerName,
	const FCavrnusUser& CavrnusUser)
{
	Super::HandleRemoteSync(SpaceConnection, ContainerName, CavrnusUser);

	for (const auto Target : SyncTargets)
	{
		if (!IsValid(Target.TargetComponent))
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("[HandleRemoteSync] SyncTarget for property %s is invalid"), *Target.PropertyName);
			continue;
		}

		BindingIds.Add(UCavrnusFunctionLibrary::BindTransformPropertyValue(
			SpaceConnection,
			ContainerName,
			Target.PropertyName,
			[Target, this](const UE::Math::TTransform<double>& Transform, const FString&, const FString&)
			{
				if (IsValid(Target.TargetComponent))
				{
					if (bIsRelativeTransform)
						Target.TargetComponent->SetRelativeTransform(Transform);
					else
						Target.TargetComponent->SetWorldTransform(Transform);
				}
			})->BindingId);
	}
}