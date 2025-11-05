// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusBasePanelWidget.h"
#include "UI/Systems/CavrnusBaseUISystem.h"
#include "UI/Systems/CavrnusUIArbiter.h"
#include "UI/Systems/ContextData/UIContextData.h"
#include "UI/Systems/Displayers/CavrnusWidgetDisplayer.h"
#include "UObject/Object.h"
#include "CavrnusPanelSystem.generated.h"

UENUM(BlueprintType)
enum class EPanelLocation : uint8
{
	LeftMiddle     UMETA(DisplayName = "Left Middle"),
	RightMiddle    UMETA(DisplayName = "Right Middle"),
	BottomMiddle   UMETA(DisplayName = "Bottom Middle"),
	TopMiddle      UMETA(DisplayName = "Top Middle"),
};

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
class CAVRNUSCONNECTOR_API UCavrnusPanelSystem : public UObject, public ICavrnusBaseUISystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer) override;

	template <typename TMessageType>
	TMessageType* Create(const FCavrnusPanelOptions& Options = FCavrnusPanelOptions(), const FUIContextData& Data = FUIContextData())
	{
		return Cast<TMessageType>(CreateInternal(TMessageType::StaticClass(), Options, Data));
	}

	virtual void Close(UCavrnusBaseUserWidget* WidgetToClose) override;
	virtual void CloseAll() override;
	virtual void Teardown() override;

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
