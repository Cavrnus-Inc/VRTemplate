// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CavrnusAppLifecycleHandler.h"
#include "Subsystems/EngineSubsystem.h"
#include "CavrnusSubsystem.generated.h"

class UCavrnusRuntimeContext;
class UCavrnusEditorContext;

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	static UCavrnusSubsystem* Get();
	
	UPROPERTY() UCavrnusEditorContext* EditorContext = nullptr;
	UPROPERTY() UCavrnusRuntimeContext* RuntimeContext = nullptr;
	
	UPROPERTY()
	UCavrnusAppLifecycleHandler* AppHandler;

	/** Checks if RuntimeContext is ready for use */
	bool IsRuntimeContextReady() const { return RuntimeContext != nullptr; }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
private:
	bool bInitialized = false;
	
	void OnAppStart(const bool IsEditor, UWorld* World);
	void OnAppEnd(const bool IsEditor);
	
	void DisposeEditorCtx();
	void DisposeRuntimeCtx();
	void CreateEditorContext();
	void CreateRuntimeContext(UWorld* World);
};