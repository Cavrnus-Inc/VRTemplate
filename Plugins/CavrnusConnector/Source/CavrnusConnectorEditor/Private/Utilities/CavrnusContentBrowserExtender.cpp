// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Utilities/CavrnusContentBrowserExtender.h"
#include "ContentBrowserModule.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Core/Contexts/CavrnusEditorContext.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "AssetManager/DataAssets/CavrnusSpawnableRegistryDataAsset.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "CavrnusContentBrowserExtender"

FDelegateHandle FCavrnusContentBrowserExtender::ExtenderDelegateHandle;

// Helper: safely get the EditorContext's DataAssetManager
static UCavrnusDataAssetManager* GetEditorDataAssetManager()
{
	UCavrnusSubsystem* Sub = UCavrnusSubsystem::Get();
	if (!Sub || !IsValid(Sub->EditorContext))
	{
		return nullptr;
	}
	return Sub->EditorContext->Get<UCavrnusDataAssetManager>();
}

static bool IsSpawnableObject(UObject* Object)
{
	if (!Object) return false;

	if (UBlueprint* BP = Cast<UBlueprint>(Object))
	{
		UClass* GenClass = BP->GeneratedClass;
		if (GenClass && !GenClass->HasAnyClassFlags(CLASS_Abstract))
		{
			if (GenClass->IsChildOf(AActor::StaticClass()) || GenClass->IsChildOf(UObject::StaticClass()))
			{
				return true;
			}
		}
	}

	UClass* ObjClass = Object->GetClass();
	if (ObjClass->IsChildOf(AActor::StaticClass()) && !ObjClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return true;
	}

	if (UClass* AsClass = Cast<UClass>(Object))
	{
		if (AsClass->IsChildOf(UObject::StaticClass()) && !AsClass->HasAnyClassFlags(CLASS_Abstract))
		{
			return true;
		}
	}

	return false;
}

static bool BuildSpawnableRefsFromObject(
	UObject* Obj,
	TSoftClassPtr<AActor>& OutActorClass,
	TSoftClassPtr<UObject>& OutObjectClass)
{
	OutActorClass = nullptr;
	OutObjectClass = nullptr;
	if (!Obj) return false;

	if (UBlueprint* BP = Cast<UBlueprint>(Obj))
	{
		if (BP->GeneratedClass && !BP->GeneratedClass->HasAnyClassFlags(CLASS_Abstract))
		{
			if (BP->GeneratedClass->IsChildOf(AActor::StaticClass()))
			{
				OutActorClass = TSoftClassPtr<AActor>(BP->GeneratedClass);
				return true;
			}
			if (BP->GeneratedClass->IsChildOf(UObject::StaticClass()))
			{
				OutObjectClass = TSoftClassPtr<UObject>(BP->GeneratedClass);
				return true;
			}
		}
	}

	if (UClass* AsClass = Cast<UClass>(Obj))
	{
		if (AsClass->IsChildOf(AActor::StaticClass()) && !AsClass->HasAnyClassFlags(CLASS_Abstract))
		{
			OutActorClass = TSoftClassPtr<AActor>(AsClass);
			return true;
		}
		if (AsClass->IsChildOf(UObject::StaticClass()) && !AsClass->HasAnyClassFlags(CLASS_Abstract))
		{
			OutObjectClass = TSoftClassPtr<UObject>(AsClass);
			return true;
		}
	}

	return false;
}

static int32 FindEntryIndexForObject(const UCavrnusSpawnableRegistryDataAsset* DataAsset, UObject* Obj)
{
	if (!DataAsset || !Obj) return INDEX_NONE;

	if (UBlueprint* BP = Cast<UBlueprint>(Obj))
	{
		Obj = BP->GeneratedClass;
	}
	if (!Obj) return INDEX_NONE;

	const FString ObjPath = Obj->GetPathName();
	const TArray<FCavrnusSpawnableEntry>& Entries = DataAsset->GetEntries();
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i].ActorClass.ToSoftObjectPath().ToString() == ObjPath ||
			Entries[i].ObjectClass.ToSoftObjectPath().ToString() == ObjPath)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

