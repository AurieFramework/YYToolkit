
#include "API.hpp"
#include <Windows.h>
#undef min
#undef max
#include <utility>
#include <filesystem>
#include "../../Utils/Error.hpp"

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
	YYTKStatus Initialize(void* pModule)
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
			gAPIVars.Window_Handle = Result.Pointer;
		}

		if (!gAPIVars.Window_Device)
		{
			FunctionInfo_t Info; RValue Result;
			ErrorOccured |= (GetFunctionByName("window_device", Info) != YYTK_OK);

			Info.Function(&Result, 0, 0, 0, 0);
			gAPIVars.Window_Device = Result.Pointer;
		}

		gAPIVars.MainModule = pModule;

		AllocConsole();
		FILE* fDummy;
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONOUT$", "w", stdout);

		SetConsoleTitleA("YYToolkit Log");
		printf("YYToolkit version %s - Error Flag: %i\n", YYSDK_VERSION, static_cast<int>(ErrorOccured));

		// Run autoexec
		namespace fs = std::filesystem;

		std::wstring Path = fs::current_path().wstring().append(L"\\autoexec");

		printf("Running from %s\n", fs::current_path().string().c_str());

		if (fs::is_directory(Path))
		{
			printf("'autoexec' folder exists, starting plugins...\n");
			for (auto& entry : fs::directory_iterator(Path))
			{
				if (entry.path().extension().string().find(".dll") != std::string::npos)
				{
					// We have a DLL, try loading it
					if (!Plugins::LoadPlugin(entry.path().string().c_str()))
						Utils::Error::Message("Plugin %s failed to load!", entry.path().filename().string().c_str());
					else
						Utils::Error::Message("Plugin %s loaded!", entry.path().filename().string().c_str());
				}
			}
		}

		return ErrorOccured ? YYTK_FAIL : YYTK_OK;
	}

	YYTKStatus Uninitialize()
	{
		for (auto& Plugin : gAPIVars.Plugins)
		{
			Plugins::UnloadPlugin(&Plugin.second, true);
		}

		gAPIVars.Plugins.clear();
		
		ShowWindow(GetConsoleWindow(), SW_HIDE);
		FreeConsole();

		return YYTK_OK;
	}

	DllExport YYTKStatus GetAPIVersion(char* outBuffer)
	{
		strncpy(outBuffer, YYSDK_VERSION, strlen(YYSDK_VERSION));
		return YYTK_OK;
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

		out.i_compiled = true;
		out.i_kind = 1;
		out.i_pName = pName;
		out.i_pVM->m_size = BytecodeSize;
		out.i_pVM->m_numLocalVarsUsed = Locals;
		memcpy(out.i_pVM->m_pBuffer, pBytecode, BytecodeSize);
		out.i_locals = Locals;
		out.i_flags = YYTK_MAGIC; //magic value

		return YYTK_OK;
	}

	DllExport YYTKStatus CreateYYCCodeObject(CCode& out, PFUNC_YYGML Routine, const char* pName)
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

		gAPIVars.Code_Function_GET_the_function(index, &outInfo.Name, (PVOID*)&outInfo.Function, &outInfo.Argc, &pUnknown);

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

	DllExport YYTKStatus GetCodeExecuteAddr(FNCodeExecute& outAddress)
	{
		ModuleInfo_t CurInfo = GetModuleInfo();
		unsigned long Base = FindPattern("\x8A\xD8\x83\xC4\x14\x80\xFB\x01\x74", "xxxxxxxxx", CurInfo.Base, CurInfo.Size);

		if (!Base)
			return YYTK_NOT_FOUND;

		while (*(WORD*)Base != 0xCCCC)
			Base -= 1;

		Base += 2; // Compensate for the extra CC bytes

		outAddress = (FNCodeExecute)Base;

		return YYTK_OK;
	}

	DllExport YYTKStatus GetCodeFunctionAddr(FNCodeFunctionGetTheFunction& outAddress)
	{
		ModuleInfo_t CurInfo = GetModuleInfo();

		if ((outAddress = (FNCodeFunctionGetTheFunction)FindPattern("\x8B\x44\x24\x04\x3B\x05\x00\x00\x00\x00\x7F", "xxxxxx????x", CurInfo.Base, CurInfo.Size)))
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

		*ppoutGlobal = Result.Object;

		return YYTK_OK;
	}

	DllExport YYTKStatus CallBuiltinFunction(CInstance* _pSelf, CInstance* _pOther, YYRValue& _result, int _argc, const char* Name, YYRValue* Args)
	{
		FunctionInfo_t Info;
		if (GetFunctionByName(Name, Info) == YYTK_OK)
		{
			YYGML_CallLegacyFunction(_pSelf, _pOther, _result.As<RValue>(), _argc, Info.Index, reinterpret_cast<RValue**>(&Args));
			return YYTK_OK;
		}

		return YYTK_NOT_FOUND;
	}

	DllExport TRoutine GetBuiltin(const char* Name)
	{
		FunctionInfo_t Info;
		if (GetFunctionByName(Name, Info) == YYTK_OK)
		{
			return Info.Function;
		}
		return nullptr;
	}

	DllExport RValue* YYGML_CallLegacyFunction(CInstance* _pSelf, CInstance* _pOther, RValue& _result, int _argc, int _id, RValue** _args)
	{
		FunctionInfo_t Info;
		if (GetFunctionByIndex(_id, Info) == YYTK_OK)
		{
			Info.Function(&_result, _pSelf, _pOther, _argc, *_args);
		}

		return &_result;
	}

	DllExport void YYGML_array_set_owner(long long _owner)
	{
		YYRValue Result;
		YYRValue Owner = _owner;
		CallBuiltinFunction(0, 0, Result, 1, "@@array_set_owner@@", &Owner);
	}

	DllExport YYRValue* YYGML_method(CInstance* _pSelf, YYRValue& _result, YYRValue& _pRef)
	{
		return &_result;
	}

	DllExport void YYGML_window_set_caption(const char* _pStr)
	{
		SetWindowTextA((HWND)gAPIVars.Window_Handle, _pStr);
	}
}

