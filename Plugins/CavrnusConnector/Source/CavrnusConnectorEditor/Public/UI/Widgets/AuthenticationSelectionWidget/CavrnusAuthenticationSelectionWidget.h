// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusConnectorSettings.h"
#include "Blueprint/UserWidget.h"
#include "UI/Components/Dropdowns/CavrnusUIDropdownWidget.h"
#include "CavrnusAuthenticationSelectionWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOREDITOR_API UCavrnusAuthenticationSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void Setup();

protected:
	virtual void NativeDestruct() override;

private:
	FDelegateHandle AuthChangedHandle = FDelegateHandle();
	FDelegateHandle LevelBlueprintButtonHandle = FDelegateHandle();

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIDropdownWidget> AuthenticationDropdown = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIButton> LevelBlueprintButton = nullptr;

	void AuthSelected(const ECavrnusAuthMethodForPIE& AuthMethod);
};
