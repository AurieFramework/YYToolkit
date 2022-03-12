#include <Windows.h>
#include "Core/Features/API/API.hpp"
#include "Core/Features/Console/Console.hpp"
#include "Core/Hooks/Hooks.hpp"

#if _WIN64
#error Don't compile in x64!
#endif

static HINSTANCE g_hDLL = 0;

void __stdcall Main()
{
	API::Internal::Initialize(g_hDLL);
	Hooks::Initialize();

	while (!GetAsyncKeyState(VK_END)) 
	{
		// Run console

		if (GetAsyncKeyState(VK_F10) & 1)
			Console::DoCommand();

		Sleep(5); 
	}

	Hooks::Uninitialize();
	API::Internal::Unload();

	MessageBoxA(
		API::gAPIVars.Globals.g_hwWindowHandle, 
		"Unloaded successfully.", 
		"YYToolkit", 
		MB_OK | MB_ICONINFORMATION | MB_TOPMOST | MB_SETFOREGROUND);

	FreeLibraryAndExitThread(g_hDLL, 0);
}

int __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		g_hDLL = hinstDLL;
		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Main, 0, 0, 0));
	}
	return TRUE;
}