// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Types/ChatEntry.h>

#include "CoreMinimal.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "CavrnusChatEntry.generated.h"

struct FCavrnusUser;
class UBorder;
class UImage;
class UTextBlock;
struct FCavrnusSpaceConnection;

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusChatEntry : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()

public:
	void Setup(const FChatEntry& InChatEntry);

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|ChatEntry", meta = (BindWidgetOptional))
	UTextBlock* CreatorName = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|ChatEntry", meta = (BindWidgetOptional))
	UTextBlock* CreationTime = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|ChatEntry", meta = (BindWidgetOptional))
	UTextBlock* Message = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|ChatEntry", meta = (BindWidgetOptional))
	UImage* ProfilePicImage = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|ChatEntry", meta = (BindWidgetOptional))
	UBorder* ChatBubbleBackground = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Cavrnus|ChatEntry")
	FLinearColor LocalUserColor = FLinearColor();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cavrnus|ChatEntry")
	void SetupComplete(FChatEntry InChatEntry);

	FChatEntry ChatData = FChatEntry();

private:
	void HandleLocalUserChat();

	FString FormatTime(const FChatEntry& ChatEntry);
};
