// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "UObject/Object.h"
#include "CavrnusUITabHandler.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUITabHandler : public UObject
{
	GENERATED_BODY()
public:
	UCavrnusUITabHandler* Register(const FString& InId, UCavrnusUIButton* InButton, UUserWidget* InWidget);
	UCavrnusUITabHandler* Register(const FString& InId, UCavrnusUIButton* InButton, TSubclassOf<UUserWidget> InBlueprint);
	UCavrnusUITabHandler* SetActive(const FString& InId);
	UCavrnusUITabHandler* SetAllowToggleOff(bool bAllow);

	void Teardown();

private:
	bool bAllowToggleOff = true;
	FString CurrentActiveId;
	
	TMap<FString, TWeakObjectPtr<UWidget>> WidgetMap;
	TMap<FString, TWeakObjectPtr<UCavrnusUIButton>> ButtonMap;
	TMap<TWeakObjectPtr<UCavrnusUIButton>, FDelegateHandle> DelegateHandles;
};
