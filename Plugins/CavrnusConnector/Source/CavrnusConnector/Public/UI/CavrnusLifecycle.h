// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CavrnusLifecycle.generated.h"

struct FCavrnusSpaceConnection;
// This class does not need to be modified.
UINTERFACE()
class UCavrnusLifecycle : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CAVRNUSCONNECTOR_API ICavrnusLifecycle
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void CavrnusSetup(const FCavrnusSpaceConnection& InSpaceConn, const FString& InContainer, const FString& InProperty) =0;
	virtual void CavrnusDispose() =0;
};
