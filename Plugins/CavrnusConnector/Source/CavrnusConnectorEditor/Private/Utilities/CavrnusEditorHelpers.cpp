// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Utilities/CavrnusEditorHelpers.h"

#include "CavrnusConnectorEditorModule.h"
#include "LevelEditor.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Interfaces/IMainFrameModule.h"
#include "Modules/ModuleManager.h"
#include "HAL/IConsoleManager.h"
#include "Modules/ModuleInterface.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/Docking/LayoutService.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/WorldSettings.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UI/Widgets/CavrnusBaseEditorUtilityWidget.h"

static const FName LevelEditorModuleName("LevelEditor");
FDelegateHandle FCavrnusEditorHelpers::HookMainFrameCreationFinished(
	const TFunction<void(TSharedPtr<SWindow> RootWindow,
	const bool bIsNewProjectWindow)>& Callback)
{
	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	return MainFrameModule.OnMainFrameCreationFinished().AddLambda(Callback);
}

FLevelEditorModule& FCavrnusEditorHelpers::GetLevelEditorModule()
{
	return FModuleManager::LoadModuleChecked<FLevelEditorModule>(LevelEditorModuleName);
}

TSharedPtr<FTabManager> FCavrnusEditorHelpers::GetLevelEditorTabManager()
{
	const auto& Lem = GetLevelEditorModule();
	return Lem.GetLevelEditorTabManager();
}

void FCavrnusEditorHelpers::DrawAttentionToTabById(const FName& TabID, const FString& LabelTab)
{
	// Bring tab to front
	if (const TSharedPtr<SDockTab> FoundTab = FindExistingLiveTab(TabID))
	{
		FoundTab->SetLabel(FText::FromString(LabelTab));
		if (const TSharedPtr<SWindow> Parent = FoundTab->GetParentWindow())
		{
			Parent->GetNativeWindow()->SetWindowFocus();
			FoundTab->DrawAttention();
		}
	}
}

TSharedPtr<SDockTab> FCavrnusEditorHelpers::FindExistingLiveTab(const FName& TabID)
{
	return GetLevelEditorTabManager()->FindExistingLiveTab(TabID);
}

bool FCavrnusEditorHelpers::TryCloseTabByID(const FName& TabID)
{
	if (GEditor)
	{
		if (UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
		{
			EditorUtilitySubsystem->CloseTabByID(TabID);
			return true;
		}
		
		return false;
	}
	
	return false;
}

bool FCavrnusEditorHelpers::TrySavingPersistentLayout()
{
	// if (const TSharedPtr<FTabManager> LevelEditorTabManager = GetLevelEditorTabManager())
	// {
	// 	LevelEditorTabManager->SavePersistentLayout();
	// 	return true;
	// }
	//
	return false;
}

UCavrnusBaseEditorUtilityWidget* FCavrnusEditorHelpers::TryCreateUtilityWidget(const FString& BlueprintPath, const FName& TabID)
{
	UCavrnusBaseEditorUtilityWidget* ReturnWidget = nullptr;
	
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	if (EditorUtilitySubsystem == nullptr)
	{
		UE_LOG(LogCavrnusConnectorEditor, Error, TEXT("[FCavrnusEditorHelpers::TryCreateUtilityWidget] EditorUtilitySubsystem is null!"))
		return nullptr;
	}
	
	UEditorUtilityWidgetBlueprint* LoadedBlueprintClass = LoadObject<UEditorUtilityWidgetBlueprint>(
		nullptr, *BlueprintPath, nullptr, LOAD_None, nullptr);
	
	if (LoadedBlueprintClass)
	{
		FName OutId = TabID;
		UEditorUtilityWidget* SpawnedWidget = EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID(LoadedBlueprintClass, OutId);

		if (UCavrnusBaseEditorUtilityWidget* CavWidget = Cast<UCavrnusBaseEditorUtilityWidget>(SpawnedWidget))
		{
			CavWidget->SetTabID(OutId);
			ReturnWidget = CavWidget;
		}

		EditorUtilitySubsystem->CloseTabByID(OutId);
	}

	return ReturnWidget;
}

bool FCavrnusEditorHelpers::IsEditorDefaultPawnSet(const UWorld* World, const FString& PawnPath)
{
	if (!World || PawnPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("IsEditorDefaultPawnSetByPath: Invalid world or empty path"));
		return false;
	}

	UClass* PawnClass = LoadObject<UClass>(nullptr, *PawnPath);
	if (!PawnClass || !PawnClass->IsChildOf(APawn::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("IsEditorDefaultPawnSetByPath: Could not load pawn class: %s"), *PawnPath);
		return false;
	}

	AGameModeBase* GameMode = World->GetAuthGameMode();

	// fallback to WorldSettings override if GetAuthGameMode() is nullptr
	if (!GameMode && World->GetWorldSettings() && World->GetWorldSettings()->DefaultGameMode)
	{
		GameMode = Cast<AGameModeBase>(World->GetWorldSettings()->DefaultGameMode->GetDefaultObject());
	}

	if (!GameMode)
	{
		return false;
	}

	return (GameMode->DefaultPawnClass == PawnClass);
}

void FCavrnusEditorHelpers::SetEditorDefaultPawnByPath(const UWorld* World, const FString& PawnPath, const FString& GameModePath)
{
	if (!World || PawnPath.IsEmpty() || GameModePath.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("EnsureEditorDefaultPawnByPath: Invalid params"));
        return;
    }

    // Load pawn class
    UClass* PawnClass = LoadObject<UClass>(nullptr, *PawnPath);
    if (!PawnClass || !PawnClass->IsChildOf(APawn::StaticClass()))
    {
        UE_LOG(LogTemp, Error, TEXT("EnsureEditorDefaultPawnByPath: Could not load pawn class: %s"), *PawnPath);
        return;
    }

    AGameModeBase* GameMode = World->GetAuthGameMode();

    // If no override, assign a new one
    if (!GameMode && World->GetWorldSettings())
    {
        if (!World->GetWorldSettings()->DefaultGameMode)
        {
            UClass* GameModeClass = LoadObject<UClass>(nullptr, *GameModePath);
            if (!GameModeClass || !GameModeClass->IsChildOf(AGameModeBase::StaticClass()))
            {
                UE_LOG(LogTemp, Error, TEXT("EnsureEditorDefaultPawnByPath: Could not load gamemode class: %s"), *GameModePath);
                return;
            }

            World->GetWorldSettings()->DefaultGameMode = GameModeClass;
            World->GetWorldSettings()->MarkPackageDirty();
            UE_LOG(LogTemp, Log, TEXT("Assigned GameMode override: %s"), *GameModeClass->GetName());

            GameMode = Cast<AGameModeBase>(GameModeClass->GetDefaultObject());
        }
        else
        {
            GameMode = Cast<AGameModeBase>(World->GetWorldSettings()->DefaultGameMode->GetDefaultObject());
        }
    }

    if (!GameMode)
    {
        UE_LOG(LogTemp, Error, TEXT("EnsureEditorDefaultPawnByPath: Could not resolve a valid GameMode"));
        return;
    }

    if (GameMode->DefaultPawnClass == PawnClass)
    {
        UE_LOG(LogTemp, Log, TEXT("EnsureEditorDefaultPawnByPath: DefaultPawnClass already set to %s"), *PawnClass->GetName());
        return;
    }

    GameMode->DefaultPawnClass = PawnClass;
    GameMode->MarkPackageDirty();

    UE_LOG(LogTemp, Log, TEXT("EnsureEditorDefaultPawnByPath: DefaultPawnClass set to %s"), *PawnClass->GetName());
}
