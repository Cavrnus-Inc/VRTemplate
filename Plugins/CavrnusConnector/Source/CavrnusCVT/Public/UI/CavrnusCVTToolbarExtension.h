// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "UObject/Object.h"
#include "CavrnusCVTToolbarExtension.generated.h"

class UCavrnusToolbarPanelWidget;
class UCavrnusUIToggleButton;

UCLASS()
class CAVRNUSCVT_API UCavrnusCVTToolbarExtension : public UObject
{
    GENERATED_BODY()

public:
    static UCavrnusCVTToolbarExtension* Get();
    
    void Initialize();
    void Shutdown();

private:
    void FindAndExtendToolbarWidgets();
    void AddBookmarkButton(UCavrnusToolbarPanelWidget* ToolbarWidget);
    
    FTSTicker::FDelegateHandle TickerHandle;
    TSet<TWeakObjectPtr<UCavrnusToolbarPanelWidget>> ExtendedWidgets;
    
    bool Tick(float DeltaTime);
};