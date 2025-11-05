// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Sliders/CavrnusSlider.h"

#include "CavrnusFunctionLibrary.h"
#include "Components/Slider.h"
#include "Types/CavrnusSpaceConnection.h"

void UCavrnusSlider::CavrnusSetup(const FCavrnusSpaceConnection& InSpaceConn, const FString& InContainer, const FString& InProperty)
{
	if (!UCavrnusFunctionLibrary::IsConnectedToAnySpace())
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to setup Cavrnus Slider! Not connect to a space"));
		return;
	}

	SpaceConn = InSpaceConn;
	ContainerName = InContainer;
	PropName = InProperty;
	BindId = UCavrnusFunctionLibrary::BindFloatPropertyValue(InSpaceConn, InContainer, InProperty,
		[this](const float ServerVal, const FString&, const FString&)
		{
			Slider->SetValue(MapValueToLocal(ServerVal));
		})->BindingId;
}

void UCavrnusSlider::CavrnusDispose()
{
	if (!BindId.IsEmpty())
		UCavrnusFunctionLibrary::UnbindWithId(BindId);

	if (Updater)
	{
		Updater->Cancel();
		Updater = nullptr;
	}
}

UCavrnusSlider* UCavrnusSlider::SetMinMax(FVector2D InLocalMinMax, FVector2D InServerMinMax)
{
	if (Slider)
	{
		Slider->SetMinValue(InLocalMinMax.X);
		Slider->SetMaxValue(InLocalMinMax.Y);

		LocalMinMax = InLocalMinMax;
	}

	ServerMinMax = InServerMinMax;
	if (ServerMinMax == FVector2D::Zero())
	{
		ServerMinMax.X = InServerMinMax.X;
		ServerMinMax.Y = InServerMinMax.Y;
	}

	return this;
}

void UCavrnusSlider::NativeConstruct()
{
	Super::NativeConstruct();

	if (Slider)
	{
		Slider->OnMouseCaptureBegin.AddDynamic(this, &UCavrnusSlider::HandleOnBegin);
		Slider->OnMouseCaptureEnd.AddDynamic(this, &UCavrnusSlider::HandleOnEnd);
		Slider->OnValueChanged.AddDynamic(this, &UCavrnusSlider::HandleOnValueChanged);
	}
}

void UCavrnusSlider::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (Slider)
	{
		Slider->OnMouseCaptureBegin.RemoveDynamic(this, &UCavrnusSlider::HandleOnBegin);
		Slider->OnMouseCaptureEnd.RemoveDynamic(this, &UCavrnusSlider::HandleOnEnd);
		Slider->OnValueChanged.RemoveDynamic(this, &UCavrnusSlider::HandleOnValueChanged);
	}

	CavrnusDispose();
}

void UCavrnusSlider::HandleOnBegin()
{
	if (OnBegin.IsBound())
		OnBegin.Broadcast();

	if (UCavrnusFunctionLibrary::IsConnectedToAnySpace())
		Updater = UCavrnusFunctionLibrary::BeginTransientFloatPropertyUpdate(SpaceConn, ContainerName, PropName, MapValueToServer(Slider->GetValue()));
}

void UCavrnusSlider::HandleOnEnd()
{
	if (OnEnd.IsBound())
		OnEnd.Broadcast();

	if (Updater)
	{
		Updater->Finalize(MapValueToServer(Slider->GetValue()));
		Updater = nullptr;
	}
}

void UCavrnusSlider::HandleOnValueChanged(const float Val)
{
	if (OnValueChanged.IsBound())
		OnValueChanged.Broadcast(Val);

	if (Updater)
		Updater->UpdateWithNewData(MapValueToServer(Val));
}

float UCavrnusSlider::RemapValue(const float TargetVal, const FVector2D& SrcMinMax, const FVector2D& TargetMinMax)
{
	const float SrcRange = SrcMinMax.Y - SrcMinMax.X;
	if (FMath::IsNearlyZero(SrcRange))
		return TargetMinMax.X;

	const float Percentage = (TargetVal - SrcMinMax.X) / SrcRange;
	const float TargetRange = TargetMinMax.Y - TargetMinMax.X;

	return TargetMinMax.X + Percentage * TargetRange;
}

float UCavrnusSlider::MapValueToLocal(const float Val)
{
	return RemapValue(Val, ServerMinMax, LocalMinMax);
}

float UCavrnusSlider::MapValueToServer(const float Val)
{
	return RemapValue(Val, LocalMinMax, ServerMinMax);
}
