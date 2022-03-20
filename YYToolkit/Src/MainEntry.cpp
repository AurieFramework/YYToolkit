#include "Core/Features/API/Internal.hpp"
#include "Core/Features/Console/Console.hpp"
#include "Core/Hooks/Hooks.hpp"
#include "Core/Utils/WinAPI/WinAPI.hpp"
#include "Core/Utils/Logging/Logging.hpp"
#include <chrono>

#if _WIN64
#error Don't compile in x64!
#endif

void __stdcall Main(HINSTANCE g_hDLL)
{
	API::gAPIVars.Globals.g_hMainModule = g_hDLL;

	API::Internal::__InitializeConsole__();

	if (Utils::WinAPI::IsPreloaded())
	{
		API::Internal::__InitializePreload__();
		API::gAPIVars.Globals.g_bWasPreloaded = true;
	}

	Utils::WinAPI::ResumeGameProcess();

	auto TimeStart = std::chrono::high_resolution_clock::now();

	API::Internal::__Initialize__(g_hDLL);
	Hooks::Initialize();

	auto TimeEnd = std::chrono::high_resolution_clock::now();

	Utils::Logging::Message(CLR_LIGHTBLUE, "Initialization done - took %d milliseconds!", 
		std::chrono::duration_cast<std::chrono::milliseconds>(TimeEnd - TimeStart).count());

	while (!GetAsyncKeyState(VK_END)) 
	{
		if (GetAsyncKeyState(VK_DOWN) && GetAsyncKeyState(VK_NEXT))
			Utils::Logging::Critical(__FILE__, __LINE__, "The user manually initiated a crash.");

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
		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Main, hinstDLL, 0, 0));
	}
	return TRUE;
}