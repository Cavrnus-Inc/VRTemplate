// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusSpaceListOptionWidget.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "Types/CavrnusSpaceInfo.h"
#include "UI/Components/Buttons/Types/CavrnusUITextButton.h"
#include "UI/Components/InputFields/CavrnusUIInputField.h"
#include "UI/Components/Lists/CavrnusListContainerWidget.h"
#include "UI/Components/Lists/CavrnusUIListHandler.h"
#include "CavrnusSpaceListWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusSpaceListWidget : public UCavrnusBaseUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	void PopulateSpaceList();

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TSubclassOf<UCavrnusSpaceListOptionWidget> SpaceListOptionBlueprint;

private:
	FDelegateHandle CreateSpaceButtonHandle = FDelegateHandle();

	FString SpacesBinding = "";
	TArray<FCavrnusSpaceInfo> AllSpaces;

	TUniquePtr<TCavrnusUIListHandler<FCavrnusSpaceInfo>> ListHandler;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUITextButton> CreateSpaceButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUIInputField> SearchInputField = nullptr;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusListContainerWidget> SpaceListContainerWidget = nullptr;

	void JoinSpace(const FString& JoinId);

	void SetWidgetVis(const bool bVis);
};
