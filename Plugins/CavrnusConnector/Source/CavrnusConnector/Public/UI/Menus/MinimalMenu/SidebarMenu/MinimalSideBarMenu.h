// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "MinimalSidebarButton.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "MinimalSideBarMenu.generated.h"

USTRUCT(BlueprintType)
struct FMenuData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|SideBarMenu")
	FString Title = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|SideBarMenu")
	TSubclassOf<UUserWidget> TargetMenu = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|SideBarMenu")
	UTexture2D* Icon = nullptr;
};

UCLASS()
class CAVRNUSCONNECTOR_API UMinimalSideBarMenu : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|SideBarMenu")
	TSubclassOf<UMinimalSidebarButton> ButtonWidgetClass = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|SideBarMenu")
	TArray<FMenuData> MenuData;
};
