// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "CavrnusBaseDialogWidget.generated.h"

class UCavrnusUITextBlock;
/**
 * 
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusBaseDialogWidget : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()
public:
	void CloseDialog();
	void HookOnClose(const TFunction<void()>& OnCloseCallback);

protected:
	FDelegateHandle ButtonHandle = FDelegateHandle();

	TMulticastDelegate<void()> OnCloseDelegate;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIButton> CloseButton = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> TitleText = nullptr;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
