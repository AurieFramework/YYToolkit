#pragma once
#include "../../../Utils/SDK.hpp"
YYTKStatus AUMI_Initialize();

// Create a CCode object using VM Bytecode
DllExport YYTKStatus AUMI_CreateCode(CCode* outCode, void* CodeBuffer, int CodeBufferSize, int LocalVarsUsed, const char* Name);

// Create a CCode object using a C or C++ function.
DllExport YYTKStatus AUMI_CreateYYCCode(CCode* outCode, TGMLRoutine Function, const char* FunctionName, const char* CodeName);

// Destroy a CCode object (doesn't matter if it's YYC or VM)
DllExport YYTKStatus AUMI_FreeCode(CCode* Code);

// Get the pointer to the global instance (g_pGlobal or globalvar in GML)
DllExport YYTKStatus AUMI_GetGlobalState(YYObjectBase** outState);

// Call Code_Execute() with given arguments
DllExport YYTKStatus AUMI_ExecuteCode(YYObjectBase* Self, YYObjectBase* Other, CCode* Code, YYRValue* Arguments);

// Get the address of Code_Execute()
DllExport YYTKStatus AUMI_GetCodeExecuteAddress(void** outAddress);

// Get the address of code_function_GET_the_function() - yes, that's a real function name.
DllExport YYTKStatus AUMI_GetCodeFunctionAddress(void** outAddress);

// Get the AUMIFunctionInfo struct from an index inside the the_functions array.
DllExport YYTKStatus AUMI_GetFunctionByIndex(int index, AUMIFunctionInfo* outInformation);

// Get the AUMIFunctionInfo struct of a function (identified by name, case insensitive).
DllExport YYTKStatus AUMI_GetFunctionByName(const char* Name, AUMIFunctionInfo* outInformation);

// Get the AUMIFunctionInfo struct of a function (identified by a pointer to the function)
DllExport YYTKStatus AUMI_GetFunctionByRoutine(TRoutine Routine, AUMIFunctionInfo* outInformation);

// Call a builtin function with predefined arguments.
DllExport YYTKStatus AUMI_CallBuiltinFunction(const char* Name, RValue* Result, YYObjectBase* Self, YYObjectBase* Other, int argc, RValue* Args);