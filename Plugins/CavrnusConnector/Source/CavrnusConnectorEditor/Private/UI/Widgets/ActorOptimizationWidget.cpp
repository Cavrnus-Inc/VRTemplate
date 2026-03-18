// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Widgets/ActorOptimizationWidget.h"
#include "Utilities/ActorOptimizationEditorLibrary.h"
#include "Utilities/ActorOptimizationLibrary.h"

#include "Editor.h"
#include "Selection.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/SpinBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/ProgressBar.h"
#include "Components/ComboBoxString.h"
#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"
#include "Misc/ScopedSlowTask.h"

DEFINE_LOG_CATEGORY_STATIC(LogActorOptimizationWidget, Log, All);

void UActorOptimizationWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Sync UI from options on construct
	SyncUIFromOptions();
	
	// Update UI to show current selection
	UpdateUI();
	
	// Bind optimize button if available
	if (OptimizeButton)
	{
		OptimizeButton->OnClicked.AddDynamic(this, &UActorOptimizationWidget::OnOptimizeButtonClicked);
	}
}

TArray<AActor*> UActorOptimizationWidget::GetSelectedActors() const
{
	TArray<AActor*> Actors;
	
	if (!GEditor)
	{
		return Actors;
	}
	
	USelection* Selection = GEditor->GetSelectedActors();
	if (!Selection)
	{
		return Actors;
	}
	
	for (FSelectionIterator It(*Selection); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			if (IsValid(Actor))
			{
				Actors.Add(Actor);
			}
		}
	}
	
	return Actors;
}

void UActorOptimizationWidget::OnOptimizeButtonClicked()
{
	OptimizeSelectedActors();
}

