#define YYSDK_YYC
#include "API.hpp"
#include <Windows.h>
#undef min
#undef max
#include <utility>

static ModuleInfo_t GetModuleInfo()
{
	using Fn = int(__stdcall*)(HANDLE, HMODULE, ModuleInfo_t*, DWORD);

	static HMODULE Module = GetModuleHandleA("kernel32.dll");
	static Fn K32GetModuleInformation = (Fn)GetProcAddress(Module, "K32GetModuleInformation");

	ModuleInfo_t modinfo = { 0 };
	HMODULE hModule = GetModuleHandleA(NULL);
	if (hModule == 0)
		return modinfo;
	K32GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(ModuleInfo_t));
	return modinfo;
}

namespace API
{
	YYTKStatus Initialize()
	{
		bool ErrorOccured = false;
		if (!gAPIVars.Code_Execute)
		{
			ErrorOccured |= (GetCodeExecuteAddr(gAPIVars.Code_Execute) != YYTK_OK);
		}

		if (!gAPIVars.Code_Function_GET_the_function)
		{
			ErrorOccured |= (GetCodeFunctionAddr(gAPIVars.Code_Function_GET_the_function) != YYTK_OK);
		}

		if (!gAPIVars.g_pGlobal)
		{
			ErrorOccured |= (GetGlobalInstance(&gAPIVars.g_pGlobal) != YYTK_OK);
		}

		if (!gAPIVars.Window_Handle)
		{
			FunctionInfo_t Info; RValue Result;
			ErrorOccured |= (GetFunctionByName("window_handle", Info) != YYTK_OK);

			Info.Function(&Result, 0, 0, 0, 0);
			gAPIVars.Window_Handle = Result;
		}

		if (!gAPIVars.Window_Device)
		{
			FunctionInfo_t Info; RValue Result;
			ErrorOccured |= (GetFunctionByName("window_device", Info) != YYTK_OK);

			Info.Function(&Result, 0, 0, 0, 0);
			gAPIVars.Window_Device = Result;
		}

		return ErrorOccured ? YYTK_FAIL : YYTK_OK;
	}

	DllExport YYTKStatus CreateCodeObject(CCode& out, char* pBytecode, size_t BytecodeSize, unsigned int Locals, const char* pName)
	{
		if (!pBytecode)
			return YYTK_INVALID;

		memset(&out, 0, sizeof(CCode));

		out.i_pVM = new VMBuffer;
		if (!out.i_pVM)
			return YYTK_NO_MEMORY;

		out.i_pVM->m_pBuffer = new char[BytecodeSize + 1];
		if (!out.i_pVM->m_pBuffer)
			return YYTK_NO_MEMORY;

		out.i_compiled = 1;
		out.i_kind = 1;
		out.i_pName = pName;
		out.i_pVM->m_size = BytecodeSize;
		out.i_pVM->m_numLocalVarsUsed = Locals;
		memcpy(out.i_pVM->m_pBuffer, pBytecode, BytecodeSize);
		out.i_locals = Locals;
		out.i_flags = YYTK_MAGIC; //magic value

		return YYTK_OK;
	}

	DllExport YYTKStatus CreateYYCCodeObject(CCode& out, TGMLRoutine Routine, const char* pName)
	{
		if (!Routine)
			return YYTK_INVALID;

		memset(&out, 0, sizeof(CCode));

		out.i_compiled = 1;
		out.i_kind = 1;
		out.i_pName = pName;
		out.i_pFunc = new YYGMLFuncs;

		if (!out.i_pFunc)
			return YYTK_NO_MEMORY;

		out.i_pFunc->pFunc = Routine;
		out.i_pFunc->pName = pName;

		out.i_flags = YYTK_MAGIC;

		return YYTK_OK;
	}

	DllExport YYTKStatus FreeCodeObject(CCode& out)
	{
		if (out.i_flags != YYTK_MAGIC)
			return YYTK_INVALID;

		if (out.i_pVM)
		{
			if (out.i_pVM->m_pBuffer)
				delete[] out.i_pVM->m_pBuffer;

			delete out.i_pVM;
		}

		if (out.i_pFunc)
			delete out.i_pFunc;

		return YYTK_OK;
	}

	DllExport YYTKStatus GetFunctionByIndex(int index, FunctionInfo_t& outInfo)
	{
		if (index < 0)
			return YYTK_INVALID;

		void* pUnknown = nullptr;

		gAPIVars.Code_Function_GET_the_function(index, &outInfo.Name, (PVOID*)&outInfo.Function, &outInfo.Arguments, &pUnknown);

		if (!outInfo.Name)
			return YYTK_NOT_FOUND;

		outInfo.Index = index;

		return YYTK_OK;
	}

