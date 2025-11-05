// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CavrnusChatMenuConversationWidget.generated.h"

class UVerticalBox;
class UCavrnusUIScrollBox;
class UCavrnusChatMenuBase;
struct FCavrnusChatMessage;
class UCavrnusChatMenuEntryWidget;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusChatMenuConversationWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeDestruct() override;
	
public:
	void Setup(UCavrnusChatMenuBase* InBaseMenu, TSubclassOf<UCavrnusChatMenuEntryWidget> InEntryBlueprint);
	void AddChat(const FCavrnusChatMessage& InChatMessage);
	
private:
	FDelegateHandle OnChatAddedHandle = FDelegateHandle();
	
	UPROPERTY()
	TWeakObjectPtr<UCavrnusChatMenuBase> BaseMenu = nullptr;
	
	UPROPERTY()
	TSubclassOf<UCavrnusChatMenuEntryWidget> EntryBlueprint = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIScrollBox> ScrollBox = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalBoxContainer = nullptr;
	
	UPROPERTY()
	TArray<UUserWidget*> ChatMessages;
};
