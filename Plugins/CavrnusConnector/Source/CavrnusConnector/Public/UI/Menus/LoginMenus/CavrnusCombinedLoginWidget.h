// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "CavrnusCombinedLoginWidget.generated.h"

class UCavrnusUIButton;
class UCavrnusUITabHandler;
class UOverlay;

/**
 * Combined login widget that embeds both member and guest login widgets via tabs.
 * Reuses existing widget blueprints from plugin settings.
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusCombinedLoginWidget : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Login", meta = (BindWidget))
	TObjectPtr<UCavrnusUIButton> MemberTabButton = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Login", meta = (BindWidget))
	TObjectPtr<UCavrnusUIButton> GuestTabButton = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Login", meta = (BindWidget))
	TObjectPtr<UOverlay> LoginContentArea = nullptr;

private:
	UPROPERTY()
	TObjectPtr<UCavrnusUITabHandler> TabHandler = nullptr;

	UPROPERTY()
	TObjectPtr<UUserWidget> MemberWidget = nullptr;

	UPROPERTY()
	TObjectPtr<UUserWidget> GuestWidget = nullptr;
};
