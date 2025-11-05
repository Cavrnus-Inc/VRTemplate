// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "CavrnusBaseUIMessageWidget.generated.h"

class UCavrnusUIButton;
/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class CAVRNUSCONNECTOR_API UCavrnusBaseUIMessageWidget : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIButton> CloseButton = nullptr;

	virtual void NativeDestruct() override;

public:
	UCavrnusBaseUIMessageWidget* HookOnClose(const TFunction<void()>& OnClose);

	UFUNCTION(BlueprintCallable, Category="Cavrnus|UI|Messages")
	void Close();
	
private:
	FDelegateHandle CloseButtonHandle = FDelegateHandle();
	TFunction<void()> CachedOnClose;
};