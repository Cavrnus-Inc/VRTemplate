// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "Types/CavrnusSpaceInfo.h"
#include "UI/Components/Lists/CavrnusBaseListItemWidget.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"
#include "UI/Helpers/CavrnusImageHelper.h"
#include "CavrnusSpaceListOptionWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusSpaceListOptionWidget : public UCavrnusBaseListItemWidget
{
	GENERATED_BODY()
public:
	void Setup(const FCavrnusSpaceInfo& InSpaceInfo);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> SpaceNameTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> LastModifiedTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ThumbnailDefault = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> ThumbnailContainer = nullptr;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Thumbnail = nullptr;

	virtual void NativeDestruct() override;

private:
	FDelegateHandle ButtonDelegate = FDelegateHandle();
	
	UPROPERTY()
	FCavrnusSpaceInfo SpaceInfo = FCavrnusSpaceInfo();
	
	UPROPERTY()
	TObjectPtr<UCavrnusImageHelper> ImageHelper = nullptr;
};
