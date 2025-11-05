// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CavrnusChatMenuBase.generated.h"

class UCavrnusChatMenuEntryWidget;
class UCavrnusChatMenuTextFieldWidget;
class UCavrnusChatMenuConversationWidget;

USTRUCT(BlueprintType)
struct FCavrnusChatMessage
{
	GENERATED_BODY()

	UPROPERTY()
	bool IsLocalUser = false;
    
	UPROPERTY()
	FString Message = TEXT("");

	UPROPERTY()
	FDateTime Timestamp = FDateTime(0);
};

/**
 * Generic chat menu
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusChatMenuBase : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
public:
	TMulticastDelegate<void(const FCavrnusChatMessage& Message)> OnMessageAdded;

	virtual UCavrnusChatMenuBase* Setup();
	
	UCavrnusChatMenuBase* SetHintText(const FText& InHintText);
	void AddMessage(const FCavrnusChatMessage& Message);
	
protected:
	UPROPERTY(EditAnywhere, Category="Cavrnus")
	TSubclassOf<UCavrnusChatMenuEntryWidget> ChatEntryBlueprint = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusChatMenuConversationWidget> ConversationWidget = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusChatMenuTextFieldWidget> TextFieldWidget = nullptr;
};