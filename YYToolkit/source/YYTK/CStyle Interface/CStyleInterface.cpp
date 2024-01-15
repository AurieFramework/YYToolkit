#include "../Tool.hpp"

using namespace Aurie;
using namespace YYTK;

// Slightly modified and more importantly COMPLETELY UNDOCUMENTED version of YYTKInterface.
// Used in C# interop.
// Do not use this in your code, use the module interface "YYTK_Main"

EXPORTED void QueryVersion(
	IN YYTKInterface* Interface, 
	OUT short& Major,
	OUT short& Minor,
	OUT short& Patch
)
{
	return Interface->QueryVersion(
		Major,
		Minor,
		Patch
	);
}

EXPORTED AurieStatus GetNamedRoutineIndex(
	IN YYTKInterface* Interface,
	IN const char* FunctionName, 
	OUT int* FunctionIndex
)
{
	return Interface->GetNamedRoutineIndex(
		FunctionName,
		FunctionIndex
	);
}

EXPORTED AurieStatus GetNamedRoutinePointer(
	IN YYTKInterface* Interface,
	IN const char* FunctionName, 
	OUT PVOID* FunctionPointer
)
{
	return Interface->GetNamedRoutinePointer(
		FunctionName,
		FunctionPointer
	);
}

EXPORTED AurieStatus GetGlobalInstance(
	IN YYTKInterface* Interface, 
	OUT CInstance** Instance
)
{
	return Interface->GetGlobalInstance(
		Instance
	);
}

// CallBuiltin left out, RValue return values not supported by C ABI

EXPORTED AurieStatus CallBuiltinEx(
	IN YYTKInterface* Interface,
	OUT RValue& Result,
	IN const char* FunctionName,
	IN CInstance* SelfInstance, 
	IN CInstance* OtherInstance,
	IN std::vector<RValue> Arguments
)
{
	return Interface->CallBuiltinEx(
		Result,
		FunctionName,
		SelfInstance,
		OtherInstance,
		Arguments
	);
}

EXPORTED void Print(
	IN YYTKInterface* Interface, 
	IN CmColor Color, 
	IN const char* Format,
	IN ...
)
{
	return Interface->Print(
		Color,
		Format
	);
}

EXPORTED void PrintInfo(
	IN YYTKInterface* Interface, 
	IN const char* Format,
	IN ...
)
{
	return Interface->PrintInfo(
		Format
	);
}

EXPORTED void PrintWarning(
	IN YYTKInterface* Interface, 
	IN const char* Format,
	IN ...
)
{
	return Interface->PrintWarning(
		Format
	);
}

EXPORTED void PrintError(
	IN YYTKInterface* Interface,
	IN const char* Filepath,
	IN const int Line,
	IN const char* Format,
	IN ...
)
{
	return Interface->PrintError(
		Filepath,
		Line,
		Format
	);
}

EXPORTED AurieStatus CreateCallback(
	IN YYTKInterface* Interface, 
	IN AurieModule* Module, 
	IN EventTriggers Trigger,
	IN PVOID Routine, 
	IN int32_t Priority
)
{
	return Interface->CreateCallback(
		Module,
		Trigger,
		Routine,
		Priority
	);
}

EXPORTED AurieStatus GetInstanceMember(
	IN YYTKInterface* Interface,
	IN RValue Instance,
	IN const char* MemberName,
	OUT RValue*& Member
)
{
	return Interface->GetInstanceMember(
		Instance,
		MemberName,
		Member
	);
}

EXPORTED AurieStatus EnumInstanceMembers(
	IN YYTKInterface* Interface,
	IN RValue Instance,
	IN bool(*EnumFunction)(IN const char* MemberName, RValue* Value)
)
{
	return Interface->EnumInstanceMembers(
		Instance,
		EnumFunction
	);
}

EXPORTED AurieStatus RValueToString(
	IN YYTKInterface* Interface,
	IN const RValue& Value,
	OUT const char*& String
)
{
	if (!Interface->GetRunnerInterface().YYGetString)
		return AURIE_MODULE_INTERNAL_ERROR;

	String = Interface->GetRunnerInterface().YYGetString(&Value, 0);
	return AURIE_SUCCESS;
}

EXPORTED const YYRunnerInterface& GetRunnerInterface(
	IN YYTKInterface* Interface
)
{
	return Interface->GetRunnerInterface();
}

EXPORTED void InvalidateAllCaches(
	IN YYTKInterface* Interface
)
{
	return Interface->InvalidateAllCaches();
}

EXPORTED AurieStatus GetScriptData(
	IN YYTKInterface* Interface,
	IN int Index,
	OUT CScript*& Script
)
{
	return Interface->GetScriptData(
		Index,
		Script
	);
}

EXPORTED AurieStatus GetBuiltinVariableIndex(
	IN YYTKInterface* Interface,
	IN const char* Name,
	OUT size_t& Index
)
{
	return Interface->GetBuiltinVariableIndex(
		Name,
		Index
	);
}

EXPORTED AurieStatus GetBuiltinVariableInformation(
	IN YYTKInterface* Interface,
	IN size_t Index,
	OUT RVariableRoutine*& VariableInformation
)
{
	return Interface->GetBuiltinVariableInformation(
		Index,
		VariableInformation
	);
}

EXPORTED AurieStatus GetBuiltin(
	IN YYTKInterface* Interface,
	IN const char* Name,
	IN CInstance* TargetInstance,
	OPTIONAL IN int ArrayIndex,
	OUT RValue& Value
)
{
	return Interface->GetBuiltin(
		Name,
		TargetInstance,
		ArrayIndex,
		Value
	);
}

EXPORTED AurieStatus SetBuiltin(
	IN YYTKInterface* Interface,
	IN const char* Name,
	IN CInstance* TargetInstance,
	OPTIONAL IN int ArrayIndex,
	IN RValue& Value
)
{
	return Interface->SetBuiltin(
		Name,
		TargetInstance,
		ArrayIndex,
		Value
	);
}

EXPORTED AurieStatus GetArrayEntry(
	IN YYTKInterface* Interface,
	IN RValue& Value,
	IN size_t ArrayIndex,
	OUT RValue*& ArrayElement
)
{
	return Interface->GetArrayEntry(
		Value,
		ArrayIndex,
		ArrayElement
	);
}

EXPORTED AurieStatus GetArraySize(
	IN YYTKInterface* Interface,
	IN RValue& Value,
	OUT size_t& Size
)
{
	return Interface->GetArraySize(
		Value,
		Size
	);
}

EXPORTED AurieStatus GetRoomData(
	IN YYTKInterface* Interface,
	IN int32_t RoomID,
	OUT CRoom*& Room
)
{
	return Interface->GetRoomData(
		RoomID,
		Room
	);
}

EXPORTED AurieStatus GetCurrentRoomData(
	IN YYTKInterface* Interface,
	OUT CRoom*& CurrentRoom
)
{
	return Interface->GetCurrentRoomData(
		CurrentRoom
	);
}

EXPORTED AurieStatus GetInstanceObject(
	IN YYTKInterface* Interface,
	IN int32_t InstanceID,
	OUT CInstance*& Instance
)
{
	return Interface->GetInstanceObject(
		InstanceID,
		Instance
	);
}

EXPORTED AurieStatus InvokeWithObject(
	IN YYTKInterface* Interface,
	IN const RValue& Object,
	IN void(*Method)(CInstance* Self, CInstance* Other)
)
{
	return Interface->InvokeWithObject(
		Object,
		Method
	);
}