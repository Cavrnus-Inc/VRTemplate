#include "PropertySync/K2Node_ResolvePropertyMetadata.h"
#include "KismetCompiler.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_CallFunction.h"
#include "EdGraphSchema_K2.h"
#include "Helpers/ReflectionHelpers.h" // Your helper function library

#include "UObject/NameTypes.h"

class UK2Node_LiteralString;
class UK2Node_LiteralBool;

void UK2Node_ResolvePropertyMetadata::AllocateDefaultPins()
{
    // Wildcard input
    UEdGraphPin* InputPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, TEXT("Input"));
    InputPin->PinFriendlyName = FText::FromString("Variable");

    // Output pins
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_String, TEXT("PropertyType"));
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, TEXT("IsArray"));
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, TEXT("IsEditable"));
}

FText UK2Node_ResolvePropertyMetadata::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromString("Resolve Property Metadata");
}

void UK2Node_ResolvePropertyMetadata::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    UEdGraphPin* InputPin = FindPin(TEXT("Input"));
    if (!InputPin || InputPin->LinkedTo.Num() == 0)
    {
        CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Missing input connection on @@")), this);
        return;
    }

    UEdGraphPin* LinkedPin = InputPin->LinkedTo[0];
    FName VariableName = LinkedPin->PinName;
    UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(SourceGraph);
    UClass* BlueprintClass = Blueprint->GeneratedClass;

    FProperty* Property = BlueprintClass->FindPropertyByName(VariableName);
    if (!Property)
    {
        CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Could not resolve property '%s' on @@"), *VariableName.ToString()), this);
        return;
    }

    FString TypeName = Property->GetClass()->GetName();
    bool bIsArray = Property->IsA<FArrayProperty>();
    bool bIsEditable = !Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance);

    // Emit PropertyType
    UK2Node_CallFunction* TypeNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    TypeNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UReflectionHelpers, MakeLiteralString), UReflectionHelpers::StaticClass());
    TypeNode->AllocateDefaultPins();
    TypeNode->FindPinChecked(TEXT("Value"))->DefaultValue = TypeName;
    CompilerContext.MovePinLinksToIntermediate(*FindPin(TEXT("PropertyType")), *TypeNode->GetReturnValuePin());

    // Emit IsArray
    UK2Node_CallFunction* ArrayNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    ArrayNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UReflectionHelpers, MakeLiteralBool), UReflectionHelpers::StaticClass());
    ArrayNode->AllocateDefaultPins();
    ArrayNode->FindPinChecked(TEXT("Value"))->DefaultValue = bIsArray ? TEXT("true") : TEXT("false");
    CompilerContext.MovePinLinksToIntermediate(*FindPin(TEXT("IsArray")), *ArrayNode->GetReturnValuePin());

    // Emit IsEditable
    UK2Node_CallFunction* EditableNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    EditableNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UReflectionHelpers, MakeLiteralBool), UReflectionHelpers::StaticClass());
    EditableNode->AllocateDefaultPins();
    EditableNode->FindPinChecked(TEXT("Value"))->DefaultValue = bIsEditable ? TEXT("true") : TEXT("false");
    CompilerContext.MovePinLinksToIntermediate(*FindPin(TEXT("IsEditable")), *EditableNode->GetReturnValuePin());

    BreakAllNodeLinks();
}

void UK2Node_ResolvePropertyMetadata::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UClass* NodeClass = GetClass();
    if (ActionRegistrar.IsOpenForRegistration(NodeClass))
    {
        UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(NodeClass);
        ActionRegistrar.AddBlueprintAction(NodeClass, Spawner);
    }
}