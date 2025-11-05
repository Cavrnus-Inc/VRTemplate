// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Widgets/AuthenticationSelectionWidget/CavrnusAuthenticationSelectionWidget.h"

#include "CavrnusConnectorEditorModule.h"
#include "CavrnusConnectorSettings.h"
#include "CavrnusSubsystem.h"
#include "Managers/CavrnusEditorAuthenticationManager.h"

void UCavrnusAuthenticationSelectionWidget::Setup()
{
	if (AuthenticationDropdown == nullptr)
		return;
	
	AuthenticationDropdown->Populate(
		FCavrnusRuntimeDropdownEntry::Create(
			FName("Editor"),
			"Join using current authentication",
			true,
			[this]
			{
				UE_LOG(LogCavrnusConnectorEditor, Error, TEXT("Selected Editor"));
				AuthSelected(ECavrnusAuthMethodForPIE::JoinAsPIE);
			}));
	
	AuthenticationDropdown->Populate(
	FCavrnusRuntimeDropdownEntry::Create(
		FName("Member"),
		"Join As Member",
		false,
		[this]
		{
			UE_LOG(LogCavrnusConnectorEditor, Error, TEXT("Selected Member"));
			AuthSelected(ECavrnusAuthMethodForPIE::JoinAsMember);
		}));
	
	AuthenticationDropdown->Populate(
	FCavrnusRuntimeDropdownEntry::Create(
		FName("Guest"),
		"Join As Guest",
		false,
		[this]
		{
			UE_LOG(LogCavrnusConnectorEditor, Error, TEXT("Selected Guest"));
			AuthSelected(ECavrnusAuthMethodForPIE::JoinAsGuest);
		}));

	auto am = UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>();
	AuthChangedHandle = am->OnEditorAuthMethodChanged.AddWeakLambda(this, [this](const ECavrnusAuthMethodForPIE& Auth)
	{
		switch (Auth)
		{
		case ECavrnusAuthMethodForPIE::JoinAsPIE:
			AuthenticationDropdown->SetSelected(FName("Editor"));
			break;
		case ECavrnusAuthMethodForPIE::JoinAsMember:
			AuthenticationDropdown->SetSelected(FName("Member"));
			break;
		case ECavrnusAuthMethodForPIE::JoinAsGuest:
			AuthenticationDropdown->SetSelected(FName("Guest"));
			break;
		}
	});
}

void UCavrnusAuthenticationSelectionWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (auto* Cs = UCavrnusSubsystem::Get())
	{
		if (auto* Am = Cs->Services->Get<UCavrnusEditorAuthenticationManager>())
			Am->OnEditorAuthMethodChanged.Remove(AuthChangedHandle);
	}

	AuthChangedHandle.Reset();
}

void UCavrnusAuthenticationSelectionWidget::AuthSelected(const ECavrnusAuthMethodForPIE& AuthMethod)
{
	UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>()->SetLoginAuthMethod(AuthMethod);
}
