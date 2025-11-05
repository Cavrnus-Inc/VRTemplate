// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CavrnusChatMenuTextFieldWidget.generated.h"

class UCavrnusUIEditableTextBox;
class UCavrnusChatMenuBase;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusChatMenuTextFieldWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeDestruct() override;
	
public:
	void Setup(UCavrnusChatMenuBase* InBaseMenu);
	void SetHintText(const FText& InHintText);

private:
	FDelegateHandle OnChatAddedHandle = FDelegateHandle();
	
	UPROPERTY()
	TWeakObjectPtr<UCavrnusChatMenuBase> BaseMenu = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIEditableTextBox> TextBox = nullptr;

	void CreateNewMessage();
};