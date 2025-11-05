// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CavrnusChatMenuEntryWidget.generated.h"

class UCavrnusUIBorder;
class UCavrnusUITextBlock;
struct FCavrnusChatMessage;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusChatMenuEntryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void Setup(const FCavrnusChatMessage& InChatMessage);

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	FLinearColor LocalUserColor = FLinearColor();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	FLinearColor RemoteUserColor = FLinearColor();

private:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> MessageTextBlock = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> DateTextBlock = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIBorder> MessageBackground = nullptr;
};