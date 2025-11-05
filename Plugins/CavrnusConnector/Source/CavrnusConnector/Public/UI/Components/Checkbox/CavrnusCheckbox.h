// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/CavrnusSpaceConnection.h"
#include "UI/CavrnusLifecycle.h"
#include "CavrnusCheckbox.generated.h"

class UCheckBox;
/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class CAVRNUSCONNECTOR_API UCavrnusCheckbox : public UUserWidget, public ICavrnusLifecycle
{
	GENERATED_BODY()
public:
	TMulticastDelegate<void(const float CheckState)> OnCheckStateChanged;
	
	bool GetValue() const { return IsChecked; }

	UFUNCTION(blueprintCallable, Category = "Cavrnus")
	void SetCheckedState(const bool bIsChecked);
	
protected:
	bool IsChecked = false;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category = "Cavrnus")
	TObjectPtr<UCheckBox> Checkbox = nullptr;

	UPROPERTY()
	FCavrnusSpaceConnection SpaceConn = FCavrnusSpaceConnection();
	UPROPERTY()
	FString ContainerName;
	UPROPERTY()
	FString PropName;
	UPROPERTY()
	FString BindId;

	// CavrnusLifecycle
	UFUNCTION(BlueprintCallable, Category = "Cavrnus")
	virtual void CavrnusSetup(const FCavrnusSpaceConnection& InSpaceConn, const FString& InContainer, const FString& InProperty) override;

	UFUNCTION(BlueprintCallable, Category = "Cavrnus")
	virtual void CavrnusDispose() override;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
