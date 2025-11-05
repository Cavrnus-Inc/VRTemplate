// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "Types/CavrnusConnectionStatusEnum.h"
#include "CavrnusServerStatusThemeAsset.generated.h"

enum class ECavrnusConnectionStatusEnum : uint8;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FCavrnusServerStatusTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Cavrnus|ServerStatus")
	ECavrnusConnectionStatusEnum Status;

	UPROPERTY(EditAnywhere, Category="Cavrnus|ServerStatus")
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, Category="Cavrnus|ServerStatus")
	FLinearColor PrimaryColor;

	FCavrnusServerStatusTheme()
	: Status(ECavrnusConnectionStatusEnum::Live)
	, PrimaryColor(FLinearColor::White)
	{}
};
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusServerStatusThemeAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|ServerStatus")
	TArray<FCavrnusServerStatusTheme> Styles;

	FCavrnusServerStatusTheme GetTheme(const ECavrnusConnectionStatusEnum& Status);
};