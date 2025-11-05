// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/RichTextBlock.h"
#include "Components/SizeBox.h"
#include "Components/Throbber.h"
#include "Managers/CavrnusEditorAuthenticationManager.h"
#include "UI/Components/Buttons/Types/CavrnusUITextButton.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"
#include "CavrnusEditorConnectToCavrnusButtonWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOREDITOR_API UCavrnusEditorConnectToCavrnusButtonWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> PrimaryConnectionStatusTextBlock = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<USizeBox> ButtonContainer = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> SecondaryConnectionStatusTextBlock = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<URichTextBlock> RichTextBlock = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextButton> Button = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UHorizontalBox> ConnectionSuccessBox = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UThrobber> Throbber = nullptr;
	FDelegateHandle ConnStateChangedHandle;
	FDelegateHandle AuthStartedHandle;
	FDelegateHandle ButtonClickedHandle;

	void HandleConnectionState(const ECavrnusEditorConnectedStateEnum& CurrentState);
	void SetPrimaryText(const FString& InText, const bool bVis = true);
	void SetSecondaryText(const FString& InText, const bool bVis = true);
	void SetRichText(const FString& InText, const bool bVis = true);
};
