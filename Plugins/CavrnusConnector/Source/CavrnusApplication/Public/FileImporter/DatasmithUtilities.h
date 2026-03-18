#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "XmlParser.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/EngineVersion.h"
#include "Misc/Paths.h"
#include "Materials/MaterialInterface.h"
#include "DatasmithUtilities.generated.h"

class DatasmithUtilities;

UENUM(BlueprintType)
enum class EDatasmithRuntimeFileType : uint8
{
    Twinmotion     UMETA(DisplayName = "Twinmotion"),
    Solidworks     UMETA(DisplayName = "SolidWorks"),
    SketchUp       UMETA(DisplayName = "Sketchup Pro"),
    Revit          UMETA(DisplayName = "Revit"),
    Navisworks     UMETA(DisplayName = "Navisworks"),
    _3DSMax        UMETA(DisplayName = "3dsmax"),
    Rhino          UMETA(DisplayName = "Rhino"),
    Archicad       UMETA(DisplayName = "Archicad"),
    FormZ          UMETA(DisplayName = "FormZ"),
    OpenFlight     UMETA(DisplayName = "OpenFlight"),
    CETDesigner    UMETA(DisplayName = "CET Designer"),
    Allplan        UMETA(DisplayName = "Allplan"),
    CDB            UMETA(DisplayName = "CDB"),
    Unknown        UMETA(DisplayName = "Unknown"),
};

USTRUCT()
struct FDatasmithMaterialMetadata
{
    GENERATED_BODY()

    FString Id;
    FString Label;
    FString PathName;
    TArray<FString> TextureRefs;
};

USTRUCT()
struct FDatasmithMeshSlotMetadata
{
    GENERATED_BODY()

    FString MeshName;
    TArray<FString> MaterialPaths; // one per slot
};

UCLASS(Blueprintable)
class UDatasmithFileLibrary : public UBlueprintFunctionLibrary 
{
    GENERATED_BODY()

public:
    static bool ExtractMeshAndMaterialMetadata(
        const FString& FilePath,
        const FString& UniqueId,
        const FString& BaseName,
        TMap<FString, FDatasmithMeshSlotMetadata>& OutMeshSlotMap,
        TMap<FString, FDatasmithMaterialMetadata>& OutMaterialMap,
		FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Datasmith")
    static EDatasmithRuntimeFileType GetFileTypeFromDatasmithFile(const FString& FilePath, FString& ErrorMessage);

    /**
     * Converts a Datasmith file type enum to a readable string.
     * @param FileType The enum value.
     * @return A human-readable string representing the file type.
     */
    UFUNCTION(BlueprintCallable, Category = "Datasmith")
    static FString GetRuntimeFileTypeString(EDatasmithRuntimeFileType FileType)
    {
        UEnum* EnumPtr = StaticEnum<EDatasmithRuntimeFileType>();
        if (!EnumPtr)
        {
            return TEXT("Unknown");
        }
        FText DisplayName = EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(FileType));
        return DisplayName.ToString();
    }

    static FString RewritePath(const FString& OriginalPath, const FString& UniqueId, const FString& BaseName);
    /**
     * Convenience function that returns both the file type and its string representation.
     */
    UFUNCTION(BlueprintCallable, Category = "Datasmith")
    static bool AnalyzeDatasmithFile(const FString& FilePath, EDatasmithRuntimeFileType& OutFileType, FString& OutFileTypeString, FString& ErrorMessage)
    {
        OutFileType = GetFileTypeFromDatasmithFile(FilePath, ErrorMessage);
        OutFileTypeString = GetRuntimeFileTypeString(OutFileType);
        if (OutFileType == EDatasmithRuntimeFileType::Unknown)
            return false;
        return true;
    }

    UFUNCTION(BlueprintCallable, Category = "Datasmith")
    static FString GetDatasmithFileVersion(const FString& FilePath)
    {
        if (!FPaths::FileExists(FilePath))
        {
            return TEXT("");
        }
        FXmlFile XmlFile(FilePath, EConstructMethod::ConstructFromFile);
        if (!XmlFile.IsValid())
        {
            return TEXT("");
        }
        const FXmlNode* RootNode = XmlFile.GetRootNode();
        if (!RootNode)
        {
            return TEXT("");
        }
        const FXmlNode* VersionNode = RootNode->FindChildNode(TEXT("SDKVersion"));
        if (!VersionNode)
        {
            return TEXT("");
        }
        return VersionNode->GetContent();
    }

    UFUNCTION(BlueprintCallable, Category = "Datasmith")
    static bool IsCompatibleWithEngineVersion(const FString& FilePath, FString& ErrorMsg)
    {
        ErrorMsg.Empty();

        FString VersionStr = GetDatasmithFileVersion(FilePath);
        if (VersionStr.IsEmpty())
        {
            ErrorMsg = FString::Printf(TEXT("No SDKVERSION found in file: %s"), *FilePath);
            UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);
            return false;
        }

        TArray<FString> VersionParts;
        VersionStr.ParseIntoArray(VersionParts, TEXT("."));
        if (VersionParts.Num() < 2)
        {
            ErrorMsg = FString::Printf(TEXT("Improperly formatted SDKVERSION in file: %s. Expected format 5.X or 5.X.X but got: %s"), *FilePath, *VersionStr);
            UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);
            return false;
        }

        int32 DatasmithMajor = FCString::Atoi(*VersionParts[0]);
        int32 DatasmithMinor = FCString::Atoi(*VersionParts[1]);
        FEngineVersion EngineVersion = FEngineVersion::Current();
        int32 EngineMajor = EngineVersion.GetMajor();
        int32 EngineMinor = EngineVersion.GetMinor();
        if (DatasmithMajor < EngineMajor || (DatasmithMajor == EngineMajor && DatasmithMinor <= EngineMinor))
        {
            ErrorMsg = FString::Printf(TEXT("Datasmith File Version %d.%d supported"), DatasmithMajor, DatasmithMinor);

            return true;
        }

        ErrorMsg = FString::Printf(TEXT("File %s rejected due to version incompatibility: Engine %d.%d < File %d.%d"),
            *FilePath, EngineMajor, EngineMinor, DatasmithMajor, DatasmithMinor);
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);
        return false;
    }

};




