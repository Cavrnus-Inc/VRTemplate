// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateWidgetStyle.h"
#include "Engine/DataAsset.h"
#include "CavrnusUIButtonStyle.generated.h"

enum class ECavrnusButtonState : uint8;

USTRUCT(BlueprintType)
struct FCavrnusUIButtonStyleState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Cavrnus|Content")
	FLinearColor ContentColor = FLinearColor::White;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Layout")
	FVector2D ContentPadding = FVector2D::ZeroVector;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Effects")
	float ContentOpacity = 1.0f;

	FCavrnusUIButtonStyleState()
	: ContentColor(FLinearColor::White)
	, ContentPadding(FVector2D::ZeroVector)
	, ContentOpacity(1.0f)
	{}
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUIButtonStyle : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Style")
	FButtonStyle ButtonStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Style")
	FButtonStyle ToggledStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|States")
	FCavrnusUIButtonStyleState Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|States")
	FCavrnusUIButtonStyleState Hovered;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|States")
	FCavrnusUIButtonStyleState Pressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|States")
	FCavrnusUIButtonStyleState Focused;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|States")
	FCavrnusUIButtonStyleState Disabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|States")
	FCavrnusUIButtonStyleState Toggled;
	
	UFUNCTION(BlueprintCallable, Category="Cavrnus|States")
	const FCavrnusUIButtonStyleState& GetStyleForState(const ECavrnusButtonState State) const;
};