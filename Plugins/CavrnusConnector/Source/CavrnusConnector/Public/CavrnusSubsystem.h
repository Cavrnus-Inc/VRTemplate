// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusConnectorSettings.h"
#include "Subsystems/EngineSubsystem.h"
#include "ServiceLocator/CavrnusServiceLocator.h"
#include "CavrnusSubsystem.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	static UCavrnusSubsystem* Get();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY()
	UCavrnusServiceLocator* Services;

	UCavrnusConnectorSettings* GetSettings() const
	{
#if WITH_EDITOR
		// Editor build: allow mutation
		if (UCavrnusConnectorSettings* FoundSettings = GetMutableDefault<UCavrnusConnectorSettings>())
		{
			return FoundSettings;
		}
#else
		// Runtime build: read-only access
		if (const UCavrnusConnectorSettings* FoundSettings = GetDefault<UCavrnusConnectorSettings>())
		{
			return const_cast<UCavrnusConnectorSettings*>(FoundSettings); // if caller expects non-const
		}
#endif
		UE_LOG(LogTemp, Error, TEXT("Unabled to find CavrnusSettings!"));
		return nullptr;
	}

private:
	FDelegateHandle WorldHandle = FDelegateHandle();

	void InitializeLoginBehavior();

	UPROPERTY()
	UCavrnusServiceLocator* ServiceLocator;
};
