// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetManager/CavrnusBaseDataAsset.h"
#include "CavrnusIconsDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FCavrnusIconEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|Icons")
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|Icons")
	TSoftObjectPtr<UTexture2D> Icon;
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusIconsDataAsset : public UCavrnusBaseDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cavrnus|Icons")
	TArray<FCavrnusIconEntry> Icons;
	
	UTexture2D* GetIcon(const FName& Key) const
	{
		for (const FCavrnusIconEntry& Entry : Icons)
			if (Entry.Key == Key)
				return Entry.Icon.LoadSynchronous();
		
		return nullptr;
	}
};
