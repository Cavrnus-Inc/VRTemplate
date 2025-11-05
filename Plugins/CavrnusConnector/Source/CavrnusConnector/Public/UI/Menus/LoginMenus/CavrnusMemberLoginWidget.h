// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "UI/CavrnusBaseUserWidget.h"
#include "CavrnusMemberLoginWidget.generated.h"

class UCavrnusUIButton;
class UCavrnusUIInputFieldValidated;

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusMemberLoginWidget : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void TryLogin();

	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Login", meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUIInputFieldValidated> EmailInputField = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Login", meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUIInputFieldValidated> PasswordInputField = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Login", meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUIButton> LoginButton = nullptr;

private:
	FDelegateHandle LoginButtonDelegateHandle = FDelegateHandle();
};
