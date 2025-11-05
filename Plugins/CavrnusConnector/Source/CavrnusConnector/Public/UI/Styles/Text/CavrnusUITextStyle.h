// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateTypes.h"
#include "CavrnusUITextStyle.generated.h"

enum class ECavrnusTextBlockSize : uint8;
enum class ECavrnusUITextState : uint8;

USTRUCT(BlueprintType)
struct FCavrnusUITextStyleState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Content")
	FLinearColor ContentColor = FLinearColor::White;

	FCavrnusUITextStyleState()
	: ContentColor(FLinearColor::White)
	{}
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUITextStyle : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Style")
	FTextBlockStyle TextStyle;
};