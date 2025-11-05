// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateTypes.h"
#include "CavrnusInputFieldStyle.generated.h"

USTRUCT(BlueprintType)
struct FInputFieldStyleStateData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FLinearColor NormalBackgroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FLinearColor HoverBackgroundColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FLinearColor FocusedBackgroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FLinearColor NormalOutlineColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FLinearColor HoverOutlineColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FLinearColor FocusedOutlineColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FLinearColor NormalInputFieldContentColorString;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FLinearColor HoveredInputFieldContentColorString;

	FInputFieldStyleStateData()
	: NormalBackgroundColor(FLinearColor::Transparent)
	, HoverBackgroundColor(FLinearColor::Transparent)
	, FocusedBackgroundColor(FLinearColor::Transparent)
	, NormalOutlineColor(FLinearColor::Transparent)
	, HoverOutlineColor(FLinearColor::Transparent)
	, FocusedOutlineColor(FLinearColor::Transparent)
	, NormalInputFieldContentColorString(FLinearColor::Transparent)
	, HoveredInputFieldContentColorString(FLinearColor::Transparent)
	{}
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusInputFieldStyle : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|InputFieldStyle")
	FInputFieldStyleStateData NormalState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FInputFieldStyleStateData FocusedState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FInputFieldStyleStateData ValidationSuccessState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FInputFieldStyleStateData ValidationErrorState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FMargin ButtonPadding;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FVector4 CornerRadii; // TopLeft, TopRight, BottomRight, BottomLeft

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	float OutlineWidth = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FLinearColor ValidationSuccessColor = FLinearColor(83.0f / 255.0f, 229.0f / 255.0f, 83.0f / 255.0f, 1.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	FLinearColor ValidationErrorColor = FLinearColor(229.0f / 255.0f, 83.0f / 255.0f, 93.0f / 255.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|InputFieldStyle")
	int FontSize = 14;

	UCavrnusInputFieldStyle()
	{
		ButtonPadding = FMargin(10, 5, 10, 5);
	}
};