void UActorOptimizationWidget::OptimizeSelectedActors()
{
	// Sync options from UI first
	SyncOptionsFromUI();
	
	// Get selected actors
	SelectedActors = GetSelectedActors();
	
	if (SelectedActors.Num() == 0)
	{
		DisplayResults(TEXT("No actors selected. Please select one or more actors in the viewport."));
		return;
	}
	
	// Clear previous results
	ResultsLog.Empty();
	AppendToResultsLog(FString::Printf(TEXT("Optimizing %d selected actor(s)...\n"), SelectedActors.Num()));
	
	// Calculate total work items: 2 steps per actor (editor + runtime optimizations)
	int32 TotalSteps = SelectedActors.Num() * 2;
	
	// Create scoped slow task for progress dialog
	FScopedSlowTask SlowTask(TotalSteps, NSLOCTEXT("ActorOptimizationWidget", "OptimizingActors", "Optimizing Actors..."));
	SlowTask.MakeDialog();
	
	int32 TotalEditorProcessed = 0;
	int32 TotalRuntimeProcessed = 0;
	int32 SuccessCount = 0;
	int32 FailureCount = 0;
	int32 CurrentStep = 0;
	float CurrentProgressPercent = 0.0f;
	
	// Initial progress update
	UpdateProgress(0.0f, FString::Printf(TEXT("Collecting actors...")));
	
	for (AActor* Actor : SelectedActors)
	{
		if (!IsValid(Actor))
		{
			++FailureCount;
			AppendToResultsLog(FString::Printf(TEXT("  Skipping invalid actor: %s\n"), Actor ? *Actor->GetName() : TEXT("nullptr")));
			// Still count as 2 steps (editor + runtime) even though we skip
			SlowTask.EnterProgressFrame(2.0f);
			CurrentStep += 2;
			CurrentProgressPercent = static_cast<float>(CurrentStep) / static_cast<float>(TotalSteps);
			UpdateProgress(CurrentProgressPercent, FString::Printf(TEXT("Skipped invalid actor")));
			continue;
		}
		
		FString ActorName = Actor->GetActorLabel();
		AppendToResultsLog(FString::Printf(TEXT("Processing actor: %s\n"), *ActorName));
		
		// Update progress: Processing actor
		int32 ActorIndex = SuccessCount + FailureCount + 1;
		CurrentProgressPercent = static_cast<float>(CurrentStep) / static_cast<float>(TotalSteps);
		UpdateProgress(CurrentProgressPercent, FString::Printf(TEXT("Processing actor %d of %d: %s"), ActorIndex, SelectedActors.Num(), *ActorName));
		
		// Apply editor-only optimizations
		#if WITH_EDITOR
		{
			SlowTask.EnterProgressFrame(1.0f, FText::FromString(FString::Printf(TEXT("Applying editor optimizations to %s..."), *ActorName)));
			UpdateProgress(CurrentProgressPercent + (0.5f / static_cast<float>(TotalSteps)), FString::Printf(TEXT("Applying editor optimizations to %s..."), *ActorName));
			
			int32 EditorProcessed = UActorOptimizationEditorLibrary::OptimizeActorInEditor(Actor, EditorOptions);
			TotalEditorProcessed += EditorProcessed;
			++CurrentStep;
			
			if (EditorProcessed > 0)
			{
				AppendToResultsLog(FString::Printf(TEXT("  Editor optimizations: %d items processed\n"), EditorProcessed));
			}
		}
		#endif
		
		// Apply runtime optimizations
		{
			SlowTask.EnterProgressFrame(1.0f, FText::FromString(FString::Printf(TEXT("Applying runtime optimizations to %s..."), *ActorName)));
			CurrentProgressPercent = static_cast<float>(CurrentStep) / static_cast<float>(TotalSteps);
			UpdateProgress(CurrentProgressPercent, FString::Printf(TEXT("Applying runtime optimizations to %s..."), *ActorName));
			
			int32 RuntimeProcessed = UActorOptimizationLibrary::OptimizeActor(Actor, RuntimeOptions);
			TotalRuntimeProcessed += RuntimeProcessed;
			++CurrentStep;
			
			if (RuntimeProcessed > 0)
			{
				AppendToResultsLog(FString::Printf(TEXT("  Runtime optimizations: %d items processed\n"), RuntimeProcessed));
			}
		}
		
		++SuccessCount;
		AppendToResultsLog(FString::Printf(TEXT("  ✓ Completed: %s\n"), *ActorName));
	}
	
	// Final progress update
	UpdateProgress(1.0f, TEXT("Finalizing..."));
	SlowTask.EnterProgressFrame(0.0f, NSLOCTEXT("ActorOptimizationWidget", "Finalizing", "Finalizing..."));
	
	// Summary
	AppendToResultsLog(TEXT("\n=== Summary ===\n"));
	AppendToResultsLog(FString::Printf(TEXT("Actors processed: %d\n"), SuccessCount));
	AppendToResultsLog(FString::Printf(TEXT("Editor optimizations: %d total items\n"), TotalEditorProcessed));
	AppendToResultsLog(FString::Printf(TEXT("Runtime optimizations: %d total items\n"), TotalRuntimeProcessed));
	
	if (FailureCount > 0)
	{
		AppendToResultsLog(FString::Printf(TEXT("Warnings: %d actor(s) skipped\n"), FailureCount));
	}
	
	AppendToResultsLog(TEXT("\nOptimization complete! You can rerun this widget to apply different settings.\n"));
	AppendToResultsLog(TEXT("Note: Some options are reversible (can be changed), others are not.\n"));
	
	// Update the results text box
	if (ResultsTextBox)
	{
		ResultsTextBox->SetText(FText::FromString(ResultsLog));
	}
	
	// Final status message
	UpdateProgress(1.0f, FString::Printf(TEXT("Complete! Processed %d actor(s)"), SuccessCount));
	
	// Update UI to reflect changes
	UpdateUI();
}

