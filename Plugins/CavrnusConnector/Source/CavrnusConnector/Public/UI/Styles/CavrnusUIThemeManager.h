// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusUIThemeAsset.h"
#include "UI/CavrnusUI.h"
#include "UObject/Object.h"
#include "CavrnusUIThemeManager.generated.h"

enum class ECavrnusThemeColorRole : uint8;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUIThemeManager : public UObject
{
	GENERATED_BODY()

public:
	UCavrnusUIThemeAsset* GetActiveTheme() const {return ActiveTheme;}

	void Initialize();

	static UCavrnusUIThemeAsset* GetEditorPreviewTheme() {return EditorPreviewTheme;}

	static void SetEditorPreviewTheme(UCavrnusUIThemeAsset* InTheme)
	{
		EditorPreviewTheme = InTheme;
	}

	static UCavrnusUIThemeAsset* GetTheme(const UObject* WorldContextObject);
	
	FLinearColor GetColorForRole(const ECavrnusThemeColorRole& Role);

private:
	UPROPERTY()
	UCavrnusUIThemeAsset* ActiveTheme = nullptr;
	
	static UCavrnusUIThemeAsset* EditorPreviewTheme;
};
