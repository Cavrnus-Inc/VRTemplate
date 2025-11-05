// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageWidget.h"
#include "CavrnusProgressToastMessageWidget.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class CAVRNUSCONNECTOR_API UCavrnusProgressToastMessageWidget : public UCavrnusInfoToastMessageWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Cavrnus|UI|Messages")
	UCavrnusProgressToastMessageWidget* SetProgress(const float InProgress);

protected:
	virtual void NativeConstruct() override;
};
