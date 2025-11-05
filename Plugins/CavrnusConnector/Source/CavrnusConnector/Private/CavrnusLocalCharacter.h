// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Character.h>

#include "CavrnusLocalCharacter.generated.h"

// Class definition
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API ACavrnusLocalCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:
	ACavrnusLocalCharacter();

private:
	void SetupInputs();
};
