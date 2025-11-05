// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "CavrnusBasePopupWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusBasePopupWidget : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()
public:
	virtual FText GetTitle();
};