#pragma once
#define YYSDK_NODEFS
#include "../Utils/SDK.hpp"
#include <d3d11.h>
#include <d3d9.h>

namespace Hooks
{
	inline void* oPresent = nullptr;
	inline void* oEndScene = nullptr;
	inline void* oWndProc = nullptr;
	inline void* oDoCallScript = nullptr;
	inline void* oCode_Execute = nullptr;
	inline void* oYYError = nullptr;

	HRESULT __stdcall EndScene(LPDIRECT3DDEVICE9 _this);
	HRESULT __stdcall Present(IDXGISwapChain* _this, unsigned int Sync, unsigned int Flags);
	
	char* DoCallScript(CScript* pScript, int argc, char* Esp, VMExec* VM, YYObjectBase* pLocals, YYObjectBase* pArgs);
	bool Code_Execute(CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags);
	void YYError(const char* pFormat, ...);

	void* YYError_Address();
	void* Present_Address();
	void* EndScene_Address();

	// I'm NOT making a cpp file for just this method...
	// Nevermind I did it
	void Initialize();
}