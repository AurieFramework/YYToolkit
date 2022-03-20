#include "Core/Features/API/Internal.hpp"
#include "Core/Features/Console/Console.hpp"
#include "Core/Hooks/Hooks.hpp"
#include "Core/Utils/WinAPI/WinAPI.hpp"
#include "Core/Utils/Logging/Logging.hpp"

#if _WIN64
#error Don't compile in x64!
#endif

static HINSTANCE g_hDLL = 0;


// I might have just grown a brain tumor writing this
static bool IsPreloaded()
{
	Utils::WinAPI::SYSTEM_PROCESS_INFORMATION* pSpi = nullptr;
	void* pBackupToFree = nullptr; // Since we iterate using pSpi, we have to backup the original malloc'ed address to free later.

	if (!Utils::WinAPI::GetSysProcInfo(&pSpi))
		return false;
	
	pBackupToFree = pSpi;

	CModule GameModule;
	API::Internal::MmGetModuleInformation(nullptr, GameModule);

	while (true)
	{
		if ((DWORD)pSpi->ProcessId == GetCurrentProcessId())
		{
			for (int n = 0; n < pSpi->NumberOfThreads; n++)
			{
				// Check if we're withing bounds
				unsigned long dwStartAddr = 0;

				// fuck you Microsoft
				HANDLE ThreadHandle = OpenThread(THREAD_ALL_ACCESS, FALSE, (DWORD)pSpi->Threads[n].ClientId.UniqueThread);

				bool Success = Utils::WinAPI::GetThreadStartAddr(ThreadHandle, dwStartAddr);

				CloseHandle(ThreadHandle);

				if (!Success)
					continue;

				if (dwStartAddr == GameModule.EntryPoint)
				{
					// Check if the thread is sleeping
					if (pSpi->Threads[n].State != Utils::WinAPI::KTHREAD_STATE::Waiting)
						continue;

					if (pSpi->Threads[n].WaitReason != Utils::WinAPI::KWAIT_REASON::Suspended)
						continue;

					// We found one!
					free(pBackupToFree);
					return true;
				}
			}
		}

		if (pSpi->NextEntryOffset == 0)
			break;

		pSpi = reinterpret_cast<Utils::WinAPI::SYSTEM_PROCESS_INFORMATION*>(reinterpret_cast<PBYTE>(pSpi) + pSpi->NextEntryOffset);
	}

	free(pBackupToFree);
	return false;
}

void __stdcall Main()
{
	API::Internal::__InitializeConsole__();

	if (IsPreloaded())
	{
		Utils::Logging::Message(Color::CLR_YELLOW, "Warning: Early Launch was used!");
		API::gAPIVars.Globals.g_bWasPreloaded = true;
	}

	// Let the runner initialize - this is NOT an adequate check!
	// The ideal check would be to see if a D3D device was initialized yet, or if the window exists.
	Sleep(1000); 

	API::Internal::__Initialize__(g_hDLL);
	Hooks::Initialize();

	while (!GetAsyncKeyState(VK_END)) 
	{
		if (GetAsyncKeyState(VK_F10) & 1)
			Console::DoCommand();

		Sleep(5); 
	}

	Hooks::Uninitialize();
	API::Internal::__Unload__();

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