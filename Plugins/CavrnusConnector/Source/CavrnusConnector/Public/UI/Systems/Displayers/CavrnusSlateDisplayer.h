// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusWidgetDisplayer.h"
#include "UObject/Object.h"
#include "CavrnusSlateDisplayer.generated.h"

class UCavrnusBaseUserWidget;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusSlateDisplayer : public UObject, public ICavrnusWidgetDisplayer
{
	GENERATED_BODY()
public:
	virtual void Setup(UCavrnusWidgetBlueprintLookup* InBlueprintLookup = nullptr) override;
	
	virtual void DisplayPopupWidget(
		UCavrnusBasePopupWidget* InWidgetToDisplay,
		const FCavrnusPopupOptions& Options,
		const TFunction<void(UCavrnusBasePopupWidget*)>& OnWidgetClosed = nullptr) override;

	virtual void DisplayDialogWidget(
		UCavrnusBaseDialogWidget* InWidgetToDisplay,
		const FCavrnusDialogOptions& Options,
		const TFunction<void(UCavrnusBaseDialogWidget*)>& OnWidgetClosed = nullptr) override;

	virtual void RemoveWidget(const FGuid& Id) override;
	virtual void RemoveAll() override;

private:
	TArray<TSharedPtr<SWindow>> AllSlateWindows;
	TMap<FGuid, TSharedPtr<SWindow>> ActiveSlateWindows;
};
