#include "DllMain.hpp"	// Include our header
#include <Windows.h>    // Include Windows's mess.
#include <vector>       // Include the STL vector.

// We save the CodeCallbackHandler attributes here, so we can unregister the callback in the unload routine.
static CallbackAttributes_t* g_pCallbackAttributes = nullptr;
static uint32_t FrameNumber = 0;

// This callback is registered on EVT_PRESENT and EVT_ENDSCENE, so it gets called every frame on DX9 / DX11 games.
YYTKStatus FrameCallback(YYTKEventBase* pEvent, void* OptionalArgument)
{
	FrameNumber++;

	if (FrameNumber % 30 == 0)
		PrintMessage("[Example Plugin] - 30 frames have passed! Current framecount: %lu", FrameNumber);

	// Tell the core the handler was successful.
	return YYTK_OK;
}

// Create an entry routine - it must be named exactly this, and must accept these exact arguments.
// It must also be declared DllExport (notice how the other functions are not).
DllExport YYTKStatus PluginEntry(YYTKPlugin* PluginObject)
{
	// Set the unload routine
	PluginObject->PluginUnload = PluginUnload;

	// Print a message to the console
	PrintMessage("[Example Plugin] - Hello from PluginEntry!");

	// Register a callback for object events, save the attributes to the global variable (evil :P)
	g_pCallbackAttributes = PmCreateCallback(PmGetPluginAttributes(PluginObject), FrameCallback, static_cast<EventType>(EVT_PRESENT | EVT_ENDSCENE), nullptr);

	if (!g_pCallbackAttributes)
		PrintError(__FILE__, __LINE__, "[Example Plugin] - Failed to create a callback!");

	// Off it goes to the core.
	return YYTK_OK;
}

// The routine that gets called on plugin unload.
// Registered in PluginEntry - you should use this to release resources.
YYTKStatus PluginUnload()
{
	YYTKStatus Removal = PmRemoveCallback(g_pCallbackAttributes);

	// If we didn't succeed in removing the callback.
	if (Removal != YYTK_OK)
	{
		PrintError(__FILE__, __LINE__, "PmRemoveCallbacks failed with 0x%x", Removal);
	}

	PrintMessage("[Example Plugin] - Goodbye!");

	return YYTK_OK;
}

// Boilerplate setup for a Windows DLL, can just return TRUE.
// This has to be here or else you get linker errors (unless you disable the main method)
BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return 1;
}