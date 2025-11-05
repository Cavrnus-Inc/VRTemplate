#pragma once

#include "CoreMinimal.h"
#include "CavrnusFileImporter.h"
#include "DatasmithRuntime.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DatasmithFileImporter.generated.h"

UCLASS(Blueprintable)
class CAVRNUSCONNECTOR_API ADatasmithFileImporter : public ACavrnusFileImporter
{
    GENERATED_BODY()

public:
    virtual bool CanImport(const FString& FileExtension) const override;

protected:
    virtual void ImportFileInternal(const FString& FilePath, const FCavrnusImportSettings& Settings) override;

private:
    void PollActorStatus();

    TWeakObjectPtr<ADatasmithRuntimeActor> TargetActor;
    FString SourcePath;
    FTimerHandle PollTimerHandle;
    bool bWaitingForLoadStart = true;
};
