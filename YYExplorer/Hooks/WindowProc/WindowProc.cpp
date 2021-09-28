#include "../Hooks.hpp"
#include "../../ImGui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void Hooks::WindowProc(YYTKWindowProcEvent* _evt, HWND _Window, UINT _Msg, WPARAM _wp, LPARAM _lp)
{
	if (ImGui_ImplWin32_WndProcHandler(_Window, _Msg, _wp, _lp))
	{
		return _evt->Cancel(true); // 'return true;' but for events
	}

	if (_Msg == WM_DESTROY) 
	{
		return PostQuitMessage(0);
	}
}