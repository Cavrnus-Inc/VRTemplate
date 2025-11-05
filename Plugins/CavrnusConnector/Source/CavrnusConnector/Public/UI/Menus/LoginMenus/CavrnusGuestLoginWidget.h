// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "CavrnusGuestLoginWidget.generated.h"

class UCavrnusUIButton;
class UCavrnusUIInputFieldValidated;

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusGuestLoginWidget : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
public:
	UFUNCTION()
	void TryJoin();

	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Login", meta = (BindWidget))
	TObjectPtr<UCavrnusUIInputFieldValidated> GuestUsernameInput = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Login", meta = (BindWidget))
	TObjectPtr<UCavrnusUIButton> LoginButton = nullptr;

private:
	FDelegateHandle LoginButtonDelegateHandle = FDelegateHandle();
};