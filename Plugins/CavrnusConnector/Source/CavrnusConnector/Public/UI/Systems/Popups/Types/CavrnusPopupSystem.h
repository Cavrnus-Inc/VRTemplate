// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Systems/CavrnusBaseUISystem.h"
#include "UI/Systems/ContextData/UIContextData.h"
#include "UI/Systems/Displayers/CavrnusWidgetDisplayer.h"
#include "UObject/Object.h"
#include "CavrnusPopupSystem.generated.h"

class UCavrnusBasePopupWidget;

USTRUCT()
struct FCavrnusPopupOptions : public FCavrnusBaseDisplayOptions
{
	GENERATED_BODY()

    bool bIsModal = true;
    float BackgroundDimAmount = 0.7f; 

    FText Title = FText::GetEmpty();

	bool Stackable = false;
    bool HasCloseButton = true; 
    bool CloseOnClickOutside = true;

	static FCavrnusPopupOptions StackablePopup()
	{
		auto Opt = FCavrnusPopupOptions();
		Opt.Stackable = true;
		return Opt;
	}
};

class ICavrnusPopupContainerInterface;
class ICavrnusWidgetDisplayer;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPopupSystem : public UObject, public ICavrnusBaseUISystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer) override;

	template <typename TMessageType>
	TMessageType* Create(const FCavrnusPopupOptions& Options = FCavrnusPopupOptions(), const FUIContextData& Data = FUIContextData())
	{
		return Cast<TMessageType>(CreateInternal(TMessageType::StaticClass(), Options));
	}
	
	template <typename TMessageType>
	TMessageType* CreateStackable(const FCavrnusPopupOptions Options = FCavrnusPopupOptions::StackablePopup(), const FUIContextData& Data = FUIContextData())
	{
		return Cast<TMessageType>(CreateInternal(TMessageType::StaticClass(), Options, Data));
	}
	
	virtual void Close(UCavrnusBaseUserWidget* WidgetToClose) override;
	virtual void CloseAll() override;

	virtual void Teardown() override;
	
private:
	UPROPERTY()
	TWeakObjectPtr<UObject> DisplayerObj;
	ICavrnusWidgetDisplayer* Displayer;
	
	UPROPERTY()
	TArray<UCavrnusBasePopupWidget*> Popups;
	UPROPERTY()
	TMap<FGuid, UCavrnusBasePopupWidget*> PopupsMap;

	UPROPERTY()
	TObjectPtr<UCavrnusWidgetBlueprintLookup> LookupAsset;

	UCavrnusBasePopupWidget* CreateInternal(const UClass* Type, const FCavrnusPopupOptions& Options, const FUIContextData& Data);
};