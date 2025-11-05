// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusTextBlockSize.h"
#include "Components/TextBlock.h"
#include "UI/Styles/CavrnusUIThemeAsset.h"
#include "UI/Styles/Text/CavrnusUITextStyle.h"
#include "CavrnusUITextBlock.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Cavrnus TextBlock")
class CAVRNUSCONNECTOR_API UCavrnusUITextBlock : public UTextBlock
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category="Style")
	ECavrnusThemeColorRole ColorRole = ECavrnusThemeColorRole::Surface; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus")
	ECavrnusTextBlockSize TextSize = ECavrnusTextBlockSize::Custom;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TObjectPtr<UCavrnusUITextStyle> Style;

	virtual void SynchronizeProperties() override;
	
public:

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
	
};