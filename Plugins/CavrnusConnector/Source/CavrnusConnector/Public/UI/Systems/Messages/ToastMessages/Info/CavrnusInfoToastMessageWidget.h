// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusInfoToastMessageEnum.h"
#include "CavrnusInfoToastMessageThemeAsset.h"
#include "UI/Components/Borders/CavrnusUIBorder.h"
#include "UI/Components/Image/CavrnusUIImage.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusBaseToastMessageWidget.h"
#include "CavrnusInfoToastMessageWidget.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class CAVRNUSCONNECTOR_API UCavrnusInfoToastMessageWidget : public UCavrnusBaseToastMessageWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category="Cavrnus")
	ECavrnusInfoToastMessageEnum ToastMessageEnum = ECavrnusInfoToastMessageEnum::Info;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TObjectPtr<UCavrnusInfoToastMessageThemeAsset> ThemeAsset;

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
	UFUNCTION(BlueprintCallable, Category="Cavrnus|UI|Messages")
	UCavrnusInfoToastMessageWidget* SetType(const ECavrnusInfoToastMessageEnum& InType);

	UFUNCTION(BlueprintCallable, Category="Cavrnus|UI|Messages")
	UCavrnusInfoToastMessageWidget* SetPrimaryText(const FString& InPrimaryText);

	UFUNCTION(BlueprintCallable, Category="Cavrnus|UI|Messages")
	UCavrnusInfoToastMessageWidget* SetSecondaryText(const FString& InSecondaryText);

private:
	void SetStyle(const ECavrnusInfoToastMessageEnum& InType);
};
