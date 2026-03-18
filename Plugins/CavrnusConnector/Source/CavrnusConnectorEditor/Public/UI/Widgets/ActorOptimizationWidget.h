// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file ActorOptimizationWidget.h
 * @brief Editor Utility Widget for optimizing actors with visual indicators for reversible vs irreversible options.
 */

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/CavrnusBaseEditorUtilityWidget.h"
#include "Utilities/ActorOptimizationEditorLibrary.h"
#include "Utilities/ActorOptimizationLibrary.h"
#include "ActorOptimizationWidget.generated.h"

class UTextBlock;
class UButton;
class UCheckBox;
class USpinBox;
class UMultiLineEditableTextBox;
class UProgressBar;
class UComboBoxString;

/**
 * @brief Enum to indicate if an optimization option is reversible.
 */
UENUM(BlueprintType)
enum class EActorOptimizationReversibility : uint8
{
	/** Option can be undone/rerun */
	Reversible,
	/** Option cannot be undone */
	Irreversible
};

/**
 * @brief Editor Utility Widget for optimizing selected actors.
 * 
 * Provides a UI for selecting optimization options and running them on selected actors.
 * Clearly indicates which options are reversible (can be rerun) vs irreversible.
 */
UCLASS(Blueprintable, BlueprintType)
class CAVRNUSCONNECTOREDITOR_API UActorOptimizationWidget : public UCavrnusBaseEditorUtilityWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/**
	 * @brief Gets the currently selected actors from the editor.
	 * @return Array of selected actors.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Actor Optimization")
	TArray<AActor*> GetSelectedActors() const;

	/**
	 * @brief Runs optimizations on all selected actors.
	 * Applies both editor-only and runtime optimizations based on current option settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Actor Optimization", CallInEditor)
	void OptimizeSelectedActors();

	/**
	 * @brief Callback for optimize button click.
	 */
	UFUNCTION()
	void OnOptimizeButtonClicked();

	/**
	 * @brief Checks if an editor optimization option is reversible.
	 * @param OptionName Name of the option (e.g., "bRemoveLights", "bEnableNanite", "bUseSimpleCollision")
	 * @return True if the option can be undone/rerun, false if irreversible.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Actor Optimization")
	bool IsEditorOptionReversible(const FString& OptionName) const;

	/**
	 * @brief Checks if a runtime optimization option is reversible.
	 * @param OptionName Name of the option (e.g., "bForceLOD", "bDisableShadowCasting", etc.)
	 * @return True if the option can be undone/rerun, false if irreversible.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Actor Optimization")
	bool IsRuntimeOptionReversible(const FString& OptionName) const;

	/**
	 * @brief Gets a human-readable description of reversibility for an option.
	 * @param OptionName Name of the option
	 * @param bIsEditorOption True if this is an editor option, false if runtime
	 * @return Description string explaining if/how the option can be reversed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Actor Optimization")
	FString GetReversibilityDescription(const FString& OptionName, bool bIsEditorOption) const;

	/**
	 * @brief Updates the UI based on current selection and settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Actor Optimization")
	void UpdateUI();

	/**
	 * @brief Displays optimization results in the results text area.
	 * @param Message Message to display.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Actor Optimization")
	void DisplayResults(const FString& Message);

protected:
	/** Editor-only optimization options */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | Actor Optimization | Editor Options")
	FActorOptimizationEditorOptions EditorOptions;

	/** Runtime optimization options */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | Actor Optimization | Runtime Options")
	FActorOptimizationOptions RuntimeOptions;

	/** Currently selected actors (cached) */
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus | Actor Optimization")
	TArray<AActor*> SelectedActors;

	/** Results log text */
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus | Actor Optimization")
	FString ResultsLog;

	/** Current progress percentage (0.0 to 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus | Actor Optimization")
	float ProgressPercent = 0.0f;

	/** Current status message */
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus | Actor Optimization")
	FString StatusMessage;

	// UI Widget Bindings (to be created in Blueprint)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectedActorsText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> OptimizeButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UMultiLineEditableTextBox> ResultsTextBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusTextBlock = nullptr;

	// Editor option checkboxes
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> RemoveLightsCheckBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> EnableNaniteCheckBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> UseSimpleCollisionCheckBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> MaskedMaterialModeComboBox = nullptr;

	// Runtime option checkboxes
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> ForceLODCheckBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USpinBox> LODIndexSpinBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> ForceMinimumLODCheckBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USpinBox> MinimumLODIndexSpinBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> DisableShadowCastingCheckBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> DisableCollisionOnDecorativeCheckBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> OptimizeMaterialsCheckBox = nullptr;

private:
	/**
	 * @brief Syncs UI checkboxes with the option structs.
	 */
	void SyncUIFromOptions();

	/**
	 * @brief Syncs option structs from UI checkboxes.
	 */
	void SyncOptionsFromUI();

	/**
	 * @brief Appends a message to the results log.
	 */
	void AppendToResultsLog(const FString& Message);

	/**
	 * @brief Updates progress bar and status text widgets.
	 * @param Percent Progress percentage (0.0 to 1.0)
	 * @param StatusMessage Status message to display
	 */
	void UpdateProgress(float Percent, const FString& StatusMessage);

	/**
	 * @brief Gets reversibility info for editor options.
	 */
	static TMap<FString, EActorOptimizationReversibility> GetEditorReversibilityMap();

	/**
	 * @brief Gets reversibility info for runtime options.
	 */
	static TMap<FString, EActorOptimizationReversibility> GetRuntimeReversibilityMap();

	/**
	 * @brief Converts EMaskedMaterialNaniteMode enum to display string.
	 * @param Mode The enum value to convert.
	 * @return Human-readable string for the mode.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Actor Optimization")
	static FString MaskedMaterialModeToString(EMaskedMaterialNaniteMode Mode);

	/**
	 * @brief Converts display string to EMaskedMaterialNaniteMode enum.
	 * @param ModeString The string to convert.
	 * @return The corresponding enum value, or UseLODs as default.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Actor Optimization")
	static EMaskedMaterialNaniteMode StringToMaskedMaterialMode(const FString& ModeString);
};