	DllExport YYTKStatus GetFunctionByName(const char* Name, FunctionInfo_t& outInfo)
	{
		int Index = 0;
		while (GetFunctionByIndex(Index, outInfo) == YYTK_OK)
		{
			if (strncmp(Name, outInfo.Name, 64) == 0)
			{
				return YYTK_OK;
			}
			Index++;
		}

		return YYTK_NOT_FOUND;
	}

	DllExport YYTKStatus GetAPIVars(APIVars_t* outVars)
	{
		*outVars = gAPIVars;
		return YYTK_OK;
	}

	DllExport YYTKStatus GetCodeExecuteAddr(TCodeExecuteRoutine& outAddress)
	{
		ModuleInfo_t CurInfo = GetModuleInfo();
		unsigned long Base = FindPattern("\x8A\xD8\x83\xC4\x14\x80\xFB\x01\x74", "xxxxxxxxx", CurInfo.Base, CurInfo.Size);

		if (!Base)
			return YYTK_NOT_FOUND;

		while (*(WORD*)Base != 0xCCCC)
			Base -= 1;

		Base += 2; // Compensate for the extra CC bytes

		outAddress = (TCodeExecuteRoutine)Base;

		return YYTK_OK;
	}

	DllExport YYTKStatus GetCodeFunctionAddr(TGetTheFunctionRoutine& outAddress)
	{
		ModuleInfo_t CurInfo = GetModuleInfo();

		if (outAddress = (TGetTheFunctionRoutine)FindPattern("\x8B\x44\x24\x04\x3B\x05\x00\x00\x00\x00\x7F", "xxxxxx????x", CurInfo.Base, CurInfo.Size))
			return YYTK_OK;

		return YYTK_NOT_FOUND;
	}

	DllExport unsigned long FindPattern(const char* Pattern, const char* Mask, long base, unsigned size)
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

	DllExport YYTKStatus GetGlobalInstance(YYObjectBase** ppoutGlobal)
	{
		if (!ppoutGlobal)
			return YYTK_INVALID;

		FunctionInfo_t FunctionEntry;
		RValue Result;

		if (YYTKStatus _ret = GetFunctionByName("@@GlobalScope@@", FunctionEntry))
			return _ret;

		if (!FunctionEntry.Function)
			return YYTK_NOT_FOUND;

		FunctionEntry.Function(&Result, NULL, NULL, 0, NULL);

		*ppoutGlobal = Result;

		return YYTK_OK;
	}

	DllExport YYTKStatus CallBuiltinFunction(CInstance* _pSelf, CInstance* _pOther, YYRValue& _result, int _argc, const char* Name, YYRValue* Args)
	{
		FunctionInfo_t Info;
		if (GetFunctionByName(Name, Info) == YYTK_OK)
		{
			YYGML_CallLegacyFunction(_pSelf, _pOther, _result, _argc, Info.Index, &Args);
			return YYTK_OK;
		}

		return YYTK_NOT_FOUND;
	}

	DllExport YYRValue& YYGML_CallLegacyFunction(CInstance* _pSelf, CInstance* _pOther, YYRValue& _result, int _argc, int _id, YYRValue** _args)
	{
		FunctionInfo_t Info;
		if (GetFunctionByIndex(_id, Info) == YYTK_OK)
		{
			Info.Function(&_result, _pSelf, _pOther, _argc, *_args);
		}

		return _result;
	}

	DllExport void YYGML_array_set_owner(int64 _owner)
	{
		RValue Result;
		RValue Owner; Owner.Kind = VALUE_REAL; Owner.Val = _owner;
		CallBuiltinFunction(0, 0, Result, 1, "@@array_set_owner@@", &Owner);
	}

	DllExport YYRValue& YYGML_method(CInstance* _pSelf, YYRValue& _result, YYRValue& _pRef)
	{
		return _result;
	}

	DllExport void YYGML_window_set_caption(const char* _pStr)
	{
		SetWindowTextA((HWND)gAPIVars.Window_Handle, _pStr);
	}

	DllExport double YYGML_StringByteAt(const char* string, int _index)
	{
		int Length = strlen(string) + 1;

		return static_cast<unsigned char>(string[std::min(Length, _index)]); // Implicit cast to double
	}
}