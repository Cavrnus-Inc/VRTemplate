// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "UI/Styles/CavrnusUIThemeAsset.h"
#include "CavrnusUIBorder.generated.h"


UCLASS(DisplayName="Cavrnus Border")
class CAVRNUSCONNECTOR_API UCavrnusUIBorder : public UBorder
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="Style")
	ECavrnusThemeColorRole ColorRole = ECavrnusThemeColorRole::Surface; 
	
	virtual void SynchronizeProperties() override;

public:
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
};
