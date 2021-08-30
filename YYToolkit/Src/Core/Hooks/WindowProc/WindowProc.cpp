#include "WindowProc.hpp"
#include "../../Utils/Error.hpp"
#include "../../Features/API/API.hpp"

namespace Hooks::WindowProc
{
	LRESULT __stdcall Function(HWND hwnd, unsigned int Msg, WPARAM w, LPARAM l)
	{
		Plugins::RunWindowCallbacks(hwnd, Msg, w, l);
		return CallWindowProc(pfnOriginal, hwnd, Msg, w, l);
	}

	void _SetWindowsHook()
	{
		pfnOriginal = reinterpret_cast<WNDPROC>(SetWindowLong(reinterpret_cast<HWND>(gAPIVars.Window_Handle), GWL_WNDPROC, reinterpret_cast<LONG>(Function)));
	}
}