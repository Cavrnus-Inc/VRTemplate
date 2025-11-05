// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "AssetManager/CavrnusBaseDataAsset.h"
#include "CavrnusInputActionsDataAsset.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusInputActionsDataAsset : public UCavrnusBaseDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category="Cavrnus|Input")
	TMap<FString, UInputAction*> InputActions;

	UPROPERTY(EditDefaultsOnly, Category="Cavrnus|Input")
	TMap<FString, UInputMappingContext*> InputContexts;

	UInputAction* GetAction(const FString& Name) const
	{
		if (UInputAction* const* Found = InputActions.Find(Name))
			return *Found;
		return nullptr;
	}

	UInputMappingContext* GetContext(const FString& Name) const
	{
		if (UInputMappingContext* const* Found = InputContexts.Find(Name))
			return *Found;
		return nullptr;
	}
};