static void ShowStringInputDialog(const FString& Prompt, TFunction<void(const FString&)> OnTextEntered)
{
	TSharedPtr<SEditableTextBox> TextBox;
	TSharedRef<SWindow> Dialog = SNew(SWindow)
		.Title(FText::FromString(Prompt))
		.ClientSize(FVector2D(400, 100))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	TWeakPtr<SWindow> WeakDialog = Dialog;

	Dialog->SetContent(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().Padding(10)
		[
			SAssignNew(TextBox, SEditableTextBox)
				.HintText(FText::FromString(TEXT("Enter text here...")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(10)
		[
			SNew(SButton)
				.Text(FText::FromString(TEXT("OK")))
				.OnClicked_Lambda([WeakDialog, TextBox, OnTextEntered]()
				{
					if (WeakDialog.IsValid())
					{
						OnTextEntered(TextBox->GetText().ToString());
						FSlateApplication::Get().RequestDestroyWindow(WeakDialog.Pin().ToSharedRef());
					}
					return FReply::Handled();
				})
		]
	);

	FSlateApplication::Get().AddModalWindow(Dialog, nullptr);
}

static void ExecuteAddToSpawnList(TArray<FAssetData> SelectedAssets)
{
	for (const FAssetData& AssetData : SelectedAssets)
	{
		UObject* Obj = AssetData.GetAsset();
		if (!Obj || !IsSpawnableObject(Obj)) continue;

		ShowStringInputDialog(TEXT("Enter Cavrnus Spawn Identifier:"),
			[Obj](const FString& UserInput)
			{
				if (UserInput.IsEmpty()) return;

				UCavrnusDataAssetManager* DataAssetManager = GetEditorDataAssetManager();
				if (!DataAssetManager)
				{
					UE_LOG(LogTemp, Warning, TEXT("Cavrnus subsystem not ready - cannot add to spawn list"));
					return;
				}

				UCavrnusSpawnableRegistryDataAsset* DataAsset = DataAssetManager->GetAsset<UCavrnusSpawnableRegistryDataAsset>();
				if (!DataAsset) return;

				TSoftClassPtr<AActor> ActorClass;
				TSoftClassPtr<UObject> ObjectClass;
				if (!BuildSpawnableRefsFromObject(Obj, ActorClass, ObjectClass))
				{
					UE_LOG(LogTemp, Warning, TEXT("Object is not a supported Cavrnus Spawnable: %s"), *Obj->GetName());
					return;
				}

				const FName Identifier(*UserInput);
				if (ActorClass.IsValid())
				{
					DataAsset->AddSpawnableActor(Identifier, ActorClass);
				}
				else if (ObjectClass.IsValid())
				{
					DataAsset->AddSpawnableObject(Identifier, ObjectClass);
				}
			});
	}
}

static void ExecuteRemoveFromSpawnList(TArray<FAssetData> SelectedAssets)
{
	UCavrnusDataAssetManager* DataAssetManager = GetEditorDataAssetManager();
	if (!DataAssetManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cavrnus subsystem not ready - cannot remove from spawn list"));
		return;
	}

	UCavrnusSpawnableRegistryDataAsset* DataAsset = DataAssetManager->GetAsset<UCavrnusSpawnableRegistryDataAsset>();
	if (!DataAsset) return;

	for (const FAssetData& AssetData : SelectedAssets)
	{
		UObject* Obj = AssetData.GetAsset();
		if (!Obj) continue;

		const int32 ExistingIdx = FindEntryIndexForObject(DataAsset, Obj);
		if (ExistingIdx == INDEX_NONE)
		{
			UE_LOG(LogTemp, Log, TEXT("Asset not found in Cavrnus spawn list: %s"), *Obj->GetName());
			continue;
		}

		const FName Key = DataAsset->GetEntries()[ExistingIdx].Key;
		DataAsset->RemoveSpawnable(Key);
	}
}

static void ExecutePrintSpawnList()
{
	UCavrnusDataAssetManager* DataAssetManager = GetEditorDataAssetManager();
	if (!DataAssetManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cavrnus subsystem not ready - cannot print spawn list"));
		return;
	}

	UCavrnusSpawnableRegistryDataAsset* DataAsset = DataAssetManager->GetAsset<UCavrnusSpawnableRegistryDataAsset>();
	if (!DataAsset) return;

	for (const FCavrnusSpawnableEntry& Entry : DataAsset->GetEntries())
	{
		FString ValueStr = TEXT("<none>");
		FString TypePrefix;

		if (!Entry.ActorClass.IsNull())
		{
			TypePrefix = TEXT("[Actor] ");
			ValueStr = Entry.ActorClass.Get() ? Entry.ActorClass.Get()->GetPathName() : Entry.ActorClass.ToSoftObjectPath().ToString();
		}
		else if (!Entry.ObjectClass.IsNull())
		{
			TypePrefix = TEXT("[Object] ");
			ValueStr = Entry.ObjectClass.Get() ? Entry.ObjectClass.Get()->GetPathName() : Entry.ObjectClass.ToSoftObjectPath().ToString();
		}

		UE_LOG(LogTemp, Log, TEXT("%s%s : %s"), *TypePrefix, *Entry.Key.ToString(), *ValueStr);
	}
}

void FCavrnusContentBrowserExtender::Register()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedAssets>& Extenders = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

	FContentBrowserMenuExtender_SelectedAssets Delegate =
		FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FCavrnusContentBrowserExtender::OnExtendAssetViewContextMenu);
	ExtenderDelegateHandle = Delegate.GetHandle();
	Extenders.Add(Delegate);
}

void FCavrnusContentBrowserExtender::Unregister()
{
	if (!FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		return;
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedAssets>& Extenders = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	Extenders.RemoveAll([](const FContentBrowserMenuExtender_SelectedAssets& Delegate)
	{
		return Delegate.GetHandle() == ExtenderDelegateHandle;
	});
}

TSharedRef<FExtender> FCavrnusContentBrowserExtender::OnExtendAssetViewContextMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();

	// Only add menu items if the DataAssetManager is available
	UCavrnusDataAssetManager* DataAssetManager = GetEditorDataAssetManager();
	if (!DataAssetManager)
	{
		return Extender;
	}

	// Check if any selected asset is spawnable (without loading heavy assets during menu build)
	bool bHasSpawnable = false;
	for (const FAssetData& AssetData : SelectedAssets)
	{
		UClass* AssetClass = AssetData.GetClass();
		if (AssetClass && (AssetClass->IsChildOf(UBlueprint::StaticClass()) ||
			AssetClass->IsChildOf(UStaticMesh::StaticClass()) ||
			AssetClass->IsChildOf(UMaterial::StaticClass()) ||
			AssetClass->IsChildOf(AActor::StaticClass())))
		{
			bHasSpawnable = true;
			break;
		}
	}

	if (!bHasSpawnable)
	{
		return Extender;
	}

	// Copy SelectedAssets for the lambda capture
	TArray<FAssetData> CapturedAssets = SelectedAssets;

	Extender->AddMenuExtension(
		"CommonAssetActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateLambda([CapturedAssets](FMenuBuilder& MenuBuilder)
		{
			BuildCavrnusMenuEntries(MenuBuilder, CapturedAssets);
		})
	);

	return Extender;
}

