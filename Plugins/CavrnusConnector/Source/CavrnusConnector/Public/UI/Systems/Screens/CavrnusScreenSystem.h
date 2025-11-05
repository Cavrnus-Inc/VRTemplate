// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusBaseScreenWidget.h"
#include "UI/Systems/CavrnusBaseUISystem.h"
#include "UObject/Object.h"
#include "CavrnusScreenSystem.generated.h"

USTRUCT()
struct FCavrnusScreenOptions
{
	GENERATED_BODY()
	
	bool bHideOtherUI = true;
	bool bAllowPopups = false;
	FName ScreenName;
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusScreenSystem : public UObject, public ICavrnusBaseUISystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer) override;

	template <typename TScreenType>
	TScreenType* ShowScreen(const FCavrnusScreenOptions& Options = FCavrnusScreenOptions(), const FUIContextData& Data = FUIContextData())
	{
		return Cast<TScreenType>(CreateInternal(TScreenType::StaticClass(), Options, Data));
	}

	virtual void Close(UCavrnusBaseUserWidget* WidgetToClose) override;
	virtual void CloseAll() override;
	virtual void Teardown() override;

private:
	UPROPERTY()
	TArray<UCavrnusBaseScreenWidget*> ScreenStack;

	UPROPERTY()
	TWeakObjectPtr<UObject> DisplayerObj;
	ICavrnusWidgetDisplayer* Displayer;

	UPROPERTY()
	TObjectPtr<UCavrnusWidgetBlueprintLookup> LookupAsset;

	UCavrnusBaseScreenWidget* CreateInternal(const UClass* Type, const FCavrnusScreenOptions& Options, const FUIContextData& Data);
};
