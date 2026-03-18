// Copyright (c) 2025 Cavrnus, Inc. All rights reserved.

#include "Utilities/SActorOptimizationEditorPanel.h"

#include "Utilities/ActorOptimizationEditorTool.h"

#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "IDetailsView.h"

#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Selection.h"
#include "Components/ActorComponent.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
void SActorOptimizationEditorPanel::Construct(const FArguments& InArgs)
{
    // Create tool instance
    ToolInstance = TStrongObjectPtr<UActorOptimizationEditorTool>(
        NewObject<UActorOptimizationEditorTool>(GetTransientPackage(), TEXT("ActorOptimizationEditorToolInstance"))
    );
    ToolInstance->AddToRoot(); // Prevent GC

    // Create details view
    FPropertyEditorModule& PropertyEditorModule =
        FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bHideSelectionTip = true;
    DetailsViewArgs.bLockable = false;
    DetailsViewArgs.bUpdatesFromSelection = false;
    DetailsViewArgs.bShowOptions = true;
    DetailsViewArgs.bAllowSearch = true;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

    DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    DetailsView->SetObject(ToolInstance.Get());

    // Initialize from current editor selection
    InitializeSelectionFromEditor();

    // Register selection change delegate
    if (GEditor && GEditor->GetSelectedActors())
    {
        GEditor->GetSelectedActors()->SelectObjectEvent.AddRaw(
            this, &SActorOptimizationEditorPanel::OnEditorSelectionChanged);
    }

    ChildSlot
        [
            SNew(SVerticalBox)

                // Details panel (RunFlags + EditorOptions + RuntimeOptions, etc.)
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                .Padding(4.0f)
                [
                    DetailsView.ToSharedRef()
                ]

                // Optimize button
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f)
                .HAlign(HAlign_Left)
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("Optimize Actors")))
                        .OnClicked(this, &SActorOptimizationEditorPanel::OnOptimizeClicked)
                ]

                // Status text (multi-line)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f)
                [
                    SNew(SMultiLineEditableTextBox)
                        .IsReadOnly(true)
                        .AutoWrapText(true)
                        .Text(this, &SActorOptimizationEditorPanel::GetProgressStatusText)
                ]

                // Progress bar
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f)
                [
                    SNew(SProgressBar)
                        .Percent(this, &SActorOptimizationEditorPanel::GetProgress)
                ]

                // Results text (multi-line)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f)
                [
                    SNew(SMultiLineEditableTextBox)
                        .IsReadOnly(true)
                        .AutoWrapText(true)
                        .Text(this, &SActorOptimizationEditorPanel::GetResultsText)
                ]

                // "What will run" label
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f, 8.0f, 4.0f, 2.0f)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("What Will Run")))
                ]

                // "What will run" summary (multi-line)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f)
                [
                    SAssignNew(WhatWillRunTextBox, SMultiLineEditableTextBox)
                        .IsReadOnly(true)
                        .AutoWrapText(true)
                        .Text(this, &SActorOptimizationEditorPanel::GetWhatWillRunText)
                ]

                // Analysis label
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f, 8.0f, 4.0f, 2.0f)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Actor Tree Analysis")))
                ]

                // Analysis text (multi-line)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f)
                [
                    SAssignNew(AnalysisTextBox, SMultiLineEditableTextBox)
                        .IsReadOnly(true)
                        .AutoWrapText(true)
                        .Text(this, &SActorOptimizationEditorPanel::GetAnalysisText)
                ]

                // Log label
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f, 8.0f, 4.0f, 2.0f)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Log")))
                ]

                // Rolling log (multi-line, scrollable)
                + SVerticalBox::Slot()
                .FillHeight(0.4f)
                .Padding(4.0f)
                [
                    SAssignNew(LogTextBox, SMultiLineEditableTextBox)
                        .IsReadOnly(true)
                        .AlwaysShowScrollbars(true)
                        .AutoWrapText(true)
                ]
        ];
}

// ------------------------------------------------------------
// Destructor — unregister selection delegate
// ------------------------------------------------------------
SActorOptimizationEditorPanel::~SActorOptimizationEditorPanel()
{
    if (GEditor && GEditor->GetSelectedActors())
    {
        GEditor->GetSelectedActors()->SelectObjectEvent.RemoveAll(this);
    }
}

