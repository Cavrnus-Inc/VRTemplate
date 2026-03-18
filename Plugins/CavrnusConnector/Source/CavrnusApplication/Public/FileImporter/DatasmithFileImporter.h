#pragma once

#include "CoreMinimal.h"
#include "CavrnusFileImporter.h"
#include "DatasmithRuntime.h"
#include "Engine/World.h"
#include "DatasmithFileImporter.generated.h"

UCLASS(Blueprintable)
class CAVRNUSAPPLICATION_API UDatasmithFileImporter : public UCavrnusFileImporter
{
    GENERATED_BODY()

public:
    virtual bool CanImport(const FString& FileExtension) const override;
	void CancelImport();
    virtual void BeginDestroy() override;
    TWeakObjectPtr<ADatasmithRuntimeActor> TargetActor;

    static void ProcessTwinmotionDatasmithMaterials(const FString& FilePath, ADatasmithRuntimeActor* DatasmithActor);
    static void GetAllRelevantActorsRecursive(AActor* RootActor, TArray<AActor*>& OutActors);

protected:
    virtual void ImportFileInternal(const FString& FilePath, const FCavrnusImportSettings& Settings) override;
    virtual bool AdditionalValidation(const FString& NormalizedFilePath, FString& StatusMessage) override;

private:
    void PollActorStatus();
    FTimerHandle PollTimerHandle;
    bool bWaitingForLoadStart = true;

    static int ProcessTwinmotionDatasmithChildUsingSlotNames(const AActor* Actor);
};
