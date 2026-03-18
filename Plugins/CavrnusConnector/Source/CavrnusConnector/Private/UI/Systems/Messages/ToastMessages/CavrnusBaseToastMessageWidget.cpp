// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Messages/ToastMessages/CavrnusBaseToastMessageWidget.h"
#include "TimerManager.h"

void UCavrnusBaseToastMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ProgressBar)
		ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
}

void UCavrnusBaseToastMessageWidget::StartTimer(const float InDuration)
{
	Duration = InDuration;
	TimerActive = true;

	if (ProgressBar)
		ProgressBar->SetVisibility(ESlateVisibility::Visible);
}

void UCavrnusBaseToastMessageWidget::StopTimer()
{
	// Only hide the progress bar if the auto-close timer was actually running.
	// If the bar is showing manual progress (e.g. download/lifecycle), keep it visible.
	if (TimerActive && ProgressBar)
		ProgressBar->SetVisibility(ESlateVisibility::Hidden);

	TimerActive = false;
}

void UCavrnusBaseToastMessageWidget::CloseWithDelay(const float Delay)
{
	if (Delay <= 0.f)
	{
		Close();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		FTimerDelegate TimerDel;
		TimerDel.BindUObject(this, &UCavrnusBaseToastMessageWidget::Close);

		World->GetTimerManager().SetTimer(CloseTimerHandle, TimerDel, Delay, false);
	}
}

UCavrnusBaseToastMessageWidget* UCavrnusBaseToastMessageWidget::SetPrimaryText(const FString& InPrimaryText)
{
	if (PrimaryText)
		PrimaryText->SetText(FText::FromString(InPrimaryText));

	return this;
}

UCavrnusBaseToastMessageWidget* UCavrnusBaseToastMessageWidget::SetSecondaryText(const FString& InSecondaryText)
{
	if (SecondaryText)
		SecondaryText->SetText(FText::FromString(InSecondaryText));

	return this;
}

void UCavrnusBaseToastMessageWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	Hovered = true;
	StopTimer();
}

void UCavrnusBaseToastMessageWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!TimerActive & !IsValid(ProgressBar))
		return;

	if (ElapsedTime < Duration)
	{
		ElapsedTime += InDeltaTime;
		const float Percent = FMath::Clamp(ElapsedTime / Duration, 0.f, 1.f);

		if (IsValid(ProgressBar))
			ProgressBar->SetPercent(1 - Percent);
	}
}
