// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/AsyncTaskDownloadImage.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "CavrnusAsyncTaskDownloadImage.generated.h"

/**
 * This is a copy of the Unreal AsyncTaskDownloadImage class, just with an added Cancel Op call
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusAsyncTaskDownloadImage : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UCavrnusAsyncTaskDownloadImage* DownloadImage(FString URL);

	void Start(FString URL);
	void Cancel();

	FDownloadImageDelegate OnSuccess;
	FDownloadImageDelegate OnFail;

private:
	/** Handles image requests coming from the web */
	void HandleImageRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

	FHttpRequestPtr CurrentRequest;
	bool bCancelled = false;
};
