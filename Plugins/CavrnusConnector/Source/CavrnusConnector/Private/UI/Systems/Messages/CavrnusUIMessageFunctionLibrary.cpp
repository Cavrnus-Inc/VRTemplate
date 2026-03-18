// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Messages/CavrnusUIMessageFunctionLibrary.h"

UCavrnusInfoToastMessageWidget* UCavrnusUIMessageFunctionLibrary::CreateCompleteInfoToastMessage(
	const FString& PrimaryText,
	const FString& SecondaryText,
	ECavrnusInfoToastMessageEnum Type,
	const bool AutoClose,
	const float CloseDuration)
{
	UCavrnusInfoToastMessageWidget* Toast = AutoClose
		? UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
		: UCavrnusUI::Get()->Messages()->Toast()->Create<UCavrnusInfoToastMessageWidget>();

	if (Toast)
	{
		Toast->SetPrimaryText(PrimaryText);
		Toast->SetSecondaryText(SecondaryText);
		Toast->SetType(Type);
	}

	return Toast;
}

UCavrnusProgressToastMessageWidget* UCavrnusUIMessageFunctionLibrary::CreateCompleteProgressToastMessage(
	const FString& PrimaryText,
	const FString& SecondaryText,
	ECavrnusInfoToastMessageEnum Type,
	const bool AutoClose,
	const float CloseDuration)
{
	UCavrnusProgressToastMessageWidget* Toast = AutoClose
		? UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusProgressToastMessageWidget>()
		: UCavrnusUI::Get()->Messages()->Toast()->Create<UCavrnusProgressToastMessageWidget>();

	if (Toast)
	{
		Toast->SetPrimaryText(PrimaryText);
		Toast->SetSecondaryText(SecondaryText);
		Toast->SetType(Type);
	}

	return Toast;
}
