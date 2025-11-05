// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "UI/Menus/UserMenu/CavrnusUserWidget.h"
#include "SidebarMenuBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSecondaryWidgetRequested, UUserWidget*, SecondaryWidget, FString, MenuName);

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API USidebarMenuBase : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|SiceBarMenuBase")
	virtual void ForceChildrenRebuild();
	
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|SideBarMenuBase")
	FSecondaryWidgetRequested SecondaryWidgetRequested;
};
