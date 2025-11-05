// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusModeBase.h"
#include "UI/Systems/Screens/Types/ScreenCapture/CavrnusScreenCaptureModeWidget.h"
#include "CavrnusSceneCaptureMode.generated.h"

/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusSceneCaptureMode : public UCavrnusModeBase
{
	GENERATED_BODY()
public:
	virtual FString GetInputMappingContextName() override { return "sceneCaptureContext";}
	virtual void BindInputActions(UEnhancedInputComponent* InputComponent, UCavrnusInputActionsDataAsset* Data) override;
	virtual void EnterMode(UWorld* World, const int32 Priority = 0) override;
	virtual void ExitMode() override;

private:
	UFUNCTION()
	void PrimaryClick(const FInputActionValue& Value);
	UFUNCTION()
	void Back(const FInputActionValue& Value);
	
	UPROPERTY()
	TObjectPtr<UCavrnusScreenCaptureModeWidget> Screen;
};
