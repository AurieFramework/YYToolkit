#include "MessageBoxW.hpp"
#include "../../Features/API/API.hpp"
#include <string>

int __stdcall Hooks::MessageBoxW::Function(HWND Hwnd, LPCWSTR lpwText, LPCWSTR lpwCaption, UINT Type)
{
	using YYTKMessageBoxEvent = YYTKEvent<int, int(__stdcall*)(HWND, LPCWSTR, LPCWSTR, UINT), EventType::EVT_MESSAGEBOX, HWND, LPCWSTR, LPCWSTR, UINT>;

	YYTKMessageBoxEvent Event = YYTKMessageBoxEvent(&Function, Hwnd, lpwText, lpwCaption, Type);
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
