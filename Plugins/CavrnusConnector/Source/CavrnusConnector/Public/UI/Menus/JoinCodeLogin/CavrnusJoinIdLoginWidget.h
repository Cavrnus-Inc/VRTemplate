// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include <Styling/SlateTypes.h>

#include "UI/Systems/Popups/CavrnusBasePopupWidget.h"
#include "CavrnusJoinIdLoginWidget.generated.h"

class UCavrnusUIButton;
class UCavrnusUIInputFieldValidated;

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusJoinIdLoginWidget : public UCavrnusBasePopupWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta=(BindWidget))
	UCavrnusUIInputFieldValidated* JoinCodeInputField = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIButton> TryJoinSpaceButton = nullptr;

	virtual void NativeConstruct() override;
	
private:
	void TryJoinSpace(const FString& JoinId);
};