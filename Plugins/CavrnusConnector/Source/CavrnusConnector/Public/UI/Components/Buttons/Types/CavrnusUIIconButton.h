// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "CavrnusUIIconButton.generated.h"

class UCavrnusUIImage;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUIIconButton : public UCavrnusUIButton
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TObjectPtr<UTexture2D> ButtonImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCavrnusUIImage> Icon = nullptr;

	virtual void SynchronizeProperties() override;
	virtual void ApplyContentStyle(const FCavrnusUIButtonStyleState& NewStyle) override;
};