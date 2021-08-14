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

inline struct APIVars_t
{
<<<<<<< Updated upstream
	YYObjectBase* g_pGlobal = nullptr;
	FNCodeExecute Code_Execute = nullptr;
	FNCodeFunctionGetTheFunction Code_Function_GET_the_function = nullptr;
	void* Window_Handle = nullptr;
	void* Window_Device = nullptr;
	std::map<unsigned long, YYTKPlugin> Plugins;
	void* MainModule = nullptr;
=======
	YYObjectBase* g_pGlobal = nullptr;			// A pointer to the global instance of the runner (globalvar)
	FNCodeExecute Code_Execute = nullptr;		// A pointer to the Code_Execute function.
	FNCodeFunctionGetTheFunction Code_Function_GET_the_function = nullptr;		// A pointer to a function of the same name.
	void* Window_Handle = nullptr;				// A handle to the window (HWND).
	void* Window_Device = nullptr;				// A D3DDevice pointer (ID3D11Device*, LPDIRECT3D9DEVICE).
	std::map<unsigned long, YYTKPlugin> Plugins;	// A map of all plugins' base address and their corresponding plugin object.
	void* MainModule = nullptr;					// A pointer to the main module (why?)
>>>>>>> Stashed changes
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
	DllExport YYTKStatus CreateYYCCodeObject(CCode& out, PFUNC_YYGML Routine, const char* pName);

	// Always call this on a code object you generated.
	DllExport YYTKStatus FreeCodeObject(CCode& out);

	DllExport YYTKStatus GetFunctionByIndex(int index, FunctionInfo_t& outInfo);

	DllExport YYTKStatus GetFunctionByName(const char* Name, FunctionInfo_t& outInfo);

	DllExport YYTKStatus GetAPIVars(APIVars_t* outVars);

	DllExport YYTKStatus GetCodeExecuteAddr(FNCodeExecute& outAddress);

	DllExport YYTKStatus GetCodeFunctionAddr(FNCodeFunctionGetTheFunction& outAddress);

	DllExport unsigned long FindPattern(const char* Pattern, const char* Mask, long base, unsigned size);

	DllExport YYTKStatus GetGlobalInstance(YYObjectBase** ppoutGlobal);

	DllExport YYTKStatus CallBuiltinFunction(CInstance* _pSelf, CInstance* _pOther, YYRValue& _result, int _argc, const char* Name, YYRValue* Args);

	DllExport TRoutine GetBuiltin(const char* Name);

	/* Reconstructed YYGML Functions */
	// Note to self: YYRValue** are actually **, not references, they're passing arrays of pointers...
	DllExport RValue* YYGML_CallLegacyFunction(CInstance* _pSelf, CInstance* _pOther, RValue& _result, int _argc, int _id, RValue** _args);

	DllExport void YYGML_array_set_owner(long long _owner);

	DllExport YYRValue* YYGML_method(CInstance* _pSelf, YYRValue& _result, YYRValue& _pRef);

	DllExport void YYGML_window_set_caption(const char* _pStr);
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

