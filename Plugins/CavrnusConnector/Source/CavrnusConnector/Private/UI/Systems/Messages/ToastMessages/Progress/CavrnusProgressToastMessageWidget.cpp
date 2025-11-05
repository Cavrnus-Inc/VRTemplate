// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Messages/ToastMessages/Progress/CavrnusProgressToastMessageWidget.h"

UCavrnusProgressToastMessageWidget* UCavrnusProgressToastMessageWidget::SetProgress(const float InProgress)
{
	if (ProgressBar)
	{
		ProgressBar->SetVisibility(ESlateVisibility::Visible);
		ProgressBar->SetPercent(FMath::Clamp(InProgress, 0.0f, 1.0f));
	}

	return this;
}

void UCavrnusProgressToastMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetType(ECavrnusInfoToastMessageEnum::Info);
}
