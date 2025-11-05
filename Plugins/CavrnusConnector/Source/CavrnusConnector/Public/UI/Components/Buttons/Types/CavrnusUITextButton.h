// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "CavrnusUITextButton.generated.h"

class UCavrnusUITextBlock;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUITextButton : public UCavrnusUIButton
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Cavrnus")
	void SetButtonText(const FText& NewText);
	
protected:
	FString LocallySetText = "";
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	FText ButtonText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCavrnusUITextBlock> TextBlock = nullptr;
	
	virtual void SynchronizeProperties() override;
	virtual void ApplyContentStyle(const FCavrnusUIButtonStyleState& NewStyle) override;
};