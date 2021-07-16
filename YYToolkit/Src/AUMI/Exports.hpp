#pragma once
#include "../Shared.hpp"

extern AUMIResult AUMI_Initialize();

// Create a CCode object using VM Bytecode
DllExport extern AUMIResult AUMI_CreateCode(CCode* outCode, void* CodeBuffer, int CodeBufferSize, int LocalVarsUsed, const char* Name);

// Create a CCode object using a C or C++ function.
DllExport extern AUMIResult AUMI_CreateYYCCode(CCode* outCode, PFUNC_YYGML Function, const char* FunctionName, const char* CodeName);

// Destroy a CCode object (doesn't matter if it's YYC or VM)
DllExport extern AUMIResult AUMI_FreeCode(CCode* Code);

// Get the pointer to the global instance (g_pGlobal or globalvar in GML)
DllExport extern AUMIResult AUMI_GetGlobalState(YYObjectBase** outState);

// Call Code_Execute() with given arguments
DllExport extern AUMIResult AUMI_ExecuteCode(YYObjectBase* Self, YYObjectBase* Other, CCode* Code, YYRValue* Arguments);

// Get the address of Code_Execute()
DllExport extern AUMIResult AUMI_GetCodeExecuteAddress(void** outAddress);

// Get the address of code_function_GET_the_function() - yes, that's a real function name.
DllExport extern AUMIResult AUMI_GetCodeFunctionAddress(void** outAddress);

// Get the AUMIFunctionInfo struct from an index inside the the_functions array.
DllExport extern AUMIResult AUMI_GetFunctionByIndex(int index, struct AUMIFunctionInfo* outInformation);

// Get the AUMIFunctionInfo struct of a function (identified by name, case insensitive).
DllExport extern AUMIResult AUMI_GetFunctionByName(const char* Name, struct AUMIFunctionInfo* outInformation);

// Get the AUMIFunctionInfo struct of a function (identified by a pointer to the function)
DllExport extern AUMIResult AUMI_GetFunctionByRoutine(PFUNC_TROUTINE Routine, struct AUMIFunctionInfo* outInformation);

// Call a builtin function with predefined arguments.
DllExport extern AUMIResult AUMI_CallBuiltinFunction(const char* Name, RValue* Result, YYObjectBase* Self, YYObjectBase* Other, int argc, RValue* Args);