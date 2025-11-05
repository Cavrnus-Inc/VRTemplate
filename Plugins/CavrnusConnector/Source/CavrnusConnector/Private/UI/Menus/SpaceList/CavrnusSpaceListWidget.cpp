// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/SpaceList/CavrnusSpaceListWidget.h"
#include "CavrnusFunctionLibrary.h"
#include "UI/CavrnusUI.h"
#include "UI/Systems/Dialogs/CavrnusDialogSystem.h"
#include "UI/Systems/Dialogs/Types/CavrnusInputFieldDialog.h"
#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"
#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageWidget.h"
#include "UI/Systems/RawWidgetHost/CavrnusRawWidgetHost.h"

void UCavrnusSpaceListWidget::NativeConstruct()
{
	Super::NativeConstruct();
	PopulateSpaceList();
	
	if (SearchInputField)
	{
		SearchInputField->SetHintText("Search spaces");
		SearchInputField->OnTextChanged.AddWeakLambda(this, [this](const FText& Text)
		{
			ListHandler->ApplySearchFilter(Text.ToString(),
				[](const FString& SearchText, const FCavrnusSpaceInfo& Target) { 
				return Target.SpaceName.Contains(SearchText);
			 });
		});
	}
	
	if (CreateSpaceButton)
	{
		CreateSpaceButtonHandle = CreateSpaceButton->OnButtonClicked.AddWeakLambda(this, []
		{
			UCavrnusUI::Get()->Dialogs()->Create<UCavrnusInputFieldDialog>()
			->SetTitleText("Create Space")
			->SetBodyText("Enter a space name")
			->SetInputFieldHintText("Name")
			->SetDismissButton("Cancel")
			->SetConfirmButton("Create Space", [](const FString& SubmittedText)
			{
				UCavrnusFunctionLibrary::CreateSpace(
					SubmittedText,
					[](const FCavrnusSpaceInfo& CavrnusSpaceInfo)
					{
						UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
							->SetPrimaryText("New space created!")
							->SetSecondaryText(FString::Printf(TEXT("Successfully created space: %s"), *CavrnusSpaceInfo.SpaceName))
							->SetType(ECavrnusInfoToastMessageEnum::Success);
					},
					[](const FString& String)
					{
						UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
							->SetPrimaryText("Error creating new space!")
							->SetSecondaryText(String)
							->SetType(ECavrnusInfoToastMessageEnum::Success);
					});
			});
		});
	}
}

void UCavrnusSpaceListWidget::NativeDestruct()
{
	Super::NativeDestruct();
	UCavrnusUI::Get(this)->GenericWidgetDisplayer()->Close(this);

	if (ListHandler)
	{
		ListHandler->Teardown();
		ListHandler.Reset();
	}
	
	ListHandler = nullptr;

	UCavrnusFunctionLibrary::UnbindWithId(SpacesBinding);
}

void UCavrnusSpaceListWidget::PopulateSpaceList()
{
	ListHandler = TCavrnusUIListHandler<FCavrnusSpaceInfo>::Initialize(
		SpaceListContainerWidget,
		SpaceListOptionBlueprint,
		[](const FCavrnusSpaceInfo& Data) { return Data.SpaceId; },
		[](const FCavrnusSpaceInfo& A, const FCavrnusSpaceInfo& B)
		{
			return A.LastAccess > B.LastAccess;
		});


	ListHandler->OnWidgetCreated<UCavrnusSpaceListOptionWidget>([](UCavrnusSpaceListOptionWidget* Widget, const FCavrnusSpaceInfo& Data)
		{
			Widget->Setup(Data);
		});

	ListHandler->SetLoadingState(true);
	ListHandler->SetFeedbackText("Loading spaces...");

	SetWidgetVis(false);
	ListHandler->BindListHasPopulated([this]
	{
		SetWidgetVis(true);
		ListHandler->SetLoadingState(false);
	});

	SpacesBinding = UCavrnusFunctionLibrary::BindJoinableSpaces(
		[this](const FCavrnusSpaceInfo& Csi)
		{
			ListHandler->AddItem(Csi);
		},
		[this](const FCavrnusSpaceInfo& Csi)
		{
			ListHandler->UpdateItem(Csi);
		},
		[this](const FCavrnusSpaceInfo& Csi)
		{
			ListHandler->RemoveItem(Csi);
		})->BindingId;

	ListHandler->OnItemSelected([this](const FCavrnusSpaceInfo& SelectedData)
	{
		JoinSpace(SelectedData.SpaceId);
	});
}

void UCavrnusSpaceListWidget::JoinSpace(const FString& JoinId)
{
	UCavrnusFunctionLibrary::JoinSpace(JoinId, [this](const FCavrnusSpaceConnection& Sc)
	{
		
	}, [](const FString& Err)
	{
		UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
			->SetPrimaryText("Failed to Join Space")
			->SetSecondaryText(FString::Printf(TEXT("%s"), *Err))
			->SetType(ECavrnusInfoToastMessageEnum::Warning);
	});

	// this assumes we are only using this popup in the generic UI flow manager...
	UCavrnusUI::Get(this)->GenericWidgetDisplayer()->Close(this);
}

void UCavrnusSpaceListWidget::SetWidgetVis(const bool bVis)
{
	if (CreateSpaceButton)
		CreateSpaceButton->SetVisibleState(bVis);

	if (SearchInputField)
		SearchInputField->SetVisibilityState(bVis);
}
