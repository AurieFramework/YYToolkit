#include "../API.hpp"
#include "../../../Dependencies/MinHook/MinHook.h"

namespace rtk
{
	void CrInitializeAPI()
	{
		// Initialize MinHook
		if (MH_STATUS Status = MH_Initialize())
		{
			MessageBoxA(0, MH_StatusToString(Status), "MinHook Initialization Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
			exit(0);
		}

		// Load plugins, don't call any plugin routines yet
		if (!RTK_Success(PmInitialize()))
		{
			MessageBoxA(0, "An unexpected error occurred during plugin loading.", "RTK Launch Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
			exit(0);
		}
	}

	void CrStopAPI()
	{
		// Uninitialize MinHook, this also removes all hooks
		if (MH_STATUS Status = MH_Uninitialize())
		{
			MessageBoxA(0, MH_StatusToString(Status), "MinHook Initialization Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
		}

		// Unload all plugins
		if (!RTK_Success(PmUninitialize()))
		{
			MessageBoxA(0, "An unexpected error occurred during plugin unloading.", "RTK Launch Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
			exit(0);
		}
	}
}