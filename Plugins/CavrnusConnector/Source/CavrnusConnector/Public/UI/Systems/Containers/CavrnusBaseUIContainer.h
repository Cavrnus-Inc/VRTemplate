// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"
#include "CavrnusBaseUIContainer.generated.h"

class UCavrnusUIIconButton;
class UCavrnusUITextBlock;
/**
 * 
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusBaseUIContainer : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()
public:
	void SetTitle(const FText& InTitle) const
	{
		if (TitleTextBlock)
			TitleTextBlock->SetText(InTitle);
	}
	
	void SetWidgetContent(UUserWidget* InContent);
	void HookOnCloseButton(const TFunction<void()>& OnCloseCallback);
	
protected:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> TitleTextBlock = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIIconButton> CloseButton = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> ContentSlot = nullptr;
	
private:
	virtual void NativeDestruct() override;

	FDelegateHandle CloseHandle = FDelegateHandle();
};
