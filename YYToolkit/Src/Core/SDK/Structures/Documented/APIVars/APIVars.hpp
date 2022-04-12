#pragma once
#include "../../../FwdDecls/FwdDecls.hpp"
#include "../../../Plugins/Plugins.hpp"
#include <d3d11.h>

#pragma pack(push, 1)

struct CGlobals
{
	union
	{
		YYObjectBase* g_pGlobalObject;
		CInstance* g_pGlobalInstance = nullptr;
	};
	
	CDynamicArray<CScript*>*						g_pScriptsArray		= nullptr;
	void*											g_pWindowDevice		= nullptr;
	HWND											g_hwWindowHandle	= nullptr;
	HMODULE											g_hMainModule		= nullptr;
	ID3D11RenderTargetView*							g_pRenderView		= nullptr;
	ID3D11DeviceContext*							g_pDeviceContext	= nullptr;
	IDXGISwapChain*									g_pSwapChain		= nullptr;
	bool											g_bWasPreloaded		= false;
};

struct CFunctions
{
	FNCodeExecute					Code_Execute						= nullptr;
	FNCodeFunctionGetTheFunction	Code_Function_GET_the_function		= nullptr;
};

struct CAPIVars
{
	CGlobals Globals;
	CFunctions Functions;
};

#pragma pack(pop)