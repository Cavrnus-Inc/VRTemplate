// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/EditableTextBox.h"
#include "CavrnusUIEditableTextBox.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Cavrnus EditableTextBox")
class CAVRNUSCONNECTOR_API UCavrnusUIEditableTextBox : public UEditableTextBox
{
	GENERATED_BODY()
	
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
};
