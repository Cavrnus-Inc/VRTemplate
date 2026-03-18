// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/Systems/Panels/CavrnusPanelLocation.h"
#include "CavrnusUI.generated.h"

class UCavrnusUISystems;
class UCavrnusToolbarPanelWidget;

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUI : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|UI")
	static UCavrnusUISystems* Get(const UObject* WorldContextObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Cavrnus|UI|Panels", meta = (CallInEditor = "true"))
	static UCavrnusToolbarPanelWidget* CreateToolbarPanel(UObject* WorldContextObject, EPanelLocation Location = EPanelLocation::LeftMiddle);
};