// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Menus/ChatMenus/CavrnusChatMenuBase.h"
#include "CavrnusAIChatMenu.generated.h"

/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusAIChatMenu : public UCavrnusChatMenuBase
{
	GENERATED_BODY()
public:
	virtual UCavrnusChatMenuBase* Setup() override;
};
