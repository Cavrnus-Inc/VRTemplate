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
	TimerActive = false;

	if (ProgressBar)
		ProgressBar->SetVisibility(ESlateVisibility::Hidden);
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
