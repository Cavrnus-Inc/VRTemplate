// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusUIInputField.h"
#include "CavrnusUIInputFieldValidated.generated.h"

class UCavrnusUIBorder;

UENUM(BlueprintType)
enum class ECavrnusInputFieldValidationType : uint8
{
	Early,
	Late
};

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusUIInputFieldValidated : public UCavrnusUIInputField
{
	GENERATED_BODY()
	
public:
	TMulticastDelegate<void(const FText& TextUpdate, const bool HasValidInput)> OnValidatedTextUpdated;
	TMulticastDelegate<void(const FText& TextUpdate, const bool HasValidInput)> OnValidatedTextCommitted;

	void SetValidationMessage(const FString& InMsg);
	
	UCavrnusUIInputFieldValidated* SetValidationType(const ECavrnusInputFieldValidationType& InType);
	UCavrnusUIInputFieldValidated* SetValidationPredicate(const TFunction<bool(const FString& Text)>& InPredicate);
	void ClearValidationMessage();
	
	bool GetHasValidInput();
	
protected:
	bool HasValidInput = false;
	bool HasFieldBeenInteractedWith = false;

	virtual void NativeConstruct() override;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional), Category="Cavrnus|InputFieldValidated")
	TObjectPtr<UCavrnusUITextBlock> ValidationMessageTextBlock = nullptr;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional), Category="Cavrnus|InputFieldValidated")
	TObjectPtr<UCavrnusUIBorder> ValidationBorder = nullptr;
	
	ECavrnusInputFieldValidationType ValidationType;
	TFunction<bool(const FString&)> ValidationPredicate;

	bool ShouldBeValidated();
	virtual void HandleTextUpdated(const FText& InText) override;
	virtual void HandleTextCommitted(const FText& InText, ETextCommit::Type InCommitMethod) override;
	
	virtual void ValidateInput(const FString& InText);

	virtual FInputFieldStyleStateData GetCurrentStateData() override;
};
