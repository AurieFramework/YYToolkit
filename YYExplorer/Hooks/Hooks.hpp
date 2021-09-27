#pragma once
#ifndef YYSDK_PLUGIN
#define YYSDK_PLUGIN
#endif // YYSDK_PLUGIN
#include "../SDK/SDK.hpp"
#include <d3d11.h>
#include <d3d9.h>

namespace Hooks
{
	void Code_Execute(YYTKCodeEvent* _evt, CInstance* _pSelf, CInstance* _pOther, CCode* _pCode, RValue* _Args);

	void DoCallScript(YYTKScriptEvent* _evt, CScript* _pScript, int _argc, char* _pSP, VMExec* _pVM);

	void Present(YYTKPresentEvent* _evt, IDXGISwapChain* _this, UINT _Sync, UINT _Flags);

	void EndScene(YYTKEndSceneEvent* _evt, LPDIRECT3DDEVICE9 _this);

	void WindowProc(YYTKWindowProcEvent* _evt, HWND _Window, UINT _Msg, WPARAM _wp, LPARAM _lp);
}