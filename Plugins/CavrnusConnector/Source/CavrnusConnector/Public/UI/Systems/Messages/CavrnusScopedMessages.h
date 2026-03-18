// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/DisposableUObject.h"
#include "UI/CavrnusUIConfigAsset.h"
#include "CavrnusScopedMessages.generated.h"

class ICavrnusWidgetDisplayer;
class UCavrnusToastMessageUISystem;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusScopedMessages : public UDisposableUObject
{
	GENERATED_BODY()
public:
	void Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer);
	
	UCavrnusToastMessageUISystem* Toast() const { return ToastMessages; }

private:
	UPROPERTY()
	TObjectPtr<UCavrnusToastMessageUISystem> ToastMessages = nullptr;
};