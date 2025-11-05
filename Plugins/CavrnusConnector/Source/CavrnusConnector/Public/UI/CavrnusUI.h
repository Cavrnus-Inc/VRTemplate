// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/CavrnusUISubsystem.h"
#include "CavrnusUI.generated.h"

class UCavrnusUISubsystem;

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUI : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|UI")
	static UCavrnusUISubsystem* Get(const UObject* WorldContextObject = nullptr);
};