// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Systems/CavrnusUIArbiter.h"
#include "UI/Systems/Scrims/CavrnusUIScrimSystem.h"
#include "UObject/Object.h"
#include "CavrnusRawWidgetHost.generated.h"

struct FCavrnusScrimOptions;
class ICavrnusWidgetDisplayer;

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusRawWidgetHost : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(UObject* InOuterObject, ICavrnusWidgetDisplayer* InDisplayer);
	
	UUserWidget* Show(const TSubclassOf<UUserWidget>& InBlueprint);
	UUserWidget* ShowWithScrim(const TSubclassOf<UUserWidget>& InBlueprint, const FCavrnusScrimOptions& ScrimOptions = FCavrnusScrimOptions());
	bool Close(const UUserWidget* WidgetToClose);
	void CloseAll();

	void SetVisibility(const bool bVis);

	void Teardown();

private:
	UPROPERTY()
	TWeakObjectPtr<UObject> OuterObject = nullptr;
	
	UPROPERTY()
	TObjectPtr<UObject> DisplayerObj = nullptr;
	ICavrnusWidgetDisplayer* Displayer = nullptr;
	
	UPROPERTY()
	TMap<UUserWidget*, FGuid> Widgets;
	
	FDelegateHandle VisDelegate = FDelegateHandle();
};
