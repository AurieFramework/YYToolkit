#include "../../../Utils/SDK.hpp"
#include <cstring>
#include <Windows.h>
#include <Psapi.h>
#include "Exports.hpp"
static PFUNC_CEXEC g_pCodeExecute = NULL;
static void(*g_pGrabCodeFunction)(int, char** Name, void** Routine, int* Argc, void**) = NULL;

static unsigned long FindPattern(const char* Pattern, const char* Mask, long base, unsigned size)
{
	size_t PatternSize = strlen(Mask);

	for (unsigned i = 0; i < size - PatternSize; i++)
	{
		int found = 1;
		for (unsigned j = 0; j < PatternSize; j++)
		{
			found &= Mask[j] == '?' || Pattern[j] == *(char*)(base + i + j);
		}

		if (found)
			return (base + i);
	}

	return (unsigned long)NULL;
}

static MODULEINFO GetCurrentModuleInfo()
{
	MODULEINFO modinfo = { 0 };
	HMODULE hModule = GetModuleHandleA(NULL);
	if (hModule == 0)
		return modinfo;
	GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
	return modinfo;
}

YYTKStatus AUMI_Initialize()
{
	YYTKStatus Result = YYTK_OK;
	if (Result = AUMI_GetCodeExecuteAddress((PVOID*)&g_pCodeExecute))
		return Result;

	if (Result = AUMI_GetCodeFunctionAddress((PVOID*)&g_pGrabCodeFunction))
		return Result;

	return Result;
}

YYTKStatus AUMI_CreateCode(CCode* outCode, void* CodeBuffer, int CodeBufferSize, int LocalVarsUsed, const char* Name)
{
	if (!outCode)
		return YYTK_INVALID;

	memset(outCode, 0, sizeof(CCode));

	outCode->i_pVM = new VMBuffer;
	if (!outCode->i_pVM)
		return YYTK_NO_MEMORY;

	outCode->i_pVM->m_pBuffer = new char[CodeBufferSize + 1];
	if (!outCode->i_pVM->m_pBuffer)
		return YYTK_NO_MEMORY;

	outCode->i_compiled = 1;
	outCode->i_kind = 1;
	outCode->i_pName = Name;
	outCode->i_pVM->m_size = CodeBufferSize;
	outCode->i_pVM->m_numLocalVarsUsed = LocalVarsUsed;
	memcpy(outCode->i_pVM->m_pBuffer, CodeBuffer, CodeBufferSize);
	outCode->i_locals = LocalVarsUsed;
	outCode->i_flags = YYTK_MAGIC; //magic value

	return YYTK_OK;
}

DllExport YYTKStatus AUMI_CreateYYCCode(CCode* outCode, TGMLRoutine Function, const char* FunctionName, const char* CodeName)
{
	if (!outCode)
		return YYTK_INVALID;

	memset(outCode, 0, sizeof(CCode));

	outCode->i_compiled = 1;
	outCode->i_kind = 1;
	outCode->i_pName = CodeName;
	outCode->i_pFunc = new YYGMLFuncs;

	if (!outCode->i_pFunc)
		return YYTK_NO_MEMORY;

	outCode->i_pFunc->pFunc = Function;
	outCode->i_pFunc->pName = FunctionName;

	outCode->i_flags = 'YYTK'; //magic value

	return YYTK_OK;
}

DllExport YYTKStatus AUMI_FreeCode(CCode* Code)
{
	if (!Code)
		return YYTK_INVALID;

	if (Code->i_flags != YYTK_MAGIC) //Not YYTK Code
		return YYTK_INVALID;

	if (Code->i_pVM)
	{
		if (Code->i_pVM->m_pBuffer)
			delete[] Code->i_pVM->m_pBuffer;

		delete Code->i_pVM;
	}

	if (Code->i_pFunc)
		delete Code->i_pFunc;

	return YYTK_OK;
}

DllExport YYTKStatus AUMI_GetGlobalState(YYObjectBase** outState)
{
	if (!outState)
		return YYTK_INVALID;

	AUMIFunctionInfo FunctionEntry;
	RValue Result;

	if (AUMI_GetFunctionByName("@@GlobalScope@@", &FunctionEntry))
		return YYTK_NOT_FOUND;

	if (!FunctionEntry.Function)
		return YYTK_NOT_FOUND;

	FunctionEntry.Function(&Result, NULL, NULL, 0, NULL);

	*outState = (YYObjectBase*)Result.Pointer;

	return YYTK_OK;
}

