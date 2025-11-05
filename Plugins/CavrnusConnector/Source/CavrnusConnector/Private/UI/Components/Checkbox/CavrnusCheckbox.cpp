// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Checkbox/CavrnusCheckbox.h"
#include "CavrnusFunctionLibrary.h"
#include "Components/CheckBox.h"

void UCavrnusCheckbox::SetCheckedState(const bool bIsChecked)
{
	if (UCavrnusFunctionLibrary::IsConnectedToAnySpace())
		UCavrnusFunctionLibrary::PostBoolPropertyUpdate(SpaceConn, ContainerName, PropName, bIsChecked);
}

void UCavrnusCheckbox::CavrnusSetup(const FCavrnusSpaceConnection& InSpaceConn, const FString& InContainer, const FString& InProperty)
{
	if (!UCavrnusFunctionLibrary::IsConnectedToAnySpace())
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to setup UCavrnusCheckbox! Not connect to a space"));
		return;
	}
	
	SpaceConn = InSpaceConn;
	ContainerName = InContainer;
	PropName = InProperty;
	BindId = UCavrnusFunctionLibrary::BindBooleanPropertyValue(InSpaceConn, InContainer, InProperty,
		[this](const bool ServerVal, const FString&, const FString&)
		{
			IsChecked = ServerVal;

			if (Checkbox)
				Checkbox->SetCheckedState(ServerVal ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);

			if (OnCheckStateChanged.IsBound())
				OnCheckStateChanged.Broadcast(ServerVal);
			
		})->BindingId;
}

void UCavrnusCheckbox::CavrnusDispose()
{
	if (!BindId.IsEmpty())
		UCavrnusFunctionLibrary::UnbindWithId(BindId);
}

void UCavrnusCheckbox::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Checkbox)
		Checkbox->OnCheckStateChanged.AddDynamic(this, &UCavrnusCheckbox::SetCheckedState);
}

void UCavrnusCheckbox::NativeDestruct()
{
	Super::NativeDestruct();
	
	CavrnusDispose();
	
	if (Checkbox)
		Checkbox->OnCheckStateChanged.RemoveDynamic(this, &UCavrnusCheckbox::SetCheckedState);
}