void FCavrnusContentBrowserExtender::BuildCavrnusMenuEntries(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	UCavrnusDataAssetManager* DataAssetManager = GetEditorDataAssetManager();
	if (!DataAssetManager) return;

	UCavrnusSpawnableRegistryDataAsset* DataAsset = DataAssetManager->GetAsset<UCavrnusSpawnableRegistryDataAsset>();
	if (!DataAsset) return;

	// Check each selected asset: load only to determine add vs remove
	bool bShowAdd = false;
	bool bShowRemove = false;

	for (const FAssetData& AssetData : SelectedAssets)
	{
		UObject* Obj = AssetData.GetAsset();
		if (!Obj || !IsSpawnableObject(Obj)) continue;

		const int32 ExistingIdx = FindEntryIndexForObject(DataAsset, Obj);
		if (ExistingIdx != INDEX_NONE)
		{
			bShowRemove = true;
		}
		else
		{
			bShowAdd = true;
		}
	}

	if (bShowAdd)
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AddToSpawnList", "Add to Cavrnus Spawn List"),
			LOCTEXT("AddToSpawnListTooltip", "Adds this asset to the Cavrnus spawnable list"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([SelectedAssets]() { ExecuteAddToSpawnList(SelectedAssets); }))
		);
	}

	if (bShowRemove)
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("RemoveFromSpawnList", "Remove From Cavrnus Spawn List"),
			LOCTEXT("RemoveFromSpawnListTooltip", "Removes this asset from the Cavrnus spawnable list"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([SelectedAssets]() { ExecuteRemoveFromSpawnList(SelectedAssets); }))
		);
	}

	MenuBuilder.AddMenuEntry(
		LOCTEXT("PrintSpawnList", "Print Cavrnus Spawn List"),
		LOCTEXT("PrintSpawnListTooltip", "Prints assets in the Cavrnus spawnable list"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]() { ExecutePrintSpawnList(); }))
	);
}

#undef LOCTEXT_NAMESPACE
