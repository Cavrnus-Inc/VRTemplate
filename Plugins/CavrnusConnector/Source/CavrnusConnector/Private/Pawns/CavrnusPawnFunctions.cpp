// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/CavrnusPawnFunctions.h"

#include "CavrnusConnectorModule.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/PrimitiveComponent.h"

void UCavrnusPawnFunctions::CavrnusUpdateMeshColor(
	TArray<UPrimitiveComponent*> MeshArray,
	const FLinearColor& NewColor,
	const float Multiplier)
{
	for (UPrimitiveComponent* Item : MeshArray)
	{
		if (IsValid(Item))
		{
			if (auto* Dmi = Item->CreateDynamicMaterialInstance(0))
			{
				Dmi->SetVectorParameterValue(TEXT("Color"), NewColor);
				Dmi->SetScalarParameterValue(TEXT("ColorMultiplier"), Multiplier);
			}
		}
	}
}

UCavrnusPawnComponent* UCavrnusPawnFunctions::CavrnusFindPawnComponent(const AActor* ActorOwner)
{
	UCavrnusPawnComponent* FoundSc = nullptr;
	if (UActorComponent* FoundAc = ActorOwner->GetComponentByClass(UCavrnusPawnComponent::StaticClass()))
		FoundSc = Cast<UCavrnusPawnComponent>(FoundAc);

	if (FoundSc == nullptr)
		UE_LOG(LogCavrnusConnector, Error, TEXT("ActorOwner is missing UCavrnusPawnComponent!"));

	return FoundSc;
}
