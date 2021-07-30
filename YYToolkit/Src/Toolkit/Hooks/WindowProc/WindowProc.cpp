#include "WindowProc.hpp"
#include "../../Features/AUMI_API/Exports.hpp"
#include "../../Utils/Error.hpp"
#include "../../Utils/ImGui/imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Hooks::WindowProc
{
	LRESULT __stdcall Function(HWND hwnd, unsigned int Msg, WPARAM w, LPARAM l)
	{
		LRESULT Result;
		if (Result = ImGui_ImplWin32_WndProcHandler(hwnd, Msg, w, l))
			return Result;

		if (Msg == WM_CLOSE)
			exit(0); // Maybe do some "Are you sure you wanna exit" stuff

		return CallWindowProc(pfnOriginal, hwnd, Msg, w, l);
	}

	void _SetWindowsHook()
	{
		RValue Result;
		if (auto Status = AUMI_CallBuiltinFunction("window_handle", &Result, 0, 0, 0, 0))
			Utils::Error::Error(1, "Failed to get the window handle.\nError Code: %s", Utils::Error::YYTKStatus_ToString(Status).data());

		pfnOriginal = (WNDPROC)(SetWindowLong((HWND)(Result.Pointer), GWL_WNDPROC, (LONG)Function));
	}
}