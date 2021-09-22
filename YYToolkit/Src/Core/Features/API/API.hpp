#pragma once
#include "../../SDK/SDK.hpp"
#include <map>
// One API to unite them all.
// If you want IPC for legacy AUMI, use the dedicated plugin.

struct ModuleInfo_t 
{
	unsigned long Base;
	unsigned long Size;
	unsigned long EntryPoint;
};

inline APIVars_t gAPIVars;

/* The actual API functions */
namespace API
{
	ModuleInfo_t GetModuleInfo();

	// This function gets called once at the start of YYToolkit, so no need to export it.
	YYTKStatus Initialize(void* pModule);

	YYTKStatus Uninitialize();

	DllExport YYTKStatus GetAPIVersion(char* outBuffer);

	DllExport YYTKStatus CreateCodeObject(CCode& out, char* pBytecode, size_t BytecodeSize, unsigned int Locals, const char* pName);

	DllExport YYTKStatus CreateYYCCodeObject(CCode& out, PFUNC_YYGML Routine, const char* pName);

	DllExport YYTKStatus FreeCodeObject(CCode& out);

	DllExport YYTKStatus GetFunctionByIndex(int index, FunctionInfo_t& outInfo);

	DllExport YYTKStatus GetFunctionByName(const char* Name, FunctionInfo_t& outInfo);

	DllExport YYTKStatus GetAPIVars(APIVars_t** ppoutVars);

	DllExport YYTKStatus GetScriptArray(CDynamicArray<CScript*>*& pOutArray);

	DllExport YYTKStatus GetScriptByName(const char* Name, CScript*& outScript);

	DllExport YYTKStatus ScriptExists(const char* Name);

	DllExport YYTKStatus GetCodeExecuteAddr(FNCodeExecute& outAddress);

	DllExport YYTKStatus GetCodeFunctionAddr(FNCodeFunctionGetTheFunction& outAddress);

	DllExport unsigned long FindPattern(const char* Pattern, const char* Mask, long base, unsigned size);

	DllExport YYTKStatus GetGlobalInstance(YYObjectBase** ppoutGlobal);

	DllExport YYTKStatus CallBuiltinFunction(CInstance* _pSelf, CInstance* _pOther, YYRValue& _result, int _argc, const char* Name, YYRValue* Args);

	DllExport TRoutine GetBuiltin(const char* Name);

	DllExport bool IsYYC();

	/* Reconstructed YYGML Functions */
	// Note to self: YYRValue** are actually **, not references, they're passing arrays of pointers...
	DllExport RValue* YYGML_CallLegacyFunction(CInstance* _pSelf, CInstance* _pOther, RValue& _result, int _argc, int _id, RValue** _args);

	DllExport void YYGML_array_set_owner(long long _owner);

	DllExport YYRValue* YYGML_method(CInstance* _pSelf, YYRValue& _result, YYRValue& _pRef);

	DllExport void YYGML_window_set_caption(const char* _pStr);
}

namespace Plugins
{
	DllExport void* GetPluginRoutine(const char* Name);

	DllExport YYTKPlugin* LoadPlugin(const char* Path);

	DllExport bool UnloadPlugin(YYTKPlugin* pPlugin, bool Notify);

	DllExport void RunCallback(YYTKEventBase* pEvent);
}

