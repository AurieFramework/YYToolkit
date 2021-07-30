#include "MessageBoxW.hpp"
#include <string>

int __stdcall Hooks::MessageBoxW::Function(HWND Hwnd, LPCWSTR lpwText, LPCWSTR lpwCaption, UINT Type)
{
	std::wstring Text = lpwText;

	if (Text._Starts_with(L"Win32 function failed"))
		return 0;
		
	return pfnOriginal(Hwnd, lpwText, lpwCaption, Type);
}

void* Hooks::MessageBoxW::GetTargetAddress()
{
	HMODULE Module = GetModuleHandleA("user32.dll");

	if (!Module)
		return nullptr;
	
	return GetProcAddress(Module, "MessageBoxW");
}
