// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusUIScrimSystem.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "CavrnusBaseScrimWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusBaseScrimWidget : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()
public:
	void InitializeScrim(const FCavrnusScrimOptions& InOptions);
	void CloseScrim();
	void HookOnClose(const TFunction<void()>& OnCloseCallback);

protected:
	TMulticastDelegate<void()> OnCloseDelegate;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	FCavrnusScrimOptions Options = FCavrnusScrimOptions();
};
