#include <Windows.h>
#include "Core/Features/API/API.hpp"
#include "Core/Hooks/Hooks.hpp"

#if _WIN64
#error Don't compile in x64!
#endif

static HINSTANCE g_hDLL = 0;

void Main()
{
	API::Initialize();
	Hooks::Initialize();

	while (!GetAsyncKeyState(VK_END)) { Sleep(1); }

	Hooks::Uninitialize();

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