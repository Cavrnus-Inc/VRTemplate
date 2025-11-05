// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "Core/Settings/Setting.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "UI/Components/Buttons/Interfaces/CavrnusToggleButtonInterface.h"
#include "CavrnusUIToggleButton.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Cavrnus Toggle")
class CAVRNUSCONNECTOR_API UCavrnusUIToggleButton : public UCavrnusUIButton, public ICavrnusToggleButtonInterface
{
	GENERATED_BODY()
protected:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> ButtonImage;
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void ApplyButtonStyle() override;
	
public:
	void InitToggleSetting(const TSharedPtr<TSetting<bool>>& InSetting);
	void SetCanToggleFunc(const TFunction<bool()>& CanToggleFunc);
	virtual void SetToggled(const bool InToggled, const bool bNotify = true) override;
	virtual bool GetIsToggled() const override;
	virtual void SetIcon(UTexture2D* IconTexture) override;
	
protected:
	virtual ECavrnusButtonState ResolvePriorityButtonVisualState() override;
	
private:
	TSharedPtr<TSetting<bool>> ToggleSetting;
	TFunction<bool()> ToggleFunc = [] { return true; };
	
	bool IsToggled = false;
	
	UFUNCTION()
	void ToggleButtonClicked();
};