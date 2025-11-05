// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LevelEditor.h"
#include "UI/Widgets/CavrnusBaseEditorUtilityWidget.h"

class CAVRNUSCONNECTOREDITOR_API FCavrnusEditorHelpers
{
public:
	/// Hook this so it's safe to interact w/ main editor!
	/// You can safely add tabs, show dialogs, load settings, etc., at this point.
	static FDelegateHandle HookMainFrameCreationFinished(const TFunction<void(TSharedPtr<SWindow> RootWindow, const bool bIsNewProjectWindow)>& Callback);
	
	static FLevelEditorModule& GetLevelEditorModule();
	static TSharedPtr<FTabManager> GetLevelEditorTabManager();

	static void DrawAttentionToTabById(const FName& TabID, const FString& LabelTab);
	static TSharedPtr<SDockTab> FindExistingLiveTab(const FName& TabID);
	static bool TryCloseTabByID(const FName& TabID);

	static bool TrySavingPersistentLayout();

	static UCavrnusBaseEditorUtilityWidget* TryCreateUtilityWidget(const FString& BlueprintPath, const FName& TabID);

	static bool IsEditorDefaultPawnSet(const UWorld* World, const FString& PawnPath);
	static void SetEditorDefaultPawnByPath(const UWorld* World, const FString& PawnPath, const FString& GameModePath);
};
