#include "MessageBoxW.hpp"
#include "../../Features/API/API.hpp"
#include "../../Features/PluginManager/PluginManager.hpp"
#include "../../SDK/Plugins/YYTKEvent/YYTKEvent.hpp"
#include <string>

namespace Hooks
{
	namespace MessageBoxW
	{
		int __stdcall Function(HWND Hwnd, LPCWSTR lpwText, LPCWSTR lpwCaption, UINT Type)
		{
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


