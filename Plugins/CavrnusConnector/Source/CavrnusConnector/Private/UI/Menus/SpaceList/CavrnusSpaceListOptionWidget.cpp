// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/SpaceList/CavrnusSpaceListOptionWidget.h"

#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "UI/Helpers/CavrnusUIHelpers.h"

void UCavrnusSpaceListOptionWidget::Setup(const FCavrnusSpaceInfo& InSpaceInfo)
{
	SpaceInfo = InSpaceInfo;

	if (SpaceNameTextBlock)
		SpaceNameTextBlock->SetText(FText::FromString(SpaceInfo.SpaceName));

	// TODO: Thumbnails need work. Slow, and inconsistent to load b/c Listviews virtualize entries (reused)
	if (ThumbnailContainer)
		ThumbnailContainer->SetVisibility(ESlateVisibility::Collapsed);
	
	// Thumbnail->SetVisibility(ESlateVisibility::Hidden);
	// ThumbnailDefault->SetVisibility(ESlateVisibility::Visible);
	//
	// if (ImageHelper)
	// 	ImageHelper->Cancel();
	//
	// if (!SpaceInfo.SpaceThumbnail.IsEmpty())
	// {
	// 	ImageHelper = NewObject<UCavrnusImageHelper>(this);
	// 	ImageHelper->FetchTexture(SpaceInfo.SpaceThumbnail, [this](UTexture2DDynamic* OutTexture, const FVector2D&)
	// 	{
	// 		Thumbnail->SetBrushFromTextureDynamic(OutTexture, true);
	// 		Thumbnail->SetVisibility(ESlateVisibility::Visible);
	// 		ThumbnailDefault->SetVisibility(ESlateVisibility::Hidden);
	// 	});
	// }

	if (LastModifiedTextBlock)
	{
		const FText DateText = FText::FromString(SpaceInfo.LastAccess.GetTicks() == 0 ? "" : FCavrnusUIHelpers::GetAbbreviatedDate(SpaceInfo.LastAccess));
		const FString CombinedString = FString::Printf(TEXT("Last visited %s"), *DateText.ToString());
		const FText FinalText = FText::FromString(CombinedString);

		LastModifiedTextBlock->SetText(FinalText);
	}
}

void UCavrnusSpaceListOptionWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (Button)
	{
		Button->OnButtonClicked.Remove(ButtonDelegate);
		ButtonDelegate.Reset();
	}
	
	ImageHelper = nullptr;
}
