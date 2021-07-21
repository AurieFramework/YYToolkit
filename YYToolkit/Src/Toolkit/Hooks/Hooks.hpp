#pragma once
#define YYSDK_NODEFS
#include "../Utils/SDK.hpp"
#include <d3d11.h>
#include <d3d9.h>

namespace Hooks
{
	HRESULT __stdcall EndScene(LPDIRECT3DDEVICE9 _this);
	HRESULT __stdcall Present(IDXGISwapChain* _this, unsigned int Sync, unsigned int Flags);
	LRESULT __stdcall WindowProc(HWND hwnd, unsigned int Msg, WPARAM w, LPARAM l);

	char* DoCallScript(CScript* pScript, int argc, char* Esp, VMExec* VM, YYObjectBase* pLocals, YYObjectBase* pArgs);
	bool Code_Execute(CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags);
	void YYError(const char* pFormat, ...);

	void* YYError_Address();
	void* Present_Address();
	void* EndScene_Address();
	void WindowProc_Init();

	inline decltype(&Present) oPresent = nullptr;
	inline decltype(&EndScene) oEndScene = nullptr;
	inline WNDPROC oWndProc = nullptr;
	inline decltype(&DoCallScript) oDoCallScript = nullptr;
	inline decltype(&Code_Execute) oCode_Execute = nullptr;
	inline decltype(&YYError) oYYError = nullptr;

	// I'm NOT making a cpp file for just this method...
	// Nevermind I did it
	void Initialize();
}