#include "AUMI/Exports.hpp"
#include "AUMI/IPC/IPC.hpp"
#include "Toolkit/Core.hpp"
#include <thread>
#include <string>
#include <Windows.h>

static HINSTANCE g_hDLL = 0;

void Main()
{
	std::thread IPCThread;

	// Phase 1 - Pre-toolkit Initialization
	{
		AUMI_Initialize();
		IPCThread = std::thread(AUMI_RunIPC);
	}

	// Phase 2 - Toolkit Initialization
	{
		Tool_Initialize();
	}

	while (!GetAsyncKeyState(VK_END)) { Sleep(1); }

	// Phase 3 - Toolkit Exit
	{
		AUMI_StopIPC();
		IPCThread.join();
		FreeLibraryAndExitThread(g_hDLL, 0);
	}
}

int __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		g_hDLL = hinstDLL;
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Main, 0, 0, 0);
	}
	return TRUE;
}