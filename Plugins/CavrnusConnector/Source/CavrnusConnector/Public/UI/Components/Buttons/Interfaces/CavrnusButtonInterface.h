// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CavrnusButtonInterface.generated.h"

enum class ECavrnusButtonState : uint8;
// This class does not need to be modified.
UINTERFACE()
class UCavrnusButtonInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CAVRNUSCONNECTOR_API ICavrnusButtonInterface
{
	GENERATED_BODY()

public:
	TMulticastDelegate<void()> OnButtonClicked;
	TMulticastDelegate<void()> OnButtonPressed;
	TMulticastDelegate<void()> OnButtonReleased;
	TMulticastDelegate<void(const bool IsHovered)> OnButtonHovered;

	virtual void SetEnabledState(bool IsEnabled) = 0;
};