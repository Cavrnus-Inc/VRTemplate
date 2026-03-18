// Copyright (c) 2025 Cavrnus, Inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Utilities/ActorOptimizationEditorTool.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

class IDetailsView;

/**
 * Slate panel for the Actor Optimization Editor Tool.
 * - Auto-populates TargetActor from editor selection
 * - Displays run flags + parameters for editor/runtime optimization
 * - Provides Optimize button, progress bar, status, results, analysis, and rolling log
 * - Shows "What will run" summary based on split intent
 */
class SActorOptimizationEditorPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SActorOptimizationEditorPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual ~SActorOptimizationEditorPanel();

    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
    /** The tool UObject that stores options, analysis, and executes optimization */
    TStrongObjectPtr<UActorOptimizationEditorTool> ToolInstance;

    /** DetailsView for editing tool properties */
    TSharedPtr<IDetailsView> DetailsView;

    /** Rolling log widget */
    TSharedPtr<SMultiLineEditableTextBox> LogTextBox;

    /** Analysis summary widget */
    TSharedPtr<SMultiLineEditableTextBox> AnalysisTextBox;

    /** "What will run" summary widget */
    TSharedPtr<SMultiLineEditableTextBox> WhatWillRunTextBox;

private:
    /** Called when the Optimize button is clicked */
    FReply OnOptimizeClicked() const;

    /** Called when the editor selection changes */
    void OnEditorSelectionChanged(UObject* NewSelection);


    /** Initialize TargetActor from current selection when tab opens */
    void InitializeSelectionFromEditor();

    /** UI bindings */
    TOptional<float> GetProgress() const;
    FText GetProgressStatusText() const;
    FText GetResultsText() const;
    FText GetAnalysisText() const;
    FText GetWhatWillRunText() const;
};
