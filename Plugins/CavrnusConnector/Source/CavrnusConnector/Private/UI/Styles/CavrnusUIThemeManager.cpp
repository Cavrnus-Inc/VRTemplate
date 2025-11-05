// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Styles/CavrnusUIThemeManager.h"
#include "UI/Styles/CavrnusUIThemeAsset.h"

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UCavrnusUIThemeAsset* UCavrnusUIThemeManager::EditorPreviewTheme = nullptr;

FLinearColor UCavrnusUIThemeManager::GetColorForRole(const ECavrnusThemeColorRole& Role)
{
	if (ActiveTheme == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to find Active Theme! Returning default white"))
		return FLinearColor::White;
	}
	
	return ActiveTheme->GetColorForRole(Role);
}

void UCavrnusUIThemeManager::Initialize()
{
	UCavrnusUIThemeAsset* ThemeAsset = LoadObject<UCavrnusUIThemeAsset>(nullptr, TEXT("/CavrnusConnector/UI/Styles/DA_Cavrnus_ThemeAsset.DA_Cavrnus_ThemeAsset"));
	if (ThemeAsset)
		SetEditorPreviewTheme(ThemeAsset);
	else
		UE_LOG(LogTemp, Warning, TEXT("Failed to load default theme asset for editor preview!"));

	ActiveTheme = ThemeAsset;
}

UCavrnusUIThemeAsset* UCavrnusUIThemeManager::GetTheme(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("GetTheme: WorldContextObject is nullptr"));
		return nullptr;
	}

	if (const UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (const auto* Tm = UCavrnusUI::Get(WorldContextObject)->ThemeManager())
			{
				if (UCavrnusUIThemeAsset* RuntimeTheme = Tm->GetActiveTheme())
				{
					return RuntimeTheme;
				}
			}
		}
	}

#if WITH_EDITOR
		return GetEditorPreviewTheme();
#else
		return nullptr;
#endif
}
