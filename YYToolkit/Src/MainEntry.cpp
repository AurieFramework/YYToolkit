#include "Toolkit/Core.hpp"
#include "Toolkit/Hooks/Present/Present.hpp"
#include "Toolkit/Hooks/WindowProc/WindowProc.hpp"
#include "Toolkit/Utils/ImGui/imgui_impl_dx11.h"
#include "Toolkit/Utils/ImGui/imgui_impl_win32.h"
#include "Toolkit/Utils/MH/MinHook.h"
#include "Toolkit/Features/API/API.hpp"
#include <thread>
#include <Windows.h>

#if _WIN64
#error Don't compile in x64!
#endif

static HINSTANCE g_hDLL = 0;

void Main()
{
	std::thread IPCThread;

	// Phase 1 - Pre-toolkit Initialization
	{
		API_Initialize();
	}

	// Phase 2 - Toolkit Initialization
	{
		Tool_Initialize();
	}

	while (!GetAsyncKeyState(VK_END)) { Sleep(1); }

	// Phase 3 - Toolkit Exit
	{
		MH_DisableHook(MH_ALL_HOOKS);
		Sleep(100);
		MH_Uninitialize();
		
		{
			RValue Result;
			AUMI_CallBuiltinFunction("window_handle", &Result, 0, 0, 0, 0);
			SetWindowLong((HWND)(Result.Pointer), GWL_WNDPROC, (LONG)Hooks::WindowProc::pfnOriginal);
		}
		
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		Hooks::Present::pView->Release();
		Hooks::Present::pContext->Release();
		Hooks::Present::pDevice->Release();

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