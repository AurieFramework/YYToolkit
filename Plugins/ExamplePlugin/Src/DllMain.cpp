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
		PrintMessage(CLR_DEFAULT, "[Example Plugin] - 30 frames have passed! Current framecount: %lu", FrameNumber);

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
	PrintMessage(CLR_DEFAULT, "[Example Plugin] - Hello from PluginEntry!");

	PluginAttributes_t* PluginAttributes = nullptr;

	// Get the attributes for the plugin - this is an opaque structure, as it may change without any warning.
	// If Status == YYTK_OK (0), then PluginAttributes is guaranteed to be valid (non-null).
	if (YYTKStatus Status = PmGetPluginAttributes(PluginObject, PluginAttributes))
	{
		PrintError(__FILE__, __LINE__, "[Example Plugin] - PmGetPluginAttributes failed with 0x%x", Status);
		return YYTK_FAIL;
	}

	// Register a callback for frame events
	YYTKStatus Status = PmCreateCallback(
		PluginAttributes,					// Plugin Attributes
		g_pCallbackAttributes,				// (out) Callback Attributes
		FrameCallback,						// The function to register as a callback
		static_cast<EventType>(EVT_PRESENT | EVT_ENDSCENE), // Which events trigger this callback
		nullptr								// The optional argument to pass to the function
	);

	if (Status)
	{
		PrintError(__FILE__, __LINE__, "[Example Plugin] - PmCreateCallback failed with 0x%x", Status);
		return YYTK_FAIL;
	}

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
		PrintError(__FILE__, __LINE__, "[Example Plugin] PmRemoveCallback failed with 0x%x", Removal);
	}

	PrintMessage(CLR_DEFAULT, "[Example Plugin] - Goodbye!");

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