DllExport YYTKStatus AUMI_ExecuteCode(YYObjectBase* Self, YYObjectBase* Other, CCode* Code, YYRValue* Arguments)
{
	if (!Code)
		return YYTK_INVALID;

	if (!g_pCodeExecute)
	{
		YYTKStatus Result;
		if (Result = AUMI_GetCodeExecuteAddress((PVOID*)&g_pCodeExecute))
			return Result;
	}

	int ret = 0;
	ret = g_pCodeExecute(Self, Other, Code, Arguments, 0);

	return (ret == 1) ? YYTK_OK : YYTK_FAIL;
}

DllExport YYTKStatus AUMI_GetCodeExecuteAddress(void** outAddress)
{
	if (!outAddress)
		return YYTK_INVALID;
	
	if (g_pCodeExecute)
	{
		*outAddress = g_pCodeExecute;
		return YYTK_OK;
	}

	MODULEINFO CurInfo = GetCurrentModuleInfo();
	char* Base = (PCHAR)FindPattern("\x8A\xD8\x83\xC4\x14\x80\xFB\x01\x74", "xxxxxxxxx", (long)CurInfo.lpBaseOfDll, CurInfo.SizeOfImage);

	if (!Base)
		return YYTK_NOT_FOUND;

	while (*(WORD*)Base != 0xCCCC)
		Base -= 1;

	Base += 2; // Compensate for the extra CC bytes

	*outAddress = Base;

	return YYTK_OK;
}

DllExport YYTKStatus AUMI_GetCodeFunctionAddress(void** outAddress)
{
	MODULEINFO CurInfo = GetCurrentModuleInfo();

	if (!(*outAddress = (void*)FindPattern("\x8B\x44\x24\x04\x3B\x05\x00\x00\x00\x00\x7F", "xxxxxx????x", (long)CurInfo.lpBaseOfDll, CurInfo.SizeOfImage)))
		return YYTK_NOT_FOUND;

	return YYTK_OK;
}

DllExport YYTKStatus AUMI_GetFunctionByIndex(int index, AUMIFunctionInfo* outInformation)
{
	if (!outInformation)
		return YYTK_INVALID;

	memset(outInformation, 0, sizeof(AUMIFunctionInfo));

	if (!g_pGrabCodeFunction)
	{
		YYTKStatus Result;
		if (Result = AUMI_GetCodeFunctionAddress((void**)&g_pGrabCodeFunction))
			return Result;
	}


	int Argc = 'YYTK'; //If this doesn't change, the function index is invalid.

	{
		void* pLastArg = NULL; // Made for compatibility with GMS 1

		g_pGrabCodeFunction(index, (char**)&outInformation->Name, (void**)&outInformation->Function, &Argc, &pLastArg);

		memcpy(outInformation->Name, *(void**)outInformation->Name, 64); // This apparently fixes strings, don't ask.
	}

	if (!*(outInformation->Name))
		return YYTK_NOT_FOUND;

	if (Argc == 'YYTK')
		return YYTK_NOT_FOUND;

	outInformation->Arguments = Argc;
	outInformation->Index = index;

	return YYTK_OK;
}

DllExport YYTKStatus AUMI_GetFunctionByName(const char* Name, AUMIFunctionInfo* outInformation)
{
	if (!outInformation)
		return YYTK_INVALID;

	int Index = 0;
	AUMIFunctionInfo Info;

	while (1)
	{
		YYTKStatus result;
		if (result = AUMI_GetFunctionByIndex(Index, &Info))
			return result;

		if (!_stricmp(Info.Name, Name))
		{
			outInformation->Index = Index;
			memcpy(outInformation, &Info, sizeof(AUMIFunctionInfo));

			return YYTK_OK;
		}

		Index++;
	}

	return YYTK_FAIL; // How did we get here?
}

DllExport YYTKStatus AUMI_GetFunctionByRoutine(TRoutine Routine, AUMIFunctionInfo* outInformation)
{
	AUMIFunctionInfo mInfo;
	int Index = 0;

	while (1)
	{
		YYTKStatus result;
		if (result = AUMI_GetFunctionByIndex(Index, &mInfo))
			return result;

		if ((char*)Routine == (char*)mInfo.Function)
		{
			outInformation->Index = Index;
			memcpy(outInformation, &mInfo, sizeof(AUMIFunctionInfo));

			return YYTK_OK;
		}

		Index++;
	}

	return YYTK_NOT_FOUND;
}

DllExport YYTKStatus AUMI_CallBuiltinFunction(const char* Name, RValue* Result, YYObjectBase* Self, YYObjectBase* Other, int argc, RValue* Args)
{
	AUMIFunctionInfo mInfo;
	YYTKStatus result;

	if (result = AUMI_GetFunctionByName(Name, &mInfo))
		return result;

	mInfo.Function(Result, Self, Other, argc, Args);

	return YYTK_OK;
}
