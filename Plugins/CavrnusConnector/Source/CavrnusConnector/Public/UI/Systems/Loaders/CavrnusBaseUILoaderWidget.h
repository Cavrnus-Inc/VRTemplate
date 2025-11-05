// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "CavrnusBaseUILoaderWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusBaseUILoaderWidget : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()
public:
	void CloseLoader() const;
	void HookOnClose(const TFunction<void()>& OnCloseCallback);

protected:
	TMulticastDelegate<void()> OnCloseDelegate;
};
