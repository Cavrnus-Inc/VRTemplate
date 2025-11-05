// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "CavrnusAIButton.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusAIButton : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> Button = nullptr;
	
	UFUNCTION()
	void ButtonClicked();
};