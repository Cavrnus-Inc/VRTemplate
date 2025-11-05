// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LivePropertyUpdates/CavrnusLiveFloatPropertyUpdate.h"
#include "Types/CavrnusSpaceConnection.h"
#include "UI/CavrnusLifecycle.h"
#include "CavrnusSlider.generated.h"

class USlider;
/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class CAVRNUSCONNECTOR_API UCavrnusSlider : public UUserWidget, public ICavrnusLifecycle
{
	GENERATED_BODY()
public:
	TMulticastDelegate<void()> OnBegin;
	TMulticastDelegate<void()> OnEnd;
	TMulticastDelegate<void(const float Val)> OnValueChanged;

	// CavrnusLifecycle
	UFUNCTION(BlueprintCallable, Category = "Cavrnus")
	virtual void CavrnusSetup(const FCavrnusSpaceConnection& InSpaceConn, const FString& InContainer, const FString& InProperty) override;

	UFUNCTION(BlueprintCallable, Category = "Cavrnus")
	virtual void CavrnusDispose() override;

	UFUNCTION(BlueprintCallable, Category = "Cavrnus")
	UCavrnusSlider* SetMinMax(FVector2D InLocalMinMax, FVector2D InServerMinMax);
	
protected:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category = "Cavrnus")
	TObjectPtr<USlider> Slider = nullptr;
	
	FVector2D LocalMinMax = FVector2D(0, 1);
	FVector2D ServerMinMax = FVector2D(0, 1);

	UPROPERTY()
	FCavrnusSpaceConnection SpaceConn = FCavrnusSpaceConnection();

	UPROPERTY()
	FString ContainerName;

	UPROPERTY()
	FString PropName;

	UPROPERTY()
	FString BindId;

	UPROPERTY()
	TObjectPtr<UCavrnusLiveFloatPropertyUpdate> Updater;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void HandleOnBegin();
	UFUNCTION()
	void HandleOnEnd();
	UFUNCTION()
	void HandleOnValueChanged(const float Val);

	float RemapValue(float TargetVal, const FVector2D& SrcMinMax, const FVector2D& TargetMinMax);
	float MapValueToLocal(float Val);
	float MapValueToServer(float Val);
};
