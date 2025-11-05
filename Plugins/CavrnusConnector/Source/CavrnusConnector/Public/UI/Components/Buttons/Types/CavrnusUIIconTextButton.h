// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "CavrnusUIIconTextButton.generated.h"

class UCavrnusUITextBlock;
class UCavrnusUIImage;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUIIconTextButton : public UCavrnusUIButton
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Cavrnus")
	void SetButtonText(const FText& NewText);
	
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TObjectPtr<UTexture2D> ButtonImage;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	FText ButtonText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCavrnusUIImage> Icon = nullptr;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCavrnusUITextBlock> TextBlock = nullptr;

	virtual void SynchronizeProperties() override;
	virtual void ApplyContentStyle(const FCavrnusUIButtonStyleState& NewStyle) override;
};