bool UActorOptimizationWidget::IsEditorOptionReversible(const FString& OptionName) const
{
	static TMap<FString, EActorOptimizationReversibility> ReversibilityMap = GetEditorReversibilityMap();
	
	if (const EActorOptimizationReversibility* Reversibility = ReversibilityMap.Find(OptionName))
	{
		return *Reversibility == EActorOptimizationReversibility::Reversible;
	}
	
	// Default to irreversible if unknown
	return false;
}

bool UActorOptimizationWidget::IsRuntimeOptionReversible(const FString& OptionName) const
{
	static TMap<FString, EActorOptimizationReversibility> ReversibilityMap = GetRuntimeReversibilityMap();
	
	if (const EActorOptimizationReversibility* Reversibility = ReversibilityMap.Find(OptionName))
	{
		return *Reversibility == EActorOptimizationReversibility::Reversible;
	}
	
	// Default to irreversible if unknown
	return false;
}

FString UActorOptimizationWidget::GetReversibilityDescription(const FString& OptionName, bool bIsEditorOption) const
{
	bool bIsReversible = bIsEditorOption ? IsEditorOptionReversible(OptionName) : IsRuntimeOptionReversible(OptionName);
	
	if (bIsReversible)
	{
		if (OptionName == TEXT("bEnableNanite"))
		{
			return TEXT("Reversible: Can be disabled by rerunning with this option unchecked");
		}
		else if (OptionName == TEXT("bForceLOD"))
		{
			return TEXT("Reversible: Can be changed or removed (set LOD to 0)");
		}
		else if (OptionName == TEXT("bForceMinimumLOD"))
		{
			return TEXT("Reversible: Can be changed or removed (set MinimumLOD to 0)");
		}
		else if (OptionName == TEXT("bDisableShadowCasting"))
		{
			return TEXT("Reversible: Can be re-enabled by rerunning with this option unchecked");
		}
		else if (OptionName == TEXT("bDisableCollisionOnDecorative"))
		{
			return TEXT("Reversible: Can be re-enabled by rerunning with this option unchecked");
		}
		else if (OptionName == TEXT("bOptimizeMaterials"))
		{
			return TEXT("Reversible: Only logs information, no permanent changes");
		}
		else if (OptionName == TEXT("bUseSimpleCollision"))
		{
			return TEXT("Partially reversible: Modifies assets, but can be changed back");
		}
		else if (OptionName == TEXT("MaskedMaterialMode"))
		{
			return TEXT("Reversible: Can be changed by rerunning with a different mode");
		}
		else
		{
			return TEXT("Reversible: Can be changed by rerunning");
		}
	}
	else
	{
		if (OptionName == TEXT("bRemoveLights"))
		{
			return TEXT("IRREVERSIBLE: Lights are destroyed and cannot be restored");
		}
		else
		{
			return TEXT("IRREVERSIBLE: This action cannot be undone");
		}
	}
}

void UActorOptimizationWidget::UpdateUI()
{
	// Update selected actors display
	SelectedActors = GetSelectedActors();
	
	if (SelectedActorsText)
	{
		if (SelectedActors.Num() == 0)
		{
			SelectedActorsText->SetText(FText::FromString(TEXT("No actors selected")));
		}
		else if (SelectedActors.Num() == 1)
		{
			FString ActorName = SelectedActors[0] ? SelectedActors[0]->GetActorLabel() : TEXT("Unknown");
			SelectedActorsText->SetText(FText::FromString(FString::Printf(TEXT("Selected: %s"), *ActorName)));
		}
		else
		{
			SelectedActorsText->SetText(FText::FromString(FString::Printf(TEXT("%d actors selected"), SelectedActors.Num())));
		}
	}
	
	// Enable/disable optimize button based on selection
	if (OptimizeButton)
	{
		OptimizeButton->SetIsEnabled(SelectedActors.Num() > 0);
	}
}

void UActorOptimizationWidget::DisplayResults(const FString& Message)
{
	AppendToResultsLog(Message);
	
	if (ResultsTextBox)
	{
		ResultsTextBox->SetText(FText::FromString(ResultsLog));
	}
}

