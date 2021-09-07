#include "MessageBoxW.hpp"
#include "../../Features/API/API.hpp"
#include <string>

int __stdcall Hooks::MessageBoxW::Function(HWND Hwnd, LPCWSTR lpwText, LPCWSTR lpwCaption, UINT Type)
{
	YYTKMessageBoxEvent Event = YYTKMessageBoxEvent(pfnOriginal, Hwnd, lpwText, lpwCaption, Type);
	Plugins::RunCallback(&Event);

	if (Event.CalledOriginal())
		return Event.GetReturn();

	if (std::wstring(lpwText)._Starts_with(L"Win32 function failed"))
		return 0;
		
	return pfnOriginal(Hwnd, lpwText, lpwCaption, Type);
}

void* Hooks::MessageBoxW::GetTargetAddress()
{
	HMODULE Module = GetModuleHandleA("user32.dll");

	if (!Module)
		return nullptr;
	
	return reinterpret_cast<void*>(GetProcAddress(Module, "MessageBoxW"));
}
