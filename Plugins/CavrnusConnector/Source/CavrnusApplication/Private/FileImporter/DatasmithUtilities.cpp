// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "FileImporter/DatasmithUtilities.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "XmlFile.h"

EDatasmithRuntimeFileType UDatasmithFileLibrary::GetFileTypeFromDatasmithFile(const FString& FilePath, FString& ErrorMessage)
{
	ErrorMessage = TEXT("");
    if (!FPaths::FileExists(FilePath))
    {
        ErrorMessage = FString::Printf(TEXT("File does not exist: %s"), *FilePath);
        return EDatasmithRuntimeFileType::Unknown;
    }

    FXmlFile XmlFile(FilePath, EConstructMethod::ConstructFromFile);
    if (!XmlFile.IsValid())
    {
        ErrorMessage = FString::Printf(TEXT("Failed to parse XML from : % s"), *FilePath);
        return EDatasmithRuntimeFileType::Unknown;
    }

    const FXmlNode* RootNode = XmlFile.GetRootNode();
    if (!RootNode)
    {
        ErrorMessage = FString::Printf(TEXT("Missing root node in: %s"), *FilePath);
        return EDatasmithRuntimeFileType::Unknown;
    }

    const FXmlNode* AppNode = RootNode->FindChildNode(TEXT("Application"));
    if (!AppNode)
    {
        ErrorMessage = FString::Printf(TEXT("Missing <Application> node in: %s"), *FilePath);
        return EDatasmithRuntimeFileType::Unknown;
    }

    FString Producer = AppNode->GetAttribute(TEXT("ProductName")).ToLower();

    // Use enum metadata to match display name
    UEnum* EnumPtr = StaticEnum<EDatasmithRuntimeFileType>();
    if (!EnumPtr)
    {
        return EDatasmithRuntimeFileType::Unknown;
    }

    for (int32 i = 0; i < EnumPtr->NumEnums(); ++i)
    {
        FText DisplayName = EnumPtr->GetDisplayNameTextByIndex(i);
        FString DisplayNameStr = DisplayName.ToString().ToLower();

        if (Producer.Contains(DisplayNameStr))
        {
            return static_cast<EDatasmithRuntimeFileType>(EnumPtr->GetValueByIndex(i));
        }
    }
    ErrorMessage = FString::Printf(TEXT("Parsing Error : % s"), *FilePath);
    return EDatasmithRuntimeFileType::Unknown;
}

FString UDatasmithFileLibrary::RewritePath(const FString& OriginalPath,
    const FString& UniqueId,
    const FString& BaseName)
{
    FString Relative = OriginalPath;
    Relative.RemoveFromStart(TEXT("/Game/"));

    // Materials and authored assets
    if (Relative.StartsWith(TEXT("Twinmotion/Materials")))
    {
        return FString::Printf(TEXT("/CavrnusCache/%s/Content/%s"),
            *UniqueId, *Relative);
    }

    // Raw payloads
    return FString::Printf(TEXT("/CavrnusCache/%s/%s_Assets/%s"),
        *UniqueId, *BaseName, *Relative);
}

// Private helper
static void GatherMeshSlotMetadataRecursive(
    const FXmlNode* Node,
    TMap<FString, FDatasmithMeshSlotMetadata>& OutMeshSlotMap)
{
    if (!Node) return;

    const FString Tag = Node->GetTag();

    // Handle StaticMesh and ActorMesh nodes
    if (Tag.Equals(TEXT("StaticMesh"), ESearchCase::IgnoreCase))
    {
        FString MeshId = Node->GetAttribute(TEXT("name"));
        FString MeshLabel = Node->GetAttribute(TEXT("label"));

        FDatasmithMeshSlotMetadata Meta;
        Meta.MeshName = MeshLabel;

        for (const FXmlNode* Child : Node->GetChildrenNodes())
        {
            if (Child->GetTag().Equals(TEXT("Material"), ESearchCase::IgnoreCase))
            {
                FString MatGuid = Child->GetAttribute(TEXT("name"));
                if (!MatGuid.IsEmpty())
                {
                    Meta.MaterialPaths.Add(MatGuid);
                }
            }
        }

        if (!MeshId.IsEmpty() && Meta.MaterialPaths.Num() > 0) // <-- only add if slots exist
        {
            OutMeshSlotMap.Add(MeshId, Meta);
            UE_LOG(LogTemp, Log, TEXT("[Datasmith] Mesh '%s' (%s) with %d slots"),
                *MeshLabel, *MeshId, Meta.MaterialPaths.Num());
        }
    }

    // Recurse into children
    for (const FXmlNode* Child : Node->GetChildrenNodes())
    {
        GatherMeshSlotMetadataRecursive(Child, OutMeshSlotMap);
    }
}


static void GatherMaterialMetadataRecursive(
    const FXmlNode* Node,
    const FString& UniqueId,
    const FString& DatasmithBaseFilename,
    TMap<FString, FDatasmithMaterialMetadata>& OutMaterialMap)
{
    if (!Node) return;

    const FString Tag = Node->GetTag();

    if (Tag.Equals(TEXT("Material"), ESearchCase::IgnoreCase) ||
        Tag.Equals(TEXT("MaterialInstance"), ESearchCase::IgnoreCase))
    {
        const FString MatId = Node->GetAttribute(TEXT("name"));
        const FString Label = Node->GetAttribute(TEXT("label"));
        const FString Path = Node->GetAttribute(TEXT("PathName"));

        if (!MatId.IsEmpty() && !Path.IsEmpty())
        {
            FDatasmithMaterialMetadata Meta;
            Meta.Label = Label;
            Meta.PathName = UDatasmithFileLibrary::RewritePath(Path, UniqueId, DatasmithBaseFilename);

            OutMaterialMap.Add(MatId, Meta);

            UE_LOG(LogTemp, Log, TEXT("[Datasmith] Gathered material '%s' (%s) rewritten path '%s'"),
                *Label, *MatId, *Meta.PathName);
        }
    }

    for (const FXmlNode* Child : Node->GetChildrenNodes())
    {
        GatherMaterialMetadataRecursive(Child, UniqueId, DatasmithBaseFilename, OutMaterialMap );
    }
}


bool UDatasmithFileLibrary::ExtractMeshAndMaterialMetadata(
    const FString& FilePath,
    const FString& UniqueId,
    const FString& BaseName,
    TMap<FString, FDatasmithMeshSlotMetadata>& OutMeshSlotMap,
    TMap<FString, FDatasmithMaterialMetadata>& OutMaterialMap,
    FString& ErrorMessage)
{
    OutMeshSlotMap.Empty();
    OutMaterialMap.Empty();

    FXmlFile XmlFile(FilePath, EConstructMethod::ConstructFromFile);
    if (!XmlFile.IsValid())
    {
        ErrorMessage = TEXT("Invalid XML: ") + FilePath;
        return false;
    }

    const FXmlNode* RootNode = XmlFile.GetRootNode();
    if (!RootNode)
    {
        ErrorMessage = TEXT("Missing root node");
        return false;
    }

    GatherMeshSlotMetadataRecursive(RootNode, OutMeshSlotMap);
    GatherMaterialMetadataRecursive(RootNode, UniqueId, BaseName, OutMaterialMap);

    return (OutMeshSlotMap.Num() > 0 || OutMaterialMap.Num() > 0);
}