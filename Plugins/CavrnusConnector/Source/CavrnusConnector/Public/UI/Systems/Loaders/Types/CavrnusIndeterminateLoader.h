// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Systems/Loaders/CavrnusBaseUILoaderWidget.h"
#include "CavrnusIndeterminateLoader.generated.h"

class UCavrnusUITextBlock;

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusIndeterminateLoader : public UCavrnusBaseUILoaderWidget
{
	GENERATED_BODY()
public:
	UCavrnusIndeterminateLoader* SetMessage(const FString& Msg);

protected:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> MessageTextBlock = nullptr;
};
