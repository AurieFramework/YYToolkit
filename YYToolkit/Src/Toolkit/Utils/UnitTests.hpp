#pragma once

#define UT_OK						0x0 // Unit test passed
#define UT_GetGlobal_PointerFail	0x1 // GetGlobalObjectTest(), AUMI_GetGlobalState(&g_pGlobal);
#define UT_GetGlobal_NullFail		0x2 // GetGlobalObjectTest(), AUMI_GetGlobalState(nullptr);
#define UT_FuncIndex_PointerFail	0x3 // FunctionIndexTest(), AUMI_GetFunctionByIndex(1, &Info);
#define UT_FuncIndex_NullFail		0x4 // FunctionIndexTest(), AUMI_GetFunctionByIndex(-1, nullptr);
#define UT_FuncPtr_CodeExec			0x5 // FunctionPointerTest(), AUMI_GetCodeExecuteAddress(&Function);
#define UT_FuncPtr_CodeFunc			0x6 // FunctionPointerTest(), AUMI_GetCodeFunctionAddress(&Function);
namespace Utils::Tests
{
	int GetGlobalObjectTest();	// AUMI test - tries getting the global instance
	int FunctionIndexTest();	// AUMI test - tries finding a function by it's index
	int FunctionPointerTest();	// AUMI test - tries getting game functions' addresses
}