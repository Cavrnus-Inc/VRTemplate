// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetManager/CavrnusBaseDataAsset.h"  // Changed from Engine/DataAsset.h
#include "UI/Systems/AssetLookup/CavrnusWidgetBlueprintLookup.h"
#include "CavrnusUIConfigAsset.generated.h"

class UCavrnusDesktopCanvasWidgetDisplayer;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUIConfigAsset : public UCavrnusBaseDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category="Cavrnus|UIConfig")
	TSoftObjectPtr<UCavrnusWidgetBlueprintLookup> MenuLookupObject;

	UPROPERTY(EditAnywhere, Category="Cavrnus|UIConfig")
	TSubclassOf<UCavrnusDesktopCanvasWidgetDisplayer> CanvasDisplayer;
};