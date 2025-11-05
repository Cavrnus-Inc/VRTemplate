// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusAsyncTaskDownloadImage.h"
#include "Blueprint/AsyncTaskDownloadImage.h"
#include "UObject/Object.h"
#include "CavrnusImageHelper.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusImageHelper : public UObject
{
	GENERATED_BODY()
public:
	void FetchTexture(const FString& InUrl, TFunction<void(UTexture2DDynamic* OutTexture, const FVector2D& OutAspectRatio)> OnFetchTexture);
	void Cancel();
protected:
	virtual void BeginDestroy() override;
private:
	UPROPERTY()
	UCavrnusAsyncTaskDownloadImage* DownloadTask = nullptr;
	TFunction<void(UTexture2DDynamic*, const FVector2D& OutDims)> OnFetchTextureCallback;

	UFUNCTION()
	void OnFetchTexture(UTexture2DDynamic* Texture);

	int32 GetGCD(int32 A, int32 B);
};
