#include "Toolkit/Features/AUMI_API/Exports.hpp"
#include "Toolkit/Features/AUMI_API/IPC/IPC.hpp"
#include "Toolkit/Core.hpp"
#include "Toolkit/Utils/StackTrace.hpp"
#include <thread>
#include <Windows.h>

#if _WIN64
#error Don't compile in x64!
#endif

static HINSTANCE g_hDLL = 0;

void Main()
{
	YYTKTrace(__FUNCTION__ "()", __LINE__);

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