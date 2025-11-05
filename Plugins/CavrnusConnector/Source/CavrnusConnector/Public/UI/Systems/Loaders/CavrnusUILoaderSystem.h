// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Systems/CavrnusBaseUISystem.h"
#include "UI/Systems/Displayers/CavrnusWidgetDisplayer.h"
#include "UObject/Object.h"
#include "CavrnusUILoaderSystem.generated.h"

class UCavrnusBaseUILoaderWidget;

USTRUCT()
struct FCavrnusUILoaderOptions : public FCavrnusBaseDisplayOptions
{
	GENERATED_BODY()
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUILoaderSystem : public UObject, public ICavrnusBaseUISystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer) override;

	template <typename TMessageType>
	TMessageType* Create(const FCavrnusUILoaderOptions& Options = FCavrnusUILoaderOptions())
	{
		return Cast<TMessageType>(CreateInternal(TMessageType::StaticClass(), Options));
	}
	
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
	TArray<UCavrnusBaseUILoaderWidget*> Loaders;
	
	UPROPERTY()
	TMap<FGuid, UCavrnusBaseUILoaderWidget*> LoaderMap;

	UCavrnusBaseUILoaderWidget* CreateInternal(const UClass* Type, const FCavrnusUILoaderOptions& Options);
};
