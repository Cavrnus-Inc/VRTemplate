// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "UObject/Object.h"
#include "InputMappingContext.h" 
#include "AssetManager/DataAssets/CavrnusInputActionsDataAsset.h"
#include "GameFramework/PlayerController.h"
#include "CavrnusModeBase.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusModeBase : public UObject
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	TWeakObjectPtr<APlayerController> PlayerController;
	UPROPERTY()
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem;
	UPROPERTY()
	TObjectPtr<UInputMappingContext> InputMapCtx;
	UPROPERTY()
	TObjectPtr<UCavrnusInputActionsDataAsset> CavInputAsset;
	
	TArray<FEnhancedInputActionEventBinding*> InputBindings;

public:
	bool IsExplicitMode = false;
	
	virtual void EnterMode(UWorld* World, const int32 Priority = 0);
	virtual void ExitMode();

protected:
	virtual FString GetInputMappingContextName();
	virtual void BindInputActions(UEnhancedInputComponent* InputComponent, UCavrnusInputActionsDataAsset* Data);
	UInputAction* GetInputAction(const FString& InputActionName) const;

	APawn* GetPawn() const
	{
		if (PlayerController != nullptr)
			return PlayerController->GetPawn();

		return nullptr;
	}
};
