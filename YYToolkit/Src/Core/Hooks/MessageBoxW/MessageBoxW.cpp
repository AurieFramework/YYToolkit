#include "MessageBoxW.hpp"
#include "../../Features/API/API.hpp"
#include <string>
#include "../../Features/PluginManager/PluginManager.hpp"

namespace Hooks
{
	namespace MessageBoxW
	{
		int __stdcall Function(HWND Hwnd, LPCWSTR lpwText, LPCWSTR lpwCaption, UINT Type)
		{
			YYTKMessageBoxEvent Event = YYTKMessageBoxEvent(pfnOriginal, Hwnd, lpwText, lpwCaption, Type);
			API::PluginManager::RunHooks(&Event);

			if (Event.CalledOriginal())
				return Event.GetReturn();

			if (std::wstring(lpwText)._Starts_with(L"Win32 function failed"))
				return 0;

			return pfnOriginal(Hwnd, lpwText, lpwCaption, Type);
		}

		void* GetTargetAddress()
		{
			HMODULE Module = GetModuleHandleA("user32.dll");

			if (!Module)
				return nullptr;

			return reinterpret_cast<void*>(GetProcAddress(Module, "MessageBoxW"));
		}
	}
}


