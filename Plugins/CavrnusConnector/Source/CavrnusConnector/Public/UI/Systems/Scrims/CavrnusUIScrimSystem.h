// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Systems/CavrnusBaseUISystem.h"
#include "UI/Systems/Displayers/CavrnusWidgetDisplayer.h"
#include "UObject/Object.h"
#include "CavrnusUIScrimSystem.generated.h"

class UCavrnusBaseScrimWidget;

USTRUCT()
struct FCavrnusScrimOptions : public FCavrnusBaseDisplayOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus|ScrimOptions")
	bool bCloseOnClick = false;

	static FCavrnusScrimOptions CloseOnClickOption()
	{
		FCavrnusScrimOptions Options;
		Options.bCloseOnClick = true;
		return Options;
	}
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUIScrimSystem : public UObject, public ICavrnusBaseUISystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer) override;

	template <typename TMessageType>
	TMessageType* AssignToWidget(FGuid WidgetId, const FCavrnusScrimOptions& Options = FCavrnusScrimOptions())
	{
		return Cast<TMessageType>(AssignToWidgetInternal(TMessageType::StaticClass(), WidgetId, Options));
	}

	template <typename TMessageType>
	TMessageType* Create(const FCavrnusScrimOptions& Options = FCavrnusScrimOptions())
	{
		return Cast<TMessageType>(CreateInternal(TMessageType::StaticClass(), Options));
	}

	void CloseWidgetScrim(const FGuid& WidgetId);
	virtual void Close(UCavrnusBaseUserWidget* WidgetToClose) override;
	
	virtual void CloseAll() override;
	virtual void Teardown() override;

private:
	UPROPERTY()
	TObjectPtr<UCavrnusWidgetBlueprintLookup> Lookup = nullptr;
	
	UPROPERTY()
	TObjectPtr<UObject> DisplayerObj = nullptr;
	ICavrnusWidgetDisplayer* Displayer = nullptr;

	UPROPERTY()
	TArray<UCavrnusBaseScrimWidget*> Scrims;
	
	UPROPERTY()
	TMap<FGuid, UCavrnusBaseScrimWidget*> ScrimsMap;

	UPROPERTY()
	TMap<FGuid, FGuid> WidgetToScrimsMap;

	UCavrnusBaseScrimWidget* CreateInternal(const UClass* Type, const FCavrnusScrimOptions& Options);
	UCavrnusBaseScrimWidget* AssignToWidgetInternal(const UClass* Type, const FGuid WidgetId, const FCavrnusScrimOptions& Options);
};
