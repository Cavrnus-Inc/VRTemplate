// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CavrnusToggleButtonInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UCavrnusToggleButtonInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CAVRNUSCONNECTOR_API ICavrnusToggleButtonInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	TMulticastDelegate<void(const bool IsToggled)> OnToggleChanged;
	
	virtual void SetToggled(const bool bInToggled, const bool bNotify = true) = 0;
	virtual bool GetIsToggled() const = 0;
};