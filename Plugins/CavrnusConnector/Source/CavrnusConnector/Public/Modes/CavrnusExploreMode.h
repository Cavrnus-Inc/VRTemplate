// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusModeBase.h"
#include "Core/Tick/CavrnusTickableObject.h"
#include "CavrnusExploreMode.generated.h"

/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusExploreMode : public UCavrnusModeBase
{
	GENERATED_BODY()
public:
	virtual FString GetInputMappingContextName() override { return "cavrnusContext"; }
	virtual void BindInputActions(UEnhancedInputComponent* InputComponent, UCavrnusInputActionsDataAsset* Data) override;

private:
	UPROPERTY()
	UCavrnusTickableObject* TickObj;
	
	FVector2D CurrentLookDelta = FVector2D();
	FVector2D TargetLookDelta = FVector2D();

	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	UFUNCTION()
	void Fly(const FInputActionValue& Value);
};
