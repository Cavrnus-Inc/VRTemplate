// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Widgets/ComponentModifierWidget/CavrnusComponentListItemWidget.h"

UCavrnusComponentListItemWidget* UCavrnusComponentListItemWidget::Setup(
	const FActorStruct& TargetActor,
	const FCavrnusComponentInfo& Data,
	const TFunction<void(const bool bState)>& OnAddCavItemToggled)
{
	OnAddCavItemToggledCallback = OnAddCavItemToggled;
	
	if (ActorNameTextBlock)
		ActorNameTextBlock->SetText(FText::FromString(Data.SanitizedName));

	if (CavrnusAttachmentCheckBox)
	{
		const bool InitialState = CavrnusComponentModifiers::HasComponent(TargetActor.Actor, Data.Class);
		CavrnusAttachmentCheckBox->SetCheckedState(InitialState ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
		CavrnusAttachmentCheckBox->OnCheckStateChanged.RemoveDynamic(this, &UCavrnusComponentListItemWidget::HandleCheckBoxChanged);
		CavrnusAttachmentCheckBox->OnCheckStateChanged.AddDynamic(this, &UCavrnusComponentListItemWidget::HandleCheckBoxChanged);
	}
	
	return this;
}

void UCavrnusComponentListItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	// if (ListItemObject)
	// {
	// 	if (UCavrnusListObjectComponentInfo* Data = Cast<UCavrnusListObjectComponentInfo>(ListItemObject))
	// 	{
	// 		if (ActorNameTextBlock)
	// 			ActorNameTextBlock->SetText(FText::FromString(Data->AllCavrnusComponentInfo.ComponentDisplayName));
	// 	}
	// }
}

void UCavrnusComponentListItemWidget::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	if (CavrnusAttachmentCheckBox)
		CavrnusAttachmentCheckBox->OnCheckStateChanged.RemoveDynamic(this, &UCavrnusComponentListItemWidget::HandleCheckBoxChanged);
}

void UCavrnusComponentListItemWidget::HandleCheckBoxChanged(const bool bIsChecked)
{
	if (OnAddCavItemToggledCallback)
		OnAddCavItemToggledCallback(bIsChecked);
}
