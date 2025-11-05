// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusMaximizedUser.h"
#include "CavrnusUserWidget.h"
#include "UI/Menus/MinimalMenu/SidebarMenu/SidebarMenuBase.h"
#include "CavrnusUserMenu.generated.h"

class UScrollBox;

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusUserMenu : public USidebarMenuBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|UserMenu", meta = (BindWidget))
	UScrollBox* ScrollBox = nullptr;

	UPROPERTY(EditAnywhere, Category = "Cavrnus|UserMenu")
	TSubclassOf<UCavrnusUserWidget> WidgetEntry = nullptr;

	UPROPERTY(EditAnywhere, Category = "Cavrnus|UserMenu")
	TSubclassOf<UCavrnusMaximizedUser> MaximizedUserEntry = nullptr;

private:
	UPROPERTY()
	TMap<FString, UCavrnusUserWidget*> Entries;

	UPROPERTY()
	UCavrnusBinding* UsersBinding;
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	CavrnusSpaceUserEvent SpaceUserEvent;

	void MaximizeUserSelected(const FCavrnusUser& MaximizedUser);
};
