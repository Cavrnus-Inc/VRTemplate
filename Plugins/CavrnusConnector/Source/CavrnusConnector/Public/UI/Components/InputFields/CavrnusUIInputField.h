// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Styles/InputFields/CavrnusInputFieldStyle.h"
#include "CavrnusUIInputField.generated.h"

class UCavrnusUIEditableTextBox;
class UCavrnusUIBorder;
class UCavrnusInputFieldStyle;
class UCavrnusUITextBlock;
class UEditableTextBox;

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusUIInputField : public UUserWidget
{
	GENERATED_BODY()

public:
	TMulticastDelegate<void(const FText& InputUpdated)> OnInputCommitted;
	TMulticastDelegate<void(const FText& InputUpdated)> OnTextChanged;

	UCavrnusUIInputField* SetToKeyboardFocusedCustom();
	UCavrnusUIInputField* SetText(const FString& InText);
	UCavrnusUIInputField* SetHintText(const FString& InText);
	UCavrnusUIInputField* SetIsPasswordField(const bool bIsPassword);
	UCavrnusUIInputField* SetLabel(const FString& InText);
	UCavrnusUIInputField* SetVisibilityState(const bool bVis);
	
	FString GetText();
	void ClearText();
	void ResetField();

protected:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category="Cavrnus|InputField")
	TObjectPtr<UCavrnusUIEditableTextBox> EditableTextBox = nullptr;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional), Category="Cavrnus|InputField")
	TObjectPtr<UCavrnusUITextBlock> Label = nullptr;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional), Category="Cavrnus|InputField")
	TObjectPtr<UCavrnusUIBorder> Border = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus|InputField")
	FString HintString = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus|InputField")
	TObjectPtr<UCavrnusInputFieldStyle> InputFieldStyleData = nullptr;
		
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void SynchronizeProperties() override;
	
	UFUNCTION()
	virtual void HandleTextUpdated(const FText& InText);
	UFUNCTION()
	virtual void HandleTextCommitted(const FText& InText, ETextCommit::Type InCommitMethod);

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	void ApplyHoverStyle(const bool HoverState);
	void ApplyCurrentStateStyle();
	virtual FInputFieldStyleStateData GetCurrentStateData();
};