void UActorOptimizationWidget::SyncUIFromOptions()
{
	// Editor options
	if (RemoveLightsCheckBox)
	{
		RemoveLightsCheckBox->SetIsChecked(EditorOptions.bRemoveLights);
	}
	
	if (EnableNaniteCheckBox)
	{
		EnableNaniteCheckBox->SetIsChecked(EditorOptions.bEnableNanite);
	}
	
	if (UseSimpleCollisionCheckBox)
	{
		UseSimpleCollisionCheckBox->SetIsChecked(EditorOptions.bUseSimpleCollision);
	}
	
	// Masked Material Mode ComboBox
	if (MaskedMaterialModeComboBox)
	{
		// Clear existing options
		MaskedMaterialModeComboBox->ClearOptions();
		
		// Add both enum options as strings
		MaskedMaterialModeComboBox->AddOption(MaskedMaterialModeToString(EMaskedMaterialNaniteMode::ForceNanite));
		MaskedMaterialModeComboBox->AddOption(MaskedMaterialModeToString(EMaskedMaterialNaniteMode::UseLODs));
		
		// Set the current selection based on EditorOptions
		FString CurrentModeString = MaskedMaterialModeToString(EditorOptions.MaskedMaterialMode);
		MaskedMaterialModeComboBox->SetSelectedOption(CurrentModeString);
	}
	
	// Runtime options
	if (ForceLODCheckBox)
	{
		ForceLODCheckBox->SetIsChecked(RuntimeOptions.bForceLOD);
	}
	
	if (LODIndexSpinBox)
	{
		LODIndexSpinBox->SetValue(RuntimeOptions.LODIndex);
	}

	if (ForceMinimumLODCheckBox)
	{
		ForceMinimumLODCheckBox->SetIsChecked(RuntimeOptions.bForceMinimumLOD);
	}

	if (MinimumLODIndexSpinBox)
	{
		MinimumLODIndexSpinBox->SetValue(RuntimeOptions.MinimumLODIndex);
	}

	if (DisableShadowCastingCheckBox)
	{
		DisableShadowCastingCheckBox->SetIsChecked(RuntimeOptions.bDisableShadowCasting);
	}
	
	if (DisableCollisionOnDecorativeCheckBox)
	{
		DisableCollisionOnDecorativeCheckBox->SetIsChecked(RuntimeOptions.bDisableCollisionOnDecorative);
	}
	
	if (OptimizeMaterialsCheckBox)
	{
		OptimizeMaterialsCheckBox->SetIsChecked(RuntimeOptions.bOptimizeMaterials);
	}
}

void UActorOptimizationWidget::SyncOptionsFromUI()
{
	// Editor options
	if (RemoveLightsCheckBox)
	{
		EditorOptions.bRemoveLights = RemoveLightsCheckBox->IsChecked();
	}
	
	if (EnableNaniteCheckBox)
	{
		EditorOptions.bEnableNanite = EnableNaniteCheckBox->IsChecked();
	}
	
	if (UseSimpleCollisionCheckBox)
	{
		EditorOptions.bUseSimpleCollision = UseSimpleCollisionCheckBox->IsChecked();
	}
	
	// Masked Material Mode ComboBox
	if (MaskedMaterialModeComboBox)
	{
		FString SelectedString = MaskedMaterialModeComboBox->GetSelectedOption();
		EditorOptions.MaskedMaterialMode = StringToMaskedMaterialMode(SelectedString);
	}
	
	// Runtime options
	if (ForceLODCheckBox)
	{
		RuntimeOptions.bForceLOD = ForceLODCheckBox->IsChecked();
	}
	
	if (LODIndexSpinBox)
	{
		RuntimeOptions.LODIndex = static_cast<int32>(LODIndexSpinBox->GetValue());
	}

	if (ForceMinimumLODCheckBox)
	{
		RuntimeOptions.bForceMinimumLOD = ForceMinimumLODCheckBox->IsChecked();
	}

	if (MinimumLODIndexSpinBox)
	{
		RuntimeOptions.MinimumLODIndex = static_cast<int32>(MinimumLODIndexSpinBox->GetValue());
	}

	if (DisableShadowCastingCheckBox)
	{
		RuntimeOptions.bDisableShadowCasting = DisableShadowCastingCheckBox->IsChecked();
	}
	
	if (DisableCollisionOnDecorativeCheckBox)
	{
		RuntimeOptions.bDisableCollisionOnDecorative = DisableCollisionOnDecorativeCheckBox->IsChecked();
	}
	
	if (OptimizeMaterialsCheckBox)
	{
		RuntimeOptions.bOptimizeMaterials = OptimizeMaterialsCheckBox->IsChecked();
	}
}

