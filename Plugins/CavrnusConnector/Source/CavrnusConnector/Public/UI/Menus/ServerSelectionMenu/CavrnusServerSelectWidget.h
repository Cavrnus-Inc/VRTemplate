// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CavrnusServerSelectWidget.generated.h"

class UCavrnusUIButton;
class UCavrnusUIInputFieldValidated;

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusServerSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|ServerSelection", meta = (BindWidget))
	TObjectPtr<UCavrnusUIInputFieldValidated> ServerTextBox = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|ServerSelection", meta = (BindWidget))
	TObjectPtr<UCavrnusUIButton> ProceedButton = nullptr;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	FDelegateHandle ProceedButtonDelegate = FDelegateHandle();
};