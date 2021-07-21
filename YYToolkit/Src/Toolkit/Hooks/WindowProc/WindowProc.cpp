#include "../Hooks.hpp"
#include "../../Features/AUMI_API/Exports.hpp"
#include "../../Utils/Error.hpp"
#include "../../Utils/ImGui/imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Hooks
{
	LRESULT __stdcall WindowProc(HWND hwnd, unsigned int Msg, WPARAM w, LPARAM l)
	{
		LRESULT Result;
		if (Result = ImGui_ImplWin32_WndProcHandler(hwnd, Msg, w, l))
			return Result;

		if (Msg == WM_CLOSE)
			exit(0);

		return CallWindowProc(oWndProc, hwnd, Msg, w, l);
	}

	void WindowProc_Init()
	{
		RValue Result;
		if (auto Status = AUMI_CallBuiltinFunction("window_handle", &Result, 0, 0, 0, 0))
			Utils::Error::Error(1, "Failed to get the window handle.\nError Code: %s", Utils::Error::YYTKStatus_ToString(Status).data());

		oWndProc = (WNDPROC)(SetWindowLong((HWND)(Result.Pointer), GWL_WNDPROC, (LONG)WindowProc));
	}
}