#include "WindowProc.hpp"
#include "../../Utils/Logging/Logging.hpp"
#include "../../Features/API/API.hpp"
#include "../../Features/PluginManager/PluginManager.hpp"
#include "../../SDK/Plugins/YYTKEvent/YYTKEvent.hpp"

namespace Hooks
{
	namespace WindowProc
	{
		LRESULT __stdcall Function(HWND hwnd, unsigned int Msg, WPARAM w, LPARAM l)
		{
			if (Msg == WM_CLOSE)
				exit(0);

			YYTKWindowProcEvent Event = YYTKWindowProcEvent(pfnOriginal, hwnd, Msg, w, l);
			API::PluginManager::RunHooks(&Event);

			if (Event.CalledOriginal())
				return Event.GetReturn();

			return CallWindowProc(pfnOriginal, hwnd, Msg, w, l);
		}

		void _SetWindowsHook()
		{
			pfnOriginal = reinterpret_cast<WNDPROC>(SetWindowLongPtr(reinterpret_cast<HWND>(API::gAPIVars.Globals.g_hwWindowHandle), GWLP_WNDPROC, reinterpret_cast<uintptr_t>(Function)));
		}
	}
}