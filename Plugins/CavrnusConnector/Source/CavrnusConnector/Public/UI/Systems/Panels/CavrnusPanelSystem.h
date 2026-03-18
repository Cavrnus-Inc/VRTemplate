// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusBasePanelWidget.h"
#include "UI/Systems/CavrnusBaseUISystem.h"
#include "UI/Systems/CavrnusUIArbiter.h"
#include "UI/Systems/ContextData/UIContextData.h"
#include "UI/Systems/Displayers/CavrnusWidgetDisplayer.h"
#include "UI/Systems/Panels/CavrnusPanelLocation.h"
#include "UObject/Object.h"
#include "CavrnusPanelSystem.generated.h"

USTRUCT()
struct FCavrnusPanelOptions : public FCavrnusBaseDisplayOptions
{
	GENERATED_BODY()

	EPanelLocation Location;

	static FCavrnusPanelOptions SetLocation(const EPanelLocation Location)
	{
		FCavrnusPanelOptions Opts;
		Opts.Location = Location;
		return Opts;
	}
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPanelSystem : public UDisposableUObject, public ICavrnusBaseUISystem
{
	GENERATED_BODY()
public:
	void Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer, UCavrnusUIArbiter* Arbiter);

	template <typename TMessageType>
	TMessageType* Create(const FCavrnusPanelOptions& Options = FCavrnusPanelOptions(), const FUIContextData& Data = FUIContextData())
	{
		return Cast<TMessageType>(CreateInternal(TMessageType::StaticClass(), Options, Data));
	}

	virtual void Close(UCavrnusBaseUserWidget* WidgetToClose) override;
	virtual void CloseAll() override;
	virtual void Dispose() override;

private:
	UPROPERTY()
	TMap<FGuid, UCavrnusBasePanelWidget*> ActivePanels;
	
	UPROPERTY()
	TWeakObjectPtr<UCavrnusWidgetBlueprintLookup> LookupAsset;
	
	UPROPERTY()
	TObjectPtr<UObject> DisplayerObj;
	ICavrnusWidgetDisplayer* Displayer;
	FDelegateHandle VisDelegate;

	UCavrnusBasePanelWidget* CreateInternal(const UClass* Type, const FCavrnusPanelOptions& Options, const FUIContextData& Data);

	void SetVisibility(bool bCond);
};
