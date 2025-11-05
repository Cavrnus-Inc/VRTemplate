// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusBaseToastMessageWidget.h"
#include "UI/Systems/CavrnusBaseUISystem.h"
#include "UI/Systems/Displayers/CavrnusWidgetDisplayer.h"
#include "UI/Systems/Messages/CavrnusBaseUIMessageWidget.h"
#include "UObject/Object.h"
#include "CavrnusToastMessageUISystem.generated.h"

/**
 * These are self-managed - they auto dismiss!
 * Should be good for server status, user join messages, general user scene actions like "User A deleted an object!"
 */

USTRUCT(BlueprintType)
struct FCavrnusToastMessageOptions : public FCavrnusBaseDisplayOptions
{
	GENERATED_BODY()
	
	bool ShouldAutoDismiss = false;
	float AutoDismissDuration = 3.0f;

	FCavrnusToastMessageOptions()
	: ShouldAutoDismiss(false)
	, AutoDismissDuration(3.0f)
	{}
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusToastMessageUISystem : public UObject, public ICavrnusBaseUISystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer) override;

	template <typename TMessageType>
	TMessageType* Create(const FCavrnusToastMessageOptions& Options = FCavrnusToastMessageOptions())
	{
		return Cast<TMessageType>(CreateInternal(TMessageType::StaticClass(), Options));
	}

	template <typename TMessageType>
	TMessageType* CreateAutoClose(const float Duration = 3.0f)
	{
		FCavrnusToastMessageOptions Options;
		Options.ShouldAutoDismiss = true;
		Options.AutoDismissDuration = Duration;
		
		return Cast<TMessageType>(CreateInternal(TMessageType::StaticClass(), Options));
	}
	
	virtual void Close(UCavrnusBaseUserWidget* WidgetToClose) override;
	virtual void CloseAll() override;
	virtual void Teardown() override;
	
private:
	UPROPERTY()
	TObjectPtr<UCavrnusWidgetBlueprintLookup> LookupAsset = nullptr;
	
	UPROPERTY()
	TObjectPtr<UObject> DisplayerObj = nullptr;
	ICavrnusWidgetDisplayer* Displayer = nullptr;

	UPROPERTY()
	TArray<UCavrnusBaseToastMessageWidget*> ActiveMessages;
	
	UPROPERTY()
	TMap<FGuid, UCavrnusBaseToastMessageWidget*> ActiveMessagesMap;

	UPROPERTY()
	TMap<UUserWidget*, FTimerHandle> AutoDismissTimersMap;
	
	void StartAutoDismissTimer(UCavrnusBaseToastMessageWidget* Widget, const float Duration);

	UCavrnusBaseUIMessageWidget* CreateInternal(const UClass* Type, const FCavrnusToastMessageOptions& Options);
};