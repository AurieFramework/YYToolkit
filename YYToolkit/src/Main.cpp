#include <Windows.h>
#include "Features/RTK API/API.hpp"

DWORD WINAPI Main(LPVOID lpInstance)
{
	// Initialize RTK API - this must be done first, to enable hooking etc
	rtk::CrInitializeAPI();

	rtk::PmRunInitializeRoutines();
	rtk::PmRunEntryRoutines();

	// Sleep indefinitely until END is pressed
	while (!(GetAsyncKeyState(VK_END) & 0x8000))
		Sleep(5);

	// This disables all hooks
	rtk::CrStopAPI();

	FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(lpInstance), 0);
	return 0;
}

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,		// handle to DLL module
	DWORD fdwReason,		// reason for calling function
	LPVOID lpReserved)		// reserved
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		HANDLE hThread = CreateThread(nullptr, 0, Main, hinstDLL, CREATE_SUSPENDED, nullptr);

		if (!hThread)
		{
			MessageBoxA(
				0, 
				"Thread creation failed!", 
				"RTK - Initialization Error"
				, MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND
			);

			return FALSE;
		}

		ResumeThread(hThread);
		CloseHandle(hThread);
	}

	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}