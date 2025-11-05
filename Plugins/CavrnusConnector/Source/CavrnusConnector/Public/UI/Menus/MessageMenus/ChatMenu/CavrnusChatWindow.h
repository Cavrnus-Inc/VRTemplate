// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Types/ChatEntry.h>

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "UI/Menus/MessageMenus/CavrnusMessageMenuBase.h"
#include "CavrnusChatWindow.generated.h"

class UScrollBox;
struct FCavrnusUser;
class UCavrnusChatEntry;
class UVerticalBox;
class UScrollBar;
class UButton;
class UEditableTextBox;
class FTimeManager;

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusChatWindow : public UCavrnusMessageMenuBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Cavrnus|ChatWindow")
	FLinearColor ButtonEnabledColor = FLinearColor();
	
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|ChatWindow", meta = (BindWidget))
	UImage* SubmitButtonIcon = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|ChatWindow", meta = (BindWidget))
	UEditableTextBox* InputTextBox = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|ChatWindow", meta = (BindWidget))
	UButton* SubmitButton = nullptr;

private:
	FLinearColor DefaultButtonIconColor = FLinearColor();
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void SubmitButtonClicked();

	UFUNCTION()
	void TextInputChanged(const FText& InputValue);

	UFUNCTION()
	void TextInputFieldSubmit(const FText& InputValue, ETextCommit::Type CommitValue);

	void SubmitChat();

	virtual void HandleChatAdded(FChatEntry ChatAdded) override;
	virtual void HandleChatUpdated(FChatEntry ChatUpdated) override;
	virtual void HandleChatRemoved(FString ChatRemoved) override;

	virtual void LocalUserArrived() override;
};
