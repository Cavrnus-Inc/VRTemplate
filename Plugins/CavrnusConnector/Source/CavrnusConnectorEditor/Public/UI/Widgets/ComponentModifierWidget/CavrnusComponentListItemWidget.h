// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/CheckBox.h"
#include "Tools/CavrnizingHelpers/CavrnusComponentModifiers.h"
#include "UI/Components/Lists/CavrnusBaseListItemWidget.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"
#include "CavrnusComponentListItemWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOREDITOR_API UCavrnusComponentListItemWidget : public UCavrnusBaseListItemWidget
{
	GENERATED_BODY()
public:
	UCavrnusComponentListItemWidget* Setup(const FActorStruct& TargetActor, const FCavrnusComponentInfo& Data, const TFunction<void(const bool bState)>& OnAddCavItemToggled);
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;
	
private:
	TMulticastDelegate<void()> OnSelectedCallback;
	TFunction<void(const bool bState)> OnAddCavItemToggledCallback;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> ActorNameTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> CavrnusAttachmentCheckBox = nullptr;

	UFUNCTION()
	void HandleCheckBoxChanged(const bool bIsChecked);
};