namespace Plugins
{
	DllExport void* GetPluginRoutine(const char* Name)
	{
		for (auto& Element : gAPIVars.Plugins)
		{
			if (void* Result = Element.second.GetExport<void*>(Name))
				return Result;
		}

		return nullptr;
	}

	DllExport YYTKPlugin* LoadPlugin(const char* Path)
	{
		char Buffer[MAX_PATH] = { 0 };
		FNPluginEntry lpPluginEntry = nullptr;

		GetFullPathNameA(Path, MAX_PATH, Buffer, 0);
		HMODULE PluginModule = LoadLibraryA(Buffer);

		if (!PluginModule)
			return nullptr;

		lpPluginEntry = (FNPluginEntry)GetProcAddress(PluginModule, "PluginEntry");

		if (!lpPluginEntry)
		{
			FreeLibraryAndExitThread(PluginModule, 1);
			return nullptr;
		}

		// Emplace in map scope
		{
			auto PluginObject = YYTKPlugin();
			memset(&PluginObject, 0, sizeof(YYTKPlugin));

			PluginObject.PluginEntry = lpPluginEntry;
			PluginObject.PluginStart = PluginModule;
			PluginObject.CoreStart = gAPIVars.MainModule;

			gAPIVars.Plugins.emplace(std::make_pair(reinterpret_cast<unsigned long>(PluginObject.PluginStart), PluginObject));
		}
		
		YYTKPlugin* refVariableInMap = &gAPIVars.Plugins.at(reinterpret_cast<unsigned long>(PluginModule));

		lpPluginEntry(refVariableInMap);

		return refVariableInMap;
	}

	bool UnloadPlugin(YYTKPlugin* pPlugin, bool Notify)
	{
		if (!pPlugin)
			return false;

		if (pPlugin->PluginUnload && Notify)
			pPlugin->PluginUnload(pPlugin);

		FreeLibrary(reinterpret_cast<HMODULE>(pPlugin->PluginStart));

		return true;
	}

	DllExport void RunCallback(YYTKEventBase* pEvent)
	{
		for (auto& Pair : gAPIVars.Plugins)
		{
			if (Pair.second.PluginHandler)
				Pair.second.PluginHandler(&Pair.second, pEvent);
		}
	}
}

