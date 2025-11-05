// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Types/CavrnusConnectionStatusEnum.h>

#include "CoreMinimal.h"
#include "CavrnusServerStatusThemeAsset.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusBaseToastMessageWidget.h"
#include "CavrnusServerStatusToastWidget.generated.h"

class UCavrnusUIBorder;
class UCavrnusUITextBlock;
class UCavrnusUIImage;

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusServerStatusToastWidget : public UCavrnusBaseToastMessageWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category="Cavrnus")
	ECavrnusConnectionStatusEnum CurrentStatus = ECavrnusConnectionStatusEnum::Live;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TObjectPtr<UCavrnusServerStatusThemeAsset> ThemeAsset;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIImage> Icon = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> PrimaryText = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> SecondaryText = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIBorder> PrimaryBorder = nullptr;

	virtual void NativeConstruct() override;
	virtual void SynchronizeProperties() override;
	
public:
	UCavrnusServerStatusToastWidget* SetStatus(const FCavrnusConnectionStatus& InStatus);
	UCavrnusServerStatusToastWidget* SetPrimaryText(const FString& InPrimaryText);
	UCavrnusServerStatusToastWidget* SetSecondaryText(const FString& InPrimaryText);
	UCavrnusServerStatusToastWidget* SetIcon(UTexture2D* InIcon);

private:
	void SetStatusInternal(const ECavrnusConnectionStatusEnum& Status, const FString& Message = "");
};