// ------------------------------------------------------------
// Tick — drive async optimizer + update log
// ------------------------------------------------------------
void SActorOptimizationEditorPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    if (ToolInstance.IsValid() && ToolInstance->IsRunning())
    {
        const int32 BatchSize = 25; // tune as needed
        ToolInstance->TickOptimization(BatchSize);
    }

    // Update rolling log text
    if (ToolInstance.IsValid() && LogTextBox.IsValid())
    {
        FString Combined;
        Combined.Reserve(4096);

        for (const FString& Line : ToolInstance->LogLines)
        {
            Combined += Line;
            Combined += TEXT("\n");
        }

        LogTextBox->SetText(FText::FromString(Combined));
    }
}

// ------------------------------------------------------------
// Optimize button handler
// ------------------------------------------------------------
FReply SActorOptimizationEditorPanel::OnOptimizeClicked() const
{
    if (ToolInstance.IsValid())
    {
        ToolInstance->StartOptimization();
    }
    return FReply::Handled();
}

// ------------------------------------------------------------
// Initialize TargetActor from current selection
// ------------------------------------------------------------
void SActorOptimizationEditorPanel::InitializeSelectionFromEditor()
{
    if (!GEditor)
        return;

    USelection* Selection = GEditor->GetSelectedActors();
    if (!Selection || Selection->Num() == 0)
        return;

    UObject* SelectedObj = Selection->GetSelectedObject(0);
    AActor* SelectedActor = Cast<AActor>(SelectedObj);

    if (!SelectedActor)
    {
        if (UActorComponent* Comp = Cast<UActorComponent>(SelectedObj))
        {
            SelectedActor = Comp->GetOwner();
        }
    }

    if (SelectedActor && ToolInstance.IsValid())
    {
        ToolInstance->TargetActor = SelectedActor;
        DetailsView->SetObject(ToolInstance.Get());  // forces property sync
        ToolInstance->OnSelectedActorChanged(SelectedActor);
    }
}

// ------------------------------------------------------------
// Editor selection changed handler
// ------------------------------------------------------------
void SActorOptimizationEditorPanel::OnEditorSelectionChanged(UObject* NewSelection)
{
    UE_LOG(LogTemp, Warning, TEXT("SelectionChangedEvent FIRED. NewSelection=%s"),
        NewSelection ? *NewSelection->GetName() : TEXT("NULL"));

    if (!ToolInstance.IsValid())
        return;

    AActor* SelectedActor = Cast<AActor>(NewSelection);

    if (!SelectedActor)
    {
        if (UActorComponent* Comp = Cast<UActorComponent>(NewSelection))
        {
            SelectedActor = Comp->GetOwner();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Resolved SelectedActor=%s"),
        SelectedActor ? *SelectedActor->GetName() : TEXT("NULL"));

    if (SelectedActor)
    {
        // Update the tool’s property FIRST
        ToolInstance->TargetActor = SelectedActor;

        UE_LOG(LogTemp, Warning, TEXT("ToolInstance->TargetActor set to %s"),
            *ToolInstance->TargetActor->GetName());

        // Sync the details panel so it doesn’t overwrite our change
        DetailsView->SetObject(ToolInstance.Get());

        // Now run the tool’s selection logic
        ToolInstance->OnSelectedActorChanged(SelectedActor);
    }
}


// ------------------------------------------------------------
// UI Bindings
// ------------------------------------------------------------
TOptional<float> SActorOptimizationEditorPanel::GetProgress() const
{
    if (ToolInstance.IsValid())
    {
        return ToolInstance->Progress;
    }
    return 0.0f;
}

FText SActorOptimizationEditorPanel::GetProgressStatusText() const
{
    if (ToolInstance.IsValid())
    {
        return FText::FromString(ToolInstance->ProgressStatusText);
    }
    return FText::FromString(TEXT("Idle"));
}

FText SActorOptimizationEditorPanel::GetResultsText() const
{
    if (ToolInstance.IsValid())
    {
        return FText::FromString(ToolInstance->ResultsText);
    }
    return FText::GetEmpty();
}

FText SActorOptimizationEditorPanel::GetAnalysisText() const
{
    if (ToolInstance.IsValid())
    {
        return FText::FromString(ToolInstance->AnalysisSummary);
    }
    return FText::GetEmpty();
}

FText SActorOptimizationEditorPanel::GetWhatWillRunText() const
{
    if (ToolInstance.IsValid())
    {
        return FText::FromString(ToolInstance->WhatWillRunSummary);
    }
    return FText::GetEmpty();
}
