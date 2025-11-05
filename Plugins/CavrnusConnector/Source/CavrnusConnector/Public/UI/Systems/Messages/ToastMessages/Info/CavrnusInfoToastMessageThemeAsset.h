// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusInfoToastMessageEnum.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "CavrnusInfoToastMessageThemeAsset.generated.h"

USTRUCT(BlueprintType)
struct FCavrnusInfoToastMessageTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Cavrnus")
	ECavrnusInfoToastMessageEnum Status;

	UPROPERTY(EditAnywhere, Category="Cavrnus")
	FLinearColor PrimaryColor;

	UPROPERTY(EditAnywhere, Category="Cavrnus")
	TObjectPtr<UTexture2D> Icon = nullptr;

	FCavrnusInfoToastMessageTheme()
		: Status(ECavrnusInfoToastMessageEnum::Info)
		, PrimaryColor(FLinearColor::White)
		, Icon(nullptr)
	{}
};
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusInfoToastMessageThemeAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|ServerStatus")
	TArray<FCavrnusInfoToastMessageTheme> Styles;

	FCavrnusInfoToastMessageTheme GetTheme(const ECavrnusInfoToastMessageEnum& Status);
};
