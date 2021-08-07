#pragma once
#include "../../Utils/SDK.hpp"
#include <map>
// One API to unite them all.
// If you want IPC for legacy AUMI, use the dedicated plugin.

struct ModuleInfo_t 
{
	unsigned long Base;
	unsigned long Size;
	unsigned long EntryPoint;
};

inline struct APIVars_t
{
	YYObjectBase* g_pGlobal = nullptr;
	TCodeExecuteRoutine Code_Execute = nullptr;
	TGetTheFunctionRoutine Code_Function_GET_the_function = nullptr;
	void* Window_Handle = nullptr;
	void* Window_Device = nullptr;
	FUNCTION_TABLE GlobalTable = { nullptr, nullptr };
	std::map<unsigned long, YYTKPlugin> Plugins;
	void* MainModule = nullptr;
} gAPIVars;


/* The actual API functions */
namespace API
{
	// This function gets called once at the start of YYToolkit, so no need to export it.
	YYTKStatus Initialize(void* pModule);

	YYTKStatus Uninitialize();

	// Create a VM code object which can be passed to Code_Execute.
	DllExport YYTKStatus CreateCodeObject(CCode& out, char* pBytecode, size_t BytecodeSize, unsigned int Locals, const char* pName);

	// Create a YYC code object, runs C++ code instead of VM bytecode.
	DllExport YYTKStatus CreateYYCCodeObject(CCode& out, TGMLRoutine Routine, const char* pName);

	// Always call this on a code object you generated.
	DllExport YYTKStatus FreeCodeObject(CCode& out);

	DllExport YYTKStatus GetFunctionByIndex(int index, FunctionInfo_t& outInfo);

	DllExport YYTKStatus GetFunctionByName(const char* Name, FunctionInfo_t& outInfo);

	DllExport YYTKStatus GetAPIVars(APIVars_t* outVars);

	DllExport YYTKStatus GetCodeExecuteAddr(TCodeExecuteRoutine& outAddress);

	DllExport YYTKStatus GetCodeFunctionAddr(TGetTheFunctionRoutine& outAddress);

	DllExport unsigned long FindPattern(const char* Pattern, const char* Mask, long base, unsigned size);

	DllExport YYTKStatus GetGlobalInstance(YYObjectBase** ppoutGlobal);

	DllExport YYTKStatus CallBuiltinFunction(CInstance* _pSelf, CInstance* _pOther, YYRValue& _result, int _argc, const char* Name, YYRValue* Args);

	DllExport TRoutine GetBuiltin(const char* Name);

	DllExport YYTKStatus CreateString(RValue& Ref, const char* String);

	/* Reconstructed YYGML Functions */
	// Note to self: YYRValue** are actually **, not references, they're passing arrays of pointers...
	DllExport YYRValue& YYGML_CallLegacyFunction(CInstance* _pSelf, CInstance* _pOther, YYRValue& _result, int _argc, int _id, YYRValue** _args);

	DllExport void YYGML_array_set_owner(int64 _owner);

	DllExport YYRValue& YYGML_method(CInstance* _pSelf, YYRValue& _result, YYRValue& _pRef);

	DllExport void YYGML_window_set_caption(const char* _pStr);

	DllExport double YYGML_StringByteAt(const char* string, int _index);
}

namespace Plugins
{
	DllExport YYTKPlugin* LoadPlugin(const char* Path);

	DllExport bool UnloadPlugin(YYTKPlugin* pPlugin, bool Notify);

	DllExport void RunCodeExecuteCallbacks(CInstance*& pSelf, CInstance*& pOther, CCode*& Code, RValue*& Res, int& Flags);

	DllExport void RunPresentCallbacks(void*& IDXGISwapChain, unsigned int& Sync, unsigned int& Flags);

	DllExport void RunEndSceneCallbacks(void*& LPDIRECT3DDEVICE);

	DllExport void RunDrawingCallbacks(float& x, float& y, const char*& str, int& linesep, int& linewidth);
}