void UActorOptimizationWidget::AppendToResultsLog(const FString& Message)
{
	ResultsLog += Message;
}

void UActorOptimizationWidget::UpdateProgress(float Percent, const FString& InStatusMessage)
{
	// Clamp percent to valid range
	ProgressPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
	StatusMessage = InStatusMessage;
	
	// Update progress bar widget if bound
	if (ProgressBar)
	{
		ProgressBar->SetPercent(ProgressPercent);
	}
	
	// Update status text widget if bound
	if (StatusTextBlock)
	{
		StatusTextBlock->SetText(FText::FromString(StatusMessage));
	}
}

TMap<FString, EActorOptimizationReversibility> UActorOptimizationWidget::GetEditorReversibilityMap()
{
	TMap<FString, EActorOptimizationReversibility> Map;
	
	// Irreversible
	Map.Add(TEXT("bRemoveLights"), EActorOptimizationReversibility::Irreversible);
	
	// Reversible
	Map.Add(TEXT("bEnableNanite"), EActorOptimizationReversibility::Reversible);
	Map.Add(TEXT("bUseSimpleCollision"), EActorOptimizationReversibility::Reversible);
	Map.Add(TEXT("MaskedMaterialMode"), EActorOptimizationReversibility::Reversible);
	
	return Map;
}

TMap<FString, EActorOptimizationReversibility> UActorOptimizationWidget::GetRuntimeReversibilityMap()
{
	TMap<FString, EActorOptimizationReversibility> Map;
	
	// All runtime options are reversible
	Map.Add(TEXT("bForceLOD"), EActorOptimizationReversibility::Reversible);
	Map.Add(TEXT("bForceMinimumLOD"), EActorOptimizationReversibility::Reversible);
	Map.Add(TEXT("bDisableShadowCasting"), EActorOptimizationReversibility::Reversible);
	Map.Add(TEXT("bDisableCollisionOnDecorative"), EActorOptimizationReversibility::Reversible);
	Map.Add(TEXT("bOptimizeMaterials"), EActorOptimizationReversibility::Reversible);
	
	return Map;
}

FString UActorOptimizationWidget::MaskedMaterialModeToString(EMaskedMaterialNaniteMode Mode)
{
	switch (Mode)
	{
		case EMaskedMaterialNaniteMode::ForceNanite:
			return TEXT("Force Nanite for Masked Materials");
		case EMaskedMaterialNaniteMode::UseLODs:
			return TEXT("Use LODs for Masked Materials");
		default:
			return TEXT("Use LODs for Masked Materials");
	}
}

EMaskedMaterialNaniteMode UActorOptimizationWidget::StringToMaskedMaterialMode(const FString& ModeString)
{
	if (ModeString.Contains(TEXT("Force Nanite"), ESearchCase::IgnoreCase))
	{
		return EMaskedMaterialNaniteMode::ForceNanite;
	}
	else
	{
		return EMaskedMaterialNaniteMode::UseLODs;
	}
}

