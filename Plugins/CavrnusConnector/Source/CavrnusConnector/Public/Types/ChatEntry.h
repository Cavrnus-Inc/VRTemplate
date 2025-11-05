// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <CoreMinimal.h>
#include "ChatEntry.generated.h"

USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FChatEntry
{
	GENERATED_BODY()
	
	FChatEntry() = default;	

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Chat")
	FString ChatId = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Chat")
	bool IsTranscription = false;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Chat")
	FString ChatDisplayText = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Chat")
	bool ChatCreatorIsLocalUser = false;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Chat")
	FString ChatCreatorName = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Chat")
	FString ChatCreatorPictureUrl = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Chat")
	FDateTime CreatedTime = FDateTime::MinValue();

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Chat")
	bool Complete = false;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Chat")
	bool WasTranslated = false;
};
