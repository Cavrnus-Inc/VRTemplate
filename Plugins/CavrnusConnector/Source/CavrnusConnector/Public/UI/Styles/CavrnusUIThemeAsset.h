// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CavrnusUIThemeAsset.generated.h"

UENUM(BlueprintType)
enum class ECavrnusThemeColorRole : uint8
{
	// Base Layers
	Surface,			//Main background, chat messages, cards
	SurfaceVariant,		//Sidebar background, secondary surfaces

	// Text
	OnSurface,			//Primary text, headings, high emphasis
	OnSurfaceVariant,	//Secondary text, labels, medium emphasis
	OnDisabled,			//Disabled text, placeholders, lowest emphasis

	// Borders / dividers
	Outline,			//Input borders, card outlines, primary dividers
	OutlineVariant,		//Subtle dividers, light separators

	Accent,				//Brand color, buttons, links, highlights
	OnAccent,			//Text on accent background (buttons, badges)

	Error,				// Errors, destructive actions
	OnError,			// Text on error background

	Custom,				// Use when color needs to be custom per widget
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUIThemeAsset : public UDataAsset
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category="Cavrnus|Theme")
	TMap<ECavrnusThemeColorRole, FLinearColor> RoleToColor;

public:
	FLinearColor GetColorForRole(ECavrnusThemeColorRole Role) const;
};
