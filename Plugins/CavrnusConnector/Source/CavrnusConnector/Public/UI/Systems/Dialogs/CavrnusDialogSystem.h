// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Systems/CavrnusBaseUISystem.h"
#include "UI/Systems/Displayers/CavrnusWidgetDisplayer.h"
#include "UObject/Object.h"
#include "CavrnusDialogSystem.generated.h"

class UCavrnusBaseDialogWidget;

USTRUCT()
struct FCavrnusDialogOptions : public FCavrnusBaseDisplayOptions
{
	GENERATED_BODY()
};

/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusDialogSystem : public UObject, public ICavrnusBaseUISystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer) override;

	template <typename TMessageType>
	TMessageType* Create(const FCavrnusDialogOptions& Options = FCavrnusDialogOptions())
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
	TArray<UCavrnusBaseDialogWidget*> Dialogs;
	
	UPROPERTY()
	TMap<FGuid, UCavrnusBaseDialogWidget*> DialogMap;

	UCavrnusBaseDialogWidget* CreateInternal(const UClass* Type, const FCavrnusDialogOptions& Options);
};
