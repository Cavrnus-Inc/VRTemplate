// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateBrush.h"
#include "CavrnusUICommonStyle.generated.h"

USTRUCT(BlueprintType)
struct FCavrnusUICommonStyleState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual")
	FLinearColor BackgroundColor = FLinearColor::Black;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual")
	FLinearColor ContentColor = FLinearColor::White;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual")
	FSlateBrush BackgroundBrush;
	
	FCavrnusUICommonStyleState()
	: BackgroundColor(FLinearColor::Black)
	, ContentColor(FLinearColor::White)
	, BackgroundBrush()
	{}
};

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusUICommonStyle : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="States")
	FCavrnusUICommonStyleState Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="States")
	FCavrnusUICommonStyleState Hovered;

	FCavrnusUICommonStyleState GetStyleForState(const bool IsHovered);
};