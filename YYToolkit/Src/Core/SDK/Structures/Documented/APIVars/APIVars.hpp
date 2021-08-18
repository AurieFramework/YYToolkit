#pragma once
#include "../../../FwdDecls/FwdDecls.hpp"
#include "../../../Plugins/Plugins.hpp"
#include <map>

struct APIVars_t
{
	YYObjectBase* g_pGlobal										= nullptr;		// A pointer to the global game instance
	FNCodeExecute Code_Execute									= nullptr;		// A pointer to the Code_Execute function
	FNCodeFunctionGetTheFunction Code_Function_GET_the_function = nullptr;		// A pointer to a function with a long-ass name
	void* Window_Handle											= nullptr;		// A pointer to the window handle (HWND)
	void* Window_Device											= nullptr;		// A pointer to either a D3D11Device*, or an DIRECT3DDEVICE9
	std::map<unsigned long, YYTKPlugin> Plugins;								// A map of all plugins loaded (Key = Base address, Value = YYTKPlugin object)
	void* MainModule											= nullptr;		// A pointer to the core module (can be casted to an HMODULE)
	void* RenderView											= nullptr;		// A pointer to an ID3D11RenderTargetView. Is nullptr is DX9!
};